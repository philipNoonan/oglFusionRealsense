#ifndef DEVICE_H
#define DEVICE_H

#include <stdio.h>
#include <iostream>
#include <fstream>


#include "opencv2/core/utility.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/rs_advanced_mode.hpp>

class Realsense2Camera
{


public:

	enum Status
	{
		CAPTURING,
		STOPPED
	};

	Realsense2Camera() 
	{
		m_status = STOPPED;
	};

	~Realsense2Camera() 
	{
		if (m_status == CAPTURING)
		{
			stop();
		}
	};

	void setDev(rs2::device dev);
	void setDepthProperties(int width, int height, int rate);
	void setColorProperties(int width, int height, int rate);
	bool start();
	bool stop();
	bool getFrames(rs2::frame_queue &depthQ, rs2::frame_queue &colorQ);
	Realsense2Camera::Status getStatus();
	rs2_intrinsics getDepthIntrinsics();
	rs2_intrinsics getColorIntrinsics();
	uint32_t getDepthUnit();
	void capture();

private:

	Status m_status;

	int m_depthHeight;
	int m_depthWidth;
	int m_depthRate;

	float m_depthUnit;

	int m_colorHeight;
	int m_colorWidth;
	int m_colorRate;
	
	std::string m_configFilename;
	bool m_valuesChanged = false;
	uint64_t m_frameArrivalTime = 0;
	int m_temperature = 0;

	STDepthTableControl m_ctrl_curr{};

	rs2::device m_dev;
	rs2::pipeline_profile m_selection;

	rs2::pipeline m_pipe;
	rs2::frameset m_data;
	rs2::frameset m_dataProcessed;

	rs2::frame m_color;
	rs2::frame m_depth;

	rs2::frame_queue m_depthQueue;
	rs2::frame_queue m_colorQueue;




};







#endif