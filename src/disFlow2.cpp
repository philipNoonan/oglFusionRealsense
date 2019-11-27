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

		sparseFlowMap = std::make_shared<gl::Texture>();
		sparseFlowMap->createStorage(numLevels, width / 4, height / 4, 4, GL_RGBA32F, gl::TextureType::FLOAT32, 0);
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
			densificationFBO[i].attach(densificationFlowMap[1], 1, i);
			densificationFBO[i].unbind();

		}


		float vertices[] = {
						0.0f, 1.0f,
						0.0f, 0.0f,
						1.0f, 1.0f,
						1.0f, 0.0f,
		};
		glCreateBuffers(1, &vertexVBO);
		glNamedBufferData(vertexVBO, sizeof(vertices), vertices, GL_STATIC_DRAW);
		
		glCreateVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
		
		GLint position_attrib = 0;  
		glEnableVertexArrayAttrib(VAO, position_attrib);
		glVertexAttribPointer(position_attrib, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

		glBindVertexArray(0);


	}

	void DisFlow::execute(
		gl::Texture::Ptr lastColorMap,
		gl::Texture::Ptr nextColorMap,
		gl::Texture::Ptr nextGradientMap
	)
	{

		for (int level = 2; level >= 0; level--)
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

			this->progs["inverseSearch"]->use();

			this->progs["inverseSearch"]->setUniform("level", level);

			lastColorMap->use(0);
			nextColorMap->use(1);

			lastFlowMap->use(2);

			nextGradientMap->bindImage(0, level, GL_READ_ONLY);

			nextFlowMap->bindImage(1, level + 1, GL_READ_ONLY);
			sparseFlowMap->bindImage(2, level, GL_READ_WRITE);

			glDispatchCompute((lastColorMap->getWidth() >> level) / 32, (lastColorMap->getHeight() >> level) / 32, 1);
			this->progs["inverseSearch"]->disuse();



			// densify
			// input sparse flow map
			// input last color map
			// input next color map
			// output dense flow map

			glBindVertexArray(VAO);



			densificationFBO[level].bind();



			std::vector<GLenum> drawBuffs = densificationFBO[level].getDrawBuffers();

			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glViewport(0, 0, densificationFBO[level].getWidth(), densificationFBO[level].getHeight());

			this->progs["densification"]->use();
			this->progs["densification"]->setUniform("level", level);
			this->progs["densification"]->setUniform("patch_size", 
				glm::vec2(
					8.0f / float(densificationFBO[level].getWidth()), 
					8.0f / float(densificationFBO[level].getHeight())
				)
			);

			lastColorMap->use(0);
			nextColorMap->use(1);

			sparseFlowMap->bindImage(0, level, GL_READ_ONLY);
			nextFlowMap->bindImage(1, level, GL_WRITE_ONLY);

			glDrawBuffers((GLsizei)drawBuffs.size(), drawBuffs.data());

			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);


			int num_layers = 2;
			int width_patches = (densificationFBO[level].getWidth()) / 8.0f;
			int height_patches = (densificationFBO[level].getHeight()) / 8.0f;



			glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, width_patches * height_patches * num_layers);

			densificationFBO[level].unbind();

			this->progs["densification"]->disuse();

			glBindVertexArray(0);


		}


	
	
		
	}	
	
	gl::Texture::Ptr DisFlow::getFlowMap() const
	{
		return nextFlowMap;
	}
}