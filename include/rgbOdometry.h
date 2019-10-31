#pragma once

#include "ComputeShader.h"
#include "glhelper.h"
#include "GLCore/Buffer.h"
#include "Frame.h"

namespace rgbd
{
	// Ref: https://github.com/GPUOpen-Effects/FidelityFX
	class RGBOdometry : public ComputeShader
	{
	private:
		std::map<std::string, const gl::Shader::Ptr> progs;
		gl::ShaderStorageBuffer<BufferReductionRGB> ssboRGBReduction;
		gl::ShaderStorageBuffer<float> ssboRGBReductionOutput;

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
			const rgbd::Frame &virtualFrame,
			const glm::vec4 kT,
			const glm::mat3 krkinv
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