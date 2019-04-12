#ifndef MARKER_H
#define MARKER_H
#define GLM_ENABLE_EXPERIMENTAL

#include "aruco.h"
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/matrix_decompose.hpp>
//#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <thread>
#include <mutex>
#include <shared_mutex>

#include "gem.h"

namespace arucoStatus
{
	enum Status
	{
		TRACKING,
		STOPPED
	};
}

namespace gemStatus
{
	enum Status
	{
		STOPPED = 0,
		COLLECTING = 1,
		AUTOCALIBRATING = 2,
		TRACKING = 3,
	};
}

class MarkerTracker
{
public:
	MarkerTracker() 
	{
		m_MDetector.loadParamsFromFile("./resources/dodecConfig.yml");
		
		//aruco::Dictionary myDict;
		//myDict.loadFromFile(m_MDetector.getParameters().dictionary);

		//std::cout << "dictionary : " << std::endl;
		//std::cout << m_MDetector.getParameters().dictionary << std::endl;

		m_status = arucoStatus::STOPPED;
		m_statusGem = gemStatus::STOPPED;
	};
	~MarkerTracker() 
	{
		if (m_status == arucoStatus::TRACKING)
		{
			stopTracking();
		}
	};

	bool detect();
	// GEM
	void configGEM();
	void setGemOption(int opt);
	void useGEM();
	void filterOutliers();

	void getMarkerData(std::vector<glm::mat4> &tMat);

	void draw();
	void setCamPams(float fx, float fy, float cx, float cy, int width, int height);
	void setMat(cv::Mat);
	void startTracking();
	void stopTracking();
	void calibrate();
	void clearCalibration();

	void exportCalibration();


private:

	void collectSamples();
	void autoCalibrate();
	void trackGEM();

	aruco::MarkerDetector m_MDetector;
	aruco::CameraParameters m_camPams;

	cv::Mat m_targetMat;
	std::thread *m_thread;
	std::mutex m_mtx;
	std::shared_timed_mutex m_shared_mtx;

	arucoStatus::Status m_status;
	std::vector<aruco::Marker> m_markers;

	gem::GeometryExtendedMarker *m_gemMarker = NULL;
	std::vector<std::vector<aruco::Marker>> calibrationSamples;
	int m_samplingCount = 0;
	gemStatus::Status m_statusGem;
	std::map<int, std::pair<glm::dvec3, glm::dquat>> m_poses;
	bool m_globalPoseTracked = false;
	glm::dvec3 m_globalPos;
	glm::dquat m_globalOri;


};




















#endif