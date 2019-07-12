#ifndef OP_WRAPPER_H
#define OP_WRAPPER_H

#include <vector>
#include <thread>
#include <chrono>
#include <mutex>

// OpenPose dependencies
#include <openpose/headers.hpp>

#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"


class OPWrapper
{

	enum Status
	{
		STOPPED = 0,
		CAPTURING = 1,
	};
public:
	OPWrapper() {};
	~OPWrapper() {};

	void start();
	void setImage(cv::Mat image, int frameNumber);
	void stop();

	bool isNewData()
	{
		return m_newData;
	}
	void setDataRead()
	{
		m_newData = false;
	}
	int getOPFrameNumber()
	{
		return m_opFrameNumber;
	}
	void getPoses(cv::Mat &poses, cv::Mat &faces, std::vector<int> &poseIds)
	{
		poses = m_detectedKeyPointsPose;
		faces = m_detectedKeyPointsFace;
		poseIds = m_detectedPoseIds;
	}


private:
	void capturingLoop();

	Status m_status = STOPPED;
	std::thread m_thread;
	std::mutex m_mtx;
	cv::Mat m_inputImage;
	cv::Mat m_detectedKeyPointsPose;
	cv::Mat m_detectedKeyPointsFace;
	std::vector<int> m_detectedPoseIds;

	int m_opFrameNumber;
	int m_temp_opFrameNumber;
	bool m_newData = false;
};



#endif // OP_WRAPPER_H