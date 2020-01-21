#ifndef MARKER_H
#define MARKER_H
#define GLM_ENABLE_EXPERIMENTAL

//#include "aruco.h"
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
//#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <thread>
#include <mutex>
#include <shared_mutex>

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API

//#include "gem.h"

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
		PAIRING = 4,
		PAIRED = 5,
	};
}

class MarkerTracker
{
public:
	MarkerTracker() 
	{
		
		//aruco::Dictionary myDict;
		//myDict.loadFromFile(m_MDetector.getParameters().dictionary);

		//std::cout << "dictionary : " << std::endl;
		//std::cout << m_MDetector.getParameters().dictionary << std::endl;

	};
	~MarkerTracker() 
	{

		//if (m_status[m_cameraDevice] == arucoStatus::TRACKING)
		//{
		//	stopTracking();
		//}
	};

	bool detect();
	// GEM
	void configGEM();
	void setGemOption(int opt);
	void useGEM();
	void filterOutliers();

	void detectPairs();
	void detectExtrinsicFromFractal();

	void setStereoPair(cv::Mat image0, cv::Mat image1);
	void stereoCalibrate(glm::mat4 &cam2cam);

	void setNumberOfCameras(int camNum)
	{
		m_numberOfCameras = camNum;
	}
	void setCameraDevice(int camDev)
	{
		m_cameraDevice = camDev;
	}
	void setupAruco();
	void getMarkerData(std::vector<glm::mat4> &tMat);

	void draw();
	void setCamPams(int camDev, float fx, float fy, float cx, float cy, int width, int height);
	void setMats(std::vector<rs2::frame_queue> fQueue);
	void startTracking();
	void stopTracking();
	void calibrate();
	void clearCalibration();

	void exportCalibration();

	gemStatus::Status getGemStatus()
	{
		return m_statusGem;
	}

	glm::mat4 getCam2CamTransform()
	{
		return m_cam2cam;
	}
	bool isPaired()
	{
		return m_pairStatus;
	}
private:

	void collectSamples();
	void autoCalibrate();
	void trackGEM();

	glm::mat4 getMatrixFromMarker(aruco::Marker marker);
	glm::mat4 getMatrixFromFractal(int devNumber);

	int m_cameraDevice;
	int m_numberOfCameras;

	std::vector<aruco::MarkerDetector> m_MDetector;
	std::vector<aruco::CameraParameters> m_camPams;

	std::vector<cv::Mat> m_targetMat;
	//std::thread *m_thread;
	//std::mutex m_mtx;
	//std::shared_timed_mutex m_shared_mtx;

	std::vector<arucoStatus::Status> m_status;
	std::vector<std::vector<aruco::Marker>> m_markers;

	std::vector<aruco::FractalDetector> FDetector;


	//gem::GeometryExtendedMarker *m_gemMarker = NULL;
	std::vector<std::vector<std::vector<aruco::Marker>>> m_calibrationSamples;
	int m_samplingCount = 0;
	gemStatus::Status m_statusGem;
	std::map<int, std::pair<glm::dvec3, glm::dquat>> m_poses;
	bool m_globalPoseTracked = false;
	glm::dvec3 m_globalPos;
	glm::dquat m_globalOri;

	glm::mat4 m_cam2cam;

	std::vector<std::pair<cv::Mat, cv::Mat>> m_stereoImages;
	bool m_pairStatus = false;

};




















#endif