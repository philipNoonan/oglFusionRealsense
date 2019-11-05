#include "GradientFilter.h"

namespace rgbd
{
	GradientFilter::GradientFilter()
	{
	}

	GradientFilter::~GradientFilter()
	{
	}

	void GradientFilter::loadShaders(
		std::map<std::string, const gl::Shader::Ptr> &progs,
		const std::string &folderPath
	)
	{
		progs.insert(std::make_pair("GradientFilter", std::make_shared<gl::Shader>(folderPath + "GradientFilter.comp")));
		}


	void GradientFilter::init(
		int width, 
		int height,
		const std::map<std::string, const gl::Shader::Ptr> &programs
	)
	{
		progs = programs;

		int numberOfLevels = GLHelper::numberOfLevels(glm::ivec3(width, height, 1));


		gradientMap = std::make_shared<gl::Texture>();
		gradientMap->createStorage(numberOfLevels, width, height, 2, GL_RG32F, gl::TextureType::FLOAT32, 0);
		gradientMap->setFiltering(gl::TextureFilter::NEAREST);
		gradientMap->setWarp(gl::TextureWarp::CLAMP_TO_EDGE);


	}

	void GradientFilter::setFrames(
		gl::Texture::Ptr lastFrame,
		gl::Texture::Ptr nextFrame
	)
	{
		I0 = lastFrame;
		I1 = nextFrame;
	}

	void GradientFilter::execute(
		gl::Texture::Ptr gradientMap,
		int level,
		float lesser,
		float upper,
		bool useGaussian
	)
	{
		progs["GradientFilter"]->setUniform("lesser", lesser);
		progs["GradientFilter"]->setUniform("upper", upper);
		progs["GradientFilter"]->setUniform("useGaussian", useGaussian);

		progs["GradientFilter"]->use();
		I0->use(0);
		I1->use(1);
		for (int lvl = 0; lvl < 3; lvl++)
		{
			gradientMap->bindImage(0, lvl, GL_WRITE_ONLY);
			progs["GradientFilter"]->setUniform("level", lvl);

			glDispatchCompute(GLHelper::divup(I0->getWidth() >> lvl, 32), GLHelper::divup(I0->getHeight() >> lvl, 32), 1);
		}

		progs["GradientFilter"]->disuse();
	}

	gl::Texture::Ptr GradientFilter::getGradientMap() const
	{
		return gradientMap;
	}
}