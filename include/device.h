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
	void setStreams();
	void setDepthTable(STDepthTableControl dTable);
	void setSensorOptions();
	void setEmitterOptions(float status, float power);
	void setDepthProperties(int profileChoice);
	void getDepthProperties(int &width, int &height, int &rate);

	void setColorProperties(int profileChoice);
	void getColorProperties(int &width, int &height, int &rate);

	int readTemperature();

	bool start();
	bool startFromFile();
	bool stop();
	bool getFrames(rs2::frame_queue &depthQ, rs2::frame_queue &colorQ, rs2::frame_queue &infraQ);
	bool getFramesFromFile(rs2::frame_queue &depthQ, rs2::frame_queue &colorQ);

	Realsense2Camera::Status getStatus();
	rs2_intrinsics getDepthIntrinsics();
	rs2_intrinsics getColorIntrinsics();
	uint32_t getDepthUnit();
	rs2_extrinsics getDepthToColorExtrinsics();
	rs2_extrinsics getColorToDepthExtrinsics();

	void capture();
	void colorThread(rs2::sensor& sens);
	void capturingColor(rs2::frame &f);

	void depthThread(rs2::sensor& sens);
	void capturingDepth(rs2::frame &f);

	void infraThread(rs2::sensor& sens);
	void capturingInfra(rs2::frame &f);

private:

	Status m_status;

	int m_depthHeight;
	int m_depthWidth;
	int m_depthRate;


	int m_colorHeight;
	int m_colorWidth;
	int m_colorRate;
	
	std::string m_configFilename;
	bool m_valuesChanged = false;
	uint64_t m_frameArrivalTime = 0;
	int m_temperature;

	STDepthTableControl m_ctrl_curr{};
	uint32_t m_depthUnit;

	rs2::device m_dev;
	rs2::pipeline_profile m_selection;

	rs2::pipeline m_pipe;
	rs2::frameset m_data;
	rs2::frameset m_dataProcessed;

	rs2::frame m_color;
	rs2::frame m_depth;

	rs2::frame_queue m_depthQueue;
	rs2::frame_queue m_colorQueue;
	rs2::frame_queue m_infraQueue;

	rs2::sensor m_depthSensor;
	rs2::sensor m_colorSensor;

	int m_depthStreamChoice; //435I
	//int m_depthStreamChoice = 211; // 415
	//int m_colorStreamChoice = 61; //435I 848 480 60hz rgb8
	int m_colorStreamChoice; //435I 848 480 30hz rgb8

	std::vector<rs2::sensor> m_sensors;


	std::vector<rs2::stream_profile> m_stream_profiles_depthIR;
	std::vector<rs2::stream_profile> m_stream_profiles_color;

	//std::vector<FrameGrabber> m_grabbers;

};







#endif