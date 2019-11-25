#pragma once

#include "ComputeShader.h"
#include "glhelper.h"

namespace rgbd
{
	class CalcVertexMap : public ComputeShader
	{
	private:
		const gl::Shader::Ptr prog;
		glm::mat4 invK;

	public:
		CalcVertexMap(
			const glm::mat4 &K,
			const gl::Shader::Ptr prog
		);

		void execute(
			gl::Texture::Ptr srcDepthMap,
			gl::Texture::Ptr dstVertexMap,
			float minDepth,
			float maxDepth,
			glm::vec2(bottomLeft),
			glm::vec2(topRight)
		);
	};
}