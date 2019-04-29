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

		//glm::mat4 trans = glm::mat4(1.0f);
		//trans[2][2] = -1.0f;
		// https://stackoverflow.com/questions/46317246/ar-with-opencv-opengl
		// but the link does contain some inaccuracies, so be careful. below works!

		// GLdouble perspMatrix[16]={2*fx/W,    0,          0,      0,
		//                           2*s/W,     2*fy/H,     0,      0,
		//                           2*(cx/W)-1,2*(cy/H)-1, (zmax+zmin)/(zmax-zmin), 1,
		//                           0,         0,          2*zmax*zmin/(zmin-zmax), 0};

		//GLdouble perspMatrix[16] = { 2 * fx / w,      0,               0,              0,
		//                             0,               2 * fy / h,      0,              0,
		//                             2 * (cx / w) - 1,2 * (cy / h) - 1, -(far + near) / (far - near),  -1,
		//                             0,               0,                -2 * far*near / (far - near),  0 };

		//projection[0][0] = (2.0f * fx / img_width);
		//projection[2][0] = (2.0f * cx / img_width) - 1.0f;
		//projection[1][1] = (2.0f * fy / img_height);
		//projection[2][1] = (2.0f * cy / img_height) - 1.0f;
		//projection[2][2] = (far_clip + near_clip) / (far_clip - near_clip);
		//projection[3][2] = 2.0 * far_clip * near_clip / (far_clip - near_clip);
		//projection[2][3] = 1.0;

		projection[0][0] = (2.0f * fx / img_width);
		projection[2][0] = (2.0f * cx / img_width) - 1.0f;
		projection[1][1] = (2.0f * fy / img_height);
		projection[2][1] = (2.0f * cy / img_height) - 1.0f;
		projection[2][2] = -(far_clip + near_clip) / (far_clip - near_clip);
		projection[3][2] = 2.0 * far_clip * near_clip / (far_clip - near_clip);
		projection[2][3] = 1.0;

		//projection[0][0] = (2.0 * fx / img_width);
		//projection[2][0] = 1.0 - (2.0 * cx / img_width);

		//projection[1][1] = (2.0 * fy / img_height);
		//projection[2][1] = 1.0 - (2.0 * cy / img_height);

		//projection[2][2] = (far_clip + near_clip) / (far_clip - near_clip);
		//projection[3][2] = 2.0 * far_clip * near_clip / (far_clip - near_clip);

		//projection[2][3] = -1.0;

		//projection = trans * projection;

		//projection[1] *= -1.0;












		// These parameters define the final viewport that is rendered into by
		//// the camera.
		//double L = 0;
		//double R = img_width;
		//double B = 0;
		//double T = img_height;

		//// near and far clipping planes, these only matter for the mapping from
		//// world-space z-coordinate into the depth coordinate for OpenGL
		//double N = near_clip;
		//double F = far_clip;

		//// set the viewport parameters
		////viewport[0] = L;
		////viewport[1] = B;
		////viewport[2] = R - L;
		////viewport[3] = T - B;

		//// construct an orthographic matrix which maps from projected
		//// coordinates to normalized device coordinates in the range
		//// [-1, 1].  OpenGL then maps coordinates in NDC to the current
		//// viewport
		//glm::mat4 ortho = glm::mat4(0.0f);
		//ortho[0][0] = 2.0 / (R - L);  ortho[0][3] = -(R + L) / (R - L);
		//ortho[1][1] = 2.0 / (T - B);  ortho[1][3] = -(T + B) / (T - B);
		//ortho[2][2] = -2.0 / (F - N); ortho[2][3] = -(F + N) / (F - N);
		//ortho[3][3] = 1.0;

		//// construct a projection matrix, this is identical to the 
		//// projection matrix computed for the intrinsicx, except an
		//// additional row is inserted to map the z-coordinate to
		//// OpenGL. 
		//glm::mat4 tproj = glm::mat4(0.0f);
		//tproj[0][0] = fx;       tproj[0][1] = skew;     tproj[0][2] = cx;
		//                        tproj[1][1] = fy;       tproj[1][2] = cy;
		//                                                tproj[2][2] = -(N + F); tproj[2][3] = -N * F;
		//                                                tproj[3][2] = 1.0;

		//// resulting OpenGL frustum is the product of the orthographic
		//// mapping to normalized device coordinates and the augmented
		//// camera intrinsic matrix
		//frustum = ortho * tproj;
		//projection = tproj;
	}

}
