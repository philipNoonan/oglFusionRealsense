#include "AlignDepthColor.h"

namespace rgbd
{
	AlignDepthColor::AlignDepthColor(
		const gl::Shader::Ptr prog
	) : prog(prog)
	{
	}

	void AlignDepthColor::execute(
		gl::Texture::Ptr srcVertexMap,
		gl::Texture::Ptr srcColorMap,
		gl::Texture::Ptr dstColorMap,
		gl::Texture::Ptr mappingC2DMap,
		gl::Texture::Ptr mappingD2CMap,
		const glm::mat4 &depthToColorExtrins,
		const glm::vec4 &colorIntrins // cx cy fx fy 
	)
	{

		for (int level = 0; level < 3; level++)
		{
			prog->use();
			prog->setUniform("functionID", 0);

			mappingC2DMap->bindImage(3, level, GL_READ_WRITE);
			mappingD2CMap->bindImage(4, level, GL_READ_WRITE);

			glDispatchCompute(GLHelper::divup(srcVertexMap->getWidth() >> level, 32), GLHelper::divup(srcVertexMap->getHeight() >> level, 32), 1);
			prog->disuse();

			glm::vec4 levelColorCam = glm::vec4(
				colorIntrins.x / (std::pow(2.0f, level)),
				colorIntrins.y / (std::pow(2.0f, level)),
				colorIntrins.z / (std::pow(2.0f, level)),
				colorIntrins.w / (std::pow(2.0f, level))
			);


			prog->use();
			prog->setUniform("functionID", 1);
			prog->setUniform("d2c", depthToColorExtrins);
			prog->setUniform("cam", levelColorCam);
			srcVertexMap->bindImage(0, level, GL_READ_ONLY);
			srcColorMap->bindImage(1, level, GL_READ_ONLY);
			dstColorMap->bindImage(2, level, GL_WRITE_ONLY);
			mappingC2DMap->bindImage(3, level, GL_READ_WRITE);
			mappingD2CMap->bindImage(4, level, GL_READ_WRITE);

			glDispatchCompute(GLHelper::divup(srcVertexMap->getWidth() >> level, 32), GLHelper::divup(srcVertexMap->getHeight() >> level, 32), 1);
			prog->disuse();
		}

		
	}
}