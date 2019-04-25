#include "markerTracking.h"

void MarkerTracker::configGEM()
{
	m_MDetector[m_cameraDevice].loadParamsFromFile("./resources/dodecConfig.yml");

	aruco::Dictionary dict;
	//auto myDict = dict.loadFromFile(m_MDetector.getParameters().dictionary);
	m_gemMarker = new gem::GeometryExtendedMarker(0.036f, dict.loadFromFile(m_MDetector[m_cameraDevice].getParameters().dictionary));
	m_gemMarker->setTrackingRange(0.01, 1.0);
	m_gemMarker->setTrackingDistanceThreshold(0.003);
	m_gemMarker->setTrackingRotationThreshold(3.0);
	m_gemMarker->setBaseSubmarker(0);

}

void MarkerTracker::setCamPams(int camDev, float fx, float fy, float cx, float cy, int width, int height)
{
	cv::Mat camMat = cv::Mat::eye(3,3,CV_32F);
	camMat.at<float>(0, 0) = fx;
	camMat.at<float>(1, 1) = fy;
	camMat.at<float>(0, 2) = cx;
	camMat.at<float>(1, 2) = cy;
	cv::Mat camDist = cv::Mat::zeros(1, 4, CV_32F);

	m_camPams[camDev].setParams(camMat, camDist, cv::Size(width, height));
	double projMat[16];
	m_camPams[0].glGetProjectionMatrix(cv::Size(848, 480), cv::Size(848, 480), projMat, 0.1, 10.0, true);



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
		case gemStatus::PAIRING:
		{
			detectPairs();
			break;
		}
		default: 
			std::cout << "Invalid GEM status selection" << std::endl;
			break;
	}

}

void MarkerTracker::detectPairs()
{
	std::vector<int> pairIDs;
	std::vector<std::pair<aruco::Marker, aruco::Marker>> markerPairs;

	if (m_numberOfCameras > 1)
	{
		for (int i = 0; i < m_numberOfCameras - 1; i++)
		{
			for (int j = 0; j < m_markers[i].size(); j++)
			{
				for (int k = 0; k < m_markers[i + 1].size(); k++)
				{
					if (m_markers[i][j].id == m_markers[i + 1][k].id)
					{
						pairIDs.push_back(m_markers[i][j].id);
						markerPairs.push_back(std::make_pair(m_markers[i][j], m_markers[i + 1][k]));
					}
				}
			}
		}
	}

	if (pairIDs.size() > 0)
	{
		//std::cout << "paired ids : ";
		//for (int i = 0; i < pairIDs.size(); i++)
		//{
		//	std::cout << pairIDs[i] << " ";
		//}
		//std::cout << std::endl;

		std::vector<glm::mat4> cam0ToCam1(pairIDs.size());
		glm::quat cumQuat(0.0f, 0.0f, 0.0f, 0.0f);
		glm::vec4 cumTrans(0.0f, 0.0f, 0.0f, 0.0f);

		float pairSize = markerPairs.size();

		for (int i = 0; i < pairSize; i++)
		{
			double outGL0[16];
			markerPairs[i].first.glGetModelViewMatrix(outGL0);
			glm::mat4 rotMat0 = glm::make_mat4(outGL0);

			double outGL1[16];
			markerPairs[i].second.glGetModelViewMatrix(outGL1);
			glm::mat4 rotMat1 = glm::make_mat4(outGL1);

			cam0ToCam1[i] = rotMat0 * glm::inverse(rotMat1);

			//std::cout << glm::to_string(cam0ToCam1[i]) << " id : " << markerPairs[i].first.id << " : " << markerPairs[i].second.id << " : " << std::endl;
		
			cumQuat += glm::quat_cast(cam0ToCam1[i]);
			cumTrans += cam0ToCam1[i][3];

			//glm::quat currQuat = cumQuat * 1.0f / float(i + 1);
			//std::cout << glm::to_string(currQuat) << std::endl;

			//glm::normalize(currQuat);
			//std::cout << glm::to_string(currQuat) << std::endl;


		}
		cumQuat *= 1.0f / pairSize;
		cumTrans *= 1.0f / pairSize;

		//std::cout << glm::to_string(cumQuat) << std::endl;

		glm::mat4 meanCam2Cam = glm::toMat4(cumQuat);
		meanCam2Cam[3] = cumTrans;

		m_cam2cam = meanCam2Cam;

		//std::cout << glm::to_string(meanCam2Cam) << std::endl;


		m_statusGem = gemStatus::PAIRED;



	}




}

void MarkerTracker::setupAruco()
{
	m_MDetector.resize(m_numberOfCameras);
	m_camPams.resize(m_numberOfCameras);
	m_status.resize(m_numberOfCameras);
	m_markers.resize(m_numberOfCameras);
	m_calibrationSamples.resize(m_numberOfCameras);

	for (int i = 0; i < m_numberOfCameras; i++)
	{
		m_MDetector[i].loadParamsFromFile("./resources/dodecConfig.yml");

		m_status[i] = arucoStatus::STOPPED;
	}
	m_statusGem = gemStatus::STOPPED;
}

void MarkerTracker::collectSamples()
{
	if (!m_markers.empty())
	{
		m_calibrationSamples.push_back(m_markers);
		m_samplingCount++;
		if (m_samplingCount % 10 == 0)
		{
			m_calibrationSamples.push_back(m_markers);
			std::cout << "Captured " << m_samplingCount / 10 << " samples." << std::endl;
		}

	}
}

void MarkerTracker::autoCalibrate()
{

}

void MarkerTracker::calibrate()
{
	if (m_gemMarker->calibrateTransformations(m_calibrationSamples[m_cameraDevice]))
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
	m_poses = m_gemMarker->estimateSubmarkerPoses(m_markers[m_cameraDevice]);
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
	m_gemMarker->saveCalibrations(m_MDetector[m_cameraDevice].getParameters().dictionary + ".calib");
}




void MarkerTracker::startTracking()
{
	for (int i = 0; i < m_numberOfCameras; i++)
	{
		if (m_status[i] == arucoStatus::STOPPED)
		{
			m_status[i] = arucoStatus::TRACKING;
			//m_thread = new std::thread(&MarkerTracker::detect, this);
		}
	}


}

void MarkerTracker::stopTracking()
{
	if (m_status[m_cameraDevice] == arucoStatus::TRACKING)
	{
		m_status[m_cameraDevice] = arucoStatus::STOPPED;

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
	if (m_status[m_cameraDevice] == arucoStatus::TRACKING)
	{
		if (!m_targetMat.empty())
		{
			//std::cout << "inside second loop" << std::endl;
			//m_mtx.lock();

			auto markers = m_MDetector[m_cameraDevice].detect(m_targetMat, m_camPams[m_cameraDevice], 0.036f);
			//std::lock_guard<std::shared_timed_mutex> lk(m_shared_mtx);
			//m_mtx.lock();
			m_markers[m_cameraDevice] = markers;
			
			//m_mtx.unlock();

			//std::cout << "marker size : " << m_markers.size() << std::endl;

		}

	}
	return false;
}

void MarkerTracker::filterOutliers()
{
	m_gemMarker->filterOutliers(m_markers[m_cameraDevice]).swap(m_markers[m_cameraDevice]);
}

void MarkerTracker::getMarkerData(std::vector<glm::mat4> &tMat)
{
	if (m_markers[m_cameraDevice].size() > 0)
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
			tMat.resize(m_markers[m_cameraDevice].size());

			//std::shared_lock<std::shared_timed_mutex> lk(m_shared_mtx);
			//m_mtx.lock();
			auto markers = m_markers[m_cameraDevice];
			for (unsigned int i = 0; i < markers.size(); i++)
			{
				//glm::mat4 rotM = glm::eulerAngleXYZ(-markers[i].Rvec.at<float>(0, 0), -markers[i].Rvec.at<float>(1, 0), markers[i].Rvec.at<float>(2, 0));


				//std::cout << "roddy : " << markers[i].Rvec.at<float>(0, 0) << " " << markers[i].Rvec.at<float>(1, 0) << " " << markers[i].Rvec.at<float>(2, 0) << " " << std::endl;

				glm::vec3 r = glm::vec3(markers[i].Rvec.at<float>(0, 0), markers[i].Rvec.at<float>(1, 0), markers[i].Rvec.at<float>(2, 0));

				float theta = glm::sqrt((r.x*r.x +
										 r.y*r.y +
										 r.z*r.z));

				glm::vec3 R = r / theta;

				glm::quat rotQuat;
				
				rotQuat.w = glm::cos(theta / 2.0);
				rotQuat.x = R.x * glm::sin(theta / 2.0);
				rotQuat.y = R.y * glm::sin(theta / 2.0);
				rotQuat.z = R.z * glm::sin(theta / 2.0);

				glm::mat4 rotMat = glm::toMat4(rotQuat);
				rotMat[0][2] *= -1.0;
				rotMat[1][2] *= -1.0;
				rotMat[2][2] *= -1.0;

				rotMat[3][0] = markers[i].Tvec.at<float>(0, 0);
				rotMat[3][1] = markers[i].Tvec.at<float>(1, 0);
				rotMat[3][2] = -markers[i].Tvec.at<float>(2, 0);


				//double outGL[16];
				//markers[i].glGetModelViewMatrix(outGL);
				//glm::mat4 rrotMat = glm::make_mat4(outGL);
				//rotMat[3][2] *= -1.0f;

				tMat[i] = rotMat;

			}
		}


		//m_mtx.unlock();
		draw();

	}

}

void MarkerTracker::draw()
{
	// this causes it to crash because opencv doesnt like drawing off main thread, i think
	//m_mtx.lock();
	cv::Mat outputImage;
	m_targetMat.copyTo(outputImage);


		for (unsigned int i = 0; i < m_markers[m_cameraDevice].size(); i++)
			aruco::CvDrawingUtils::draw3dCube(outputImage, m_markers[m_cameraDevice][i], m_camPams[m_cameraDevice]);

		cv::namedWindow("detected", cv::WINDOW_NORMAL);
		cv::imshow("detected", outputImage);
		cv::waitKey(1);

	//m_mtx.unlock();
}