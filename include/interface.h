#ifndef INTERF_H
#define INTERF_H

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <thread>

#include <glm/glm.hpp>

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/rs_advanced_mode.hpp>
#include<librealsense2/rsutil.h>

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
	void getDepthPropertiesFromFile(int &w, int &h, int &r);

	void setColorProperties(int devNumber, int w, int h, int r);
	void getColorProperties(int devNumber, int &w, int &h, int &r);
	void getColorPropertiesFromFile(int &w, int &h, int &r);

	void setDepthTable(int devNumber, int depthMax, int depthMin, int depthUnit, int disparityMode, int disparity);

	void startDevice(int devNumber, int depthProfile, int colorProfile);
	void startDeviceFromFile(std::string filename, int depthProfile, int colorProfile);
	void stopDevice(int devNumber);
	bool collateFrames();
	bool collateFramesFromFile();

	void setEmitterOptions(int devNumber, bool status, float power);


	std::vector<rs2::frame_queue> getDepthQueues();
	std::vector<rs2::frame_queue> getColorQueues();
	std::vector<rs2::frame_queue> getInfraQueues();

	void getColorFrame(int devNumber, std::vector<uint16_t> &colorArray);
	void getDepthFrame(int devNumber, std::vector<uint16_t> &depthArray);
	FrameIntrinsics getDepthIntrinsics(int devNumber);
	FrameIntrinsics getColorIntrinsics(int devNumber);
	uint32_t getDepthUnit(int devNumber);
	float getDepthUnitFromFile();

	//rs2_extrinsics getDepthToColorExtrinsics(int devNumber);
	glm::mat4 getDepthToColorExtrinsics(int devNumber);
	glm::mat4 getColorToDepthExtrinsics(int devNumber);


private:
	std::string getDeviceName(const rs2::device &dev);
	void setDepthIntrinsics(int devNumber);
	void setColorIntrinsics(int devNumber);

	rs2::device_list m_devices;
	rs2::pipeline m_pipe;
	float m_depthUnitFromFile;

	std::vector<Realsense2Camera> m_cameras;
	std::vector<std::thread> m_threads;
	std::vector<rs2::frame> m_depthFrames;
	std::vector<rs2::frame> m_colorFrames;
	std::vector<rs2::frame> m_infraFrames;

	std::vector<STDepthTableControl> m_depthTables;

	std::vector<FrameProperties> m_depthProps;
	std::vector<FrameProperties> m_colorProps;

	std::vector<FrameIntrinsics> m_depthIntrinsics;
	std::vector<FrameIntrinsics> m_colorIntrinsics;

	std::vector<rs2::frame_queue> m_depthQueues;
	std::vector<rs2::frame_queue> m_colorQueues;
	std::vector<rs2::frame_queue> m_infraQueues;


};






#endif

