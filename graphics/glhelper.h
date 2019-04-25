#ifndef GLHELPER_H
#define GLHELPER_H

#define GLUT_NO_LIB_PRAGMA
//##### OpenGL ######
#include <GL/glew.h>
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

#include <algorithm>

namespace GLHelper
{
	GLuint createTexture(GLuint ID, GLenum target, int levels, int w, int h, int d, GLuint internalformat);
	glm::mat4 getInverseCameraMatrix(const glm::vec4 & k);
	glm::mat4 getCameraMatrix(const glm::vec4 & k);
	
	inline glm::uint divup(int a, int b) { return (a % b != 0) ? (a / b + 1) : (a / b); }
	inline glm::uvec3 divup(glm::uvec2 a, glm::uvec3 b) { return glm::uvec3(divup(a.x, b.x), divup(a.y, b.y), 1); }
	inline glm::uvec3 divup(glm::uvec3 a, glm::uvec3 b) { return glm::uvec3(divup(a.x, b.x), divup(a.y, b.y), divup(a.z, b.z)); }

	template <typename T>
	T max3(T w, T h, T d) {
		return std::max(std::max(w, h), d);
	}

	uint32_t nextPowerOfTwo(uint32_t n);

	uint32_t numberOfLevels(glm::ivec3 dims);

	void projectionFromIntrinsics(glm::mat4 &projection, double fx, double fy, double skew, double cx, double cy, int img_width, int img_height, double near_clip, double far_clip);


}




#endif