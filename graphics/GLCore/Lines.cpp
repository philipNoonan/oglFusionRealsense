#include "Lines.h"

namespace gl
{
	Lines::Lines()
	{

		std::vector<GLfloat> vert(21 * 3, 0.75);

		std::vector<GLshort> index = { 1, 8, 1, 2, 1,
									5, 2, 3, 3, 4,
									5, 6, 6, 7, 8,
									9, 9, 10, 10, 11,
									8, 12, 12, 13, 13,
									14, 1, 0, 0, 15,
									15, 17, 0, 16, 16,
									18, 1, 19, 19, 20,
									2, 9, 5, 12 };

		vertices.create(vert.data(), (int)vert.size(), GL_DYNAMIC_DRAW);
		vao.addVertexAttrib(2, 3, vertices, 0, 0);

		indices.create(index.data(), (int)index.size(), GL_STATIC_READ);
		vao.bind();
		indices.bind();
		vao.unbind();
	}

	Lines::~Lines()
	{
	}

	void Lines::renderMulti(gl::Texture::Ptr depthTex, gl::Texture::Ptr normalTex, gl::Texture::Ptr colorTex, gl::Texture::Ptr infraTex)
	{
		vao.bind();
		depthTex->use(0);
		normalTex->use(1);
		colorTex->use(2);
		infraTex->use(3);

		glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_SHORT, 0);
		vao.unbind();
	}

	void Lines::render(gl::Texture::Ptr tex)
	{
		vao.bind();
		tex->use();
		glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_SHORT, 0);
		vao.unbind();
	}
}