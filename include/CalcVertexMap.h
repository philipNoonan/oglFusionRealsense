#pragma once

#include "ComputeShader.h"

namespace rgbd
{
	class CalcVertexMap : public ComputeShader
	{
	private:
		const gl::Shader::Ptr prog;
		glm::mat4 invK;

	public:
		CalcVertexMap(
			float minDepth,
			float maxDepth,
			const glm::mat4 &K,
			const gl::Shader::Ptr prog
		);

		void execute(
			gl::Texture::Ptr srcDepthMap,
			gl::Texture::Ptr dstVertexMap
		);
	};
}