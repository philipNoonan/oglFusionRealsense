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
		//progs.insert(std::make_pair("variationalRefine", std::make_shared<gl::Shader>(folderPath + "variationalRefine.comp")));

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

		maxLevels = (int)(log((2 * width) / (4.0 * 8.0)) / log(2.0) + 0.5) - 1; // 8 is patch size
		maxLevels = std::max(0, (int)std::floor(log2((2.0f*(float)width) / ((float)5 * (float)8)))) - 1;
		maxLevels = 5;

		sparseFlowMap = std::make_shared<gl::Texture>();
		sparseFlowMap->createStorage(maxLevels, width / 2, height / 2, 4, GL_RGBA32F, gl::TextureType::FLOAT32, 0);
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

		meanI_Map = std::make_shared<gl::Texture>();
		meanI_Map->createStorage(numLevels, width, height, 1, GL_R32F, gl::TextureType::FLOAT32, 0);
		meanI_Map->setFiltering(GL_LINEAR, GL_LINEAR);
		meanI_Map->setWarp(gl::TextureWarp::CLAMP_TO_EDGE);

		I_t_Map = std::make_shared<gl::Texture>();
		I_t_Map->createStorage(numLevels, width, height, 1, GL_R32F, gl::TextureType::FLOAT32, 0);
		I_t_Map->setFiltering(GL_LINEAR, GL_LINEAR);
		I_t_Map->setWarp(gl::TextureWarp::CLAMP_TO_EDGE);

		pixelBaseFlow = std::make_shared<gl::Texture>();
		pixelBaseFlow->createStorage(numLevels, width, height, 2, GL_RG32F, gl::TextureType::FLOAT32, 0);
		pixelBaseFlow->setFiltering(GL_LINEAR, GL_LINEAR);
		pixelBaseFlow->setWarp(gl::TextureWarp::CLAMP_TO_EDGE);

		I_x_y_Map = std::make_shared<gl::Texture>();
		I_x_y_Map->createStorage(numLevels, width, height, 2, GL_RG32F, gl::TextureType::FLOAT32, 0);
		I_x_y_Map->setFiltering(GL_LINEAR, GL_LINEAR);
		I_x_y_Map->setWarp(gl::TextureWarp::CLAMP_TO_EDGE);

		pixelFlowMap = std::make_shared<gl::Texture>();
		pixelFlowMap->createStorage(numLevels, width, height, 2, GL_RG32F, gl::TextureType::FLOAT32, 0);
		pixelFlowMap->setFiltering(GL_LINEAR, GL_LINEAR);
		pixelFlowMap->setWarp(gl::TextureWarp::CLAMP_TO_EDGE);

		pixelDiffFlowMap.resize(2);
		for (int i = 0; i < 2; i++)
		{
			pixelDiffFlowMap[i] = std::make_shared<gl::Texture>();
			pixelDiffFlowMap[i]->createStorage(numLevels, width, height, 2, GL_RG32F, gl::TextureType::FLOAT32, 0);
			pixelDiffFlowMap[i]->setFiltering(GL_LINEAR, GL_LINEAR);
			pixelDiffFlowMap[i]->setWarp(gl::TextureWarp::CLAMP_TO_EDGE);
		}
		diffusivityMap = std::make_shared<gl::Texture>();
		diffusivityMap->createStorage(numLevels, width, height, 1, GL_R32F, gl::TextureType::FLOAT32, 0);
		diffusivityMap->setFiltering(GL_LINEAR, GL_LINEAR);
		diffusivityMap->setWarp(gl::TextureWarp::CLAMP_TO_EDGE);

		std::vector<float> tempSORData(6 * width * height);

		ssboSOR.bind();
		ssboSOR.create(tempSORData.data(), 6 * width * height, GL_DYNAMIC_DRAW);
		ssboSOR.bindBase(0);
		ssboSOR.unbind();

		densificationFlowMap.resize(2);

		densificationFBO.resize(numLevels);

		for (int i = 0; i < 2; i++)
		{
			densificationFlowMap[i] = std::make_shared<gl::Texture>();
			densificationFlowMap[i]->createStorage(numLevels, width, height, 4, GL_RGBA32F, gl::TextureType::FLOAT32, 0);
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

		//variational_refinement_iter = 5;
		//variational_refinement_alpha = 20.f;
		//variational_refinement_gamma = 10.f;
		//variational_refinement_delta = 5.f;

		///* Use separate variational refinement instances for different scales to avoid repeated memory allocation: */
		//int max_possible_scales = 10;
		//for (int i = 0; i < max_possible_scales; i++)
		//	variational_refinement_processors.push_back(cv::VariationalRefinement::create());


	}

	//void variRef()
	//{
	//	cv::Mat I0imq = cv::Mat(nextFlowMap->getHeight() >> level, nextFlowMap->getWidth() >> level, CV_8UC4);
	//	cv::Mat I1imq = cv::Mat(nextFlowMap->getHeight() >> level, nextFlowMap->getWidth() >> level, CV_8UC4);



	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_2D, currentFrame.getColorPreviousMap()->getID());
	//	glGetTexImage(GL_TEXTURE_2D, level, GL_RGBA, GL_UNSIGNED_BYTE, I0imq.data);
	//	glBindTexture(GL_TEXTURE_2D, 0);
	//	//glActiveTexture(0); 

	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_2D, currentFrame.getColorFilteredMap()->getID());
	//	glGetTexImage(GL_TEXTURE_2D, level, GL_RGBA, GL_UNSIGNED_BYTE, I1imq.data);
	//	glBindTexture(GL_TEXTURE_2D, 0);
	//	//glActiveTexture(0);  

	//	cv::Mat I0C1;
	//	cv::cvtColor(I0imq, I0C1, cv::COLOR_BGRA2GRAY);

	//	cv::Mat I1C1;
	//	cv::cvtColor(I1imq, I1C1, cv::COLOR_BGRA2GRAY);

	//	cv::Mat sxx3 = cv::Mat(nextFlowMap->getHeight() >> level, nextFlowMap->getWidth() >> level, CV_32FC2);
	//	cv::Mat sxx4 = cv::Mat(nextFlowMap->getHeight() >> level, nextFlowMap->getWidth() >> level, CV_32FC2);

	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_2D, densificationFlowMap[0]->getID());
	//	// 	glBindTexture(GL_TEXTURE_2D, m_textureU_x_y);

	//	glGetTexImage(GL_TEXTURE_2D, level, GL_RG, GL_FLOAT, sxx3.data);
	//	glBindTexture(GL_TEXTURE_2D, 0);
	//	//glActiveTexture(0);  

	//	//cv::imshow("dens1", sxx3);  

	//	cv::Mat image2[2];
	//	cv::split(sxx3, image2);


	//	variational_refinement_processors[0]->calcUV(I0C1, I1C1,
	//		image2[0], image2[1]);

	//	cv::merge(image2, 2, sxx3);

	//	////glBindTexture(GL_TEXTURE_2D, m_textureU_x_y);
	//	////glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_texture_width >> level, m_texture_height >> level, GL_RG, GL_FLOAT, sxx3.ptr());
	//	//  

	//	glBindTexture(GL_TEXTURE_2D, densificationFlowMap[0]->getID());
	//	// 	glBindTexture(GL_TEXTURE_2D, m_textureU_x_y);



	//	//if (imageArray != NULL)
	//	//{

	//	cv::merge(image2, 2, sxx4);

	//	glTexSubImage2D(GL_TEXTURE_2D, level, 0, 0, nextFlowMap->getWidth() >> level, nextFlowMap->getHeight() >> level, GL_RG, GL_FLOAT, sxx4.data);

	//	if (level == 0)
	//	{
	//		cv::Mat mag, ang;
	//		cv::Mat hsv_split[3], hsv;
	//		cv::Mat rgb;
	//		cv::cartToPolar(image2[0], image2[1], mag, ang, true);
	//		cv::normalize(mag, mag, 0, 1, cv::NORM_MINMAX);
	//		hsv_split[0] = ang;
	//		hsv_split[1] = mag;
	//		hsv_split[2] = cv::Mat::ones(ang.size(), ang.type());
	//		cv::merge(hsv_split, 3, hsv);
	//		cv::cvtColor(hsv, rgb, cv::COLOR_HSV2BGR);
	//		cv::imshow("flowvar", rgb);
	//	}

	//}

	void DisFlow::execute(
		const rgbd::Frame &currentFrame,
		gl::Texture::Ptr lastGradientMap
	)
	{

		for (int level = maxLevels - 1; level >= 0; level--)
		{
			// search
			// input gradient map
			// input gradprod map
			// input gradsum map
			// input last flow map
			// input output sparse flow map

			glm::vec2 invDenseSize = glm::vec2(1.0f / (nextFlowMap->getWidth() >> level), 1.0f / (nextFlowMap->getHeight() >> level));
			glm::vec2 invPreviousDenseSize = glm::vec2(1.0f / (nextFlowMap->getWidth() >> (level + 1)), 1.0f / (nextFlowMap->getHeight() >> (level + 1)));


			this->progs["inverseSearch"]->use();

			this->progs["inverseSearch"]->setUniform("level", level);
			this->progs["inverseSearch"]->setUniform("invImageSize", invDenseSize);
			this->progs["inverseSearch"]->setUniform("invPreviousImageSize", invPreviousDenseSize);

			currentFrame.getColorPreviousMap()->use(0);
			currentFrame.getColorFilteredMap()->use(1);

			lastGradientMap->bindImage(0, level, GL_READ_ONLY);

			densificationFlowMap[0]->bindImage(1, level + 1, GL_READ_ONLY);
			sparseFlowMap->bindImage(2, level, GL_READ_WRITE);

			currentFrame.getTestMap()->bindImage(3, level, GL_READ_WRITE);
			densificationFlowMap[0]->bindImage(5, level, GL_WRITE_ONLY);// for wiping
			densificationFlowMap[1]->bindImage(6, level, GL_READ_ONLY);

			int sparseWidth = (currentFrame.getWidth() >> level) / 2;
			int sparseHeight = (currentFrame.getHeight() >> level) / 2;

			int compWidth = GLHelper::divup(sparseWidth, 32);
			int compHeight = GLHelper::divup(sparseHeight, 32);


			glDispatchCompute(compWidth, compHeight, 1);

			this->progs["inverseSearch"]->disuse();

			// densify
			// input sparse flow map
			// input last color map
			// input next color map
			// output dense flow map
			// this is run at the output resolution of the full image at this level

			densificationFBO[level].bind();

			std::vector<GLenum> drawBuffs = densificationFBO[level].getDrawBuffers();
			glDisable(GL_DEPTH_TEST); //need to disable since patches are rendered at exactly z = 0;

			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE); // overlapping patches are equally blended

			glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

			glViewport(0, 0, (nextFlowMap->getWidth() >> (level)), (nextFlowMap->getHeight() >> (level)));
			//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			//glClear(GL_COLOR_BUFFER_BIT);

			glm::ivec2 sparseSize = glm::ivec2(sparseWidth, sparseHeight);

			this->progs["densification"]->use();
			this->progs["densification"]->setUniform("level", level);
			this->progs["densification"]->setUniform("invDenseTexSize", invDenseSize);
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


			glCopyImageSubData(densificationFlowMap[0]->getID(), GL_TEXTURE_2D, level, 0, 0, 0,
				densificationFlowMap[1]->getID(), GL_TEXTURE_2D, level, 0, 0, 0,
				nextFlowMap->getWidth() >> level, nextFlowMap->getHeight() >> level, 1);

			glBindTexture(GL_TEXTURE_2D, densificationFlowMap[1]->getID());
			glGenerateMipmap(GL_TEXTURE_2D);





			//this->progs["variationalRefine"]->use();


			//this->progs["variationalRefine"]->setUniform("level", level);
			//// warp flow images
			//this->progs["variationalRefine"]->setUniform("functionID", 0);
			//densificationFlowMap[0]->use(0);
			//currentFrame.getColorPreviousMap()->use(1);
			//currentFrame.getColorFilteredMap()->use(2);
			//meanI_Map->bindImage(0, level, GL_WRITE_ONLY);
			//I_t_Map->bindImage(1, level, GL_WRITE_ONLY);
			//pixelBaseFlow->bindImage(2, level, GL_WRITE_ONLY);
			//compWidth = GLHelper::divup(nextFlowMap->getWidth() >> (level), 32);
			//compHeight = GLHelper::divup(nextFlowMap->getHeight() >> (level), 32);
			//glDispatchCompute(compWidth, compHeight, 1);





			//// calc derivatives
			//this->progs["variationalRefine"]->setUniform("functionID", 1);
			//meanI_Map->bindImage(0, level, GL_READ_ONLY);
			//I_x_y_Map->bindImage(5, level, GL_WRITE_ONLY);
			//compWidth = GLHelper::divup(nextFlowMap->getWidth() >> (level), 32);
			//compHeight = GLHelper::divup(nextFlowMap->getHeight() >> (level), 32);
			//glDispatchCompute(compWidth, compHeight, 1);


			//for (int outer_idx = 0; outer_idx < level + 1; ++outer_idx)
			//{
			//	int flipflop = 0;

			//	// calc diffusivity
			//	this->progs["variationalRefine"]->setUniform("functionID", 2);
			//	this->progs["variationalRefine"]->setUniform("zeroDiffFlow", outer_idx == 0 ? 1 : 0);
			//	this->progs["variationalRefine"]->setUniform("alpha", 1.0f);
			//	pixelBaseFlow->bindImage(2, level, GL_READ_ONLY);
			//	pixelDiffFlowMap[flipflop]->bindImage(3, level, GL_READ_ONLY);
			//	diffusivityMap->bindImage(6, level, GL_WRITE_ONLY);
			//	compWidth = GLHelper::divup(nextFlowMap->getWidth() >> (level), 32);
			//	compHeight = GLHelper::divup(nextFlowMap->getHeight() >> (level), 32);
			//	glDispatchCompute(compWidth, compHeight, 1);

			//	// comp equations
			//	this->progs["variationalRefine"]->setUniform("functionID", 3);
			//	this->progs["variationalRefine"]->setUniform("zeroDiffFlow", outer_idx == 0 ? 1 : 0);
			//	this->progs["variationalRefine"]->setUniform("delta", 0.25f);
			//	this->progs["variationalRefine"]->setUniform("gamma", 0.25f);

			//	I_t_Map->bindImage(1, level, GL_READ_ONLY);
			//	pixelDiffFlowMap[flipflop]->bindImage(3, level, GL_READ_ONLY);
			//	I_x_y_Map->bindImage(5, level, GL_READ_ONLY);
			//	pixelBaseFlow->bindImage(2, level, GL_READ_ONLY);
			//	diffusivityMap->bindImage(6, level, GL_READ_ONLY);

			//	ssboSOR.bindBase(0);
			//	compWidth = GLHelper::divup(nextFlowMap->getWidth() >> (level), 32);
			//	compHeight = GLHelper::divup(nextFlowMap->getHeight() >> (level), 32);
			//	glDispatchCompute(compWidth, compHeight, 1);

			//	// perform SOR
			//	this->progs["variationalRefine"]->setUniform("functionID", 4);
			//	this->progs["variationalRefine"]->setUniform("numNonZeroPhases", outer_idx == 0 ? 1 : 0);
			//	
			//	for (int inner_idx = 0; inner_idx < 4; inner_idx++)
			//	{
			//		this->progs["variationalRefine"]->setUniform("functionID", 4);
			//		this->progs["variationalRefine"]->setUniform("numNonZeroPhases", outer_idx == 0 ? 1 : 0);
			//		this->progs["variationalRefine"]->setUniform("level", level);
			//		this->progs["variationalRefine"]->setUniform("iter", inner_idx);
			//		diffusivityMap->bindImage(6, level, GL_READ_ONLY);
			//		pixelDiffFlowMap[flipflop]->bindImage(3, level, GL_READ_ONLY);
			//		pixelDiffFlowMap[1 - flipflop]->bindImage(4, level, GL_WRITE_ONLY);
			//		ssboSOR.bindBase(0);
			//		compWidth = GLHelper::divup(nextFlowMap->getWidth() >> (level), 32);
			//		compHeight = GLHelper::divup(nextFlowMap->getHeight() >> (level), 32);
			//		glDispatchCompute(compWidth, compHeight, 1);
			//		flipflop = 1 - flipflop;
			//	}
			//}

			//// add base flow
			//this->progs["variationalRefine"]->setUniform("functionID", 5);

			//pixelBaseFlow->bindImage(2, level, GL_READ_ONLY);
			//pixelDiffFlowMap[0]->bindImage(3, level, GL_READ_ONLY);
			//densificationFlowMap[0]->bindImage(7, level, GL_WRITE_ONLY);
			//compWidth = GLHelper::divup(nextFlowMap->getWidth() >> (level), 32);
			//compHeight = GLHelper::divup(nextFlowMap->getHeight() >> (level), 32);
			//glDispatchCompute(compWidth, compHeight, 1);

			this->progs["variationalRefine"]->disuse();

	


		}



		//cv::Mat image00[2]; 

		//cv::Mat col = cv::Mat(nextFlowMap->getHeight(), nextFlowMap->getWidth(), CV_32FC2);
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, densificationFlowMap[0]->getID());
		//glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, col.data);
		//glBindTexture(GL_TEXTURE_2D, 0);
		//glActiveTexture(0);

		//cv::split(col, image00);

		//cv::imshow("colo1", image00[0]);
		//cv::imshow("colo2", image00[1]);


		//cv::Mat col = cv::Mat(nextFlowMap->getHeight(), nextFlowMap->getWidth(), CV_32FC1);
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, diffusivityMap->getID());
		//glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, col.data);
		//glBindTexture(GL_TEXTURE_2D, 0);
		//glActiveTexture(0);

		//cv::imshow("colo", col);


		//glBindTexture(GL_TEXTURE_2D, densificationFlowMap[0]->getID());
		//glGenerateMipmap(GL_TEXTURE_2D);
	
	
		
	}	
	
	gl::Texture::Ptr DisFlow::getFlowMap() const
	{
		return densificationFlowMap[0];
	}
}