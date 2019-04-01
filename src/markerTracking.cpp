#include "markerTracking.h"

void MarkerTracker::setCamPams(float fx, float fy, float cx, float cy)
{
	cv::Mat camMat = cv::Mat::eye(3,3,CV_32F);
	camMat.at<float>(0, 0) = fx;
	camMat.at<float>(1, 1) = fy;
	camMat.at<float>(0, 2) = cx;
	camMat.at<float>(1, 2) = cy;
	cv::Mat camDist = cv::Mat::zeros(1, 4, CV_32F);

	m_camPams.setParams(camMat, camDist, cv::Size(848, 480));
}

bool MarkerTracker::detect(cv::Mat colMat)
{

	cv::Mat greyIm(480, 848, CV_8UC3);
	colMat.copyTo(greyIm);
	//cv::cvtColor(colMat, greyIm, cv::COLOR_RGBA2BGR, 3);
	std::vector<aruco::Marker> markers = m_MDetector.detect(colMat, m_camPams, 50.0f);
	//for (size_t i = 0; i < markers.size(); i++) {
	//	markers[i].draw(greyIm);
	//}

	//for (unsigned int i = 0; i < markers.size(); i++)
	//	aruco::CvDrawingUtils::draw3dCube(greyIm, markers[i], m_camPams);

	//cv::namedWindow("detected", cv::WINDOW_NORMAL);
	//cv::imshow("detected", greyIm);
	//cv::waitKey(1);
	return false;
}