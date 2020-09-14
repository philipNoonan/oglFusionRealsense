#include "GlobalMap.h"

namespace rgbd
{
	const unsigned int GlobalMapConstParam::MAX_MAP_SIZE = 5000000;
	const float GlobalMapConstParam::CSTABLE = 10.0f;

	GlobalMap::GlobalMap(
		int width,
		int height,
		const glm::mat4 &K,
		const std::map<std::string, const gl::Shader::Ptr> &progs
	) : width(width), height(height),
		progs{ { "IndexMapGeneration", progs.at("IndexMapGeneration") },
		{ "GlobalMapUpdate", progs.at("GlobalMapUpdate") },
		{ "UnnecessaryPointRemoval", progs.at("UnnecessaryPointRemoval") },
		{ "SurfaceSplatting", progs.at("SurfaceSplatting") } }
	{
		indexMap = std::make_shared<gl::Texture>();
		indexMap->create(0, width * 4, height * 4, 1, gl::TextureType::FLOAT32);
		indexMap->setFiltering(GL_NEAREST, GL_NEAREST);
		indexMap->setWarp(gl::TextureWarp::CLAMP_TO_EDGE);

		virtualFrameFBO.create(width, height);
		indexMapFBO.create(indexMap->getWidth(), indexMap->getHeight());
		indexMapFBO.attach(indexMap, 0);

		indexMapFBO.bind();
		glClearColor(-1.0f, -1.0f, -1.0f, -1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		indexMapFBO.unbind();

		buffSwitch = 0;
		mapSize = 0;

		GLuint valueZero(0);
		for (int i = 0; i < atomic.size(); ++i)
		{
			atomic[i].bind();
			atomic[i].create(&valueZero, 1, GL_DYNAMIC_DRAW);
			atomic[i].bindBase(i);
			atomic[i].unbind();
		}
		std::vector<GlobalMapData> tmpMapData(GlobalMapConstParam::MAX_MAP_SIZE);
		for (int i = 0; i < ssbo.size(); ++i)
		{
			ssbo[i].bind();
			ssbo[i].create(tmpMapData.data(), GlobalMapConstParam::MAX_MAP_SIZE, GL_DYNAMIC_DRAW);
			ssbo[i].bindBase(i);
			ssbo[i].unbind();
		}

		glm::mat4 P = rgbd::calibratedPerspective(
			ICPConstParam::MIN_DEPTH, ICPConstParam::MAX_DEPTH,
			width, height, K[2][0], K[2][1], K[0][0], K[1][1], 0.0f
		);

		//glm::vec4 testVec4 = glm::vec4(K[2][0] * 1.0, K[2][1] * 1.0, K[0][0] * 1.0, K[1][1] * 1.0);

		this->progs["IndexMapGeneration"]->setUniform("P", P);
		this->progs["IndexMapGeneration"]->setUniform("maxDepth", 10.0f);
		this->progs["IndexMapGeneration"]->setUniform("imSize", glm::vec2(width, height));
		this->progs["IndexMapGeneration"]->setUniform("cam", glm::vec4(K[2][0] * 1.0, K[2][1] * 1.0, K[0][0] * 1.0, K[1][1] * 1.0));


		this->progs["GlobalMapUpdate"]->setUniform("timestamp", 0);
		this->progs["GlobalMapUpdate"]->setUniform("sigma", 0.6f);
		this->progs["GlobalMapUpdate"]->setUniform("c_stable", GlobalMapConstParam::CSTABLE);
		this->progs["GlobalMapUpdate"]->setUniform("K", K);
		this->progs["GlobalMapUpdate"]->setUniform("maxMapSize", GlobalMapConstParam::MAX_MAP_SIZE);

		this->progs["SurfaceSplatting"]->setUniform("P", P);
		this->progs["SurfaceSplatting"]->setUniform("c_stable", GlobalMapConstParam::CSTABLE);
		this->progs["SurfaceSplatting"]->setUniform("scale", 1.0f);
		this->progs["SurfaceSplatting"]->setUniform("maxDepth", 10.0f);
		this->progs["SurfaceSplatting"]->setUniform("imSize", glm::vec2(width, height));
		this->progs["SurfaceSplatting"]->setUniform("cam", glm::vec4(K[2][0], K[2][1], K[0][0], K[1][1]));

		this->progs["UnnecessaryPointRemoval"]->setUniform("c_stable", GlobalMapConstParam::CSTABLE);
	}

	GlobalMap::~GlobalMap()
	{
	}

	GLuint GlobalMap::getMapSize()
	{
		return mapSize;
	}

	void GlobalMap::genIndexMap(const glm::mat4 &invT)
	{

		//GLuint query;
		//glGenQueries(1, &query);
		//glBeginQuery(GL_TIME_ELAPSED, query);

		glEnable(GL_DEPTH_TEST);

		indexMapFBO.bind();
		glClearColor(-1.0f, -1.0f, -1.0f, -1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, indexMapFBO.getWidth(), indexMapFBO.getHeight());

		progs["IndexMapGeneration"]->use();
		progs["IndexMapGeneration"]->setUniform("invT", invT);

		ssbo[buffSwitch].bindBase(0);

		glDrawArrays(GL_POINTS, 0, getMapSize());

		progs["IndexMapGeneration"]->disuse();
		indexMapFBO.unbind();



		//glEndQuery(GL_TIME_ELAPSED);
		//GLuint available = 0;
		//while (!available) {
		//	glGetQueryObjectuiv(query, GL_QUERY_RESULT_AVAILABLE, &available);
		//}
		//// elapsed time in nanoseconds
		//GLuint64 elapsed;
		//glGetQueryObjectui64vEXT(query, GL_QUERY_RESULT, &elapsed);
		//std::cout << "gen id time : " << elapsed / 1000000.0 << std::endl;

		//GLuint query;
		//glGenQueries(1, &query);
		//glBeginQuery(GL_TIME_ELAPSED, query);


		//glEnable(GL_DEPTH_TEST);

		//indexMapFBO.bind();
		//glClearColor(-1.0f, -1.0f, -1.0f, -1.0f);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glViewport(0, 0, indexMapFBO.getWidth(), indexMapFBO.getHeight());

		//indexMapFBO.unbind();


		//progs["IndexMapGeneration"]->use();
		//progs["IndexMapGeneration"]->setUniform("invT", invT);

		//ssbo[buffSwitch].bindBase(0);
		//indexMap->bindImage(0, 0, GL_READ_WRITE);

		//glDrawArrays(GL_POINTS, 0, getMapSize());
		//glDispatchCompute(GLHelper::divup(getMapSize(), 512) ,1 , 1);

		//progs["IndexMapGeneration"]->disuse();

		//glEndQuery(GL_TIME_ELAPSED);
		//GLuint available = 0;
		//while (!available) {
		//	glGetQueryObjectuiv(query, GL_QUERY_RESULT_AVAILABLE, &available);
		//}
		//// elapsed time in nanoseconds
		//GLuint64 elapsed;
		//glGetQueryObjectui64vEXT(query, GL_QUERY_RESULT, &elapsed);
		//std::cout << "gen id time : " << elapsed / 1000000.0 << std::endl;



	}

	void GlobalMap::updateGlobalMap(
		rgbd::Frame &srcFrame,
		bool firstFrame,
		const glm::mat4 &T
	)
	{

		//GLuint query;
		//glGenQueries(1, &query);
		//glBeginQuery(GL_TIME_ELAPSED, query);

		int timestamp = srcFrame.getDepthFrameCount();

		std::cout << "ts " << mapSize << std::endl;

		progs["GlobalMapUpdate"]->use();
		progs["GlobalMapUpdate"]->setUniform("T", T);
		progs["GlobalMapUpdate"]->setUniform("invT", glm::inverse(T));
		progs["GlobalMapUpdate"]->setUniform("timestamp", timestamp);
		progs["GlobalMapUpdate"]->setUniform("firstFrame", firstFrame);

	
		indexMapFBO.getColorAttachment(0)->bindImage(0, 0, GL_READ_ONLY);
		srcFrame.getVertexMap()->bindImage(1, 0, GL_READ_ONLY);
		srcFrame.getNormalMap()->bindImage(2, 0, GL_READ_ONLY);
		srcFrame.getColorAlignedToDepthMap()->bindImage(3, 0, GL_READ_ONLY);	// <-- debugging: color integration
		srcFrame.getTrackMap()->bindImage(4, 0, GL_READ_ONLY);	// <-- debugging: color integration
		srcFrame.getTestMap()->bindImage(5, 0, GL_WRITE_ONLY);

		atomic[buffSwitch].update(&mapSize, 0, 1);
		atomic[buffSwitch].bindBase(0);
		ssbo[buffSwitch].bindBase(0);

		glDispatchCompute(GLHelper::divup(width, 32), GLHelper::divup(height, 32), 1);


		atomic[buffSwitch].read(&mapSize, 0, 1);
		progs["GlobalMapUpdate"]->disuse();

		//glEndQuery(GL_TIME_ELAPSED);
		//GLuint available = 0;
		//while (!available) {
		//	glGetQueryObjectuiv(query, GL_QUERY_RESULT_AVAILABLE, &available);
		//}
		//// elapsed time in nanoseconds
		//GLuint64 elapsed;
		//glGetQueryObjectui64vEXT(query, GL_QUERY_RESULT, &elapsed);
		////std::cout << "gmu time : " << elapsed / 1000000.0 << std::endl;

	}

	void GlobalMap::removeUnnecessaryPoints(int timestamp)
	{
		//if (timestamp % 10)
		//{
		//	GLuint query;
		//	glGenQueries(1, &query);
		//	glBeginQuery(GL_TIME_ELAPSED, query);


			std::array<int, 2> _buffSwitch = { buffSwitch, (buffSwitch + 1) % 2 };

			progs["UnnecessaryPointRemoval"]->use();
			progs["UnnecessaryPointRemoval"]->setUniform("timestamp", timestamp);
			progs["UnnecessaryPointRemoval"]->setUniform("c_stable", GlobalMapConstParam::CSTABLE);

			GLuint valueZero(0);
			atomic[_buffSwitch.back()].bindBase(1);
			atomic[_buffSwitch.back()].update(&valueZero, 0, 1);
			ssbo[_buffSwitch.front()].bindBase(0);
			ssbo[_buffSwitch.back()].bindBase(1);

			glDispatchCompute(GLHelper::divup(mapSize, 400), 1, 1);

			atomic[_buffSwitch.back()].read(&mapSize, 0, 1);
			progs["UnnecessaryPointRemoval"]->disuse();

			buffSwitch = _buffSwitch.back();

		//	glEndQuery(GL_TIME_ELAPSED);
		//	GLuint available = 0;
		//	while (!available) {
		//		glGetQueryObjectuiv(query, GL_QUERY_RESULT_AVAILABLE, &available);
		//	}
		//	// elapsed time in nanoseconds
		//	GLuint64 elapsed;
		//	glGetQueryObjectui64vEXT(query, GL_QUERY_RESULT, &elapsed);
		//	//std::cout << timestamp << " upr time : " << elapsed / 1000000.0 << std::endl;
		//}


	}

	void GlobalMap::genVirtualFrame(
		rgbd::Frame &dstFrame,
		const glm::mat4 &invT
	)
	{

		//GLuint query;
		//glGenQueries(1, &query);
		//glBeginQuery(GL_TIME_ELAPSED, query);




		virtualFrameFBO.attach(dstFrame.getVertexMap(), 0);
		virtualFrameFBO.attach(dstFrame.getNormalMap(), 1);
		virtualFrameFBO.attach(dstFrame.getDepthMap(), 2);
		virtualFrameFBO.attach(dstFrame.getColorMap(), 3);

		std::vector<GLenum> drawBuffs = virtualFrameFBO.getDrawBuffers();

		virtualFrameFBO.bind();
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, virtualFrameFBO.getWidth(), virtualFrameFBO.getHeight());

		progs["SurfaceSplatting"]->use();
		progs["SurfaceSplatting"]->setUniform("invT", invT);

		glEnable(GL_PROGRAM_POINT_SIZE);
		glEnable(GL_POINT_SPRITE);

		ssbo[buffSwitch].bindBase(0);

		glDrawBuffers((GLsizei)drawBuffs.size(), drawBuffs.data());
		glDrawArrays(GL_POINTS, 0, getMapSize());

		glBindTexture(GL_TEXTURE_2D, 0);
		progs["SurfaceSplatting"]->disuse();
		virtualFrameFBO.unbind();

		glDisable(GL_PROGRAM_POINT_SIZE);
		glDisable(GL_POINT_SPRITE);

		//glEndQuery(GL_TIME_ELAPSED);
		//GLuint available = 0;
		//while (!available) {
		//	glGetQueryObjectuiv(query, GL_QUERY_RESULT_AVAILABLE, &available);
		//}
		//// elapsed time in nanoseconds
		//GLuint64 elapsed;
		//glGetQueryObjectui64vEXT(query, GL_QUERY_RESULT, &elapsed);
		//std::cout << "ss time : " << elapsed / 1000000.0 << std::endl;

	}

	void GlobalMap::clearAll()
	{
		GLuint valueZero(0);
		for (int i = 0; i < atomic.size(); ++i)
		{
			atomic[i].bind();
			atomic[i].create(&valueZero, 1, GL_DYNAMIC_DRAW);
			atomic[i].bindBase(i);
			atomic[i].unbind();
		}
		std::vector<GlobalMapData> tmpMapData(GlobalMapConstParam::MAX_MAP_SIZE);
		for (int i = 0; i < ssbo.size(); ++i)
		{
			ssbo[i].bind();
			ssbo[i].create(tmpMapData.data(), GlobalMapConstParam::MAX_MAP_SIZE, GL_DYNAMIC_DRAW);
			ssbo[i].bindBase(i);
			ssbo[i].unbind();
		}
	}

	void GlobalMap::exportPointCloud(std::vector<glm::vec4> &outputVertexData, std::vector<glm::vec3> &outputNormalData, std::vector<glm::vec3> &outputColorData)
	{
		std::vector<GlobalMapData> outputMapData(mapSize);
		outputVertexData.resize(mapSize);
		outputNormalData.resize(mapSize);
		outputColorData.resize(mapSize);

		ssbo[0].read(outputMapData.data(), 0, mapSize);
		int val = 0;
		for (auto i : outputMapData)
		{
			outputVertexData[val] = glm::vec4(i.vertex.x, i.vertex.y, i.vertex.z, i.data.x);
			outputNormalData[val] = glm::vec3(i.normal.x, i.normal.y, i.normal.z);
			outputColorData[val] = glm::vec3(i.color.x, i.color.y, i.color.z);

			val++;
		}



	}

	void GlobalMap::savePointCloud(std::string filename)
	{
		std::vector<glm::vec4> outputVerts;
		std::vector<glm::vec3> outputNorms;
		std::vector<glm::vec3> outputColor;

		exportPointCloud(outputVerts, outputNorms, outputColor);

		std::ofstream outFile(filename, std::ios::out | std::ios::binary);

		if (!outFile)
		{
			//cerr << "Error opening output file: " << FileName << "!" << endl;
			printf("Error opening output file: %s!\n", filename);
			exit(1);
		}

		int pointNum = outputVerts.size();

		outFile << "ply" << std::endl;

		// https://stackoverflow.com/questions/8571089/how-can-i-find-endian-ness-of-my-pc-programmatically-using-c
		int num = 1;
		if (*(char *)&num == 1)
		{
			outFile << "format binary_little_endian 1.0" << std::endl;
		}
		else
		{
			outFile << "format binary_big_endian 1.0" << std::endl;
		}

		// outFile << "format ascii 1.0" << std::endl;

		outFile << "element vertex " << pointNum << std::endl;
		outFile << "property float x" << std::endl;
		outFile << "property float y" << std::endl;
		outFile << "property float z" << std::endl;
		outFile << "property uchar red" << std::endl;
		outFile << "property uchar green" << std::endl;
		outFile << "property uchar blue" << std::endl;
		outFile << "property float nx" << std::endl;
		outFile << "property float ny" << std::endl;
		outFile << "property float nz" << std::endl;
		outFile << "property float radius" << std::endl;

		outFile << "end_header" << std::endl;

		for (int i = 0; i < outputVerts.size(); i++)
		{
			outFile.write((char*)&outputVerts[i].x, sizeof(float));
			outFile.write((char*)&outputVerts[i].y, sizeof(float));
			outFile.write((char*)&outputVerts[i].z, sizeof(float));

			unsigned char r = int(outputColor[i].x * 255.0f);
			unsigned char g = int(outputColor[i].y * 255.0f);
			unsigned char b = int(outputColor[i].z * 255.0f);

			outFile.write((char*)&r, sizeof(unsigned char));
			outFile.write((char*)&g, sizeof(unsigned char));
			outFile.write((char*)&b, sizeof(unsigned char));

			outFile.write((char*)&outputNorms[i].x, sizeof(float));
			outFile.write((char*)&outputNorms[i].y, sizeof(float));
			outFile.write((char*)&outputNorms[i].z, sizeof(float));



			outFile.write((char*)&outputVerts[i].w, sizeof(float));

		}

		outFile.close();
	}

	GLuint GlobalMap::getGlobalBuffer(GLuint &mapsize)
	{
		mapsize = mapSize;
		return ssbo[0];
	}


}