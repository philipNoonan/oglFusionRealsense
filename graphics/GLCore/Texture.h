#pragma once

#include <memory>
#include <GL/glew.h>

namespace gl
{
	enum class TextureType
	{
		COLOR,
		SRGB,
		DEPTH,
		DEPTH_STENCIL,
		STENCIL,
		FLOAT16,
		FLOAT32,
		UINT16,
		UINT32,
	};

	enum class TextureFilter
	{
		NEAREST,
		LINEAR,
		NEAREST_LINEAR,
		NEAREST_NEAREST,
		LINEAR_NEAREST,
		LINEAR_LINEAR
	};

	enum class TextureWarp
	{
		REPEAT,
		MIRRORED_REPEAT,
		CLAMP_TO_EDGE,
		CLAM_TO_BORDER
	};

	class Texture
	{
	private:
		int width;
		int height;

		GLuint id;

		GLint internalFormat;
		GLenum format;
		GLenum dataType;

		GLint getInternalFormat(TextureType type, int channels);
		GLenum getFormat(TextureType type, int channels, bool invertChannels, bool normalized);

	public:
		Texture();
		~Texture();

		void create(const void* data, int width, int height, int channels, TextureType type, bool invertChannels = false);
		void createStorage(int levels, int w, int h, int channels, GLuint intFormat, TextureType type, bool normalized);

		void update(const void* data);
		void mipmap();
		void setFiltering(GLenum maxfilter, GLenum minfilter);
		void setWarp(TextureWarp warp);

		void read(const void* data);
		void read(const void* data, GLenum format, GLenum dataType);

		void bind();
		void unbind();
		void bindImage(int idx, int layer, GLenum access);
		void bindImage(int idx, int layer, GLenum access, GLenum internalFormat);

		void use(int idx = 0);
		
		int getWidth() const;
		int getHeight() const;
		int getID() const;

		typedef std::shared_ptr<Texture> Ptr;
	};
}