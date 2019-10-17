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
		const glm::mat4 &depthToColorExtrins,
		const glm::vec4 &colorIntrins // cx cy fx fy 
	)
	{
		prog->setUniform("d2c", depthToColorExtrins);
		prog->setUniform("cam", colorIntrins);

		prog->use();
		srcVertexMap->bindImage(0, 0, GL_READ_ONLY);
		srcColorMap->bindImage(1, 0, GL_READ_ONLY);
		dstColorMap->bindImage(2, 0, GL_WRITE_ONLY);
		glDispatchCompute(srcVertexMap->getWidth() / 32, srcVertexMap->getHeight() / 32, 1);
		prog->disuse();
	}
}