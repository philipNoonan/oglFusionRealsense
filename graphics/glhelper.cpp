#include "glhelper.h"

namespace GLHelper
{
	GLuint createTexture(GLuint ID, GLenum target, int levels, int w, int h, int d, GLuint internalformat)
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
		glBindTexture(target, texid);


		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

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
			glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexStorage3D(target, levels, internalformat, w, h, d);
		}
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

	
	


}
