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
#include "PyramidricalICP.h"
#include "GlobalVolume.h"

namespace rgbd
{
	class p2pFusion
	{
	private:
		rgbd::GlobalVolume::Ptr gVol;

		rgbd::PyramidricalICP::Ptr icp;
		std::vector<glm::mat4> vT;
		glm::mat4 T;
		glm::vec3 volSize;

	public:
		p2pFusion();
		~p2pFusion();

		void loadShaders(
			std::map<std::string, const gl::Shader::Ptr> &progs,
			const std::string &folderPath = "../../shaders/"
		);

		void init(
			rgbd::GlobalVolume::Ptr volume,
			const rgbd::Frame &currentFrame,
			const rgbd::Frame &virtualFrame,
			glm::vec3 &dim,
			glm::vec3 &size,
			const glm::mat4 &initPose,
			const glm::mat4 &K,
			const float maxWeight,
			const float distThresh, 
			const float normThresh,
			const float largeStep,
			const float step,
			const float nearPlane,
			const float farPlane,
			const std::map<std::string, const gl::Shader::Ptr> &progs
		);

		glm::mat4 calcDevicePose(
			const rgbd::Frame &currentFrame,
			const rgbd::Frame &virtualFrame
		);

		void integrate(
			const rgbd::Frame &currentFrame
		);

		void raycast(
			const rgbd::Frame &virtualFrame
		);

		glm::mat4 getPose()
		{
			return vT.back();
		}

		//void exportGlobalVertexMap();
		GLuint getVolumeID();

		void clear(
			const glm::mat4 & resetPose,
			glm::vec3 volumeSize
		);

	};
}