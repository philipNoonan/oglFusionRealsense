#pragma once

#include "ComputeShader.h"
#include "glhelper.h"
#include "GLCore/Texture.h"
#include "Frame.h"


namespace rgbd
{
	// Ref: https://www.shadertoy.com/view/4dfGDH
	class GradientFilter : public ComputeShader
	{
	private:
		std::map<std::string, const gl::Shader::Ptr> progs;

		gl::Texture::Ptr gradientMap;


	public:
		GradientFilter(
		);
		~GradientFilter();

		void loadShaders(
			std::map<std::string, const gl::Shader::Ptr> &progs,
			const std::string &folderPath = "../../shaders/"
		);

		void init(
			int width,
			int height,
			const std::map<std::string, const gl::Shader::Ptr> &progs
		);



		void execute(
			gl::Texture::Ptr imageMap,
			int level,
			float lower,
			float upper,
			bool useGaussian
		);

		gl::Texture::Ptr getGradientMap() const;


	};
}