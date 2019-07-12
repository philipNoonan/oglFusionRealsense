#include "glhelper.h"

namespace GLHelper
{
	GLuint createTexture(GLuint ID, GLenum target, int levels, int w, int h, int d, GLuint internalformat, GLenum magFilter, GLenum minFilter)
	{
		GLuint texid;

		if (ID == 0)
		{
			glGenTextures(1, &texid);
		}
		else
		{
			glDeleteTextures(1, &ID);
			texid = ID;
			glGenTextures(1, &texid);
		}

		glGenTextures(1, &texid);
		glBindTexture(target, texid);

		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilter);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		// https://stackoverflow.com/questions/15405869/is-gltexstorage2d-imperative-when-auto-generating-mipmaps
		//glTexImage2D(target, 0, internalformat, w, h, 0, format, type, 0); // cretes mutable storage that requires glTexImage2D

		if (target == GL_TEXTURE_1D)
		{
			glTexStorage1D(target, levels, internalformat, w);
		}
		else if (target == GL_TEXTURE_2D)
		{
			glTexStorage2D(target, levels, internalformat, w, h); // creates immutable storage and requires glTexSubImage2D

		}
		else if (target == GL_TEXTURE_3D || d > 0)
		{
			glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
			glTexStorage3D(target, levels, internalformat, w, h, d);
		}

		float color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, color);

		return texid;
	}

	glm::mat4 getInverseCameraMatrix(const glm::vec4 & k) { // [col][row]
		glm::mat4 invK;
		invK[0][0] = 1.0f / k.x;	invK[1][0] = 0;				invK[2][0] = -k.z / k.x;	invK[3][0] = 0;
		invK[0][1] = 0;				invK[1][1] = 1.0f / k.y;	invK[2][1] = -k.w / k.y;	invK[3][1] = 0;
		invK[0][2] = 0;				invK[1][2] = 0;				invK[2][2] = 1;				invK[3][2] = 0;
		invK[0][3] = 0;				invK[1][3] = 0;				invK[2][3] = 0;				invK[3][3] = 1;

		return invK;
	}

	glm::mat4 getCameraMatrix(const glm::vec4 & k) {
		glm::mat4 K;

		K[0][0] = k.x;	K[1][0] = 0;	K[2][0] = k.z;	K[3][0] = 0;
		K[0][1] = 0;	K[1][1] = k.y;	K[2][1] = k.w;	K[3][1] = 0;
		K[0][2] = 0;	K[1][2] = 0;	K[2][2] = 1;	K[3][2] = 0;
		K[0][3] = 0;	K[1][3] = 0;	K[2][3] = 0;	K[3][3] = 1;

		return K;
	}

	uint32_t nextPowerOfTwo(uint32_t n)
	{
		--n;

		n |= n >> 1;
		n |= n >> 2;
		n |= n >> 4;
		n |= n >> 8;
		n |= n >> 16;

		return n + 1;
	}

	uint32_t numberOfLevels(glm::ivec3 dims)
	{
		return 1 + floor(std::log2(max3(dims.x, dims.y, dims.z)));
	}
	
	void projectionFromIntrinsics(glm::mat4 &projection, double fx, double fy, double skew, double cx, double cy, int img_width, int img_height, double near_clip, double far_clip) {

		projection[0][0] = (2.0f * fx / img_width);
		projection[2][0] = (2.0f * cx / img_width) - 1.0f;
		projection[1][1] = (2.0f * fy / img_height);
		projection[2][1] = (2.0f * cy / img_height) - 1.0f;
		projection[2][2] = -(far_clip + near_clip) / (far_clip - near_clip);
		projection[3][2] = 2.0 * far_clip * near_clip / (far_clip - near_clip);
		projection[2][3] = 1.0;
	}

}
