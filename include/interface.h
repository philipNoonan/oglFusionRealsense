#pragma once

#define GLUT_NO_LIB_PRAGMA
//##### OpenGL ######
#include <GL/glew.h>
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

#include "glutils.h"
#include "glslprogram.h"

#include "glhelper.h"

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/rs_advanced_mode.hpp>

#include <iostream>
#include <fstream>

#include <thread>
#include <mutex>
#include <vector>

//#include "opencv2/core/utility.hpp"
//#include "opencv2/opencv.hpp"
//#include "opencv2/imgproc/imgproc.hpp"
//#include "opencv2/highgui/highgui.hpp"

class Realsense2Camera
{
	enum Status
	{
		CAPTURING,
		STOPPED
	};

public:
	Realsense2Camera()
		: m_color_frame()
		, m_depth_frame()
		, m_infra_frame()
		, m_rawColor()
		, m_rawBigDepth()
		, m_frame_width(0)
		, m_frame_height(0)
		, m_depth_fx(0)
		, m_depth_fy(0)
		, m_depth_ppx(0)
		, m_depth_ppy(0)
		, m_frames_ready(false)
		, m_thread(nullptr)
		, m_mtx()
		, m_status(STOPPED)
	{
	}

	~Realsense2Camera()
	{
		if (m_status == CAPTURING)
		{
			stop();
		}
	}

	void setPreset(int rate, int res);

	void start();

	void stop();

	bool ready()
	{
		return m_frames_ready;
	}

	std::vector<float> getColorCameraParameters();

	void setFrameHandles(GLuint colorFrameHandle, GLuint depthFrameHandle)
	{
		m_textureColor = colorFrameHandle;
		m_textureDepth = depthFrameHandle;
		//m_infraFrameHandle = colorFrameHandle;
		//m_colorFrameHandle = colorFrameHandle;


	}

	GLuint getColorFrameHandle()
	{
		return m_textureColor;
	}

	GLuint getDepthFramehandle()
	{
		return m_textureDepth;
	}

	void frames(unsigned char * colorArray, float * bigDepthArray);

	void frames(unsigned char * colorArray, std::vector<uint16_t> &depthArray, float * infraredArray, float * bigDepthArray, int * colorDepthMapping);

	//void frames(cv::Mat &color, cv::Mat &depth, cv::Mat &infra, float & fullColor);

	//void frames(cv::Mat &color, cv::Mat &depth, cv::Mat &infra);

	//void frames(cv::Mat &color, cv::Mat &depth);

	//int width()
	//{
	//	return m_frame_width;
	//}

	//int height()
	//{
	//	return m_frame_height;
	//}

	void setDepthFrameHeight(int height)
	{
		m_depthframe_height = height;
	}
	int getDepthFrameHeight()
	{
		return m_depthframe_height;
	}
	void setDepthFrameWidth(int width)
	{
		m_depthframe_width = width;
	}
	int getDepthFrameWidth()
	{
		return m_depthframe_width;
	}
	float getDepthUnit()
	{
		return (float)m_ctrl_curr.depthUnits;
	}
	void setDepthControlGroupValues(int32_t clampMax, int32_t clampMin, uint32_t depthUnits, uint32_t dispMode, uint32_t dispShift)
	{
		//m_ctrl_curr.depthClampMax = clampMax;
		//m_ctrl_curr.depthClampMin = clampMin;
		//m_ctrl_curr.depthUnits = depthUnits;
		//m_ctrl_curr.disparityMode = dispMode;
		m_ctrl_curr.disparityShift = dispShift;
		m_valuesChanged = true;
	}
	int getTemperature()
	{
		return m_temperature;
	}
	float fx()
	{
		return m_depth_fx;
	}

	float fy()
	{
		return m_depth_fy;
	}

	float ppx()
	{
		return m_depth_ppx;
	}

	float ppy()
	{
		return m_depth_ppy;
	}


	float fx_col()
	{
		return m_color_fx;
	}

	float fy_col()
	{
		return m_color_fy;
	}

	float ppx_col()
	{
		return m_color_ppx;
	}

	float ppy_col()
	{
		return m_color_ppy;
	}


private:
	void captureLoop();

	// openGL stuff
	GLuint m_textureColor;
	GLuint m_textureDepth;


	float* m_color_frame;
	uint16_t * m_depth_frame;
	float* m_infra_frame;
	int * m_color_Depth_Map;

	float *m_rawColor;
	float *m_rawBigDepth;

	int m_frame_width;
	int m_frame_height;

	int m_colorframe_width = 1920;
	int m_colorframe_height = 1080;

	int m_depthRate;
	int m_depthframe_width = 640;
	int m_depthframe_height = 480;

	float m_depth_fx;
	float m_depth_fy;
	float m_depth_ppx;
	float m_depth_ppy;

	float m_color_fx;
	float m_color_fy;
	float m_color_ppx;
	float m_color_ppy;

	float m_depth_k1;
	float m_depth_k2;
	float m_depth_k3;
	float m_depth_p1;
	float m_depth_p2;

	int m_temperature = 0;

	Status m_status;
	bool m_frames_ready;
	std::thread *m_thread;
	std::mutex m_mtx;
	//libfreenect2::Freenect2Device::ColorCameraParams m_colorCamPams;

	bool m_valuesChanged = false;
	STDepthTableControl m_ctrl_curr{};

	rs2::pipeline_profile selection;
	rs2::device dev;

	rs2::pipeline pipe;
	rs2::frameset data;


};
