#pragma once

#include "ComputeShader.h"
#include "glhelper.h"

namespace rgbd
{
	// Ref: https://www.shadertoy.com/view/4dfGDH
	class AlignDepthColor : public ComputeShader
	{
	private:
		const gl::Shader::Ptr prog;

	public:
		AlignDepthColor(
			const gl::Shader::Ptr prog
		);
		void execute(
			gl::Texture::Ptr srcVertexMap,
			gl::Texture::Ptr srcColorMap,
			gl::Texture::Ptr dstColorMap,
			gl::Texture::Ptr mappingC2DMap,
			gl::Texture::Ptr mappingD2CMap,
			const glm::mat4 &depthToColorExtrins,
			const glm::vec4 &colorIntrins
		);
	};
}