#ifndef MARKER_H
#define MARKER_H

#include "aruco.h"
#include <iostream>
#include <opencv2/highgui/highgui.hpp>


class MarkerTracker
{
public:
	MarkerTracker() 
	{
		m_MDetector.loadParamsFromFile("./resources/dodecConfig.yml");

	};
	~MarkerTracker() {};

	bool detect(cv::Mat colMat);
	void setCamPams(float fx, float fy, float cx, float cy);

private:
	aruco::MarkerDetector m_MDetector;
	aruco::CameraParameters m_camPams;



};




















#endif