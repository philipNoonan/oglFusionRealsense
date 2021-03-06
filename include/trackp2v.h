#pragma once

#include "glhelper.h"

#include <opencv2/opencv.hpp>
#include <GLCore/Shader.h>
#include "ConstantParameters.h"
#include "Frame.h"
#include "GLCore/Buffer.h"
#include "GlobalVolume.h"


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


	class p2vICP
	{
	private:
		std::map<std::string, const gl::Shader::Ptr> progs;


		
		std::vector<float> outputReductionData;

		void track(
			GLuint gVolID,
			const rgbd::Frame &currentFrame,
			const rgbd::Frame &virtualFrame,
			gl::ShaderStorageBuffer<rgbd::BufferReductionP2V> &ssboRed,
			glm::vec3 volDim,
			glm::vec3 volSize,
			glm::mat4 &T,
			int level
		);

		void reduce(
			gl::ShaderStorageBuffer<rgbd::BufferReductionP2V> &ssboRed,
			gl::ShaderStorageBuffer<float> &ssboRedOut,
			const glm::ivec2 &imSize
		);

		void getReduction(
			gl::ShaderStorageBuffer<float> &ssboRedOut,
			std::vector<float> &b,
			std::vector<float> &C,
			float &AE,
			uint32_t &icpCount
		);


		std::vector<float> makeJTJ(
			std::vector<float> v
		);

		Eigen::Matrix4f Twist(const Eigen::Matrix<double, 6, 1> &xi)
		{
			Eigen::Matrix4f M;

			M << 0.0, -xi(2), xi(1), xi(3),
				xi(2), 0.0, -xi(0), xi(4),
				-xi(1), xi(0), 0.0, xi(5),
				0.0, 0.0, 0.0, 0.0;

			return M;
		};

		inline int divup(int a, int b) { return (a % b != 0) ? (a / b + 1) : (a / b); }


	public:
		p2vICP(
			int width,
			int height,
			float distThresh,
			float normThresh,
			const glm::mat4 &K,
			const std::map<std::string, const gl::Shader::Ptr> &progs
		);
		~p2vICP();



		bool calc(
			const int level,
			GLuint gVolID,
			const rgbd::Frame &currentFrame,
			const rgbd::Frame &virtualFrame,
			gl::ShaderStorageBuffer<rgbd::BufferReductionP2V> &ssboRed,
			gl::ShaderStorageBuffer<float> &ssboRedOut,
			Eigen::Matrix4f &T,
			float &AE,
			uint32_t &icpCount,
			glm::vec3 volDim,
			glm::vec3 volSize,
			Eigen::Matrix<double, 6, 1> &result,
			Eigen::Matrix<double, 6, 1> &resultPrev,
			const float finThresh = 1.0e-4F
		);

		typedef std::shared_ptr<rgbd::p2vICP> Ptr;
	};
}
