#include "markerTracking.h"

void MarkerTracker::configGEM()
{
	aruco::Dictionary dict;
	//auto myDict = dict.loadFromFile(m_MDetector.getParameters().dictionary);
	m_gemMarker = new gem::GeometryExtendedMarker(0.05f, dict.loadFromFile(m_MDetector.getParameters().dictionary));
	m_gemMarker->setTrackingRange(0.01, 1.0);
	m_gemMarker->setTrackingDistanceThreshold(0.003);
	m_gemMarker->setTrackingRotationThreshold(3.0);
	m_gemMarker->setBaseSubmarker(0);

}

void MarkerTracker::setCamPams(float fx, float fy, float cx, float cy, int width, int height)
{
	cv::Mat camMat = cv::Mat::eye(3,3,CV_32F);
	camMat.at<float>(0, 0) = fx;
	camMat.at<float>(1, 1) = fy;
	camMat.at<float>(0, 2) = cx;
	camMat.at<float>(1, 2) = cy;
	cv::Mat camDist = cv::Mat::zeros(1, 4, CV_32F);

	m_camPams.setParams(camMat, camDist, cv::Size(width, height));
	//double projMat[16];
	//m_camPams.glGetProjectionMatrix(cv::Size(1920, 1080), cv::Size(720, 592), projMat, 0.1, 10000.0, false);
}

void MarkerTracker::setMat(cv::Mat input)
{
	m_targetMat = input;
}

void MarkerTracker::setGemOption(int option)
{
	m_statusGem = static_cast<gemStatus::Status>(option);
}

void MarkerTracker::useGEM()
{
	switch (m_statusGem)
	{
		case gemStatus::COLLECTING:
		{
			collectSamples();
			break;
		}
		case gemStatus::AUTOCALIBRATING:
		{
			autoCalibrate();
			break;
		}
		case gemStatus::TRACKING:
		{
			trackGEM();
			break;
		}
		case gemStatus::STOPPED:
		{
			break;
		}
		default: 
			std::cout << "Invalid GEM status selection" << std::endl;
			break;
	}

}

void MarkerTracker::detectPairs()
{






}



void MarkerTracker::collectSamples()
{
	if (!m_markers.empty())
	{
		calibrationSamples.push_back(m_markers);
		m_samplingCount++;
		if (m_samplingCount % 10 == 0)
		{
			calibrationSamples.push_back(m_markers);
			std::cout << "Captured " << m_samplingCount / 10 << " samples." << std::endl;
		}

	}
}

void MarkerTracker::autoCalibrate()
{

}

void MarkerTracker::calibrate()
{
	if (m_gemMarker->calibrateTransformations(calibrationSamples))
	{
		m_gemMarker->printTransformationSummary();
	}
	else
	{
		std::cout << "Calibration failed." << std::endl;
	}
}

void MarkerTracker::clearCalibration()
{

}

void MarkerTracker::trackGEM()
{
	std::map<int, std::pair<glm::dvec3, glm::dquat>> empty;
	empty.swap(m_poses);
	m_poses = m_gemMarker->estimateSubmarkerPoses(m_markers);
	m_globalPoseTracked = m_gemMarker->estimateGlobalPose(m_globalPos, m_globalOri, m_poses);
	//std::cout << "size of detected markers : " << m_markers.size() << " : size of submarkers : " << m_poses.size() << std::endl;
	//std::cout << "global tracked : " << m_globalPoseTracked << std::endl;
	//for (map<int, pair<glm::dvec3, glm::dquat>>::iterator pose = m_poses.begin(); pose != m_poses.end(); pose++)
	//{
	//	glm::dmat4 modelview = gem::glMatFromVecQuat(pose->second.first, pose->second.second);
	//}



}


void MarkerTracker::exportCalibration()
{
	m_gemMarker->saveCalibrations(m_MDetector.getParameters().dictionary + ".calib");
}




void MarkerTracker::startTracking()
{
	if (m_status == arucoStatus::STOPPED)
	{
		m_status = arucoStatus::TRACKING;
		//m_thread = new std::thread(&MarkerTracker::detect, this);
	}
}

void MarkerTracker::stopTracking()
{
	if (m_status == arucoStatus::TRACKING)
	{
		m_status = arucoStatus::STOPPED;

		//if (m_thread->joinable())
		//{
		//	m_thread->join();
		//}

		//m_thread = nullptr;
	}
}


bool MarkerTracker::detect()
{

	//cv::Mat greyIm;// (480, 848, CV_8UC3);
	//colMat.copyTo(greyIm);
	//cv::cvtColor(colMat, greyIm, cv::COLOR_RGBA2BGR, 3);
	if (m_status == arucoStatus::TRACKING)
	{
		if (!m_targetMat.empty())
		{
			//std::cout << "inside second loop" << std::endl;
			//m_mtx.lock();

			auto markers = m_MDetector.detect(m_targetMat, m_camPams, 0.05f);
			//std::lock_guard<std::shared_timed_mutex> lk(m_shared_mtx);
			//m_mtx.lock();
			m_markers = markers;
			
			//m_mtx.unlock();

			//std::cout << "marker size : " << m_markers.size() << std::endl;

		}
	}
	return false;
}

void MarkerTracker::filterOutliers()
{
	m_gemMarker->filterOutliers(m_markers).swap(m_markers);
}

void MarkerTracker::getMarkerData(std::vector<glm::mat4> &tMat)
{
	if (m_markers.size() > 0)
	{
		if (m_statusGem == gemStatus::TRACKING)
		{
			//m_mtx.lock();
//::cout << "m size " << m_markers.size() << std::endl;
			tMat.resize(m_poses.size());

			//std::shared_lock<std::shared_timed_mutex> lk(m_shared_mtx);
			//m_mtx.lock();
			int i = 0;
			for (std::map<int, std::pair<glm::dvec3, glm::dquat>>::iterator pose = m_poses.begin(); pose != m_poses.end(); pose++, i++)
			{
				tMat[i] = gem::glMatFromVecQuat(pose->second.first, pose->second.second);
			}


			
		}
		else
		{
			//m_mtx.lock();
//::cout << "m size " << m_markers.size() << std::endl;
			tMat.resize(m_markers.size());

			//std::shared_lock<std::shared_timed_mutex> lk(m_shared_mtx);
			//m_mtx.lock();
			auto markers = m_markers;
			for (unsigned int i = 0; i < m_markers.size(); i++)
			{
				double outGL[16];
				markers[i].glGetModelViewMatrix(outGL);
				glm::mat4 rotMat = glm::make_mat4(outGL);

				tMat[i] = rotMat;

			}
		}


		//m_mtx.unlock();

	}

}

void MarkerTracker::draw()
{
	// this causes it to crash because opencv doesnt like drawing off main thread, i think
	m_mtx.lock();
	cv::Mat outputImage;
	m_targetMat.copyTo(outputImage);


		for (unsigned int i = 0; i < m_markers.size(); i++)
			aruco::CvDrawingUtils::draw3dCube(outputImage, m_markers[i], m_camPams);

		cv::namedWindow("detected", cv::WINDOW_NORMAL);
		cv::imshow("detected", outputImage);
		cv::waitKey(1);

	m_mtx.unlock();
}