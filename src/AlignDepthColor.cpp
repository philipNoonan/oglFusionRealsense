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


		prog->use();
		prog->setUniform("functionID", 0);

		mappingC2DMap->bindImage(3, 0, GL_READ_WRITE);
		mappingD2CMap->bindImage(4, 0, GL_READ_WRITE);

		glDispatchCompute(GLHelper::divup(srcVertexMap->getWidth(), 32), GLHelper::divup(srcVertexMap->getHeight(), 32), 1);
		prog->disuse();




		prog->use();
		prog->setUniform("functionID", 1);
		prog->setUniform("d2c", depthToColorExtrins);
		prog->setUniform("cam", colorIntrins);
		srcVertexMap->bindImage(0, 0, GL_READ_ONLY);
		srcColorMap->bindImage(1, 0, GL_READ_ONLY);
		dstColorMap->bindImage(2, 0, GL_WRITE_ONLY);
		mappingC2DMap->bindImage(3, 0, GL_READ_WRITE);
		mappingD2CMap->bindImage(4, 0, GL_READ_WRITE);

		glDispatchCompute(GLHelper::divup(srcVertexMap->getWidth(), 32), GLHelper::divup(srcVertexMap->getHeight(), 32), 1);
		prog->disuse();
	}
}