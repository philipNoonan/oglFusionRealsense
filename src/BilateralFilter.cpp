#include "BilateralFilter.h"

namespace rgbd
{
	BilateralFilter::BilateralFilter(
		const gl::Shader::Ptr prog
	) : prog(prog)
	{
	}

	void BilateralFilter::execute(
		gl::Texture::Ptr srcShortDepthMap,
		gl::Texture::Ptr rawDepthMap,
		gl::Texture::Ptr dstDepthMap,
		float depthScale,
		float sigma,
		float bSigma
	)
	{
		prog->setUniform("sigma", sigma);
		prog->setUniform("bSigma", bSigma);
		prog->setUniform("depthScale", depthScale);

		prog->use();
		srcShortDepthMap->bindImage(0, 0, GL_READ_ONLY);
		rawDepthMap->bindImage(1, 0, GL_WRITE_ONLY);
		dstDepthMap->bindImage(2, 0, GL_WRITE_ONLY);
		glDispatchCompute(dstDepthMap->getWidth() / 32, dstDepthMap->getHeight() / 32, 1);
		prog->disuse();
	}
}