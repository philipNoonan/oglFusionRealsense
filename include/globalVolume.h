#pragma once

#include <array>
#include <map>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "GLCore/Framebuffer.h"
#include "GLCore/Buffer.h"
#include "GLCore/Texture3D.h"
#include "GLCore/Shader.h"
#include "GLCore/Texture.h"
#include "Frame.h"
#include "ConstantParameters.h"
#include "ClibratedProjectionMatrix.h"

namespace rgbd
{
	class GlobalVolume
	{
	private:
		const int width;
		const int height;
		glm::vec3 size;
		glm::vec3 dim;
		const glm::mat4 K;
		gl::Texture3D::Ptr volumeTex;

		std::map<std::string, const gl::Shader::Ptr> progs;


	public:
		GlobalVolume(
			int width,
			int height,
			glm::vec3 &dim,
			glm::vec3 &size,
			const glm::mat4 &K,
			const float maxWeight,
			const float largeStep,
			const float step,
			const float nearPlane,
			const float farPlane,
			const std::map<std::string, const gl::Shader::Ptr> &progs
		);
		~GlobalVolume();


		void integrate(
			const int fType,
			const rgbd::Frame &srcFrame,
			const glm::mat4 &T
		);

		void raycast(
			const rgbd::Frame & dstFrame,
			const glm::mat4 &T
		);

		GLuint getID();



		//void sample(
		//	const glm::vec3 &dim
		//);

		void exportVertexPointCloud(std::vector<glm::vec3> &outputVertexData);

		void reset();

		void setVolDim(glm::vec3 vDim);

		void resize(glm::vec3 vSize);

		typedef std::shared_ptr<rgbd::GlobalVolume> Ptr;
	};
}