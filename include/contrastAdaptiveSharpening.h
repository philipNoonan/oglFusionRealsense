#pragma once

#include "ComputeShader.h"
#include "glhelper.h"

namespace rgbd
{
	// Ref: https://github.com/GPUOpen-Effects/FidelityFX
	class CASFilter : public ComputeShader
	{
	private:
		const gl::Shader::Ptr prog;

	public:
		CASFilter(
			const gl::Shader::Ptr prog
		);
		void execute(
			gl::Texture::Ptr srcColorMap,
			gl::Texture::Ptr dstColorMap,
			const float sharpVal
		);
	};
}