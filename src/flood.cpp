#include "flood.h"


#include "opencv2/core/utility.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

gFlood::gFlood(
	const float size, 
	const float dim,
	const std::map<std::string, const gl::Shader::Ptr> &progs
) : volSize(size), volDim(dim),
	progs{ {"jumpFlood", progs.at("jumpFlood") },
		   {"jumpFloodInitVert", progs.at("jumpFloodInitVert") },
		   {"jumpFloodInitGlobal", progs.at("jumpFloodInitGlobal") } }
{

	std::vector<uint32_t> blankFloodData((size / 2) * (size / 2) * (size / 2), 4294967292);
	encodedTex = std::make_shared<gl::Texture3D>();
	encodedTex->create(blankFloodData.data(), size / 2, size / 2, size / 2, 1, gl::Texture3DType::UINT32, false);
	encodedTex->setFiltering(gl::Texture3DFilter::LINEAR);
	encodedTex->setWarp(gl::Texture3DWarp::CLAMP_TO_EDGE);

	for (int i = 0; i < 2; i++)
	{
		jfaTex[i] = std::make_shared<gl::Texture3D>();
		jfaTex[i]->createStorage(3, size, size, size, 1, GL_R32UI, gl::Texture3DType::UINT32, false);
		jfaTex[i]->setFiltering(gl::Texture3DFilter::LINEAR);
		jfaTex[i]->setWarp(gl::Texture3DWarp::CLAMP_TO_EDGE);
	}

	glGenQueries(1, timeQuery);

}

void gFlood::loadShaders(
	std::map<std::string, const gl::Shader::Ptr> &progs,
	const std::string &folderPath
)
{
	progs.insert(std::make_pair("jumpFlood", std::make_shared<gl::Shader>(folderPath + "jumpFlood.comp")));
	progs.insert(std::make_pair("jumpFloodInitVert", std::make_shared<gl::Shader>(folderPath + "jumpFloodInitVert.comp")));
	progs.insert(std::make_pair("jumpFloodInitGlobal", std::make_shared<gl::Shader>(folderPath + "jumpFloodInitGlobal.comp")));
}

void gFlood::setFloodInitialFromDepth(const rgbd::Frame &srcFrame, const glm::mat4 & T)
{

	wipeFlood();

	int compWidth = divup(srcFrame.getVertexMap()->getWidth(), 4); // image2D dimensions, set this as variable
	int compHeight = divup(srcFrame.getVertexMap()->getHeight(), 4);

	progs["jumpFloodInitVert"]->use();
	progs["jumpFloodInitVert"]->setUniform("T", T);
	progs["jumpFloodInitVert"]->setUniform("scaleFactor", (volSize / 2.0f ) / volDim);

	jfaTex[1]->bindImage(0, 1, GL_WRITE_ONLY);
	srcFrame.getVertexMap()->bindImage(1, GL_READ_ONLY);



	glDispatchCompute(compWidth, compHeight, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	progs["jumpFloodInitVert"]->disuse();


	// 1 + JFA

	progs["jumpFlood"]->use();
	progs["jumpFlood"]->setUniform("jump", 1.0f);

	compWidth = divup(volSize / 2, 4);
	compHeight = divup(volSize / 2, 4);
	int compDepth = divup(volSize / 2, 4);

	jfaTex[1]->bindImage(0, 1, GL_READ_ONLY);
	jfaTex[0]->bindImage(1, 1, GL_WRITE_ONLY);


	glBindImageTexture(0, m_textureImage1, 1, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
	glBindImageTexture(1, m_textureImage0, 1, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);


	glDispatchCompute(compWidth, compHeight, compDepth);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);



}

void gFlood::setFloodInitialFromGlobalMap(const glm::mat4 & T)
{
	wipeFlood();

	progs["jumpFloodInitGlobal"]->use();
	progs["jumpFloodInitGlobal"]->setUniform("T", T);
	progs["jumpFloodInitGlobal"]->setUniform("scaleFactor", (volSize / 2.0f) / volDim);

	jfaTex[1]->bindImage(0, 1, GL_WRITE_ONLY);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_globalBuffer);

	glDispatchCompute(divup(m_globalMapBufferSize, 1024), 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	progs["jumpFloodInitVert"]->disuse();


	// 1 + JFA

	progs["jumpFlood"]->use();
	progs["jumpFlood"]->setUniform("jump", 1);
	progs["jumpFlood"]->setUniform("update", 1);
	progs["jumpFlood"]->setUniform("upscale", 0);

	jfaTex[1]->bindImage(0, 1, GL_READ_ONLY);
	jfaTex[0]->bindImage(1, 1, GL_WRITE_ONLY);

	glDispatchCompute(divup(volSize / 2, 4), divup(volSize / 2, 4), divup(volSize / 2, 4));

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	progs["jumpFlood"]->disuse();


}





void gFlood::wipeFlood()
{
	glCopyImageSubData(encodedTex->getID(), GL_TEXTURE_3D, 0, 0, 0, 0, jfaTex[1]->getID(), GL_TEXTURE_3D, 1, 0, 0, 0, encodedTex->getWidth(), encodedTex->getHeight(), encodedTex->getDepth());
}    



     
void gFlood::jumpFloodCalc(const glm::mat4 &T)
{

	glm::mat4 initPose = glm::translate(glm::mat4(1.0f), glm::vec3(volDim / 2.0f, volDim / 2.0f, 0.0f));


	glBeginQuery(GL_TIME_ELAPSED, timeQuery[0]);

	setFloodInitialFromGlobalMap(T * initPose);





	int compWidth = divup(volSize / 2, 4);
	int compHeight = divup(volSize / 2, 4);
	int compDepth = divup(volSize / 2, 4);




	int numLevels = std::log2(volSize / 2);

	for (int level = 0; level < numLevels; level++)
	{
		int stepWidth = std::max(1, (int(volSize / 2) >> (level + 1)));


		progs["jumpFlood"]->use();
		progs["jumpFlood"]->setUniform("jump", stepWidth);
		progs["jumpFlood"]->setUniform("update", 1);
		progs["jumpFlood"]->setUniform("upscale", 0);

		if (level % 2 == 0)
		{
			//glBindImageTexture(0, m_texture_jfa_0, 1, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
			//glBindImageTexture(1, m_texture_jfa_1, 1, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

			jfaTex[0]->bindImage(0, 1, GL_READ_ONLY);
			jfaTex[1]->bindImage(1, 1, GL_WRITE_ONLY);

			//glBindImageTexture(0, m_textureImage0, 1, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
			//glBindImageTexture(1, m_textureImage1, 1, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);

		}
		else
		{
			//glBindImageTexture(0, m_texture_jfa_1, 1, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
			//glBindImageTexture(1, m_texture_jfa_0, 1, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

			jfaTex[0]->bindImage(1, 1, GL_WRITE_ONLY);
			jfaTex[1]->bindImage(0, 1, GL_READ_ONLY);

			//glBindImageTexture(0, m_textureImage1, 1, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
			//glBindImageTexture(1, m_textureImage0, 1, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);

		}



		glDispatchCompute(compWidth, compHeight, compDepth);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		progs["jumpFlood"]->disuse();

	}


	//// 1 + JFA half reso variant https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=4276119&tag=1

	compWidth = divup(volSize, 4);
	compHeight = divup(volSize, 4);
	compDepth = divup(volSize, 4);

	//// UPSCALE

	progs["jumpFlood"]->use();
	progs["jumpFlood"]->setUniform("jump", 1);
	progs["jumpFlood"]->setUniform("update", 0);
	progs["jumpFlood"]->setUniform("upscale", 1);

	jfaTex[0]->bindImage(0, 1, GL_READ_ONLY);
	jfaTex[0]->bindImage(1, 0, GL_WRITE_ONLY);

	glDispatchCompute(compWidth, compHeight, compDepth);
	//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	progs["jumpFlood"]->disuse();



	//// FINAL FLOOD STEP LENGTH 1.0

	progs["jumpFlood"]->use();
	progs["jumpFlood"]->setUniform("jump", 1);
	progs["jumpFlood"]->setUniform("update", 1);
	progs["jumpFlood"]->setUniform("upscale", 0);

	jfaTex[0]->bindImage(0, 0, GL_READ_ONLY);
	jfaTex[1]->bindImage(1, 0, GL_WRITE_ONLY);

	glDispatchCompute(compWidth, compHeight, compDepth);

	progs["jumpFlood"]->disuse();


	glEndQuery(GL_TIME_ELAPSED);
	GLuint available = 0;
	while (!available) {
		glGetQueryObjectuiv(timeQuery[0], GL_QUERY_RESULT_AVAILABLE, &available);
	}
	// elapsed time in nanoseconds
	GLuint64 elapsed;
	glGetQueryObjectui64vEXT(timeQuery[0], GL_QUERY_RESULT, &elapsed);
	//std::cout << elapsed / 1000000.0 << std::endl;
	m_timeElapsed = elapsed / 1000000.0;
	

}
