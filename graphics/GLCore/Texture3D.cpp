#include "Texture3D.h"

namespace gl
{
	GLint Texture3D::getInternalFormat(Texture3DType type, int channels)
	{
		GLint internalFormat = GL_RGBA32F;
		switch (type)
		{
		case Texture3DType::FLOAT16:
			switch (channels)
			{
			case 1: internalFormat = GL_R16F; break;
			case 2:	internalFormat = GL_RG16F; break;
			case 3:	internalFormat = GL_RGB16F; break;
			case 4:	internalFormat = GL_RGBA16F; break;
			}
			break;
		case Texture3DType::FLOAT32:
			switch (channels)
			{
			case 1: internalFormat = GL_R32F; break;
			case 2:	internalFormat = GL_RG32F; break;
			case 3:	internalFormat = GL_RGB32F; break;
			case 4:	internalFormat = GL_RGBA32F; break;
			}
			break;
		case Texture3DType::UINT16:
			switch (channels)
			{
			case 1: internalFormat = GL_R16; break;
			case 2: internalFormat = GL_RG16; break;
			case 3: internalFormat = GL_RGB16; break;
			case 4: internalFormat = GL_RGBA16; break;
			}
		case Texture3DType::UINT32:
			switch (channels)
			{
			case 1: internalFormat = GL_R32UI; break;
			case 2: internalFormat = GL_RG32UI; break;
			case 3: internalFormat = GL_RGB32UI; break;
			case 4: internalFormat = GL_RGBA32UI; break;
			}
		}

		return internalFormat;
	}

	GLenum Texture3D::getFormat(Texture3DType type, int channels, bool normalized, bool invertChannels)
	{
		GLenum format = GL_RGBA;

		if (normalized)
		{
			if (invertChannels)
			{
				switch (channels)
				{
				case 1:	format = GL_RED; break;
				case 2:	format = GL_RG; break;
				case 3:	format = GL_BGR; break;
				case 4:	format = GL_BGRA; break;
				}
			}
			else
			{
				switch (channels)
				{
				case 1:	format = GL_RED; break;
				case 2:	format = GL_RG; break;
				case 3:	format = GL_RGB; break;
				case 4:	format = GL_RGBA; break;
				}
			}
		}
		else
		{
			if (invertChannels)
			{
				switch (channels)
				{
				case 1:	format = GL_RED_INTEGER; break;
				case 2:	format = GL_RG_INTEGER; break;
				case 3:	format = GL_BGR_INTEGER; break;
				case 4:	format = GL_BGRA_INTEGER; break;
				}
			}
			else
			{
				switch (channels)
				{
				case 1:	format = GL_RED_INTEGER; break;
				case 2:	format = GL_RG_INTEGER; break;
				case 3:	format = GL_RGB_INTEGER; break;
				case 4:	format = GL_RGBA_INTEGER; break;
				}
			}
		}

		return format;
	}


	Texture3D::Texture3D()
	{
		glGenTextures(1, &id);
	}


	Texture3D::~Texture3D()
	{
		glDeleteTextures(1, &id);
	}

	void Texture3D::create(const void* data, int width, int height, int depth, int channels, Texture3DType type, bool normalized, bool invertChannels)
	{
		this->width = width;
		this->height = height;
		this->depth = depth;

		internalFormat = getInternalFormat(type, channels);
		format = getFormat(type, channels, normalized, invertChannels);
		if (type == Texture3DType::FLOAT16 || type == Texture3DType::FLOAT32)
		{
			dataType = GL_FLOAT;
		}
		else if (type == Texture3DType::UINT16)
		{
			dataType = GL_UNSIGNED_SHORT;
		}
		else if (type == Texture3DType::UINT32)
		{
			dataType = GL_UNSIGNED_INT;
		}

		bind();
		//glTexStorage2D(GL_TEXTURE_2D, levels, internalformat, w, h); // use mipmapping!
		glTexImage3D(GL_TEXTURE_3D, 0, internalFormat, width, height, depth, 0, format, dataType, data);
		unbind();

	}

	void Texture3D::createStorage(int levels, int w, int h, int d, int channels, GLuint intFormat, Texture3DType type, bool normalized)
	{

		this->width = w;
		this->height = h;
		this->depth = d;

		internalFormat = intFormat;// getInternalFormat(type, channels);
		format = getFormat(type, channels, normalized, 0);
		if (type == Texture3DType::FLOAT16 || type == Texture3DType::FLOAT32)
		{
			dataType = GL_FLOAT;
		}
		else if (type == Texture3DType::UINT16)
		{
			dataType = GL_UNSIGNED_SHORT;
		}
		else if (type == Texture3DType::UINT32)
		{
			dataType = GL_UNSIGNED_INT;
		}

		bind();
		glTexStorage3D(GL_TEXTURE_3D, levels, internalFormat, w, h, d);
		unbind();

	}
	


	void Texture3D::update(const void* data)
	{
		glBindTexture(GL_TEXTURE_3D, id);
		glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, width, height, depth, format, dataType, data);
		glBindTexture(GL_TEXTURE_3D, 0);
	}

	void Texture3D::setFiltering(Texture3DFilter filter)
	{
		GLint minFilter = GL_NEAREST;
		GLint maxFilter = GL_NEAREST;

		switch (filter)
		{
		case Texture3DFilter::NEAREST:
			minFilter = GL_NEAREST;
			maxFilter = GL_NEAREST;
			break;
		case Texture3DFilter::LINEAR:
			minFilter = GL_LINEAR;
			maxFilter = GL_LINEAR;
			break;
		}

		bind();
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, maxFilter);
		unbind();
	}

	void Texture3D::setWarp(Texture3DWarp warp)
	{
		GLint warpMode;

		switch (warp)
		{
		case Texture3DWarp::REPEAT:
			warpMode = GL_REPEAT;
			break;
		case Texture3DWarp::MIRRORED_REPEAT:
			warpMode = GL_MIRRORED_REPEAT;
			break;
		case Texture3DWarp::CLAMP_TO_EDGE:
			warpMode = GL_CLAMP_TO_EDGE;
			break;
		case Texture3DWarp::CLAM_TO_BORDER:
			warpMode = GL_CLAMP_TO_BORDER;
			break;
		}

		bind();
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, warpMode);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, warpMode);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, warpMode);

		unbind();
	}
	
	void Texture3D::read(const void* data)
	{
		bind();
		glGetTexImage(GL_TEXTURE_3D, 0, format, dataType, (GLvoid*)data);
		unbind();
	}

	void Texture3D::read(const void* data, GLenum format, GLenum dataType)
	{
		bind();
		glGetTexImage(GL_TEXTURE_3D, 0, format, dataType, (GLvoid*)data);
		unbind();
	}

	void Texture3D::clear(int level, const void * data)
	{
		//bind();
		glClearTexSubImage(GL_TEXTURE_3D, level, 0, 0, 0, width >> level, height >> level, depth >> level, GL_RED_INTEGER, GL_INT, data);
		//unbind();
	}

	void Texture3D::bind()
	{
		glBindTexture(GL_TEXTURE_3D, id);
	}

	void Texture3D::unbind()
	{
		glBindTexture(GL_TEXTURE_3D, 0);
	}

	void Texture3D::bindImage(int idx, int level, GLenum access)
	{

		glBindImageTexture(idx, id, level, GL_FALSE, 0, access, internalFormat);
	}

	void Texture3D::bindImage(int idx, int level, GLenum access, GLenum internalFormat)
	{
		glBindImageTexture(idx, id, level, GL_FALSE, 0, access, internalFormat);
	}

	void Texture3D::use(int idx)
	{
		glActiveTexture(GL_TEXTURE0 + idx);
		bind();
	}

	int Texture3D::getWidth() const
	{
		return width;
	}

	int Texture3D::getHeight() const
	{
		return height;
	}

	int Texture3D::getDepth() const
	{
		return depth;
	}
	
	int Texture3D::getID() const
	{
		return id;
	}

	void Texture3D::deleteTexture()
	{
		glDeleteTextures(1, &id);
	}

}