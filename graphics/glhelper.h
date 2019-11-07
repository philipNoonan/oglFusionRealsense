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

#include <Eigen/Dense>

#include <algorithm>


namespace rgbd
{
	struct BufferReduction
	{
		GLint result;
		GLfloat error;
		GLfloat J[6];

		BufferReduction()
		{
			result = 0;
			error = 0.0f;
			for (int i = 0; i < 6; i++)
			{
				J[i] = 0.0f;
			}
		}
	};


	struct BufferReductionP2V
	{
		GLint result;
		GLfloat h;
		GLfloat D;
		GLfloat J[6];

		BufferReductionP2V()
		{
			result = 0;
			h = 0.0f;
			D = 0.0f;
			for (int i = 0; i < 6; i++)
			{
				J[i] = 0.0f;
			}
		}
	};

	struct BufferReductionRGB
	{
		GLint zero[2];
		GLint one[2];
		GLfloat diff;
		bool valid;

		BufferReductionRGB()
		{
			zero[0] = 0;
			zero[1] = 0;
			one[0] = 0;
			one[1] = 0;

			diff = 0.0f;
			valid = false;
		}
	};

	struct BufferReductionDTAM
	{
		GLfloat row[4];
		bool valid;
		BufferReductionDTAM()
		{
			for (int i = 0; i < 4; i++)
			{
				row[i] = 0.0f;
			}
			valid = false;
		}

	};


}


namespace GLHelper
{

	//template<typename T, int m, int n>
	//inline glm::mat<m, n, float, glm::precision::highp> E2GLM(const Eigen::Matrix<T, m, n>& em)
	//{
	//	glm::mat<m, n, float, glm::precision::highp> mat;
	//	for (int i = 0; i < m; ++i)
	//	{
	//		for (int j = 0; j < n; ++j)
	//		{
	//			mat[j][i] = em(i, j);
	//		}
	//	}
	//	return mat;
	//}

	//template<typename T, int m>
	//inline glm::vec<m, float, glm::precision::highp> E2GLM(const Eigen::Matrix<T, m, 1>& em)
	//{
	//	glm::vec<m, float, glm::precision::highp> v;
	//	for (int i = 0; i < m; ++i)
	//	{
	//		v[i] = em(i);
	//	}
	//	return v;
	//}




	GLuint createTexture(GLuint ID, GLenum target, int levels, int w, int h, int d, GLuint internalformat, GLenum magFilter, GLenum minFilter);
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