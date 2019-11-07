#pragma once

#include "ComputeShader.h"
#include "glhelper.h"
#include "GLCore/Buffer.h"
#include "Frame.h"

#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Geometry> 
#include <unsupported/Eigen/MatrixFunctions>
#include "eigen_utils.h"

namespace rgbd
{
	// Ref: https://github.com/GPUOpen-Effects/FidelityFX
	class RGBDtam : public ComputeShader
	{
	private:
		std::map<std::string, const gl::Shader::Ptr> progs;
		gl::ShaderStorageBuffer<BufferReductionDTAM> ssboDTAMReduction;
		gl::ShaderStorageBuffer<float> ssboDTAMReductionOutput;


		//gl::ShaderStorageBuffer<BufferReductionRGB> ssboRGBJtJJtrSE3;
		gl::ShaderStorageBuffer<float> ssboRGBRGBJtJJtrSE3Output;

		Eigen::Matrix<double, 3, 3, Eigen::RowMajor> rodrigues(
			const Eigen::Vector3d & src
		);



	public:
		RGBDtam();
		~RGBDtam();

		void init(
			int width,
			int height,
			const std::map<std::string, const gl::Shader::Ptr> &progs
			);

		glm::mat4 calcDevicePose(
			const rgbd::Frame &currentFrame,
			const glm::vec4 cam,
			bool &tracked
			);

		typedef std::shared_ptr<rgbd::RGBDtam> Ptr;

	};
}