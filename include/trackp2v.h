#pragma once

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
	struct BufferReductionP2V
	{
		GLint result;
		GLfloat h;
		GLfloat D;
		GLfloat J[6];

		BufferReductionP2V()
		{
			result = 0;
			h = 0.0f;
			D = 0.0f;
			for (int i = 0; i < 6; i++)
			{
				J[i] = 0.0f;
			}			
		}
	};

	class p2vICP
	{
	private:
		std::map<std::string, const gl::Shader::Ptr> progs;

		gl::ShaderStorageBuffer<BufferReductionP2V> ssboReduction;
		gl::ShaderStorageBuffer<float> ssboReductionOutput;
		
		std::vector<float> outputReductionData;


		void track(
			GLuint gVolID,
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

	public:
		p2vICP(
			int width,
			int height,
			float distThresh,
			float normThresh,
			const glm::mat4 &K,
			const glm::vec3 &volDim,
			const glm::vec3 &volSize,
			const float dMin,
			const float dMax,
			const std::map<std::string, const gl::Shader::Ptr> &progs
		);
		~p2vICP();



		bool calc(
			const int level,
			GLuint gVolID,
			const rgbd::Frame &currentFrame,
			const rgbd::Frame &virtualFrame,
			Eigen::Matrix4f &T,
			float &AE,
			uint32_t &icpCount,
			Eigen::Matrix<double, 6, 1> &result,
			Eigen::Matrix<double, 6, 1> &resultPrev,
			const float finThresh = 1.0e-4F
		);

		typedef std::shared_ptr<rgbd::p2vICP> Ptr;
	};
}
