#pragma once

#include "glhelper.h"

#include <opencv2/opencv.hpp>

#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Geometry> 
#include <unsupported/Eigen/MatrixFunctions>
#include "eigen_utils.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <GLCore/Shader.h>
#include "ConstantParameters.h"
#include "VirtualFrameRenderer.h"
#include "ProjectiveDataAssoc.h"

// Ref: "Linear Least - squares Optimization for Point-to-plane ICP Surface Registration"
namespace rgbd
{
	class splatterICP
	{
	private:

		std::map<std::string, const gl::Shader::Ptr> progs;

		//VirtualFrameRenderer::Ptr virtualFrameRenderer;
		//ProjectiveDataAssoc::Ptr dataAssoc;

		std::vector<float> outputReductionData;

		gl::ShaderStorageBuffer<BufferReduction> ssboReduction;
		gl::ShaderStorageBuffer<float> ssboReductionOutput;


		glm::mat4 Kmat;

		std::vector<float> makeJTJ(
			std::vector<float> v
		);

	public:
		splatterICP();
		~splatterICP();

		void loadShaders(std::map<std::string, const gl::Shader::Ptr> &progs,
			const std::string &folderPath
		);

		void init(
			int width,
			int height,
			const glm::mat4 &K,
			const std::map<std::string, const gl::Shader::Ptr> &progs
		);
		
		void paramToMat(
			const cv::Mat &params,	// parameters (6 x 1 matrix)
			glm::mat4 &T			// 4x4 matrix in glm::mat4
		);
		
		void track(
			const rgbd::Frame &currentFrame,
			const rgbd::Frame &virtualFrame,
			glm::mat4 &T,
			int level
		);

		void reduce(
			const glm::ivec2 &imSize
		);

		void getReduction(
			std::vector<float> &b,
			std::vector<float> &C,
			float &AE,
			uint32_t &icpCount
		);

		void getReduction(
			float *matrixA_host,
			float *vectorB_host,
			float &AE,
			uint32_t &icpCount
		);

		void calc(
			const int level,
			const rgbd::Frame &prevFrame,
			const rgbd::Frame &currFrame,
			glm::mat4 &T,
			float &alignmentEnergy,
			float &lastICPCount,
			const float finThresh = 1.0e-4F
		);

		typedef std::shared_ptr<rgbd::splatterICP> Ptr;
	};
}
