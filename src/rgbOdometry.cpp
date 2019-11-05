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
		ssboRGBRGBJtJJtrSE3Output.create(tempJtJOutputData.data(), 29, GL_DYNAMIC_DRAW);
		ssboRGBRGBJtJJtrSE3Output.bindBase(2);
		ssboRGBRGBJtJJtrSE3Output.unbind();


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
		const gl::Texture::Ptr & gradientMap,
		const glm::vec3 kT,
		const glm::mat3 krkinv
	)
	{
		progs["rgbOdometry"]->setUniform("functionID", 0);
		progs["rgbOdometry"]->setUniform("minScale", 0.0f);
		progs["rgbOdometry"]->setUniform("maxDepthDelta", 0.07f);
		progs["rgbOdometry"]->setUniform("kt", kT);
		progs["rgbOdometry"]->setUniform("krkinv", krkinv);

		progs["rgbOdometry"]->use();
		currentFrame.getColorPreviousMap()->bindImage(0, 0, GL_READ_ONLY);
		currentFrame.getColorFilteredMap()->bindImage(1, 0, GL_READ_ONLY);
		gradientMap->bindImage(2, 0, GL_READ_ONLY);
		currentFrame.getDepthPreviousMap()->bindImage(3, 0, GL_READ_ONLY);
		currentFrame.getDepthMap()->bindImage(4, 0, GL_READ_ONLY);

		ssboRGBReduction.bindBase(0);


		glDispatchCompute(GLHelper::divup(currentFrame.getColorMap()->getWidth(), 32), GLHelper::divup(currentFrame.getColorMap()->getHeight(), 32), 1);
		progs["rgbOdometry"]->disuse();

		progs["rgbOdometryReduce"]->use();
		progs["rgbOdometryReduce"]->setUniform("imSize", glm::ivec2(currentFrame.getColorMap()->getWidth(), currentFrame.getColorMap()->getHeight()));

		ssboRGBReduction.bindBase(0);
		ssboRGBReductionOutput.bindBase(1);

		glDispatchCompute(8, 1, 1);
		progs["rgbOdometryReduce"]->disuse();

		std::vector<float> tmpOutputData(8);
		std::vector<BufferReductionRGB> tmpMapData(currentFrame.getColorMap()->getWidth() * currentFrame.getColorMap()->getHeight());

		ssboRGBReductionOutput.read(tmpOutputData.data(), 0, 8);
		ssboRGBReduction.read(tmpMapData.data(), 0, currentFrame.getColorMap()->getWidth() * currentFrame.getColorMap()->getHeight());


	}

	void RGBOdometry::computeStep(
		const rgbd::Frame &currentFrame,
		const rgbd::Frame &virtualFrame
	)
	{


	}
}