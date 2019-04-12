#ifndef INTERF_H
#define INTERF_H

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <thread>

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/rs_advanced_mode.hpp>

#include "opencv2/core/utility.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "device.h"

struct FrameProperties
{
	int width;
	int height;
	int rate;
	int depthUnits;
};

struct FrameIntrinsics
{
	float cx;
	float cy;
	float fx;
	float fy;

	float k1, k2, p1, p2, k3;
};

class Realsense2Interface
{
public:
	Realsense2Interface() {};
	~Realsense2Interface() {};
	int searchForCameras();
	rs2::device_list getDeviceList();
	void setDepthProperties(int devNumber, int w, int h, int r);
	void getDepthProperties(int devNumber, int &w, int &h, int &r);
	void setColorProperties(int devNumber, int w, int h, int r);
	void getColorProperties(int devNumber, int &w, int &h, int &r);

	void startDevice(int devNumber, int depthProfile, int colorProfile);
	void stopDevice(int devNumber);
	bool collateFrames();
	std::vector<rs2::frame_queue> getDepthQueues();
	std::vector<rs2::frame_queue> getColorQueues();
	void getColorFrame(int devNumber, std::vector<uint16_t> &colorArray);
	void getDepthFrame(int devNumber, std::vector<uint16_t> &depthArray);
	FrameIntrinsics getDepthIntrinsics(int devNumber);
	FrameIntrinsics getColorIntrinsics(int devNumber);
	uint32_t getDepthUnit(int devNumber);
	rs2_extrinsics getDepthToColorIntrinsics(int devNumber);

private:
	std::string getDeviceName(const rs2::device &dev);
	void setDepthIntrinsics(int devNumber);
	void setColorIntrinsics(int devNumber);

	rs2::device_list m_devices;

	std::vector<Realsense2Camera> m_cameras;
	std::vector<std::thread> m_threads;
	std::vector<rs2::frame> m_depthFrames;
	std::vector<rs2::frame> m_colorFrames;
	std::vector<FrameProperties> m_depthProps;
	std::vector<FrameProperties> m_colorProps;

	std::vector<FrameIntrinsics> m_depthIntrinsics;
	std::vector<FrameIntrinsics> m_colorIntrinsics;

	std::vector<rs2::frame_queue> m_depthQueues;
	std::vector<rs2::frame_queue> m_colorQueues;


};






#endif

