#include "disFlow2.h"

namespace rgbd
{
	DisFlow::DisFlow() {};
	DisFlow::~DisFlow() {};

	void DisFlow::loadShaders(
		std::map<std::string, const gl::Shader::Ptr> &progs,
		const std::string &folderPath
	)
	{
		//progs.insert(std::make_pair("makePatches", std::make_shared<gl::Shader>(folderPath + "disMakePatches.comp")));
		progs.insert(std::make_pair("inverseSearch", std::make_shared<gl::Shader>(folderPath + "disSearch.comp")));
		progs.insert(std::make_pair("densification", std::make_shared<gl::Shader>(folderPath + "disDensification.vert", folderPath + "disDensification.frag")));
		//progs.insert(std::make_pair("medianFilter", std::make_shared<gl::Shader>(folderPath + "medianFilter.comp")));
	}
	void DisFlow::init(
		int numLevels,
		int width,
		int height,
		const std::map<std::string, const gl::Shader::Ptr> &programs
	)
	{
		progs = programs;
		maxLevels = numLevels;

		sparseFlowMap = std::make_shared<gl::Texture>();
		sparseFlowMap->createStorage(numLevels, width, height, 4, GL_RGBA32F, gl::TextureType::FLOAT32, 0);
		sparseFlowMap->setFiltering(GL_LINEAR, GL_LINEAR);
		sparseFlowMap->setWarp(gl::TextureWarp::CLAMP_TO_EDGE);

		lastFlowMap = std::make_shared<gl::Texture>();
		lastFlowMap->createStorage(numLevels, width, height, 2, GL_RG32F, gl::TextureType::FLOAT32, 0);
		lastFlowMap->setFiltering(GL_LINEAR, GL_LINEAR);
		lastFlowMap->setWarp(gl::TextureWarp::CLAMP_TO_EDGE);

		nextFlowMap = std::make_shared<gl::Texture>();
		nextFlowMap->createStorage(numLevels, width, height, 2, GL_RG32F, gl::TextureType::FLOAT32, 0);
		nextFlowMap->setFiltering(GL_LINEAR, GL_LINEAR);
		nextFlowMap->setWarp(gl::TextureWarp::CLAMP_TO_EDGE);

		densificationFlowMap.resize(2);

		densificationFBO.resize(numLevels);

		for (int i = 0; i < 2; i++)
		{
			densificationFlowMap[i] = std::make_shared<gl::Texture>();
			densificationFlowMap[i]->createStorage(numLevels, width, height, 2, GL_RG32F, gl::TextureType::FLOAT32, 0);
			densificationFlowMap[i]->setFiltering(GL_LINEAR, GL_LINEAR);
			densificationFlowMap[i]->setWarp(gl::TextureWarp::CLAMP_TO_EDGE);
		}

		for (int i = 0; i < numLevels; i++)
		{
			densificationFBO[i].create(densificationFlowMap[0]->getWidth() >> i, densificationFlowMap[0]->getHeight() >> i);
			densificationFBO[i].attach(densificationFlowMap[0], 0, i);
			//densificationFBO[i].attach(densificationFlowMap[1], 1, i);
			densificationFBO[i].unbind();

		}


	}

	void DisFlow::execute(
		const rgbd::Frame &currentFrame,
		gl::Texture::Ptr nextGradientMap
	)
	{

		for (int level = maxLevels - 1; level >= 0; level--)
		{
			// make patches
			// input gradient map
			// output product map xx yy xy
			// output sum x y

			//this->progs["makePatches"]->use();

			//nextGradientMap->bindImage(0, level, GL_READ_ONLY);
			//gradProdMap->bindImage(1, level, GL_WRITE_ONLY);
			//gradSumMap->bindImage(2, level, GL_WRITE_ONLY);

			//glDispatchCompute((lastColorMap->getWidth() >> level) / 32, (lastColorMap->getHeight() >> level) / 32, 1);
			//this->progs["makePatches"]->disuse();


			// search
			// input gradient map
			// input gradprod map
			// input gradsum map
			// input last flow map
			// input output sparse flow map

			glm::vec2 invDenseSize = glm::vec2(1.0f / (nextFlowMap->getWidth() >> level), 1.0f / (nextFlowMap->getHeight() >> level));


			this->progs["inverseSearch"]->use();

			this->progs["inverseSearch"]->setUniform("level", level);
			this->progs["inverseSearch"]->setUniform("invImageSize", invDenseSize);

			currentFrame.getColorPreviousMap()->use(0);
			currentFrame.getColorFilteredMap()->use(1);
			densificationFlowMap[0]->use(2);

			nextGradientMap->bindImage(0, level, GL_READ_ONLY);

			densificationFlowMap[0]->bindImage(1, level == (maxLevels - 1) ? level : level + 1, GL_READ_ONLY);
			sparseFlowMap->bindImage(2, level, GL_READ_WRITE);

			currentFrame.getTestMap()->bindImage(3, level, GL_READ_WRITE);
			densificationFlowMap[0]->bindImage(5, level, GL_WRITE_ONLY);// for wiping

			int sparseWidth = (currentFrame.getWidth() >> level) / 4;
			int sparseHeight = (currentFrame.getHeight() >> level) / 4;

			int compWidth = GLHelper::divup(sparseWidth, 32);
			int compHeight = GLHelper::divup(sparseHeight, 32);


			glDispatchCompute(compWidth, compHeight, 1);

			this->progs["inverseSearch"]->disuse();



			// densify
			// input sparse flow map
			// input last color map
			// input next color map
			// output dense flow map




			densificationFBO[level].bind();



			std::vector<GLenum> drawBuffs = densificationFBO[level].getDrawBuffers();
			glDisable(GL_DEPTH_TEST);

			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);

			glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);


			glViewport(0, 0, (nextFlowMap->getWidth() >> level), (nextFlowMap->getHeight() >> level));
			//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			//glClear(GL_COLOR_BUFFER_BIT);

			glm::ivec2 sparseSize = glm::ivec2(sparseWidth, sparseHeight);
			glm::vec2 levelUpSize = glm::vec2(1.0f / (nextFlowMap->getWidth() >> level), 1.0f / (nextFlowMap->getHeight() >> level));

			this->progs["densification"]->use();
			this->progs["densification"]->setUniform("level", level);
			this->progs["densification"]->setUniform("invDenseTexSize", levelUpSize);
			this->progs["densification"]->setUniform("sparseTexSize", sparseSize);

			currentFrame.getColorPreviousMap()->use(0);
			currentFrame.getColorFilteredMap()->use(1);

			sparseFlowMap->bindImage(0, level, GL_READ_ONLY);
			currentFrame.getTestMap()->bindImage(1, level, GL_READ_WRITE);

			glDrawBuffers((GLsizei)drawBuffs.size(), drawBuffs.data());





			int numberOfPatches = sparseSize.x * sparseSize.y;

			glDrawArrays(GL_POINTS, 0, numberOfPatches);

			densificationFBO[level].unbind();

			this->progs["densification"]->disuse();

			glEnable(GL_DEPTH_TEST);

			glDisable(GL_BLEND);
			glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);


		}


	
	
		
	}	
	
	gl::Texture::Ptr DisFlow::getFlowMap() const
	{
		return densificationFlowMap[0];
	}
}