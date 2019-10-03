#pragma once

#include <memory>
#include <GL/glew.h>

namespace gl
{
	enum class Texture3DType
	{
		FLOAT16,
		FLOAT32,
		UINT16,
		UINT32,
	};

	enum class Texture3DFilter
	{
		NEAREST,
		LINEAR
	};

	enum class Texture3DWarp
	{
		REPEAT,
		MIRRORED_REPEAT,
		CLAMP_TO_EDGE,
		CLAM_TO_BORDER
	};

	class Texture3D
	{
	private:
		int width;
		int height;
		int depth;

		GLuint id;

		GLint internalFormat;
		GLenum format;
		GLenum dataType;

		GLint getInternalFormat(Texture3DType type, int channels);
		GLenum getFormat(Texture3DType type, int channels, bool normalized, bool invertChannels);

	public:
		Texture3D();
		~Texture3D();

		void create(const void* data, int width, int height, int depth, int channels, Texture3DType type, bool normalized, bool invertChannels = false);
		void createStorage(int levels, int w, int h, int d, int channels, GLuint internalformat, Texture3DType type, bool normalized);

		void update(const void* data);

		void setFiltering(Texture3DFilter filter);
		void setWarp(Texture3DWarp warp);

		void read(const void* data);
		void read(const void* data, GLenum format, GLenum dataType);

		void clear(int level, const void * data);

		void bind();
		void unbind();
		void bindImage(int idx, int level, GLenum access);
		void bindImage(int idx, int level, GLenum access, GLenum internalFormat);

		void use(int idx = 0);
		
		int getWidth() const;
		int getHeight() const;
		int getDepth() const;
		int getID() const;

		typedef std::shared_ptr<Texture3D> Ptr;
	};
}