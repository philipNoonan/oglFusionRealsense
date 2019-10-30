#include "contrastAdaptiveSharpening.h"

namespace rgbd
{
	CASFilter::CASFilter(
		const gl::Shader::Ptr prog
	) : prog(prog)
	{
	}

	void CASFilter::execute(
		gl::Texture::Ptr srcColorMap,
		gl::Texture::Ptr dstColorMap,
		const float sharpVal
	)
	{
		prog->setUniform("sharpness", sharpVal);

		prog->use();
		srcColorMap->bindImage(0, 0, GL_READ_ONLY);
		dstColorMap->bindImage(1, 0, GL_WRITE_ONLY);

		glDispatchCompute(GLHelper::divup(srcColorMap->getWidth(), 32), GLHelper::divup(srcColorMap->getHeight(), 32), 1);
		prog->disuse();
	}
}