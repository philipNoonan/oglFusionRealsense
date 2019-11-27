#pragma once

#include "ComputeShader.h"
#include "glhelper.h"
#include "GLCore/Framebuffer.h"

namespace rgbd
{
	
	class DisFlow : public ComputeShader
	{
	private:
		std::map<std::string, const gl::Shader::Ptr> progs;

		gl::Texture::Ptr sparseFlowMap;

		gl::Texture::Ptr lastFlowMap;
		gl::Texture::Ptr nextFlowMap;

		GLuint vertexVBO;
		GLuint VAO;


		std::vector<gl::Texture::Ptr> densificationFlowMap;

		std::vector<gl::Framebuffer> densificationFBO;


	public:
		DisFlow();
		~DisFlow();

		void DisFlow::loadShaders(
			std::map<std::string, const gl::Shader::Ptr> &progs,
			const std::string &folderPath
		);

		
		void init(
			int numLevels,
			int width,
			int height,
			const std::map<std::string, const gl::Shader::Ptr> &programs
		);

		void execute(
			gl::Texture::Ptr lastColorMap,
			gl::Texture::Ptr nextColorMap,
			gl::Texture::Ptr nextGradientMap
		);

		gl::Texture::Ptr getFlowMap() const;

	};
}