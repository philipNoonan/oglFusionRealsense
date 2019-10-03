#pragma once
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>

#include "GLCore/Shader.h"

#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <string>

#include "Frame.h"
//#include "SLAM/Utilities/IniFileReader.h"
//#include "SLAM/Utilities/TUMRGBDUtilities.h"
#include "PyramidricalICP.h"
#include "GlobalMap.h"

namespace rgbd
{
	class splatterFusion
	{
	private:
		rgbd::PyramidricalICP::Ptr icp;
		rgbd::GlobalMap::Ptr gMap;
		std::vector<glm::mat4> vT;
		glm::mat4 T;

	public:
		splatterFusion();
		~splatterFusion();

		void loadShaders(
			std::map<std::string, const gl::Shader::Ptr> &progs,
			const std::string &folderPath = "../../shaders/"
		);

		void init(
			const rgbd::Frame &currentFrame,
			const rgbd::Frame &virtualFrame,
			const glm::mat4 &K,
			const std::map<std::string, const gl::Shader::Ptr> &progs
		);

		glm::mat4 calcDevicePose(
			const rgbd::Frame &currentFrame,
			const rgbd::Frame &virtualFrame
		);

		void updateGlobalMap(
			const rgbd::Frame &currentFrame,
			const rgbd::Frame &virtualFrame
		);

		void exportGlobalVertexMap();

		GLuint getGlobalModelBuffer(GLuint &mSize)
		{
			return gMap->getGlobalBuffer(mSize);
		}

		glm::mat4 getPose()
		{
			return vT.back();
		}

		void clear();
	};
}