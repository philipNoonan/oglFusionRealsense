#pragma once

#include "glhelper.h"
#include <opencv2/opencv.hpp>
#include <GLCore/Shader.h>
#include "ConstantParameters.h"
#include "Frame.h"
#include "GLCore/Buffer.h"



#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Geometry> 
#include <unsupported/Eigen/MatrixFunctions>
#include "eigen_utils.h"


// Ref: kfusion 
namespace rgbd
{


	class p2pICP
	{
	private:
		std::map<std::string, const gl::Shader::Ptr> progs;

		gl::ShaderStorageBuffer<BufferReduction> ssboReduction;
		gl::ShaderStorageBuffer<float> ssboReductionOutput;
		
		std::vector<float> outputReductionData;





		std::vector<float> makeJTJ(
			std::vector<float> v
		);


	public:
		p2pICP();
		~p2pICP();

		void loadShaders(
			std::map<std::string, const gl::Shader::Ptr> &progs,
			const std::string &folderPath
		);

		void init(
			int width,
			int height,
			float distThresh,
			float normThresh,
			const glm::mat4 &K,
			const std::map<std::string, const gl::Shader::Ptr> &progs
		);



		void calc(
			const int level,
			const rgbd::Frame &currentFrame,
			const rgbd::Frame &virtualFrame,
			glm::mat4 &T,
			float &AE,
			uint32_t &icpCount,
			const float finThresh = 1.0e-4F
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


		typedef std::shared_ptr<rgbd::p2pICP> Ptr;
	};
}
