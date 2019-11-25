#include "CalcVertexMap.h"

namespace rgbd
{
	CalcVertexMap::CalcVertexMap(
		const glm::mat4 &K,
		const gl::Shader::Ptr prog
	) : prog(prog), invK(glm::inverse(K))
	{
		invK = glm::inverse(K);
		prog->setUniform("invK", invK);

	}

	void CalcVertexMap::execute(
		gl::Texture::Ptr srcDepthMap,
		gl::Texture::Ptr dstVertexMap,
		float minDepth,
		float maxDepth,
		glm::vec2(bottomLeft),
		glm::vec2(topRight)
	)
	{
		prog->setUniform("minDepth", minDepth);
		prog->setUniform("maxDepth", maxDepth);
		prog->setUniform("bottomLeft", bottomLeft);
		prog->setUniform("topRight", topRight);

		prog->use();
		srcDepthMap->bindImage(0, 0, GL_READ_ONLY);
		dstVertexMap->bindImage(1, 0, GL_WRITE_ONLY);
		glDispatchCompute(GLHelper::divup(dstVertexMap->getWidth(), 32), GLHelper::divup(dstVertexMap->getHeight(), 32), 1);
		prog->disuse();
	}
}