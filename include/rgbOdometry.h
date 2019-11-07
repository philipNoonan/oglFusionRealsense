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
	class RGBOdometry : public ComputeShader
	{
	private:
		std::map<std::string, const gl::Shader::Ptr> progs;
		gl::ShaderStorageBuffer<BufferReductionRGB> ssboRGBReduction;
		gl::ShaderStorageBuffer<float> ssboRGBReductionOutput;
		gl::ShaderStorageBuffer<float> ssboRGBRGBJtJJtrSE3;

		//gl::ShaderStorageBuffer<BufferReductionRGB> ssboRGBJtJJtrSE3;
		gl::ShaderStorageBuffer<float> ssboRGBRGBJtJJtrSE3Output;

		Eigen::Matrix<double, 3, 3, Eigen::RowMajor> rodrigues(
			const Eigen::Vector3d & src
		);

		void computeUpdateSE3(
			Eigen::Matrix<double, 4, 4, Eigen::RowMajor> & resultRt,
			const Eigen::Matrix<double, 6, 1> & result,
			Eigen::Isometry3f & rgbOdom
		);


	public:
		RGBOdometry(
			int width,
			int height,
			const std::map<std::string, const gl::Shader::Ptr> &progs
			);
		void computeDerivativeImages(
			gl::Texture::Ptr srcNextImage,
			gl::Texture::Ptr dstNextIdxy
		);
		void computeResiduals(
			const rgbd::Frame &currentFrame,
			const gl::Texture::Ptr & gradientMap,
			const glm::vec3 kT,
			const glm::mat3 krkinv,
			float &sigmaVal,
			float &rgbError
		);


		/* 
		
		d rgbStep(const DeviceArray2D<DataTerm> & corresImg,
             const float & sigma,
             const DeviceArray2D<float3> & cloud,
             const float & fx,
             const float & fy,
             const DeviceArray2D<short> & dIdx,
             const DeviceArray2D<short> & dIdy,
             const float & sobelScale,
             DeviceArray<JtJJtrSE3> & sum,
             DeviceArray<JtJJtrSE3> & out,
             float * matrixA_host,
             float * vectorB_host,
			 
			 */
		void computeStep(
			const rgbd::Frame &currentFrame,
			const gl::Texture::Ptr &gradientMap,
			const glm::vec4 &cam,
			float sigmaVal,
			float rgbError,
			glm::mat4 &resultRt,
			Eigen::Isometry3f &rgbodomiso3f
		);
		//void getProducts(
		//	gl::Texture::Ptr srcColorMap,
		//	gl::Texture::Ptr dstColorMap,
		//	const float sharpVal
		//);
		//void getProducts(
		//	gl::Texture::Ptr srcColorMap,
		//	gl::Texture::Ptr dstColorMap,
		//	const float sharpVal
		//);

		typedef std::shared_ptr<rgbd::RGBOdometry> Ptr;

	};
}