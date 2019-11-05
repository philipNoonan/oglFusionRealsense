#include "rgbOdometry.h"

namespace rgbd
{
	RGBOdometry::RGBOdometry(
		int width, 
		int height,
		const std::map<std::string, const gl::Shader::Ptr> &progs
		) : progs{ { "rgbOdometry", progs.at("rgbOdometry") },
			       { "rgbOdometryReduce", progs.at("rgbOdometryReduce") },
			       { "rgbOdometryStep", progs.at("rgbOdometryStep") },
				   { "rgbOdometryStepReduce", progs.at("rgbOdometryStepReduce") } }
	{
		std::vector<BufferReductionRGB> tmpMapData(width * height);
		std::vector<float> tempOutputData(8);
		std::vector<float> tempJtJOutputData(29);

		ssboRGBReduction.bind();
		ssboRGBReduction.create(tmpMapData.data(), width * height, GL_DYNAMIC_DRAW);
		ssboRGBReduction.bindBase(0);
		ssboRGBReduction.unbind();

		ssboRGBReductionOutput.bind();
		ssboRGBReductionOutput.create(tempOutputData.data(), 8, GL_DYNAMIC_DRAW);
		ssboRGBReductionOutput.bindBase(1);
		ssboRGBReductionOutput.unbind();

		ssboRGBRGBJtJJtrSE3Output.bind();
		ssboRGBReductionOutput.create(tempJtJOutputData.data(), 29, GL_DYNAMIC_DRAW);
		ssboRGBReductionOutput.bindBase(2);
		ssboRGBReductionOutput.unbind();


	}

	void RGBOdometry::computeDerivativeImages(
		gl::Texture::Ptr srcNextImage,
		gl::Texture::Ptr dstNextIdxy
	)
	{
		progs["rgbOdometry"]->setUniform("functionID", 0);

		progs["rgbOdometry"]->use();
		srcNextImage->bindImage(0, 0, GL_READ_ONLY);
		dstNextIdxy->bindImage(2, 0, GL_WRITE_ONLY);

		glDispatchCompute(GLHelper::divup(srcNextImage->getWidth(), 32), GLHelper::divup(srcNextImage->getHeight(), 32), 1);
		progs["rgbOdometry"]->disuse();
	}

	void RGBOdometry::computeResiduals(
		const rgbd::Frame &currentFrame,
		const rgbd::Frame &virtualFrame,
		const gl::Texture::Ptr & gradientMap,
		const glm::vec3 kT,
		const glm::mat3 krkinv
	)
	{
		progs["rgbOdometry"]->setUniform("functionID", 1);
		progs["rgbOdometry"]->setUniform("minScale", 1.0f);
		progs["rgbOdometry"]->setUniform("depthScale", 1.0f);
		progs["rgbOdometry"]->setUniform("kt", kT);
		progs["rgbOdometry"]->setUniform("krkinv", krkinv);

		progs["rgbOdometry"]->use();
		currentFrame.getColorMap()->bindImage(0, 0, GL_READ_ONLY);
		virtualFrame.getColorMap()->bindImage(1, 0, GL_READ_ONLY);
		gradientMap->bindImage(2, 0, GL_READ_ONLY);
		currentFrame.getDepthMap()->bindImage(3, 0, GL_READ_ONLY);
		virtualFrame.getDepthMap()->bindImage(4, 0, GL_READ_ONLY);

		ssboRGBReduction.bindBase(0);


		glDispatchCompute(GLHelper::divup(currentFrame.getColorMap()->getWidth(), 32), GLHelper::divup(currentFrame.getColorMap()->getHeight(), 32), 1);
		progs["rgbOdometry"]->disuse();

		progs["rgbOdometryReduce"]->use();
		progs["rgbOdometryReduce"]->setUniform("imSize", glm::ivec2(currentFrame.getColorMap()->getWidth(), currentFrame.getColorMap()->getHeight()));

		ssboRGBReduction.bindBase(0);
		ssboRGBReductionOutput.bindBase(1);

		glDispatchCompute(8, 1, 1);
		progs["rgbOdometryReduce"]->disuse();
	}

	void RGBOdometry::computeStep(
		const rgbd::Frame &currentFrame,
		const rgbd::Frame &virtualFrame
	)
	{


	}
}