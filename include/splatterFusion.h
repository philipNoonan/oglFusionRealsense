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


#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Geometry> 
#include <unsupported/Eigen/MatrixFunctions>
#include "eigen_utils.h"

namespace rgbd
{
	class splatterFusion
	{
	private:
		rgbd::PyramidricalICP::Ptr icp;
		rgbd::GlobalMap::Ptr gMap;
		//std::vector<glm::mat4> vT;

		glm::mat4 vT;

		glm::mat4 T;

	public:
		splatterFusion();
		~splatterFusion();

		void loadShaders(
			std::map<std::string, const gl::Shader::Ptr> &progs,
			const std::string &folderPath = "../../shaders/"
		);

		void init(
			rgbd::Frame &currentFrame,
			rgbd::Frame &virtualFrame,
			const glm::mat4 &K,
			const std::map<std::string, const gl::Shader::Ptr> &progs
		);

		void performColorTracking(
			rgbd::Frame &currentFrame,
			rgbd::Frame &virtualFrame,
			const gl::Texture::Ptr &gradientMap,
			glm::mat4 &pose,
			glm::vec4 cam
		);

		glm::mat4 calcDevicePose(
			rgbd::Frame &currentFrame,
			rgbd::Frame &virtualFrame,
			bool &tracked
		);

		void updateGlobalMap(
			rgbd::Frame &currentFrame,
			rgbd::Frame &virtualFrame,
			bool integrate
		);

		void renderGlobalMap(
			glm::mat4 renderPose, 
			rgbd::Frame &virtualFrame
		);


		void exportGlobalVertexMap();

		GLuint getGlobalModelBuffer(GLuint &mSize)
		{
			return gMap->getGlobalBuffer(mSize);
		}

		glm::mat4 getPose()
		{
			return vT;
		}

		void clear();

		void splatterFusion::SetPrePose(
			glm::mat4 prePose
		);
	};
}