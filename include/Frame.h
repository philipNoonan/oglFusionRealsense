#pragma once

#include <iostream>
#include <map>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <librealsense2/rs.hpp>


#include "GLCore/Texture.h"
#include "ComputeShader.h"
#include "BilateralFilter.h"
#include "CalcVertexMap.h"
#include "CalcNormalMap.h"
#include "CalcNormalMap.h"
#include "DownSampling.h"
#include "AlignDepthColor.h"

namespace rgbd
{
	enum FRAME
	{
		VIRTUAL, CURRENT, GLOBAL
	};

	struct FrameData
	{
		gl::Texture::Ptr colorMap;
		gl::Texture::Ptr colorAlignedToDepthMap;
		gl::Texture::Ptr depthMap;
		gl::Texture::Ptr vertexMap;
		gl::Texture::Ptr normalMap;

		typedef std::shared_ptr<rgbd::FrameData> Ptr;
	};

	class Frame
	{
	private:
		int width;
		int height;

		gl::Texture::Ptr shortDepthMap;
		gl::Texture::Ptr rawDepthMap;
		gl::Texture::Ptr trackMap;
		gl::Texture::Ptr testMap;
		gl::Texture::Ptr infraMap;

		std::vector<rgbd::FrameData> frameData;

		rgbd::ComputeShader::Ptr bilateralFilter;
		rgbd::ComputeShader::Ptr alignDC;
		rgbd::ComputeShader::Ptr vertexMapProc;
		rgbd::ComputeShader::Ptr normalMapProc;
		std::vector<rgbd::ComputeShader::Ptr> downSampling;

		glm::mat4 K;

	public:
		Frame();
		~Frame();

		void create(
			int width,
			int height,
			int maxLevel,
			float minDepth,
			float maxDepth,
			const glm::mat4 K,
			float depthScale,
			std::map<std::string, const gl::Shader::Ptr> &progs
		);

		// for the current frame
		void update(
			std::vector<rs2::frame_queue> colorQ,
			std::vector<rs2::frame_queue> depthQ,
			std::vector<rs2::frame_queue> infraQ,
			int numberOfCameras,
			float depthScale,
			const glm::ivec2 pixel,
			glm::vec3 &vertex,
			float bfSigma = 15.0f,
			float bfDSigma = 0.05f
		) const;




		void update(
			const void *colorData,
			const void *depthData,
			float bfSigma = 15.0f,
			float bfDSigma = 0.05f
		) const;
		// for the synthetic frame
		void update() const;

		void alignDepthTocolor(
			glm::mat4 extrins, 
			glm::vec4 depthIntrins, 
			glm::vec4 colorIntrins
		);

		void clearAll();

		int getWidth(int lv = 0) const;
		int getHeight(int lv = 0) const;

		gl::Texture::Ptr getColorMap(int lv = 0) const;
		gl::Texture::Ptr getColorAlignedToDepthMap(int lv = 0) const;

		gl::Texture::Ptr getDepthMap(int lv = 0) const;
		gl::Texture::Ptr getVertexMap(int lv = 0) const;
		gl::Texture::Ptr getNormalMap(int lv = 0) const;
		gl::Texture::Ptr getTrackMap() const;
		gl::Texture::Ptr getTestMap() const;
		gl::Texture::Ptr getInfraMap() const;

		typedef std::shared_ptr<rgbd::Frame> Ptr;
	};
}