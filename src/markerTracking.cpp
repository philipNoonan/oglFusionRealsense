#include "markerTracking.h"

void MarkerTracker::configGEM()
{
	//m_MDetector[m_cameraDevice].loadParamsFromFile("./resources/dodecConfig.yml");

	//aruco::Dictionary dict;
	////auto myDict = dict.loadFromFile(m_MDetector.getParameters().dictionary);
	//m_gemMarker = new gem::GeometryExtendedMarker(0.036f, dict.loadFromFile(m_MDetector[m_cameraDevice].getParameters().dictionary));
	//m_gemMarker->setTrackingRange(0.01, 1.0);
	//m_gemMarker->setTrackingDistanceThreshold(0.003);
	//m_gemMarker->setTrackingRotationThreshold(3.0);
	//m_gemMarker->setBaseSubmarker(0);

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
	//double projMat[16];
	//m_camPams[0].glGetProjectionMatrix(cv::Size(848, 480), cv::Size(848, 480), projMat, 0.1, 10.0, true);
	FDetector[camDev].setConfiguration("FRACTAL_3L_6");

	FDetector[camDev].setParams(m_camPams[camDev], 0.112f);

	//FDetector[camDev].conver

}

void MarkerTracker::setStereoPair(cv::Mat image0, cv::Mat image1)
{
	cv::Mat im0, im1;
	image0.copyTo(im0);
	image1.copyTo(im1);

	m_stereoImages.push_back(std::make_pair(im0, im1));
}

void MarkerTracker::stereoCalibrate(glm::mat4 &cam2cam)
{
	cv::Size boardSize(6, 9);
	float squareSize = 0.026f;

	std::pair<std::vector<std::vector<cv::Point2f>>, std::vector<std::vector<cv::Point2f>>> imagePoints; //[camera][image]

	std::vector<std::vector<cv::Point3f> > objectPoints;

	int nimages = m_stereoImages.size();

	imagePoints.first.resize(nimages);
	imagePoints.second.resize(nimages);

	std::vector<int> goodImageList, badImageList;

	for (int i = 0; i < nimages; i++)
	{
		bool found0 = cv::findChessboardCorners(m_stereoImages[i].first, boardSize, imagePoints.first[i], cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE);
		if (found0)
		{
			cv::cornerSubPix(m_stereoImages[i].first, imagePoints.first[i], cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 30, 0.01));
		}
		else
		{
			cv::imshow("bad0", m_stereoImages[i].first);
			cv::waitKey(0);

		}
		bool found1 = cv::findChessboardCorners(m_stereoImages[i].second, boardSize, imagePoints.second[i], cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE);
		if (found1)
		{
			cv::cornerSubPix(m_stereoImages[i].second, imagePoints.second[i], cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 30, 0.01));
		}
		else
		{
			cv::imshow("bad1", m_stereoImages[i].second);
			cv::waitKey(0);

		}

		if (found0 && found1)
		{
			goodImageList.push_back(i);
		}
		else
		{
			badImageList.push_back(i);
		}

	}

	std::cout << goodImageList.size() << " : good image pairs found out of : " << nimages << std::endl;

	for (int i = badImageList.size() - 1; i >= 0; i--)
	{
		imagePoints.first.erase(imagePoints.first.begin() + badImageList[i]);
		imagePoints.second.erase(imagePoints.second.begin() + badImageList[i]);

	}
	objectPoints.resize(goodImageList.size());

	for (int i = 0; i < goodImageList.size(); i++)
	{
		for (int j = 0; j < boardSize.height; j++)
			for (int k = 0; k < boardSize.width; k++)
				objectPoints[i].push_back(cv::Point3f(k*squareSize, j*squareSize, 0));
	}

	cv::Mat R, T, E, F;

	double rms = cv::stereoCalibrate(objectPoints, imagePoints.first, imagePoints.second,
		m_camPams[0].CameraMatrix, m_camPams[0].Distorsion,
		m_camPams[1].CameraMatrix, m_camPams[1].Distorsion,
		cv::Size(1, 1), //this shouldnt matter
		R, T, E, F,
		cv::CALIB_FIX_INTRINSIC,
		cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 100, 1e-5));
											
	std::cout << "Finished calibration with rms : " << rms << std::endl;

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			cam2cam[i][j] = R.at<double>(i, j);
		}
	}

	cam2cam[3][0] = T.at<double>(0, 0);
	cam2cam[3][1] = T.at<double>(1, 0);
	cam2cam[3][2] = T.at<double>(2, 0);


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
			//detectPairs();
			detectExtrinsicFromFractal();
			break;
		}
		default: 
			std::cout << "Invalid GEM status selection" << std::endl;
			break;
	}

}

glm::mat4 MarkerTracker::getMatrixFromFractal(int devNumber)
{
	cv::Mat rvec = FDetector[devNumber].getRvec();
	cv::Mat tvec = FDetector[devNumber].getTvec();

	glm::vec3 r = glm::vec3(rvec.at<double>(0, 0), rvec.at<double>(1, 0), rvec.at<double>(2, 0));
	float theta = glm::sqrt((r.x*r.x +
		r.y*r.y +
		r.z*r.z));

	glm::vec3 R = r / theta;

	float lenR = glm::length(R);

	glm::quat rotQuat;

	rotQuat.w = glm::cos(theta / 2.0);
	rotQuat.x = R.x * glm::sin(theta / 2.0);
	rotQuat.y = R.y * glm::sin(theta / 2.0);
	rotQuat.z = R.z * glm::sin(theta / 2.0);

	glm::mat4 rotMat = glm::toMat4(rotQuat);

	rotMat[3][0] = tvec.at<double>(0, 0) *  (0.112 / 2.0);
	rotMat[3][1] = tvec.at<double>(1, 0) *  (0.112 / 2.0);
	rotMat[3][2] = tvec.at<double>(2, 0) *  (0.112 / 2.0);

	return rotMat;
}

glm::mat4 MarkerTracker::getMatrixFromMarker(aruco::Marker marker)
{
	glm::vec3 r = glm::vec3(marker.Rvec.at<float>(0, 0), marker.Rvec.at<float>(1, 0), marker.Rvec.at<float>(2, 0));

	float theta = glm::sqrt((r.x*r.x +
		r.y*r.y +
		r.z*r.z));

	glm::vec3 R = r / theta;

	float lenR = glm::length(R);

	//if (lenR != 1.0f)
	//{
	//	std::cout << "axis angle not normalised with length : " << lenR << std::endl;
	//}

	glm::quat rotQuat;

	rotQuat.w = glm::cos(theta / 2.0);
	rotQuat.x = R.x * glm::sin(theta / 2.0);
	rotQuat.y = R.y * glm::sin(theta / 2.0);
	rotQuat.z = R.z * glm::sin(theta / 2.0);

	glm::mat4 rotMat = glm::toMat4(rotQuat);

	rotMat[3][0] = marker.Tvec.at<float>(0, 0);
	rotMat[3][1] = marker.Tvec.at<float>(1, 0);
	rotMat[3][2] = marker.Tvec.at<float>(2, 0);

	return rotMat;
}

void MarkerTracker::detectExtrinsicFromFractal()
{
	glm::mat4 rotMat0 = getMatrixFromFractal(0);
	glm::mat4 rotMat1 = getMatrixFromFractal(1);

	m_cam2cam = rotMat0 * glm::inverse(rotMat1);

	

	std::cout << "paired : " << glm::to_string(m_cam2cam) << std::endl;
	//std::cout << "inverted : " << glm::to_string(glm::inverse(meanCam2Cam)) << std::endl;


	m_statusGem = gemStatus::PAIRED;

	m_pairStatus = true;

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

			glm::mat4 rotMat0 = getMatrixFromMarker(markerPairs[i].first);
			glm::mat4 rotMat1 = getMatrixFromMarker(markerPairs[i].second);

			//glm::mat4 rotMat0 = glm::make_mat4(outGL0);

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

		meanCam2Cam[3] = cumTrans; // a negative here makes the marker pairs output match the stereo calibrate output

		m_cam2cam = meanCam2Cam;

		std::cout << "paired : " << glm::to_string(meanCam2Cam) << std::endl;
		//std::cout << "inverted : " << glm::to_string(glm::inverse(meanCam2Cam)) << std::endl;


		m_statusGem = gemStatus::PAIRED;

		m_pairStatus = true;

	}




}

void MarkerTracker::setupAruco()
{
	m_MDetector.resize(m_numberOfCameras);
	m_camPams.resize(m_numberOfCameras);
	m_status.resize(m_numberOfCameras);
	m_markers.resize(m_numberOfCameras);
	m_calibrationSamples.resize(m_numberOfCameras);
	FDetector.resize(m_numberOfCameras);

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
	/*if (m_gemMarker->calibrateTransformations(m_calibrationSamples[m_cameraDevice]))
	{
		m_gemMarker->printTransformationSummary();
	}
	else
	{
		std::cout << "Calibration failed." << std::endl;
	}*/
}

void MarkerTracker::clearCalibration()
{

}

void MarkerTracker::trackGEM()
{
	//std::map<int, std::pair<glm::dvec3, glm::dquat>> empty;
	//empty.swap(m_poses);
	//m_poses = m_gemMarker->estimateSubmarkerPoses(m_markers[m_cameraDevice]);
	//m_globalPoseTracked = m_gemMarker->estimateGlobalPose(m_globalPos, m_globalOri, m_poses);
	////std::cout << "size of detected markers : " << m_markers.size() << " : size of submarkers : " << m_poses.size() << std::endl;
	////std::cout << "global tracked : " << m_globalPoseTracked << std::endl;
	////for (map<int, pair<glm::dvec3, glm::dquat>>::iterator pose = m_poses.begin(); pose != m_poses.end(); pose++)
	////{
	////	glm::dmat4 modelview = gem::glMatFromVecQuat(pose->second.first, pose->second.second);
	////}



}


void MarkerTracker::exportCalibration()
{
	//m_gemMarker->saveCalibrations(m_MDetector[m_cameraDevice].getParameters().dictionary + ".calib");
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

			cv::Mat outMat;
			m_targetMat.copyTo(outMat);


			if (FDetector[m_cameraDevice].detect(outMat))
			{
				FDetector[m_cameraDevice].drawMarkers(outMat);
			}
			if (FDetector[m_cameraDevice].poseEstimation()) {

				FDetector[m_cameraDevice].draw3d(outMat); //3d
				cv::Mat tvec = FDetector[m_cameraDevice].getTvec();

				//std::cout << tvec.at<double>(2, 0) * (0.112 / 2.0) << std::endl;
			}
			else
				FDetector[m_cameraDevice].draw2d(outMat); //Ok, show me at least the inner corners!

			cv::imshow("FD", outMat);
			cv::waitKey(1);

			//m_markers[m_cameraDevice] = FDetector.getMarkers();

			









			////std::cout << "inside second loop" << std::endl;
			////m_mtx.lock();

			//auto markers = m_MDetector[m_cameraDevice].detect(m_targetMat, m_camPams[m_cameraDevice], 0.036f);
			////std::lock_guard<std::shared_timed_mutex> lk(m_shared_mtx);
			////m_mtx.lock();
			//m_markers[m_cameraDevice] = markers;
			//
			////m_mtx.unlock();

			////std::cout << "marker size : " << m_markers.size() << std::endl;

		}

	}
	return false;
}

void MarkerTracker::filterOutliers()
{
	//m_gemMarker->filterOutliers(m_markers[m_cameraDevice]).swap(m_markers[m_cameraDevice]);
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
				//tMat[i] = gem::glMatFromVecQuat(pose->second.first, pose->second.second);
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

				//cv::Mat Rrod, trod;

				//cv::Rodrigues(markers[i].Rvec, Rrod);


				//std::cout << "roddy : " << Rrod.at<float>(0, 0) << " " << Rrod.at<float>(1, 0) << " " << Rrod.at<float>(2, 0) << " " << std::endl;
				//std::cout << "roddy : " << Rrod.at<float>(0, 1) << " " << Rrod.at<float>(1, 1) << " " << Rrod.at<float>(2, 1) << " " << std::endl;
				//std::cout << "roddy : " << Rrod.at<float>(0, 2) << " " << Rrod.at<float>(1, 2) << " " << Rrod.at<float>(2, 2) << " " << std::endl;

				//glm::vec3 r = glm::vec3(markers[i].Rvec.at<float>(0, 0), markers[i].Rvec.at<float>(1, 0), markers[i].Rvec.at<float>(2, 0));
				glm::vec3 r = glm::vec3(FDetector[m_cameraDevice].getRvec().at<float>(0, 0), FDetector[m_cameraDevice].getRvec().at<float>(1, 0), FDetector[m_cameraDevice].getRvec().at<float>(2, 0));

				float theta = glm::sqrt((r.x*r.x +
										 r.y*r.y +
										 r.z*r.z));

				glm::vec3 R = r / theta;

				//float lenR = glm::length(R);

				//std::cout << "len : " << lenR << std::endl;

				glm::quat rotQuat;
				
				rotQuat.w = glm::cos(theta / 2.0);
				rotQuat.x = R.x * glm::sin(theta / 2.0);
				rotQuat.y = R.y * glm::sin(theta / 2.0);
				rotQuat.z = R.z * glm::sin(theta / 2.0);

				glm::mat4 rotMat = glm::toMat4(rotQuat);
				//rotMat[0][2] *= -1.0;
				//rotMat[1][2] *= -1.0;
				//rotMat[2][2] *= -1.0;

				rotMat[3][0] = FDetector[m_cameraDevice].getTvec().at<float>(0, 0);
				rotMat[3][1] = FDetector[m_cameraDevice].getTvec().at<float>(1, 0);
				rotMat[3][2] = FDetector[m_cameraDevice].getTvec().at<float>(2, 0);


				//double outGL[16];
				//markers[i].glGetModelViewMatrix(outGL);
				//glm::mat4 rrotMat = glm::make_mat4(outGL);
				//rotMat[3][2] *= -1.0f;

				tMat[i] = rotMat;

			}
		}


		//m_mtx.unlock();
		//draw();

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