#include "App.h"

//#define USE_TORCH
//#define USEINFRARED
//#define USEWEBCAM
//#define USEIMAGES
//#define USEVIDEO
//#define USETESTIMAGE


App::App(int width, int height, const std::string &windowName) : Window(width, height, windowName)
{
	krender.setWindow(window);
	glfwSetWindowSize(window, width, height);

	//camera->type = Camera::CameraType::lookat;


}


App::~App()
{
}

void App::initGradient()
{
	std::map<std::string, const gl::Shader::Ptr> progsForGrdient;
	std::string pathToShaders("./shaders/");
	dtam.loadShaders(progsForGrdient, pathToShaders);

	dtam.init(frame[rgbd::FRAME::CURRENT].getWidth(), frame[rgbd::FRAME::CURRENT].getHeight(), progsForGrdient);
}

void App::initSplatter()
{
	glm::mat4 K(1.0f);
	K[0][0] = cameraInterface.getDepthIntrinsics(0).fx;
	K[1][1] = cameraInterface.getDepthIntrinsics(0).fy;
	K[2][0] = cameraInterface.getDepthIntrinsics(0).cx;
	K[2][1] = cameraInterface.getDepthIntrinsics(0).cy;

	std::map<std::string, const gl::Shader::Ptr> progsForSlam;
	std::string pathToShaders("./shaders/");
	splaticp.loadShaders(progsForSlam, pathToShaders);

	for (auto &f : frame)
	{
		f.create(gconfig.depthFrameSize.x, 
			gconfig.depthFrameSize.y,
			desiredColorWidth,
			desiredColorHeight,
			GLHelper::numberOfLevels(glm::ivec3(gconfig.depthFrameSize.x, gconfig.depthFrameSize.y, 1)),
			K, 
			cameraInterface.getDepthUnit(0) / 1000000.0f,
			progsForSlam);

		f.update(cameraInterface.getColorQueues(), 
			cameraInterface.getDepthQueues(), 
			cameraInterface.getInfraQueues(), 
			numberOfCameras, 
			cameraInterface.getDepthUnit(0) / 1000000.0f, 
			depthMin,
			depthMax,
			bottomLeft,
			topRight,
			glm::ivec2(m_pointX, m_pointY), 
			iOff, 
			depthMat, 
			sharpnessValue);
	}

	splaticp.init(frame[rgbd::FRAME::CURRENT].getWidth(),
				  frame[rgbd::FRAME::CURRENT].getHeight(),
				  K,
				  progsForSlam);

	if (!gMap)
	{
		gMap = std::make_shared<rgbd::GlobalMap>(frame[rgbd::FRAME::CURRENT].getWidth(),
												 frame[rgbd::FRAME::CURRENT].getHeight(),
												 K,
												 progsForSlam);
	}

	//glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);

	//gMap->genVirtualFrame(frame[rgbd::FRAME::VIRTUAL], glm::mat4(1.0f));
	//frame[rgbd::FRAME::VIRTUAL].update();

	//gMap->updateGlobalMap(frame[rgbd::FRAME::CURRENT], true, glm::mat4(1.0f));
	//gMap->removeUnnecessaryPoints(0);
	//gMap->genIndexMap(glm::mat4(1.0f));

	//glDisable(GL_CULL_FACE);

}

void App::clearSplatter()
{
	gMap->clearAll();
	currPose = glm::mat4(1.0f);
	//slam.clear();
	//for (auto &f : frame)
	//{
	//	f.clearAll();
	//}
}


// this is done on the coursest level in elasticfusion
void App::initDTAM()
{
	std::map<std::string, const gl::Shader::Ptr> progsForDtam;
	std::string pathToShaders("./shaders/");
	gradFilter.loadShaders(progsForDtam, pathToShaders);

	gradFilter.init(frame[rgbd::FRAME::CURRENT].getWidth(), frame[rgbd::FRAME::CURRENT].getHeight(), progsForDtam);

}

void App::initRGBodo()
{
	std::map<std::string, const gl::Shader::Ptr> progsForDtam;
	std::string pathToShaders("./shaders/");
	rgbodo.loadShaders(progsForDtam, pathToShaders);

	rgbodo.init(frame[rgbd::FRAME::CURRENT].getWidth(), frame[rgbd::FRAME::CURRENT].getHeight(), progsForDtam);



}


void App::initP2PFusion()
{
	glm::mat4 K(1.0f);
	K[0][0] = cameraInterface.getDepthIntrinsics(0).fx;
	K[1][1] = cameraInterface.getDepthIntrinsics(0).fy;
	K[2][0] = cameraInterface.getDepthIntrinsics(0).cx;
	K[2][1] = cameraInterface.getDepthIntrinsics(0).cy;

	std::map<std::string, const gl::Shader::Ptr> progsForP2P;
	std::string pathToShaders("./shaders/");
	p2picp.loadShaders(progsForP2P, pathToShaders);



	for (auto &f : frame)
	{
		f.create(gconfig.depthFrameSize.x, 
			gconfig.depthFrameSize.y, 
			desiredColorWidth,
			desiredColorHeight,
			GLHelper::numberOfLevels(glm::ivec3(gconfig.depthFrameSize.x, gconfig.depthFrameSize.y, 1)),
			K,
			cameraInterface.getDepthUnit(0) / 1000000.0f, 
			progsForP2P);
		f.update(cameraInterface.getColorQueues(), 
			cameraInterface.getDepthQueues(), 
			cameraInterface.getInfraQueues(), 
			numberOfCameras, 
			cameraInterface.getDepthUnit(0) / 1000000.0f, 
			depthMin,
			depthMax,
			bottomLeft,
			topRight,
			glm::ivec2(m_pointX, m_pointY), 
			iOff, 
			depthMat, 
			sharpnessValue);
	}

	glm::vec3 dim = gconfig.volumeDimensions;
	glm::vec3 size = gconfig.volumeSize;
	glm::mat4 initPose = glm::translate(glm::mat4(1.0f), glm::vec3(gconfig.volumeDimensions.x / 2.0f, gconfig.volumeDimensions.y / 2.0f, 0.0f));

	float dMin = -dim.x / 20.0f;
	float dMax = dim.x / 10.0f;

	float maxWeight = 100.0f;
	float distThresh = 0.05f;
	float normThresh = 0.9;

	float largeStep = 0.5f * 0.75f; // no idea why

	float minxy = glm::min(dim.x, dim.y);
	float minxyz = glm::min(minxy, dim.z);

	float maxxy = glm::max(size.x, size.y);
	float maxxyz = glm::max(maxxy, size.z);

	float step =  minxyz / maxxyz;
	
	float nearPlane = 0.1f;
	float farPlane = 3.0f;

	if (!volume)
	{
		volume = std::make_shared<rgbd::GlobalVolume>(frame[rgbd::FRAME::CURRENT].getWidth(), frame[rgbd::FRAME::CURRENT].getHeight(), dim, size, K, maxWeight, largeStep, step, nearPlane, farPlane, progsForP2P);
	}

	p2picp.init(width, height, distThresh, normThresh, K, progsForP2P);

	//p2pFusion.init(volume, frame[rgbd::FRAME::CURRENT], frame[rgbd::FRAME::VIRTUAL], dim, size, initPose, K, maxWeight, distThresh, normThresh, largeStep, step, nearPlane, farPlane, progsForP2P);

}

void App::initP2VFusion()
{
	glm::mat4 K(1.0f);
	K[0][0] = cameraInterface.getDepthIntrinsics(0).fx;
	K[1][1] = cameraInterface.getDepthIntrinsics(0).fy;
	K[2][0] = cameraInterface.getDepthIntrinsics(0).cx;
	K[2][1] = cameraInterface.getDepthIntrinsics(0).cy;

	std::map<std::string, const gl::Shader::Ptr> progsForP2V;
	std::string pathToShaders("./shaders/");
	p2vFusion.loadShaders(progsForP2V, pathToShaders);

	for (auto &f : frame)
	{
		f.create(gconfig.depthFrameSize.x, 
			gconfig.depthFrameSize.y, 
			desiredColorWidth,
			desiredColorHeight,
			GLHelper::numberOfLevels(glm::ivec3(gconfig.depthFrameSize.x, gconfig.depthFrameSize.y, 1)),
			K, 
			cameraInterface.getDepthUnit(0) / 1000000.0f, 
			progsForP2V);

		f.update(cameraInterface.getColorQueues(), 
			cameraInterface.getDepthQueues(), 
			cameraInterface.getInfraQueues(), 
			numberOfCameras, 
			cameraInterface.getDepthUnit(0) / 1000000.0f, 
			depthMin,
			depthMax,
			bottomLeft,
			topRight,
			glm::ivec2(m_pointX, m_pointY), 
			iOff, 
			depthMat, 
			sharpnessValue);
	}

	glm::vec3 dim = gconfig.volumeDimensions;
	glm::vec3 size = gconfig.volumeSize;

	glm::mat4 initPose = glm::translate(glm::mat4(1.0f), glm::vec3(gconfig.volumeDimensions.x / 2.0f, gconfig.volumeDimensions.y / 2.0f, 0.0f));

	//float dMin = -gconfig.volumeDimensions.x / 20.0f;
	//float dMax = gconfig.volumeDimensions.x / 10.0f;

	float maxWeight = 100.0f;
	float distThresh = 0.05f;
	float normThresh = 0.9;

	float largeStep = 0.5f * 0.75f; // no idea why

	float minxy = glm::min(dim.x, dim.y);
	float minxyz = glm::min(minxy, dim.z);

	float maxxy = glm::max(size.x, size.y);
	float maxxyz = glm::max(maxxy, size.z);

	float step = minxyz / maxxyz;

	float nearPlane = 0.1f;
	float farPlane = 3.0f;

	if (!volume)
	{
		volume = std::make_shared<rgbd::GlobalVolume>(frame[rgbd::FRAME::CURRENT].getWidth(), frame[rgbd::FRAME::CURRENT].getHeight(), dim, size, K, maxWeight, largeStep, step, nearPlane, farPlane, progsForP2V);
	}
	p2vFusion.init(volume, frame[rgbd::FRAME::CURRENT], frame[rgbd::FRAME::VIRTUAL], dim, size, initPose, K, maxWeight, distThresh, normThresh, largeStep, step, nearPlane, farPlane, progsForP2V);

}

void App::updateFrames()
{
	glViewport(0, 0, width, height);
	frame[rgbd::FRAME::CURRENT].update(cameraInterface.getColorQueues(), 
		cameraInterface.getDepthQueues(), 
		cameraInterface.getInfraQueues(), 
		numberOfCameras, 
		cameraInterface.getDepthUnit(0) / 1000000.0f,
		depthMin,
		depthMax,
		bottomLeft,
		topRight,
		glm::ivec2(m_pointX, m_pointY), 
		iOff,
		depthMat,
		sharpnessValue);

	frame[rgbd::FRAME::CURRENT].alignDepthTocolor(
		cameraInterface.getDepthToColorExtrinsics(0),
		glm::vec4(cameraInterface.getDepthIntrinsics(0).cx, cameraInterface.getDepthIntrinsics(0).cy, cameraInterface.getDepthIntrinsics(0).fx, cameraInterface.getDepthIntrinsics(0).fy),
		glm::vec4(cameraInterface.getColorIntrinsics(0).cx, cameraInterface.getColorIntrinsics(0).cy, cameraInterface.getColorIntrinsics(0).fx, cameraInterface.getColorIntrinsics(0).fy),
		colorVec
	);

}


// its not full dtam, just so3 alignment using just rgb
bool App::runDTAM(
	glm::mat4 &prePose)
{
	GLuint query;
	glGenQueries(1, &query);
	glBeginQuery(GL_TIME_ELAPSED, query);

	glViewport(0, 0, width, height);
	bool status = true;
	//frame[rgbd::FRAME::CURRENT].update(cameraInterface.getColorQueues(), cameraInterface.getDepthQueues(), cameraInterface.getInfraQueues(), numberOfCameras, cameraInterface.getDepthUnit(0) / 1000000.0f, glm::ivec2(m_pointX, m_pointY), iOff, depthMat, sharpnessValue);
	//frame[rgbd::FRAME::CURRENT].alignDepthTocolor(
	//	cameraInterface.getDepthToColorExtrinsics(0),
	//	glm::vec4(cameraInterface.getDepthIntrinsics(0).cx, cameraInterface.getDepthIntrinsics(0).cy, cameraInterface.getDepthIntrinsics(0).fx, cameraInterface.getDepthIntrinsics(0).fy),
	//	glm::vec4(cameraInterface.getColorIntrinsics(0).cx, cameraInterface.getColorIntrinsics(0).cy, cameraInterface.getColorIntrinsics(0).fx, cameraInterface.getColorIntrinsics(0).fy),
	//	colorVec
	//);
	bool tracked = true;
	so3Pose = dtam.calcDevicePose(
		frame[rgbd::FRAME::CURRENT],
		glm::vec4(cameraInterface.getColorIntrinsics(0).cx, cameraInterface.getColorIntrinsics(0).cy, cameraInterface.getColorIntrinsics(0).fx, cameraInterface.getColorIntrinsics(0).fy),
		so3Pose,
		tracked);

	prePose = so3Pose;
	if (!useSE3)
	{
		se3Pose = se3Pose * prePose;
	}
	
	currPose = currPose * so3Pose; // this way round

	glEndQuery(GL_TIME_ELAPSED);
	GLuint available = 0;
	while (!available) {
		glGetQueryObjectuiv(query, GL_QUERY_RESULT_AVAILABLE, &available);
	}
	// elapsed time in nanoseconds
	GLuint64 elapsed;
	glGetQueryObjectui64vEXT(query, GL_QUERY_RESULT, &elapsed);
	//std::cout << "dtam time : " << elapsed / 1000000.0 << std::endl; // 3 ms for 10 iter at full reso

	return tracked;
}

bool App::runRGBOdo(
	glm::mat4 &prePose)
{
	bool tracked = true; // CHECK THIS!
	//GLuint query;
	//glGenQueries(1, &query);
	//glBeginQuery(GL_TIME_ELAPSED, query);

	//gradFilter.execute(frame[rgbd::FRAME::CURRENT].getColorFilteredMap(), 0, 0.52201f, 0.79451f, true);
	gradFilter.execute(frame[rgbd::FRAME::CURRENT].getColorFilteredMap(), 0, 3.0f, 10.0f, false);




	rgbodo.performColorTracking(frame[rgbd::FRAME::CURRENT], 
		frame[rgbd::FRAME::VIRTUAL], 
		gradFilter.getGradientMap(), 
		prePose, 
		glm::vec4(cameraInterface.getDepthIntrinsics(0).cx, 
			cameraInterface.getDepthIntrinsics(0).cy, 
			cameraInterface.getDepthIntrinsics(0).fx, 
			cameraInterface.getDepthIntrinsics(0).fy));

	se3Pose = se3Pose * prePose;


	//glEndQuery(GL_TIME_ELAPSED);
	//GLuint available = 0;
	//while (!available) {
	//	glGetQueryObjectuiv(query, GL_QUERY_RESULT_AVAILABLE, &available);
	//}
	//// elapsed time in nanoseconds
	//GLuint64 elapsed;
	//glGetQueryObjectui64vEXT(query, GL_QUERY_RESULT, &elapsed);
	//std::cout << "se3 time : " << elapsed / 1000000.0 << std::endl; // 1 ms for 1 iter at full reso

	//std::cout << " se3\n " << glm::to_string(se3Pose) << std::endl;


	return tracked;
}

bool App::runSLAM(
	glm::mat4 &prePose)
{
	bool tracked = true;

	//glm::mat4 pose = currPose;
	glm::mat4 T = glm::mat4(1.0f);

	float AE;
	uint32_t icpCount;

	for (int lvl = rgbd::ICPConstParam::MAX_LEVEL - 1; lvl >= 0; lvl--)
	{

		for (int loop = 0; loop < rgbd::ICPConstParam::MAX_ITR_NUM[lvl]; loop++)
		{
			Eigen::Matrix<float, 6, 6, Eigen::RowMajor> A_icp;
			Eigen::Matrix<float, 6, 1> b_icp;

			splaticp.track(frame[rgbd::FRAME::CURRENT],
				frame[rgbd::FRAME::VIRTUAL],
				T,
				lvl
			);

			splaticp.reduce(
				glm::ivec2(frame[rgbd::FRAME::CURRENT].getWidth(lvl),
					       frame[rgbd::FRAME::CURRENT].getHeight(lvl))
			);

			splaticp.getReduction(
				A_icp.data(),
				b_icp.data(),
				AE,
				icpCount);

			Eigen::Matrix<double, 6, 1> result;
			Eigen::Matrix<double, 6, 6, Eigen::RowMajor> dA_icp = A_icp.cast<double>();
			Eigen::Matrix<double, 6, 1> db_icp = b_icp.cast<double>();

			result = dA_icp.ldlt().solve(db_icp);

			glm::mat4 delta = glm::eulerAngleXYZ(result(3), result(4), result(5));
			delta[3][0] = result(0);
			delta[3][1] = result(1);
			delta[3][2] = result(2);

			T = delta * T;

			//if (result.norm() < 1e-5 && result.norm() != 0)
			//	break;

		} // iter
	} // pyr level

	currPose = currPose * T;

	//frame[rgbd::FRAME::VIRTUAL].update();

	glm::mat4 invT = glm::inverse(currPose);

	gMap->genIndexMap(invT);

	if (integratingFlag)
	{
		gMap->updateGlobalMap(frame[rgbd::FRAME::CURRENT], false, currPose); // 2 ms

		gMap->removeUnnecessaryPoints(static_cast<int>(frame[rgbd::FRAME::CURRENT].getDepthFrameCount())); // 3.5 ms

	}

	gMap->genVirtualFrame(frame[rgbd::FRAME::VIRTUAL], invT); // 1.5 ms


	return tracked;
}


bool App::runOdoSplat(
	glm::mat4 &prePose
)
{
	bool tracked = true;
	bool trackingOK;
	//prePose = currPose;

	gradFilter.execute(frame[rgbd::FRAME::CURRENT].getColorFilteredMap(), 0, 3.0f, 10.0f, false);
	glm::vec4 cam = glm::vec4(cameraInterface.getDepthIntrinsics(0).cx, 
		cameraInterface.getDepthIntrinsics(0).cy, 
		cameraInterface.getDepthIntrinsics(0).fx, 
		cameraInterface.getDepthIntrinsics(0).fy);

	float sigma = 0;
	float rgbError = 0;

	Eigen::Matrix<double, 6, 6, Eigen::RowMajor> lastA; // make global?
	Eigen::Matrix<double, 6, 1> lastb;

	//glm::mat4 prevPose = currPose;

	//Eigen::Matrix<float, 3, 3, Eigen::RowMajor> Rprev;
	//for (int i = 0; i < 3; i++)
	//{
	//	for (int j = 0; j < 3; j++)
	//	{
	//		Rprev(i, j) = prevPose[j][i];
	//	}
	//}

	//Eigen::Vector3f tprev;
	//tprev(0) = prevPose[3][0];
	//tprev(1) = prevPose[3][1];
	//tprev(2) = prevPose[3][2];

	//Eigen::Matrix<float, 3, 3, Eigen::RowMajor> Rcurr = Rprev;
	//Eigen::Vector3f tcurr = tprev;

	glm::mat4 T = prePose;// glm::mat4(1.0f);

	float AE;
	uint32_t icpCount;

	Eigen::Matrix<double, 4, 4, Eigen::RowMajor> resultRt_eig = Eigen::Matrix<double, 4, 4, Eigen::RowMajor>::Identity();

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			resultRt_eig(i, j) = prePose[j][i];
		}
	}

	//GLuint query;
	//glGenQueries(1, &query);
	//glBeginQuery(GL_TIME_ELAPSED, query);
	for (int lvl = rgbd::ICPConstParam::MAX_LEVEL - 1; lvl >= 0; lvl--)
	{
		glm::mat3 K = glm::mat3(1.0f);

		glm::vec4 levelCam = glm::vec4(
			cam.x / (std::pow(2.0f, lvl)),
			cam.y / (std::pow(2.0f, lvl)),
			cam.z / (std::pow(2.0f, lvl)),
			cam.w / (std::pow(2.0f, lvl))
		);

		K[0][0] = levelCam.z;
		K[1][1] = levelCam.w;
		K[2][0] = levelCam.x;
		K[2][1] = levelCam.y;

		Eigen::Matrix<double, 3, 3, Eigen::RowMajor> K_eig = Eigen::Matrix<double, 3, 3, Eigen::RowMajor>::Zero();

		K_eig(0, 0) = levelCam.z;
		K_eig(1, 1) = levelCam.w;
		K_eig(0, 2) = levelCam.x;
		K_eig(1, 2) = levelCam.y;
		K_eig(2, 2) = 1;




		for (int iter = 0; iter < rgbd::ICPConstParam::MAX_ITR_NUM[lvl]; iter++)
		{



			Eigen::Matrix<double, 4, 4, Eigen::RowMajor> Rt_eig = resultRt_eig.inverse();

			Eigen::Matrix<double, 3, 3, Eigen::RowMajor> R_eig = Rt_eig.topLeftCorner(3, 3);

			Eigen::Matrix<double, 3, 3, Eigen::RowMajor> KRK_inv_eig = K_eig * R_eig * K_eig.inverse();

			glm::mat3 KRK_inv;

			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					KRK_inv[i][j] = KRK_inv_eig(j, i);
				}
			}

			Eigen::Vector3d Kt_eig = Rt_eig.topRightCorner(3, 1);
			Kt_eig = K_eig * Kt_eig;

			glm::vec3 kT(Kt_eig(0), Kt_eig(1), Kt_eig(2));

			Eigen::Matrix<float, 6, 6, Eigen::RowMajor> A_rgbd;
			Eigen::Matrix<float, 6, 1> b_rgbd;


			
			rgbodo.computeResiduals(
				frame[rgbd::FRAME::CURRENT],
				gradFilter.getGradientMap(),
				lvl,
				kT,
				KRK_inv,
				sigma,
				rgbError
			);

			rgbodo.computeStep(
				frame[rgbd::FRAME::CURRENT],
				gradFilter.getGradientMap(),
				lvl,
				levelCam,
				sigma,
				rgbError,
				A_rgbd.data(),
				b_rgbd.data()
			);




			// ICP time
			Eigen::Matrix<float, 6, 6, Eigen::RowMajor> A_icp;
			Eigen::Matrix<float, 6, 1> b_icp;



			splaticp.track(frame[rgbd::FRAME::CURRENT],
				frame[rgbd::FRAME::VIRTUAL],
				T,
				lvl
			);

			splaticp.reduce(
				glm::ivec2(frame[rgbd::FRAME::CURRENT].getWidth(lvl),
					frame[rgbd::FRAME::CURRENT].getHeight(lvl))
			);

			splaticp.getReduction(
				A_icp.data(),
				b_icp.data(),
				AE,
				icpCount);


			Eigen::Matrix<double, 6, 1> result;
			Eigen::Matrix<double, 6, 6, Eigen::RowMajor> dA_rgbd = A_rgbd.cast<double>();
			Eigen::Matrix<double, 6, 6, Eigen::RowMajor> dA_icp = A_icp.cast<double>();
			Eigen::Matrix<double, 6, 1> db_rgbd = b_rgbd.cast<double>();
			Eigen::Matrix<double, 6, 1> db_icp = b_icp.cast<double>();

			//std::cout << "icp " << db_icp.transpose() << std::endl;
			//std::cout << "rgb " << db_rgbd.transpose() << std::endl;


			bool useBoth = true;

			if (useBoth)
			{
				double w = 2;
				lastA = dA_rgbd + w * w * dA_icp;
				lastb = db_rgbd + w * db_icp;
			}
			else
			{
				lastA = dA_icp;
				lastb = db_icp;
			}

			result = dA_icp.ldlt().solve(db_icp);

			//result = dA_icp.ldlt().solve(db_icp);
			//result = dA_rgbd.ldlt().solve(db_rgbd);

			glm::mat4 delta = glm::eulerAngleXYZ(result(3), result(4), result(5));
			delta[3][0] = result(0);
			delta[3][1] = result(1);
			delta[3][2] = result(2);

			T = delta * T;

			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					resultRt_eig(i, j) = T[j][i];
				}
			}

			//if (result.norm() < 1e-5 && result.norm() != 0)
			//	break;

			//currPose = delta * currPose;

		} // loop iterations

	} // pyramid levels

	//glEndQuery(GL_TIME_ELAPSED);
	//GLuint available = 0;
	//while (!available) {
	//	glGetQueryObjectuiv(query, GL_QUERY_RESULT_AVAILABLE, &available);
	//}
	//// elapsed time in nanoseconds
	//GLuint64 elapsed;
	//glGetQueryObjectui64vEXT(query, GL_QUERY_RESULT, &elapsed);
	//std::cout << "odo time : " << elapsed / 1000000.0 << std::endl;


	//Eigen::MatrixXd covariance = lastA.cast<double>().lu().inverse();

	trackingOK = AE < 1e-2 || trackingCount < 10;

	//std::cout << "AE " << AE << std::endl;

	if (trackingOK)
	{
		trackingCount++;
		currPose = currPose * T;
	}
	frame[rgbd::FRAME::VIRTUAL].update();

	glm::mat4 invT = glm::inverse(currPose);

	gMap->genIndexMap(invT);

	if (integratingFlag)
	{
		gMap->updateGlobalMap(frame[rgbd::FRAME::CURRENT], false, currPose); // 2 ms

		gMap->removeUnnecessaryPoints(static_cast<int>(frame[rgbd::FRAME::CURRENT].getDepthFrameCount())); // 3.5 ms
	}

	gMap->genVirtualFrame(frame[rgbd::FRAME::VIRTUAL], invT); // 1.5 ms




	return tracked;
}

bool App::runOdoP2P(
	glm::mat4 &prePose
)
{
	//prePose = currPose;
	volume->raycast(frame[rgbd::FRAME::VIRTUAL], currPose);

	gradFilter.execute(frame[rgbd::FRAME::CURRENT].getColorFilteredMap(), 0, 3.0f, 10.0f, false);
	glm::vec4 cam = glm::vec4(cameraInterface.getDepthIntrinsics(0).cx,
		cameraInterface.getDepthIntrinsics(0).cy,
		cameraInterface.getDepthIntrinsics(0).fx,
		cameraInterface.getDepthIntrinsics(0).fy);

	float sigma = 0;
	float rgbError = 0;

	Eigen::Matrix<double, 6, 6, Eigen::RowMajor> lastA; // make global?
	Eigen::Matrix<double, 6, 1> lastb;

	glm::mat4 prevPose = currPose;

	Eigen::Matrix<float, 3, 3, Eigen::RowMajor> Rprev;
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			Rprev(i, j) = prevPose[j][i];
		}
	}

	Eigen::Vector3f tprev;
	tprev(0) = prevPose[3][0];
	tprev(1) = prevPose[3][1];
	tprev(2) = prevPose[3][2];

	Eigen::Matrix<float, 3, 3, Eigen::RowMajor> Rcurr = Rprev;
	Eigen::Vector3f tcurr = tprev;


	for (int lvl = rgbd::ICPConstParam::MAX_LEVEL - 1; lvl >= 0; lvl--)
	{
		glm::mat3 K = glm::mat3(1.0f);

		glm::vec4 levelCam = glm::vec4(
			cam.x / (std::pow(2.0f, lvl)),
			cam.y / (std::pow(2.0f, lvl)),
			cam.z / (std::pow(2.0f, lvl)),
			cam.w / (std::pow(2.0f, lvl))
		);

		K[0][0] = levelCam.z;
		K[1][1] = levelCam.w;
		K[2][0] = levelCam.x;
		K[2][1] = levelCam.y;

		Eigen::Matrix<double, 3, 3, Eigen::RowMajor> K_eig = Eigen::Matrix<double, 3, 3, Eigen::RowMajor>::Zero();

		K_eig(0, 0) = levelCam.z;
		K_eig(1, 1) = levelCam.w;
		K_eig(0, 2) = levelCam.x;
		K_eig(1, 2) = levelCam.y;
		K_eig(2, 2) = 1;

		//glm::mat4 resultRt = glm::mat4(1.0f);
		Eigen::Matrix<double, 4, 4, Eigen::RowMajor> resultRt_eig = Eigen::Matrix<double, 4, 4, Eigen::RowMajor>::Identity();

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				resultRt_eig(i, j) = prePose[j][i];
			}
		}


		for (int iter = 0; iter < rgbd::ICPConstParam::MAX_ITR_NUM[lvl]; iter++)
		{
			Eigen::Matrix<double, 4, 4, Eigen::RowMajor> Rt_eig = resultRt_eig.inverse();

			Eigen::Matrix<double, 3, 3, Eigen::RowMajor> R_eig = Rt_eig.topLeftCorner(3, 3);

			Eigen::Matrix<double, 3, 3, Eigen::RowMajor> KRK_inv_eig = K_eig * R_eig * K_eig.inverse();

			glm::mat3 KRK_inv;

			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					KRK_inv[i][j] = KRK_inv_eig(j, i);
				}
			}

			Eigen::Vector3d Kt_eig = Rt_eig.topRightCorner(3, 1);
			Kt_eig = K_eig * Kt_eig;

			glm::vec3 kT(Kt_eig(0), Kt_eig(1), Kt_eig(2));

			Eigen::Matrix<float, 6, 6, Eigen::RowMajor> A_rgbd;
			Eigen::Matrix<float, 6, 1> b_rgbd;

			rgbodo.computeResiduals(
				frame[rgbd::FRAME::CURRENT],
				gradFilter.getGradientMap(),
				lvl,
				kT,
				KRK_inv,
				sigma,
				rgbError
			);

			rgbodo.computeStep(
				frame[rgbd::FRAME::CURRENT],
				gradFilter.getGradientMap(),
				lvl,
				levelCam,
				sigma,
				rgbError,
				A_rgbd.data(),
				b_rgbd.data()
			);

			// ICP time
			Eigen::Matrix<float, 6, 6, Eigen::RowMajor> A_icp;
			Eigen::Matrix<float, 6, 1> b_icp;

			float AE;
			uint32_t icpCount;

			p2picp.track(
				frame[rgbd::FRAME::CURRENT],
				frame[rgbd::FRAME::VIRTUAL],
				currPose,
				lvl
			);

			p2picp.reduce(
				glm::ivec2(frame[rgbd::FRAME::CURRENT].getWidth(lvl),
					frame[rgbd::FRAME::CURRENT].getHeight(lvl))
			);

			p2picp.getReduction(
				A_icp.data(),
				b_icp.data(),
				AE,
				icpCount
			);

			Eigen::Matrix<double, 6, 1> result;
			Eigen::Matrix<double, 6, 6, Eigen::RowMajor> dA_rgbd = A_rgbd.cast<double>();
			Eigen::Matrix<double, 6, 6, Eigen::RowMajor> dA_icp = A_icp.cast<double>();
			Eigen::Matrix<double, 6, 1> db_rgbd = b_rgbd.cast<double>();
			Eigen::Matrix<double, 6, 1> db_icp = b_icp.cast<double>();

			//std::cout << "icp " << db_icp.transpose() << std::endl;
			//std::cout << "rgb " << db_rgbd.transpose() << std::endl;


			bool useBoth = true;

			if (useBoth)
			{
				double w = 10;
				lastA = dA_rgbd + w * w * dA_icp;
				lastb = db_rgbd + w * db_icp;
			}
			else
			{
				lastA = dA_icp;
				lastb = db_icp;
			}

			result = lastA.ldlt().solve(lastb);

			//result = dA_icp.ldlt().solve(db_icp);
			//result = dA_rgbd.ldlt().solve(db_rgbd);

			glm::mat4 delta = glm::eulerAngleXYZ(result(3), result(4), result(5));
			delta[3][0] = result(0);
			delta[3][1] = result(1);
			delta[3][2] = result(2);


			currPose = delta * currPose;

		} // loop iterations

	} // pyramid levels


	//if (sigma == 0 || (tcurr - tprev).norm() > 0.3 || isnan(tcurr(0)))
	//{
		//Rcurr = Rprev;
		//tcurr = tprev;

	//	currPose = prevPose;
	//}

	if (integratingFlag)
	{
		volume->integrate(0, frame[rgbd::FRAME::CURRENT], currPose);
	}



	return false;
}


bool App::runP2P(
	glm::mat4 &prePose
)
{


	bool tracked = true;
	GLuint query;
	glGenQueries(1, &query);
	glBeginQuery(GL_TIME_ELAPSED, query);

	volume->raycast(frame[rgbd::FRAME::VIRTUAL], currPose);


	for (int lvl = rgbd::ICPConstParam::MAX_LEVEL - 1; lvl >= 0; lvl--)
	{
		for (int iter = 0; iter < rgbd::ICPConstParam::MAX_ITR_NUM[lvl]; iter++)
		{

			Eigen::Matrix<float, 6, 6, Eigen::RowMajor> A_icp;
			Eigen::Matrix<float, 6, 1> b_icp;
			float AE;
			uint32_t icpCount;

			p2picp.track(
				frame[rgbd::FRAME::CURRENT],
				frame[rgbd::FRAME::VIRTUAL],
				currPose,
				lvl
			);

			p2picp.reduce(
				glm::ivec2(frame[rgbd::FRAME::CURRENT].getWidth(lvl),
					frame[rgbd::FRAME::CURRENT].getHeight(lvl))
			);

			p2picp.getReduction(
				A_icp.data(),
				b_icp.data(),
				AE,
				icpCount
			);

			Eigen::Matrix<double, 6, 1> result;
			Eigen::Matrix<double, 6, 6, Eigen::RowMajor> dA_icp = A_icp.cast<double>();
			Eigen::Matrix<double, 6, 1> db_icp = b_icp.cast<double>();

			result = dA_icp.ldlt().solve(db_icp);

			glm::mat4 delta = glm::eulerAngleXYZ(result(3), result(4), result(5));
			delta[3][0] = result(0);
			delta[3][1] = result(1);
			delta[3][2] = result(2);

			currPose = delta * currPose;

			if (result.norm() < 1e-5 && result.norm() != 0)
				break;

		}// iter

	} // lvl

	if (integratingFlag)
	{
		volume->integrate(0, frame[rgbd::FRAME::CURRENT], currPose);
	}

	//if (useSO3)
	//{
	//	//p2pFusion.setT(so3Pose);
	//}

	//if (useSE3)
	//{

	////	p2pFusion.setT( (colorToDepth[0] * se3Pose ));
	//}

	////p2pFusion.raycast(frame[rgbd::FRAME::VIRTUAL]);
	////
	////glm::mat4 T = p2pFusion.calcDevicePose(frame[rgbd::FRAME::CURRENT], frame[rgbd::FRAME::VIRTUAL]);

	////if (integratingFlag)
	////{
	////	p2pFusion.integrate(frame[rgbd::FRAME::CURRENT]);
	////}


	glEndQuery(GL_TIME_ELAPSED);
	GLuint available = 0;
	while (!available) {
		glGetQueryObjectuiv(query, GL_QUERY_RESULT_AVAILABLE, &available);
	}
	// elapsed time in nanoseconds
	GLuint64 elapsed;
	glGetQueryObjectui64vEXT(query, GL_QUERY_RESULT, &elapsed);
	//std::cout << "time : " << elapsed / 1000000.0 << std::endl;
	//std::cout << " p2p\n " << glm::to_string(T) << std::endl;
	//std::cout << " dif\n " << glm::to_string(se3Pose * glm::inverse(T)) << std::endl;

	return tracked;
}

bool App::runP2V(
	glm::mat4 &prePose)
{
	GLuint query;
	glGenQueries(1, &query);
	glBeginQuery(GL_TIME_ELAPSED, query);

	bool tracked = true;
	
	glm::mat4 T = p2vFusion.calcDevicePose(frame[rgbd::FRAME::CURRENT], frame[rgbd::FRAME::VIRTUAL]);

	if (integratingFlag)
	{
		p2vFusion.integrate(frame[rgbd::FRAME::CURRENT]);
	}

	glEndQuery(GL_TIME_ELAPSED);
	GLuint available = 0;
	while (!available) {
		glGetQueryObjectuiv(query, GL_QUERY_RESULT_AVAILABLE, &available);
	}
	// elapsed time in nanoseconds
	GLuint64 elapsed;
	glGetQueryObjectui64vEXT(query, GL_QUERY_RESULT, &elapsed);
	//std::cout << "p2v time : " << elapsed / 1000000.0 << std::endl;

	return tracked;

}

#ifdef USE_TORCH
#include <torch/torch.h>
#include <torchvision/vision.h>
#endif

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error %d: %s\n", error, description);
}

void App::kRenderInit()
{
	// The render shader bit can be elsewhere
	std::string pathToRenderShaders("./shaders/");
	progs.insert(std::make_pair("ScreenQuad", std::make_shared<gl::Shader>(pathToRenderShaders + "ScreenQuad.vert", pathToRenderShaders + "ScreenQuad.frag")));

	progs["ScreenQuad"]->setUniform("isYFlip", 1);
	progs["ScreenQuad"]->setUniform("maxDepth", rgbd::ICPConstParam::MAX_DEPTH);

	krender.SetCallbackFunctions();
	krender.compileAndLinkShader();

	krender.setNumberOfCameras(numberOfCameras);

	for (int i = 0; i < numberOfCameras; i++)
	{
		krender.setCameraParams(i, glm::vec4(cameraInterface.getDepthIntrinsics(i).fx,
			cameraInterface.getDepthIntrinsics(i).fy,
			cameraInterface.getDepthIntrinsics(i).cx,
			cameraInterface.getDepthIntrinsics(i).cy),
			glm::vec4(cameraInterface.getColorIntrinsics(i).fx,
				cameraInterface.getColorIntrinsics(i).fy,
				cameraInterface.getColorIntrinsics(i).cx,
				cameraInterface.getColorIntrinsics(i).cy));
	}

	// Set locations
	krender.setLocations();
	krender.setVertPositions();
	krender.allocateBuffers();
	//krender.setTextures(gfusion.getDepthImage(), gflow.getColorTexture(), gfusion.getVerts(), gfusion.getNorms(), gfusion.getVolume(), gfusion.getTrackImage(), gfusion.getPVPNorms(), gfusion.getPVDNorms(), gfusion.getSplatterDepth(), gfusion.getSplatterNormal()); //needs texture uints from gfusion init
	krender.anchorMW(std::make_pair<int, int>(1920 - 512 - krender.guiPadding().first, krender.guiPadding().second));

	krender.setFusionType(trackDepthToPoint, trackDepthToVolume, useSplatter);

	//gflow.setColorTexture(cameraInterface.getColorQueues(), col);
	krender.setDepthMinMax(depthMin, depthMax);

}


void App::gDisOptFlowInit()
{
	std::map<std::string, const gl::Shader::Ptr> progsForDisFlow;
	std::string pathToShaders("./shaders/");
	disflow.loadShaders(progsForDisFlow, pathToShaders);

	int numLevel = GLHelper::numberOfLevels(glm::ivec3(colorFrameSize[0].x, colorFrameSize[0].x, 1));

	disflow.init(numLevel,
		colorFrameSize[0].x,
		colorFrameSize[0].y,
		progsForDisFlow
	);

//	int devNumber = 0;
//	gflow.compileAndLinkShader();
//	gflow.setLocations();
//#ifdef USEINFRARED
//	gdisoptflow.setNumLevels(depthWidth);
//	gdisoptflow.setTextureParameters(depthWidth, depthHeight);
//	gdisoptflow.allocateTextures(true);
//#else
//	gflow.setNumLevels(colorFrameSize[devNumber].x);
//	gflow.setTextureParameters(colorFrameSize[devNumber].x, colorFrameSize[devNumber].y);
//	gflow.allocateTextures(4);
//
//	krender.setFlowTexture(gflow.getFlowTexture());
//
//#endif
//
//	gflow.allocateBuffers();

}

void App::gFloodInit()
{
	//gflood.setCameraParams(glm::vec4(cameraInterface.getDepthIntrinsics(cameraDevice).fx,
	//	cameraInterface.getDepthIntrinsics(cameraDevice).fy,
	//	cameraInterface.getDepthIntrinsics(cameraDevice).cx,
	//	cameraInterface.getDepthIntrinsics(cameraDevice).cy));

	//gflood.compileAndLinkShader();
	//gflood.setLocations();

	glm::vec3 dim(0.5f);
	glm::vec3 size(128.0f, 128.0f, 128.0f);


	std::map<std::string, const gl::Shader::Ptr> progsForFlood;
	std::string pathToShaders("./shaders/");
	gflood->loadShaders(progsForFlood, pathToShaders);

	gflood = std::make_shared<gFlood>(size.x, dim.x, progsForFlood);



}

void mCubeInit()
{
	//mcconfig.gridSize = gconfig.volumeSize;
	//mcconfig.gridSizeMask = glm::uvec3(mcconfig.gridSize.x - 1, mcconfig.gridSize.y - 1, mcconfig.gridSize.z - 1);
	//mcconfig.gridSizeShift = glm::uvec3(0, log2(mcconfig.gridSize.x), log2(mcconfig.gridSize.y) + log2(mcconfig.gridSize.z));
	//mcconfig.numVoxels = mcconfig.gridSize.x * mcconfig.gridSize.y * mcconfig.gridSize.z;
	//mcconfig.voxelSize = glm::vec3(gconfig.volumeDimensions.x * 1000.0f / gconfig.volumeSize.x, gconfig.volumeDimensions.y * 1000.0f / gconfig.volumeSize.y, gconfig.volumeDimensions.z * 1000.0f / gconfig.volumeSize.z);
	//mcconfig.maxVerts = std::min(mcconfig.gridSize.x * mcconfig.gridSize.y * 128, uint32_t(128 * 128 * 128));

	//gfusion.setMcConfig(mcconfig);

}

void App::resetVolume()
{
	rotation = glm::vec3(0.0);
	cameraPos = glm::vec3(0.0);



	frame[rgbd::FRAME::CURRENT].reset();

	lost = false;
	trackingCount = 0;

	int devNumber = 0;
	//glm::mat4 initPose = glm::translate(glm::mat4(1.0f), glm::vec3(gconfig.volumeDimensions.x / 2.0f, gconfig.volumeDimensions.y / 2.0f, 0.0f));
	gflow.wipeFlow();

	bool deleteFlag = false;

	if (glm::vec3(std::stoi(sizes[sizeX]), std::stoi(sizes[sizeY]), std::stoi(sizes[sizeZ])) != gconfig.volumeSize)
	{
		deleteFlag = true;
	}

	gconfig.volumeSize = glm::vec3(std::stoi(sizes[sizeX]), std::stoi(sizes[sizeY]), std::stoi(sizes[sizeZ]));
	gconfig.volumeDimensions = glm::vec3(dimension);
	gconfig.dMax = dimension / 10.0f;
	gconfig.dMin = -dimension / 20.0f;

	//gfusion.setConfig(gconfig);

	initOff = iOff;

	//std::cout << "lalala " << mousePos.x - controlPoint0.x << " " << controlPoint0.y - mousePos.y<< std::endl;

	//std::cout << glm::to_string(iOff) << std::endl;

	// AND THE OTHER ONE

	if (trackDepthToPoint || trackDepthToVolume)
	{
		initPose = glm::translate(glm::mat4(1.0f), glm::vec3(-iOff.x + gconfig.volumeDimensions.x / 2.0f, -iOff.y + gconfig.volumeDimensions.y / 2.0f, -iOff.z + dimension / 2.0));
		so3Pose = glm::mat4(1.0f);
		se3Pose = initPose;
		currPose = initPose;
	}
	else
	{
		initPose = glm::mat4(1.0f);// glm::translate(glm::mat4(1.0f), glm::vec3(gconfig.volumeDimensions.x / 2.0f, gconfig.volumeDimensions.y / 2.0f, 0.0f));
		so3Pose = initPose;
		se3Pose = initPose;
		currPose = initPose;

	}

	if (deleteFlag)
	{
		volume->resize(gconfig.volumeSize);
	}
	volume->setVolDim(gconfig.volumeDimensions);
	volume->reset();

	for (int i = 0; i < graphPoints.size(); i++)
	{
		graphPoints[i] = glm::vec4(iOff.x, iOff.y, iOff.z, 1.0f);

		arrayX[i] = iOff.x;
		arrayY[i] = iOff.y;
		arrayZ[i] = iOff.z;
	}


	if (useSplatter || useODOSplat)
	{
		clearSplatter();
	}

	if (trackDepthToPoint)
	{
		//if (deleteFlag)
		//{
		//	volume->resize(gconfig.volumeSize);
		//}
		//volume->setVolDim(gconfig.volumeDimensions);
		//p2pFusion.clear(initPose, gconfig.volumeSize);
	}

	if (trackDepthToVolume)
	{
		if (deleteFlag)
		{
			volume->resize(gconfig.volumeSize);
		}
		volume->setVolDim(gconfig.volumeDimensions);
		p2vFusion.clear(initPose, gconfig.volumeSize, gconfig.volumeDimensions);
	}


	volSlice = gconfig.volumeSize.z / 2.0f;

	krender.setVolumeSize(gconfig.volumeSize);


	//gfusion.setPose(initPose);
	//gfusion.setPoseP2V(initPose);


	//gfusion.Reset(initPose, deleteFlag);
	//reset = true;
	//if (trackDepthToPoint)
	//{
	//	gfusion.raycast(cameraDevice);
	//}

	counter = 0;
	if (performFlood)
	{
		//gflood.setVolumeConfig(gconfig.volumeSize.x, gconfig.volumeDimensions.x);
		//gflood.allocateTextures();
	}
}

void App::saveSTL()
{
	mcconfig.gridSize = glm::uvec3(gconfig.volumeSize.x, gconfig.volumeSize.y, gconfig.volumeSize.z);
	mcconfig.voxelSize = glm::vec3(gconfig.volumeDimensions.x / gconfig.volumeSize.x, gconfig.volumeDimensions.y / gconfig.volumeSize.y, gconfig.volumeDimensions.z / gconfig.volumeSize.z);

	mcubes.setConfig(mcconfig);

	if (trackDepthToPoint)
	{
		mcubes.setVolumeTexture(p2pFusion.getVolumeID());
	}
	else if (trackDepthToVolume)
	{
		mcubes.setVolumeTexture(p2vFusion.getVolumeID());
	}
	//mcubes.setVolumeTexture(gflood.getFloodSDFTexture());
	mcubes.init();
	mcubes.setIsolevel(0.0f);
	mcubes.generateMarchingCubes();
	mcubes.exportMesh();
}

void App::saveSplatter()
{
	gMap->savePointCloud("data/meshes/splatterVertsBin.ply");
}

void App::loadPreviousExtrinsicCalibrationFromFile()
{
	//std::ifstream extrinsicFile("./data/calib/extrinsic.txt");

	//if (extrinsicFile.is_open())
	//{
	//	float inValues[16];

	//	for (int i = 0; i < 16; i++)
	//	{
	//		extrinsicFile >> inValues[i];
	//	}

	//	std::memcpy(glm::value_ptr(cam2camTrans), inValues, sizeof(inValues));
	//	extrinsicFile.close();

	//	std::cout << "Values from file : " << std::endl;
	//	std::cout << glm::to_string(cam2camTrans) << std::endl;
	//	gfusion.setDepthToDepth(cam2camTrans);
	//	pairFromFile = true;

	//}
	//else
	//{
	//	std::cout << "Calib file could not be opened, does it exist?" << std::endl;
	//}


}

void App::saveExtrinsicCalibrationToFile()
{
	//std::ofstream extrinsicFile("./data/calib/extrinsic.txt");

	//if (extrinsicFile.is_open())
	//{
	//	float outValues[16];
	//	std::memcpy(outValues, glm::value_ptr(cam2camTrans), sizeof(outValues));
	//	for (int i = 0; i < 16; i++)
	//	{
	//		extrinsicFile << outValues[i] << " ";
	//	}

	//	extrinsicFile.close();
	//}
	//else
	//{
	//	std::cout << "Could not open file for writing" << std::endl;
	//}

}

void App::startRealsense()
{


	//int prevERate = eRate;
	//int prevERes = eRes;

	//if (cameraRunning) eRate = prevERate;
	//if (eRate == 90) eRes = 0;

	//if (cameraRunning) eRes = prevERes;
	//if (eRes == 1) eRate = 30;


	if (cameraRunning == false)
	{
		cameraRunning = true;
		//kcamera.setPreset(eRate, eRes);
		numberOfCameras = cameraInterface.searchForCameras();

		if (numberOfCameras > 0)
		{
			// need to send target refresh rate, and target horizontal resolution, vertical also perhaps
			// datatype should be fxed to Y8 IR, Y16 depth, and RGB color

			if (true) { // hack for 515

				desiredWidth = 1024;
				desiredHeight = 768;
				desiredRate = 30;

				desiredColorWidth = 1920;
				desiredColorHeight = 1080;
			}

			infraProfiles.resize(numberOfCameras, std::make_tuple(desiredWidth, desiredHeight, desiredRate, RS2_FORMAT_Y8));
			depthProfiles.resize(numberOfCameras, std::make_tuple(desiredWidth, desiredHeight, desiredRate, RS2_FORMAT_Z16));
			colorProfiles.resize(numberOfCameras, std::make_tuple(desiredColorWidth, desiredColorHeight, desiredColorRate, RS2_FORMAT_RGBA8));

			depthFrameSize.resize(numberOfCameras);
			colorFrameSize.resize(numberOfCameras);
			infraFrameSize.resize(numberOfCameras);

			colorToDepth.resize(numberOfCameras);
			depthToColor.resize(numberOfCameras);

			//gfusion.setNumberOfCameras(numberOfCameras);

			for (int camera = 0; camera < numberOfCameras; camera++)
			{
				cameraInterface.startDevice(camera, depthProfiles[camera], infraProfiles[camera], colorProfiles[camera]);
				
				if (false) { // HACK FOR 515
					cameraInterface.setDepthTable(camera, 500, 0, 100, 0, 0);
				}

				int wd, hd, rd;
				int wc, hc, rc;
				cameraInterface.getDepthProperties(camera, wd, hd, rd);
				cameraInterface.getColorProperties(camera, wc, hc, rc);

				depthToColor[camera] = cameraInterface.getDepthToColorExtrinsics(camera);

				colorToDepth[camera] = cameraInterface.getColorToDepthExtrinsics(camera);

				//gfusion.setDepthToColorExtrinsics(cameraInterface.getDepthToColorExtrinsics(camera), camera);
				//gfusion.setDepthUnit(cameraInterface.getDepthUnit(camera)); // this may bug out if you have two different cameras with different scales

				depthFrameSize[camera].x = wd;
				depthFrameSize[camera].y = hd;

				infraFrameSize[camera].x = wd;
				infraFrameSize[camera].y = hd;

				colorFrameSize[camera].x = wc;
				colorFrameSize[camera].y = hc;
			}

			krender.setColorFrameSize(colorFrameSize[0].x, colorFrameSize[0].y);
			krender.allocateTextures();

		}

		//if (eRes == 0)
		//{
		//	depthWidth = 848;
		//	depthHeight = 480;
		//}
		//else if (eRes == 1)
		//{
		//	depthWidth = 1280;
		//	depthHeight = 720;
		//}

		//kcamera.start();

		setUpGPU();

	}


}

void App::loadFromFile()
{

	int camera = 0;
	depthFrameSize.resize(1);
	colorFrameSize.resize(1);
	colorToDepth.resize(1);
	//gfusion.setNumberOfCameras(1);

	if (cameraRunning == false)
	{
		cameraRunning = true;
		usingDataFromFile = true;

		cameraInterface.startDeviceFromFile("D:\\data\\bags\\20190423_162114.bag", 1, 0);
		int wd, hd, rd;
		int wc, hc, rc;
		cameraInterface.getDepthPropertiesFromFile(wd, hd, rd);
		cameraInterface.getColorPropertiesFromFile(wc, hc, rc);



		//colorToDepth[camera] = cameraInterface.getColorToDepthExtrinsics(cameraDevice);

		//gfusion.setDepthToColorExtrinsics(cameraInterface.getDepthToColorExtrinsics(camera), camera);
		//gfusion.setDepthUnit(cameraInterface.getDepthUnitFromFile());

		depthFrameSize[camera].x = wd;
		depthFrameSize[camera].y = hd;

		colorFrameSize[camera].x = wc;
		colorFrameSize[camera].y = hc;

		krender.setColorFrameSize(colorFrameSize[0].x, colorFrameSize[0].y);
		krender.allocateTextures();

		setUpGPU();
	}
}

void App::setImguiWindows()
{
	int width;
	int height;
	int topBarHeight = 128;

	glfwGetFramebufferSize(window, &width, &height);

	controlPoint0.x = width / 4.0f;
	controlPoint0.y = height - height / 3.0f;

	navigationWindow.set(0, topBarHeight, controlPoint0.x, height - topBarHeight, true, true, "navi");

	graphWindow.set(controlPoint0.x, controlPoint0.y, width - controlPoint0.x, height - controlPoint0.y, true, true, "graphs"); // 420 is its height

	display2DWindow.set(controlPoint0.x, topBarHeight, (width - controlPoint0.x) / 2, controlPoint0.y - topBarHeight, true, true, "2d");

	display3DWindow.set(controlPoint0.x + (width - controlPoint0.x) / 2, topBarHeight, (width - controlPoint0.x) / 2, controlPoint0.y - topBarHeight, true, true, "3d");

}

void App::setUIStyle()
{
	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();

	style.Alpha = 1.0f;

	style.WindowBorderSize = 1.0f;
	style.WindowRounding = 0.0f;

	style.FrameBorderSize = 1.0f;
	style.FrameRounding = 12.0f;

	style.PopupBorderSize = 0.0f;

	style.ScrollbarRounding = 12.0f;
	style.GrabRounding = 12.0f;
	style.GrabMinSize = 20.0f;


	// spacings
	style.ItemSpacing = ImVec2(8.0f, 8.0f);
	style.FramePadding = ImVec2(8.0f, 8.0f);
	style.WindowPadding = ImVec2(8.0f, 8.0f);



}

void App::setUI()
{
	setImguiWindows();
	// graphs
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

		//ImGui::SetNextWindowSize(ImVec2(graphWindow.w, graphWindow.h), ImGuiSetCond_Always);
		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		//window_flags |= ImGuiWindowFlags_ShowBorders;
		//if (graphWindow.resize == false) window_flags |= ImGuiWindowFlags_NoResize;
		//window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoCollapse;

		ImGui::Begin("Slider Graph", &graphWindow.visible, window_flags);
		//ImGui::PushItemWidth(-krender.guiPadding().first);
		ImGui::SetWindowPos(ImVec2(graphWindow.x, graphWindow.y));
		ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(1.0, 0.0, 0.0, 1.0));
		ImGui::PlotLines("X", &arrayX[0], graphWindow.w, 0, "", minmaxX.first, minmaxX.second, ImVec2(graphWindow.w, graphWindow.h / 3));
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.0, 1.0, 0.0, 1.0));
		ImGui::PlotLines("Y", &arrayY[0], graphWindow.w, 0, "", minmaxY.first, minmaxY.second, ImVec2(graphWindow.w, graphWindow.h / 3));
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.0, 0.0, 1.0, 1.0));
		ImGui::PlotLines("Z", &arrayZ[0], graphWindow.w, 0, "", minmaxZ.first, minmaxZ.second, ImVec2(graphWindow.w, graphWindow.h / 3));
		ImGui::PopStyleColor();

		ImGui::End();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();

	}

	// 2d data
	{
		ImGui::SetNextWindowPos(ImVec2(display2DWindow.x, display2DWindow.y));
		//ImGui::SetNextWindowSize(ImVec2(display2DWindow.w, display2DWindow.h), ImGuiSetCond_Always);

		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		//window_flags |= ImGuiWindowFlags_ShowBorders;
		//window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoCollapse;

		ImGui::Begin("Video ", &display2DWindow.visible, window_flags);
		imguiFocusLW = ImGui::IsWindowHovered();
		ImGui::End();
	}

	//3d data
	{
		ImGui::SetNextWindowPos(ImVec2(display3DWindow.x, display3DWindow.y));
		//ImGui::SetNextWindowSize(ImVec2(display3DWindow.w, display3DWindow.h), ImGuiSetCond_Always);

		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		//window_flags |= ImGuiWindowFlags_ShowBorders;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoCollapse;

		ImGui::Begin("Video Sources", &display3DWindow.visible, window_flags);
		imguiFocusRW = ImGui::IsWindowHovered();

		//ImGui::ShowDemoWindow();

		ImGui::End();
	}
	// navigation
	{
		ImGui::SetNextWindowPos(ImVec2(navigationWindow.x, navigationWindow.y));
		//ImGui::SetNextWindowSize(ImVec2(navigationWindow.w, navigationWindow.h), ImGuiSetCond_Always);
		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		//window_flags |= ImGuiWindowFlags_ShowBorders;
		//window_flags |= ImGuiWindowFlags_NoResize;
		//window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoCollapse;

		float arr[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		static float maxArr[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		static int frameCounter = 0;

		arr[0] = gflow.getTimeElapsed();
		//gfusion.getTimes(arr);
		//arr[2] = gflood->getTimeElapsed();
		arr[8] = arr[0] + arr[1] + arr[2] + arr[3] + arr[4] + arr[5] + arr[6] + arr[7];

		for (int i = 0; i < 9; i++)
		{
			if (arr[i] > maxArr[i])
			{
				maxArr[i] = arr[i];
			}
		}
		frameCounter++;
		if (frameCounter > 100)
		{
			frameCounter = 0;
			float maxArr[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		}
		ImGui::Begin("Menu", &navigationWindow.visible, window_flags);
		//ImGui::Text("Timing");
		//ImGui::Separator();

		ImGui::Text("Framerate %.3f ms/frame (%.1f FPS)", arr[8], 1000.0f / arr[8]);
		if (ImGui::BeginPopupContextItem("item context menu"))
		{
			ImGui::Text("Flow %.3f ms/frame (max : %.3f)", arr[0], maxArr[0]);
			ImGui::Text("Raycasting %.3f ms/frame (max : %.3f)", arr[1], maxArr[1]);
			ImGui::Text("Flood %.3f ms/frame (max : %.3f)", arr[2], maxArr[2]);
			ImGui::Text("Track P2P %.3f ms/frame (max : %.3f)", arr[3], maxArr[3]);
			ImGui::Text("TBC %.3f ms/frame (max : %.3f)", arr[4], maxArr[4]);
			ImGui::Text("Integration %.3f ms/frame (max : %.3f)", arr[5], maxArr[5]);
			ImGui::Text("Track P2V %.3f ms/frame (max : %.3f)", arr[6], maxArr[6]);
			ImGui::Text("TBC %.3f ms/frame (max : %.3f)", arr[7], maxArr[7]);
			ImGui::EndPopup();
		}

		//ImGui::PushItemWidth(-krender.guiPadding().first);
		//ImGui::SetWindowPos(ImVec2(display_w - (display_w / 4) - krender.guiPadding().first, ((krender.guiPadding().second) + (0))));
		ImGui::PushItemWidth(-1);
		ImGui::PlotHistogram("", arr, IM_ARRAYSIZE(arr), 0, NULL, 0.0f, 33.0f, ImVec2(0, 80));
		ImGui::PopItemWidth();

		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.33f, 1.0f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.33f, 0.7f, 0.2f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.33f, 0.8f, 0.3f));
		if (ImGui::BeginPopupContextItem("plot context menu"))
		{
			ImGui::Text("Jump Flooding %.3f ms/frame (max : %.3f)", arr[0], maxArr[0]);
			ImGui::Text("Raycasting %.3f ms/frame (max : %.3f)", arr[1], maxArr[1]);
			ImGui::Text("TBC %.3f ms/frame (max : %.3f)", arr[2], maxArr[2]);
			ImGui::Text("Track P2P %.3f ms/frame (max : %.3f)", arr[3], maxArr[3]);
			ImGui::Text("TBC %.3f ms/frame (max : %.3f)", arr[4], maxArr[4]);
			ImGui::Text("Integration %.3f ms/frame (max : %.3f)", arr[5], maxArr[5]);
			ImGui::Text("Track P2V %.3f ms/frame (max : %.3f)", arr[6], maxArr[6]);
			ImGui::Text("TBC %.3f ms/frame (max : %.3f)", arr[7], maxArr[7]);
			ImGui::EndPopup();
		}

		if (ImGui::Button("Start Realsense"))
		{
			startRealsense();
		}
		ImGui::PopStyleColor(3);
		if (ImGui::Button("Load From File"))
		{
			loadFromFile();
		}
		//ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::Button("Camera 0"))
		{
			cameraDevice = 0;
		}
		ImGui::SameLine();
		if (ImGui::Button("Camera 1"))
		{
			cameraDevice = 1;
		}

		ImGui::Separator();
		if (ImGui::CollapsingHeader("Camera"))
		{

			ImGui::Text("Depth and Infrared");
			if (ImGui::Button("30 Hz"))
			{
				desiredRate = 30;
			}
			ImGui::SameLine();
			if (ImGui::Button("60 Hz"))
			{
				desiredRate = 60;
			}
			ImGui::SameLine();
			if (ImGui::Button("90 Hz"))
			{
				desiredRate = 90;
			}
			ImGui::SameLine();


			if (ImGui::Button("848x480"))
			{
				desiredWidth = 848;
				desiredHeight = 480;
			}
			ImGui::SameLine();
			if (ImGui::Button("1280x720"))
			{
				desiredWidth = 1280;
				desiredHeight = 720;
				desiredRate = 30;
			}

			auto textSelection = "Selected Width : " + std::to_string(desiredWidth) + " height : " + std::to_string(desiredHeight) + " rate : " + std::to_string(desiredRate);
			ImGui::Text(textSelection.c_str());

			ImGui::Text("Color");
			if (ImGui::Button("6 Hz ##Color"))
			{

				desiredColorRate = 6;
			}
			ImGui::SameLine();
			if (ImGui::Button("30 Hz ##Color"))
			{
				desiredColorRate = 30;
			}
			ImGui::SameLine();
			if (ImGui::Button("60 Hz ##Color"))
			{
				desiredColorRate = 60;
				if (desiredColorWidth == 1920)
				{
					desiredColorWidth == 848;
					desiredColorHeight == 480;
				}
			}
			ImGui::SameLine();


			if (ImGui::Button("848x480 ##Color"))
			{
				desiredColorWidth = 848;
				desiredColorHeight = 480;
			}
			ImGui::SameLine();
			if (ImGui::Button("1920x1080 ##Color"))
			{
				desiredColorWidth = 1920;
				desiredColorHeight = 1080;
				if (desiredColorRate == 60)
				{
					desiredColorRate = 30;
				}
			}

			auto textColorSelection = "Selected Width : " + std::to_string(desiredColorWidth) + " height : " + std::to_string(desiredColorHeight) + " rate : " + std::to_string(desiredColorRate);
			ImGui::Text(textColorSelection.c_str());


			if (ImGui::Button("Emitter"))
			{
				emitterStatus ^= 1;
				cameraInterface.setEmitterOptions(cameraDevice, emitterStatus, 100.0f);
			}
			ImGui::SameLine();	ImGui::Checkbox("", &emitterStatus);

			int prevShift = dispShift;
			ImGui::Text("Disparity");
			ImGui::PushItemWidth(-1);
			ImGui::SliderInt("Disparity", &dispShift, 0, 300);
			if (prevShift != dispShift)
			{
				//////////////////////////kcamera.setDepthControlGroupValues(0, 0, 0, 0, (uint32_t)dispShift); // TODO make this work with depth min and depth max
				cameraInterface.setDepthTable(cameraDevice, 10000, 0, 100, 0, dispShift);
			}
			ImGui::PopItemWidth();


		}


		static bool openFileDialog = false;

		/*if (ImGui::Button("Open File Dialog"))
		{
			openFileDialog = true;
		}

		static std::string filePathName = "";
		static std::string path = "";
		static std::string fileName = "";
		static std::string filter = "";

		if (openFileDialog)
		{
			if (ImGuiFileDialog::Instance()->FileDialog("Choose File", ".cpp\0.h\0.hpp\0\0", ".", ""))
			{
				if (ImGuiFileDialog::Instance()->IsOk == true)
				{
					filePathName = ImGuiFileDialog::Instance()->GetFilepathName();
					path = ImGuiFileDialog::Instance()->GetCurrentPath();
					fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();
					filter = ImGuiFileDialog::Instance()->GetCurrentFilter();
				}
				else
				{
					filePathName = "";
					path = "";
					fileName = "";
					filter = "";
				}
				openFileDialog = false;
			}
		}

		if (filePathName.size() > 0) ImGui::Text("Choosed File Path Name : %s", filePathName.c_str());
		if (path.size() > 0) ImGui::Text("Choosed Path Name : %s", path.c_str());
		if (fileName.size() > 0) ImGui::Text("Choosed File Name : %s", fileName.c_str());
		if (filter.size() > 0) ImGui::Text("Choosed Filter : %s", filter.c_str());*/



		ImGui::Separator();
		if (ImGui::CollapsingHeader("Fusion"))
		{
			ImGui::Text("Mode");

			if (ImGui::Button("P2P")) trackDepthToPoint ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &trackDepthToPoint); ImGui::SameLine();
			if (ImGui::Button("P2V")) trackDepthToVolume ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &trackDepthToVolume);
			if (ImGui::Button("Splat")) useSplatter ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &useSplatter);
			ImGui::Text("Prealignment");

			if (ImGui::Button("RGB")) useSO3 ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &useSO3); ImGui::SameLine();
			if (ImGui::Button("RGB+D")) useSE3 ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &useSE3); 

			ImGui::Text("combo");
			if (ImGui::Button("odop2p")) useODOP2P ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &useODOP2P); ImGui::SameLine();
			if (ImGui::Button("odosplat")) useODOSplat ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &useODOSplat);


			ImGui::Text("Resolution");
			ImGui::PushItemWidth(-1);
			float avail_width = ImGui::CalcItemWidth();
			float label_width = ImGui::CalcTextSize(" X ").x;
			ImGui::PopItemWidth();
			ImGui::PushItemWidth((avail_width / 3) - label_width);
			ImGui::Combo("X  ", &sizeX, sizes, IM_ARRAYSIZE(sizes)); ImGui::SameLine(0.0f, 0.0f);
			ImGui::Combo("Y  ", &sizeX, sizes, IM_ARRAYSIZE(sizes)); ImGui::SameLine(0.0f, 0.0f);
			ImGui::Combo("Z  ", &sizeX, sizes, IM_ARRAYSIZE(sizes));
			sizeY = sizeX;
			sizeZ = sizeX;
			ImGui::PopItemWidth();

			ImGui::Text("Length");
			ImGui::PushItemWidth(-1);
			ImGui::SliderFloat("Length", &dimension, 0.005f, 2.0f);
			ImGui::PopItemWidth();

			ImGui::Text("Volume Options");

			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 1.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.7f, 0.7f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.8f, 0.8f));

			if (ImGui::Button("Reset Volume"))
			{
				resetVolume();
			}
			ImGui::PopStyleColor(3);


			if (ImGui::Button("Integrate")) integratingFlag ^= 1; ImGui::SameLine();
			ImGui::Checkbox("", &integratingFlag);
			krender.setSelectInitPose(integratingFlag);

			if (ImGui::Button("save stl"))
			{
				saveSTL();
			}
			if (ImGui::Button("save splatter"))
			{
				saveSplatter();
			}
		}
		ImGui::Separator();
		if (ImGui::CollapsingHeader("Image"))
		{
			ImGui::Text("Image Processing");


			ImGui::PushItemWidth(-1);
			ImGui::SliderFloat("sharpness", &sharpnessValue, 0.0f, 1.0f);
			ImGui::PopItemWidth();

			if (ImGui::Button("Use Sharp")) useSharp ^= 1; ImGui::SameLine();
			ImGui::Checkbox("", &useSharp);



			if (ImGui::Button("Pose"))
			{
				if (cameraRunning)
				{
					//opwrapper.start();
				}
				else
				{
					// make a popup?
				}

			}

			if (ImGui::Button("Flood"))
			{
				performFlood ^= 1;
				if (performFlood)
				{
					//gflood.setVolumeConfig(gconfig.volumeSize.x, gconfig.volumeDimensions.x);

					//gflood.allocateTextures();

					krender.setFloodTexture(gflood->getFloodOutputTexture());
					////krender.setFloodTexture(gflood.getFloodSDFTexture());

					//gflood.setVertices(gfusion.getVerts());
					//gflood.setNormals(gfusion.getNorms());

					GLuint mSize;
					
					gflood->setGlobalMapBuffer(gMap->getGlobalBuffer(mSize));
					gflood->setGlobalMapBufferSize(mSize);

				}
			} ImGui::SameLine(); ImGui::Checkbox("", &performFlood); ImGui::SameLine();
			if (ImGui::Button("Flow"))
			{
				performFlow ^= 1;
				gflow.resetTimeElapsed();

			}ImGui::SameLine(); ImGui::Checkbox("", &performFlow);

			//if (ImGui::Button("Stereo"))
			//{
			//	mTracker.setNumberOfCameras(numberOfCameras);
			//	mTracker.setCameraDevice(cameraDevice);
			//	mTracker.setupAruco();

			//	for (int i = 0; i < numberOfCameras; i++)
			//	{
			//		//mTracker.setCamPams(i, cameraInterface.getDepthIntrinsics(i).fx,
			//		//	cameraInterface.getDepthIntrinsics(i).fy,
			//		//	cameraInterface.getDepthIntrinsics(i).cx,
			//		//	cameraInterface.getDepthIntrinsics(i).cy,
			//		//	depthFrameSize[i].x,
			//		//	depthFrameSize[i].y);

			//		mTracker.setCamPams(i, cameraInterface.getColorIntrinsics(i).fx,
			//			cameraInterface.getColorIntrinsics(i).fy,
			//			cameraInterface.getColorIntrinsics(i).cx,
			//			cameraInterface.getColorIntrinsics(i).cy,
			//			colorFrameSize[i].x,
			//			colorFrameSize[i].y);
			//	}
			//	performStereo ^= 1;

			//}ImGui::SameLine(); ImGui::Checkbox("", &performStereo);

			//if (performStereo)
			//{
			//	if (ImGui::Button("Capture"))
			//	{
			//		rs2::frame infraFrame0, infraFrame1;
			//		cv::Mat irMat0, irMat1;
			//		if (cameraInterface.getInfraQueues()[0].poll_for_frame(&infraFrame0))
			//		{
			//			irMat0 = cv::Mat(infraFrameSize[0].y, infraFrameSize[0].x, CV_8UC1, (void*)infraFrame0.get_data());
			//		}
			//		if (cameraInterface.getInfraQueues()[1].poll_for_frame(&infraFrame1))
			//		{
			//			irMat1 = cv::Mat(infraFrameSize[1].y, infraFrameSize[1].x, CV_8UC1, (void*)infraFrame1.get_data());
			//		}
			//		mTracker.setStereoPair(irMat0, irMat1);
			//	}

			//	if (ImGui::Button("StereoCalibrate"))
			//	{
			//		mTracker.stereoCalibrate(depthToDepth);

			//		std::cout << glm::to_string(depthToDepth) << std::endl;
			//	}
			//}



			//if (ImGui::Button("Aruco"))
			//{
			//	performAruco ^= 1;
			//	if (performAruco)
			//	{
			//		mTracker.setNumberOfCameras(numberOfCameras);
			//		mTracker.setCameraDevice(cameraDevice);
			//		mTracker.setupAruco();

			//		for (int i = 0; i < numberOfCameras; i++)
			//		{
			//			//mTracker.setCamPams(i, cameraInterface.getDepthIntrinsics(i).fx,
			//			//	cameraInterface.getDepthIntrinsics(i).fy,
			//			//	cameraInterface.getDepthIntrinsics(i).cx,
			//			//	cameraInterface.getDepthIntrinsics(i).cy,
			//			//	depthFrameSize[i].x,
			//			//	depthFrameSize[i].y);

			//			mTracker.setCamPams(i, cameraInterface.getColorIntrinsics(i).fx,
			//				cameraInterface.getColorIntrinsics(i).fy,
			//				cameraInterface.getColorIntrinsics(i).cx,
			//				cameraInterface.getColorIntrinsics(i).cy,
			//				colorFrameSize[i].x,
			//				colorFrameSize[i].y);
			//		}

			//		mTracker.configGEM();
			//		mTracker.startTracking();
			//	}
			//	else
			//	{
			//		mTracker.stopTracking();
			//	}

			//}
			//ImGui::SameLine(); ImGui::Checkbox("", &performAruco);
			//static int gemOption = gemStatus::STOPPED;

			//if (ImGui::Button("Load Previous"))
			//{
			//	loadPreviousExtrinsicCalibrationFromFile();
			//}
			//if (ImGui::Button("Save Current"))
			//{
			//	saveExtrinsicCalibrationToFile();
			//}
			////static int gemOpt = 0;
			//if (performAruco)
			//{



			//	ImGui::RadioButton("Stopped", &gemOption, 0); ImGui::SameLine();
			//	ImGui::RadioButton("Collect", &gemOption, 1); ImGui::SameLine();
			//	ImGui::RadioButton("AutoCalibrate", &gemOption, 2); ImGui::SameLine();
			//	ImGui::RadioButton("Track", &gemOption, 3);

			//	mTracker.setGemOption(gemOption);

			//	if (ImGui::Button("Calibrate"))
			//	{
			//		mTracker.calibrate();
			//		gemOption = gemStatus::TRACKING;
			//	}
			//	ImGui::SameLine();
			//	if (ImGui::Button("Clear Calibration"))
			//	{
			//		mTracker.clearCalibration();
			//		gemOption = gemStatus::STOPPED;
			//	}
			//	ImGui::SameLine();

			//	if (ImGui::Button("Export Calibration")) mTracker.exportCalibration();


			//	if (ImGui::Button("Detect Marker Pairs"))
			//	{
			//		//mTracker.detectPairs();
			//		gemOption = gemStatus::PAIRING;
			//	}



			//}



		}








		ImGui::Separator();
		if (ImGui::CollapsingHeader("Display"))
		{
			ImGui::Text("View Options");

			ImGui::Text("texLevel");
			ImGui::PushItemWidth(-1);
			ImGui::SliderInt("texLevel", &texLevel, 0, 10);
			ImGui::PopItemWidth();

			if (ImGui::Button("Depth")) showDepthFlag ^= 1; ImGui::SameLine();	ImGui::Checkbox("", &showDepthFlag);
			ImGui::SameLine();
			if (ImGui::Button("Filter")) showDepthFilteredFlag ^= 1; ImGui::SameLine();	ImGui::Checkbox("", &showDepthFilteredFlag);
			ImGui::SameLine();
			if (ImGui::Button("Color")) showColorFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showColorFlag);

			if (ImGui::Button("Infra")) showInfraFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showInfraFlag);

			if (ImGui::Button("Flood##show")) showSDFVolume ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showSDFVolume);
			ImGui::SameLine();
			if (ImGui::Button("Flow##show")) showFlowFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showFlowFlag);


			if (ImGui::Button("Model")) showNormalFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showNormalFlag);
			ImGui::SameLine();
			if (ImGui::Button("Volume")) showVolumeFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showVolumeFlag);

			if (ImGui::Button("Track##show")) showTrackFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showTrackFlag);
			ImGui::SameLine();
			if (ImGui::Button("Marker")) showMarkerFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showMarkerFlag);

			if (ImGui::Button("Points")) showPointFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showPointFlag);

			if (ImGui::Button("Splat D")) showDepthSplatFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showDepthSplatFlag);
			ImGui::SameLine();
			if (ImGui::Button("Splat N")) showNormalSplatFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showNormalSplatFlag);


			ImGui::Text("slice");
			ImGui::PushItemWidth(-1);
			ImGui::SliderFloat("slice", &volSlice, 0, gconfig.volumeSize.z - 1);
			ImGui::PopItemWidth();

		}

		ImGui::Separator();
		if (ImGui::CollapsingHeader("Misc"))
		{

			ImGui::SliderFloat("vFOV", &vertFov, 1.0f, 90.0f);
			krender.setFov(vertFov);


			ImGui::SliderFloat("xRot", &xRot, 0.0f, 90.0f);
			ImGui::SliderFloat("yRot", &yRot, 0.0f, 90.0f);
			ImGui::SliderFloat("zRot", &zRot, 0.0f, 90.0f);




			ImGui::Separator();
			ImGui::Text("Infrared Adj.");

			//cv::imshow("irg", infraGrey);

			//if (ImGui::Button("Save Infra")) OCVStuff.saveImage(1);  // saving infra image (flag == 1)
			ImGui::SliderFloat("depthMin", &depthMin, 0.0f, 10.0f);
			/*if (irLow > (irHigh - 255.0f))
			{
			irHigh = irLow + 255.0f;
			}*/
			ImGui::SliderFloat("depthMax", &depthMax, 0.0f, 10.0f);
			//if (irHigh < (irLow + 255.0f))
			//{
			//	irLow = irHigh - 255.0f;
			//}
			krender.setDepthMinMax(depthMin, depthMax);
			int laserPower = 10;

			

		}


		ImGui::End();

	}

	ImGuiIO& io = ImGui::GetIO();
	if (ImGui::IsMouseClicked(0) && imguiFocusLW == true)
	{
		int devNumber = 0;
		ImVec2 mPos = ImGui::GetMousePos();
		mousePos.x = mPos.x;
		mousePos.y = mPos.y;
		//std::cout << "from imgui " << mousePos.x << " " << mousePos.y << std::endl;
		//std::cout << "lalala " << mousePos.x - controlPoint0.x << " " << controlPoint0.y - mousePos.y << std::endl;
		//iOff = initOffset(devNumber, mousePos.x - controlPoint0.x, controlPoint0.y - mousePos.y);

		m_pointX = float(mousePos.x - controlPoint0.x) * (float(depthFrameSize[devNumber].x) / float(display2DWindow.w));
		m_pointY = depthFrameSize[devNumber].y - float(controlPoint0.y - mousePos.y) * (float(depthFrameSize[devNumber].y) / float(display2DWindow.h));

		//gfusion.setClickedPoint(m_pointX, m_pointY);

	}
}

void App::setUpGPU()
{
	//gFusionInit();
	mCubeInit();
	
	//krender.setBuffersFromMarchingCubes(gfusion.getVertsMC(), gfusion.getNormsMC(), gfusion.getNumVerts());

	gDisOptFlowInit();

	kRenderInit();

	gFloodInit();

	
	initSplatter();
	
	initP2PFusion();

	initP2VFusion();

	initGradient();

	initDTAM();

	initRGBodo();


}

int App::getRenderOptions(bool depth, bool normal, bool color, bool infra, bool flow)
{
	int opts = depth << 0 |
		normal << 1 |
		color << 2 |
		infra << 3 |
		flow << 4;

	return opts;
}

void App::renderGlobal(bool reset)
{
	bool imguiFocus = ImGui::IsAnyItemActive();

	//std::cout << "focus : " << imguiFocusLW << std::endl;

	ImGuiIO& io = ImGui::GetIO();
	if (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1) && imguiFocus == false)
	{
		ImVec2 mPos = ImGui::GetMousePos();
		mousePos.x = mPos.x;
		mousePos.y = mPos.y;
	}

	if (ImGui::IsMouseDragging(0) && imguiFocusRW == true)
	{
		// get mouse dragging states and pixel locations
		ImVec2 mPos = ImGui::GetMousePos();

		rotation.x += (mousePos.y - mPos.y) * 1.25f * 0.1f; // 1.0f == rotation speed change for faster 
		rotation.y -= (mousePos.x - mPos.x) * 1.25f * 0.1f;
		//renderer.setRotation(rotation);

		//camera->setRotation(rotation);
		mousePos.x = mPos.x;
		mousePos.y = mPos.y;

		

	}

	if (ImGui::IsMouseDragging(1) && imguiFocus == false)
	{
		ImVec2 mPos = ImGui::GetMousePos();

		cameraPos.x -= (mousePos.x - mPos.x) * 0.01f;
		cameraPos.y += (mousePos.y - mPos.y) * 0.01f;
		
		//camera->setPosition(cameraPos);
		//renderer.setCameraPos(cameraPos);
		mousePos.x = mPos.x;
		mousePos.y = mPos.y;
	}



	if (io.MouseWheel != 0.0f && imguiFocusRW == true)
	{
		cameraPos.z += io.MouseWheel * 0.01f;
	}

	glm::mat4 rotM = glm::mat4(1.0f);
	glm::mat4 transM(1.0f);

	rotM = glm::rotate(rotM, glm::radians(rotation.x), glm::vec3(-1.0f, 0.0f, 0.0f));
	rotM = glm::rotate(rotM, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	rotM = glm::rotate(rotM, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	transM = glm::translate(transM, cameraPos);


	glm::mat4 view = transM * rotM;


	//camera->update(0.01f);

	gMap->genVirtualFrame(frame[rgbd::FRAME::GLOBAL], view);
	//slam.renderGlobalMap(view, frame[rgbd::FRAME::GLOBAL]);


}

void App::getIncrementalTransform()
{
	bool slammed = false;
	bool tracked = false;
	bool so3Tracked = false;
	bool se3Tracked = false;

	glm::mat4 prealignPose = glm::mat4(1.0f);

	// prealignment using color (or perhaps inertial sensor?)
	if (useSO3)
	{
		so3Tracked = runDTAM(prealignPose);
	}

	// if use color rgbd odometery
	if (useSE3)
	{
		se3Tracked = runRGBOdo(prealignPose);
	}

	if (trackDepthToPoint)
	{
		tracked = runP2P(prealignPose);
	}

	if (trackDepthToVolume)
	{
		tracked = runP2V(prealignPose);
	}

	if (useSplatter)
	{
		glEnable(GL_CULL_FACE);

		slammed = runSLAM(prealignPose);

		glDisable(GL_CULL_FACE);

		//runDTAM();
	}

	if (useODOP2P)
	{
		so3Tracked = runDTAM(prealignPose);

		runOdoP2P(prealignPose);
	}

	if (useODOSplat)
	{
		glEnable(GL_CULL_FACE);

		//so3Tracked = runDTAM(prealignPose);

		runOdoSplat(prealignPose);

		glDisable(GL_CULL_FACE);

	}
}

void App::mainLoop()
{

#ifdef USE_TORCH
	torch::Tensor tensor = torch::rand({ 2, 3 });
	std::cout << "my first c++ torch tensor " << tensor << std::endl;

	vision::models::ResNet18 network;
	torch::load(network, "./resources/resnet18_python.pt");

	torch::save(network, "./resources/resnet18_cpp_NEW.pt");

	torch::Tensor inputs;
	inputs = torch::rand({ 1, 3, 224, 224 });

	std::vector<torch::jit::IValue> input;
	input.push_back(inputs);
	network->forward(inputs);

	//std::cout << "my second c++ torch tensor " << outputs << std::endl;

#endif






	int display_w, display_h;
	// load openGL window
	//window = krender.loadGLFWWindow();

	glfwGetFramebufferSize(window, &display_w, &display_h);

	controlPoint0.x = display_w / 4.0f;
	controlPoint0.y = display_h - display_h / 3.0f;

	// Setup ImGui binding
	ImGui::CreateContext();
	ImGui_ImplGlfwGL3_Init(window, true);
	ImVec4 clear_color = ImColor(114, 144, 154);

	setUIStyle();
	setImguiWindows();


	//cv::Ptr<cv::DenseOpticalFlow> algorithm = cv::optflow::createOptFlow_DIS(cv::optflow::DISOpticalFlow::PRESET_MEDIUM);

	//cv::Mat prevgray, gray, graySmall, rgb, frame;
	//cv::Mat I0x = cv::Mat(1080, 1920, CV_16SC1);
	//cv::Mat I0y = cv::Mat(1080, 1920, CV_16SC1);
	//cv::Mat flow, flow_uv[2];
	//cv::Mat mag, ang;
	//cv::Mat hsv_split[3], hsv;
	//char ret;

	//cv::namedWindow("flow", 1);
	//cv::namedWindow("orig", 1);

	const int samples = 50;
	float time[samples];
	int index = 0;

	bool newFrame = false;
	bool show_slider_graph = true;

	float midDepth1 = 0.0f;
	float midDepth2 = 0.0f;
	float midDepth3 = 0.0f;

	std::stringstream filenameSS;
	filenameSS << "data/motion/motion_" << return_current_time_and_date() << ".txt";
	std::ofstream outputFile(filenameSS.str(), std::ios::out | std::ios::app);

	uint64_t previousTime = 0;// ();

	bool frameReady = false;

	cv::Mat colMat, infraMat, depthMat;
	bool slammed = false;
	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		glfwGetFramebufferSize(window, &display_w, &display_h);

		//// Rendering
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);






		if (usingDataFromFile)
		{
			frameReady = cameraInterface.collateFramesFromFile();
		}
		else
		{
			frameReady = cameraInterface.collateFrames();
		}

		if (frameReady)
		{
			updateFrames();




			//gfusion.splatterModel();

			//gfusion.resetTimes();

			//auto eTime = epchTime();
			// THIS SHOULD ONLY BE COPIED WHENEVER THERES A NEW COLOR FRAME, NOT EVERY DEPTH FRAME
			//krender.setColorFrame(cameraInterface.getColorQueues(), cameraDevice, colMat);

			//krender.setInfraFrame(cameraInterface.getInfraQueues(), cameraDevice, infraMat);

			//if ()

			//if (!emitterStatus) {
				getIncrementalTransform();
			//}
			//else {

			//}

			//emitterStatus = !emitterStatus;

			//cameraInterface.setEmitterOptions(cameraDevice, emitterStatus, 100.0f);

			if (colorVec.size() > 0)
			{
				//cv::Mat cMat = cv::Mat(480, 848, CV_8UC4, colorVec.data());
				//cv::imshow("col", cMat);
				//cv::waitKey(1);
				//opwrapper.setImage(cMat, 0);
			}

			//if (performAruco)
			//{
			//	mTracker.useGEM();

			//	mTracker.getMarkerData(markerMats);

			//	krender.setColorToDepth(colorToDepth[cameraDevice]);
			//	krender.setDepthToColor(depthToColor[cameraDevice]);

			//	if (markerMats.size() > 0)
			//	{
			//		//if (mTracker.gemStatus == gemStatus::TRACKING)
			//		//{
			//		//	krender.setMarkerData(tMat);

			//		//}
			//		//else
			//		//{
			//		krender.setMarkerData(markerMats);
			//		//}
			//	}
			//}

			//if (numberOfCameras > 1 && mTracker.getGemStatus() == 5)
			//{
			//	//cam2camTrans = mTracker.getCam2CamTransform();

			//	//glm::mat4 test0 = colorToDepth[0] * cam2camTrans * depthToColor[1];
			//	//glm::mat4 test1 = colorToDepth[1] * cam2camTrans * depthToColor[0];
			//	//glm::mat4 test01 = colorToDepth[0] * glm::inverse(cam2camTrans) * depthToColor[1];
			//	//glm::mat4 test11 = colorToDepth[1] * glm::inverse(cam2camTrans) * depthToColor[0];

			//	//std::cout << glm::to_string(test0) << std::endl;
			//	//std::cout << glm::to_string(test1) << std::endl;
			//	//std::cout << glm::to_string(test01) << std::endl;
			//	//std::cout << glm::to_string(test11) << std::endl;

			//	//gfusion.setColorToColor(cam2camTrans);
			//	//gfusion.setDepthToDepth(test0);
			//	//krender.setDepthToDepth(test0);
			//	if (cameraDevice == 0)
			//	{
			//		krender.setOtherMarkerData(markerMats);
			//	}
			//}

			double currentTime = epchTime();
			double deltaTime = currentTime - previousTime;
			previousTime = currentTime;
			//std::cout << (int)(deltaTime) << std::endl;

			if (performFlow)
			{
				//gflow.setTexture();
				//gflow.setColorTexture(cameraInterface.getColorQueues(), col);

				if (frame[rgbd::FRAME::CURRENT].getColorTime() > previousColorTime) // WILL THIS OVERFOW?
				{
					previousColorTime = frame[rgbd::FRAME::CURRENT].getColorTime();

					if (useSharp)
					{
						//gflow.setFrameTexture(frame[rgbd::FRAME::CURRENT].getColorFilteredMap());
					}
					else
					{
						//gflow.setFrameTexture(frame[rgbd::FRAME::CURRENT].getColorMap());
					}

					// gradient is from old image
					gradFilter.execute(frame[rgbd::FRAME::CURRENT].getColorPreviousMap(), 0, 3.0f, 10.0f, false);

					disflow.execute(frame[rgbd::FRAME::CURRENT],
						gradFilter.getGradientMap()
					);
				}



				//gflow.setFrameTexture(frame[rgbd::FRAME::CURRENT].getColorAlignedToDepthMap());
				//gflow.calc(false);


			}



			cv::Mat poses;
			cv::Mat faces;
			std::vector<int> poseIds;
			//opwrapper.getPoses(poses, faces, poseIds);


			if (!poses.empty())
			{
				poseFound = true;
				int person = 0;
				int part = 0; // head/nose?
				if (poses.at<cv::Vec3f>(person, part)[2] > 0)
				{
					//std::cout << poses.at<cv::Vec3f>(person, part)[0] << " " << poses.at<cv::Vec3f>(person, part)[1] << std::endl;
					//gfusion.setClickedPoint(poses.at<cv::Vec3f>(person, part)[0], poses.at<cv::Vec3f>(person, part)[1]);
				}

				cv::Size poseSize = poses.size();
				cv::Size faceSize = faces.size();

				std::vector<std::valarray<float>> bpp(poseSize.height, std::valarray<float>(poseSize.width * 3));
				std::vector<std::valarray<float>> RA(poseSize.height, std::valarray<float>(poseSize.width * 3));
				std::vector<std::valarray<float>> faceVec(faceSize.height, std::valarray<float>(faceSize.width * 3));

				std::vector<glm::vec2> neckPos(poseSize.height);

				for (int person = 0; person < poseSize.height; person++)
				{
					//std::cout << "id : " << poseIds[person] << std::endl;
					for (int part = 0; part < poseSize.width; part++)
					{
						if (poses.at<cv::Vec3f>(person, part)[2] > 0)
						{
							bpp[person][part * 3] = poses.at<cv::Vec3f>(person, part)[0];
							bpp[person][part * 3 + 1] = poses.at<cv::Vec3f>(person, part)[1];
							bpp[person][part * 3 + 2] = poses.at<cv::Vec3f>(person, part)[2];
						}
						else // get the last frames position
						{
							auto it = rollingAverage.find(poseIds[person]);
							if (it != rollingAverage.end())
							{
								bpp[person][part * 3] = it->second[0][part * 3];
								bpp[person][part * 3 + 1] = it->second[0][part * 3 + 1];
								bpp[person][part * 3 + 2] = 0; // reduce the weighting of these points
							}
							else
							{
								bpp[person][part * 3] = 0;
								bpp[person][part * 3 + 1] = 0;
								bpp[person][part * 3 + 2] = 0;
							}

						}

						//std::cout << "x : " << x << " y : " << y << std::endl;

					}

					auto it = rollingAverage.find(poseIds[person]);
					// person already in map
					if (it != rollingAverage.end())
					{
						it->second.push_front(bpp[person]);
						if (it->second.size() > windowWidth)
						{
							it->second.pop_back();
						}

						for (std::deque<std::valarray<float>>::iterator dit = it->second.begin(); dit != it->second.end(); dit++)
						{
							RA[person] += *dit;
						}

						RA[person] /= windowWidth;

						neckPos[person] = glm::vec2(RA[person][1 * 3], RA[person][1 * 3 + 1] + 50.0f);


					}
					else // add person to map
					{
						std::deque<std::valarray<float>> tempbpp(windowWidth, bpp[person]);
						rollingAverage.insert(std::pair<int, std::deque<std::valarray<float>>>(poseIds[person], tempbpp));
					}
					//rollingAverage[poseIDs[person]].push_front(bpp[person]);

					// [person][frame][part]

					//faces
					for (int facePart = 0; facePart < faceSize.width; facePart++)
					{
						if (faces.at<cv::Vec3f>(person, facePart)[2] > 0)
						{
							faceVec[person][facePart * 3] = faces.at<cv::Vec3f>(person, facePart)[0];
							faceVec[person][facePart * 3 + 1] = faces.at<cv::Vec3f>(person, facePart)[1];
							faceVec[person][facePart * 3 + 2] = faces.at<cv::Vec3f>(person, facePart)[2];
						}


						//std::cout << "x : " << x << " y : " << y << std::endl;

					}


				}

				krender.setBodyPosePoints(RA);


			}

			//gfusion.uploadDepthToBuffer(cameraInterface.getDepthQueues(), cameraDevice, iOff, counter);
			//gfusion.runBilateralFilter();
			//gfusion.uploadDepth(cameraInterface.getDepthQueues(), cameraDevice, iOff);
			//gfusion.depthToVertex();
			//gfusion.vertexToNormal();
			//}
		//
		//else
		//{
		//	gfusion.depthToVertex(cameraInterface.getDepthQueues(), cameraDevice, iOff);
			//gfusion.vertexToNormal(cameraDevice);
		//}


			if (performFlood)
			{
				//gflood->setPose(gfusion.getPose());
				if (useSplatter)
				{
					gflood->jumpFloodCalc(currPose);
				}
				else if (trackDepthToPoint)
				{
					gflood->jumpFloodCalc(p2pFusion.getPose());
				}
				else if (trackDepthToVolume)
				{
					gflood->jumpFloodCalc(p2vFusion.getPose());
				}
			}




			//if (tracked && integratingFlag && ((counter % 1) == 0) || reset)
			//{
				//if (useMultipleFusion)
				//{


				//gfusion.integrate(reset); // use this one
				//}
				//else
				//{
				//	gfusion.integrate(cameraDevice);
				//}

			//	if (counter > 2)
			//		reset = false;
			//}

			//if (!tracked)
			//{
			//	if (!poses.empty())
			//	{
				//	glm::mat4 recoveryPose = glm::translate(glm::mat4(1.0f), glm::vec3(-iOff.x + gconfig.volumeDimensions.x / 2.0f, -iOff.y + gconfig.volumeDimensions.y / 2.0f, -iOff.z + dimension / 2.0));


				//	tracked = gfusion.recoverPose(recoveryPose);
			//	}
			//}


















			counter++;
			

			//FOR ONE CAMERA ONLY
			//outputFile << std::to_string(gfusion.getFrameTime()[0]) << " " << gfusion.alignmentEnergy() << " " << cameraInterface.getTemperature()[0];
			glm::mat4 currentPose;

			if (useSplatter || useODOSplat)
			{
				currentPose = currPose;
			}
			else if (trackDepthToPoint)
			{
				//currentPose = p2pFusion.getPose();
			}
			else if (trackDepthToVolume)
			{
				currentPose = p2vFusion.getPose();
			}
			
			//std::cout << cameraInterface.getTemperature()[0] << std::endl;

			outputFile << frame[rgbd::FRAME::CURRENT].getDepthTime() << " " << cameraInterface.getTemperature()[0] << " " << currentPose[0].x << " " << currentPose[0].y << " " << currentPose[0].z << " " << currentPose[0].w << \
				" " << currentPose[1].x << " " << currentPose[1].y << " " << currentPose[1].z << " " << currentPose[1].w << \
				" " << currentPose[2].x << " " << currentPose[2].y << " " << currentPose[2].z << " " << currentPose[2].w << \
				" " << currentPose[3].x << " " << currentPose[3].y << " " << currentPose[3].z << " " << currentPose[3].w << std::endl;

			//graphPoints.push_back(gfusion.getTransPose());
			glm::vec4 transformedInitOff = currentPose * glm::vec4(initOff, 1.0f);

			//glm::vec4 transformedInitOff = colorPose * glm::vec4(0,0,0, 1.0f);
			//glm::vec4 transformedInitOff;
			//if (useSE3)
			//{
			//	transformedInitOff = se3Pose * glm::vec4(initOff, 1.0f);
			//}
			//else if (useSO3)
			//{
			//	transformedInitOff = so3Pose * glm::vec4(0, 0, 100, 1.0f);
			//}

			graphPoints.push_back(transformedInitOff);
			if (graphPoints.size() > graphWindow.w)
			{
				graphPoints.pop_front();
			}

			minmaxX = std::make_pair<float, float>(1000, -1000.f);
			minmaxY = std::make_pair<float, float>(1000, -1000.f);;
			minmaxZ = std::make_pair<float, float>(1000, -1000.f);;

			int idx = 0;
			for (auto i : graphPoints)
			{
				if (i[0] < minmaxX.first) minmaxX.first = i.x;
				if (i[0] > minmaxX.second) minmaxX.second = i.x;

				if (i[1] < minmaxY.first) minmaxY.first = i.y;
				if (i[1] > minmaxY.second) minmaxY.second = i.y;

				if (i[2] < minmaxZ.first) minmaxZ.first = i.z;
				if (i[2] > minmaxZ.second) minmaxZ.second = i.z;

				arrayX[idx] = i.x;
				arrayY[idx] = i.y;
				arrayZ[idx] = i.z;

				idx++;
			}

		}



		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();

		if ((useSplatter || useODOSplat) && cameraRunning)
		{
			renderGlobal(false);
		}


		setUI();


		krender.setRenderingOptions(showDepthFlag, showBigDepthFlag, showInfraFlag, showColorFlag, showLightFlag, showPointFlag, showFlowFlag, showEdgesFlag, showNormalFlag, showVolumeFlag, showTrackFlag, showSDFVolume, showMarkerFlag, showDepthSplatFlag, showNormalSplatFlag);
		krender.setFusionType(trackDepthToPoint, trackDepthToVolume, useSplatter);


		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());





		krender.setDepthImageRenderPosition(vertFov);
		krender.setRayNormImageRenderPosition(vertFov);
		krender.setTrackImageRenderPosition(vertFov);

		krender.setInfraImageRenderPosition();
		krender.setColorImageRenderPosition(vertFov);

		krender.setFlowImageRenderPosition(vertFov);

		krender.setVolumeSDFRenderPosition(volSlice);

		krender.setMarchingCubesRenderPosition(zModelPC_offset);
		krender.setViewMatrix(xRot, yRot, zRot, xTran, yTran, zTran);




		if (cameraRunning)
		{
			GLuint volID = 0;
			if (trackDepthToPoint)
			{
				//volID = p2pFusion.getVolumeID();
			}
			else if (trackDepthToVolume)
			{
				volID = p2vFusion.getVolumeID();
			}
			krender.setProjectionMatrix(cameraDevice);
			//krender.setTextures(showDepthFilteredFlag ? gfusion.getDepthFilteredImage() : gfusion.getDepthImage(), gflow.getColorTexture(), gfusion.getVerts(), gfusion.getNorms(), volID, gfusion.getTrackImage(), gfusion.getPVPNorms(), gfusion.getPVDNorms(), gfusion.getSplatterDepth(), gfusion.getSplatterNormal()); //needs texture uints from gfusion init

			krender.setDisplayOriSize(display2DWindow.x, display_h - display2DWindow.y - display2DWindow.h, display2DWindow.w, display2DWindow.h);
			krender.set3DDisplayOriSize(display3DWindow.x, display_h - display3DWindow.y - display3DWindow.h, display3DWindow.w, display3DWindow.h);
			krender.setColorDisplayOriSize(display3DWindow.x, display_h - display3DWindow.y - display3DWindow.h, colorFrameSize[cameraDevice]);

			krender.Render(false, cameraDevice);
		}
		
		int renderOpts;


			renderOpts = getRenderOptions(showDepthFlag, showNormalFlag, showColorFlag, showInfraFlag, showFlowFlag);

			if (renderOpts > 0 && cameraRunning)
			{
				glDisable(GL_DEPTH_TEST);
				glViewport(display2DWindow.x, display_h - display2DWindow.y - display2DWindow.h, display2DWindow.w, display2DWindow.h);
				//glViewport(display3DWindow.x, display_h - display3DWindow.y - display3DWindow.h, display3DWindow.w, display3DWindow.h);
				progs["ScreenQuad"]->use();
				progs["ScreenQuad"]->setUniform("mapType", int(rgbd::MAP_TYPE::NORMAL));
				progs["ScreenQuad"]->setUniform("renderOptions", renderOpts);
				progs["ScreenQuad"]->setUniform("maxDepth", 10.0f);
				progs["ScreenQuad"]->setUniform("depthRange", glm::vec2(depthMin, depthMax));
				progs["ScreenQuad"]->setUniform("renderType", 0);
				progs["ScreenQuad"]->setUniform("flowType", 0);
				progs["ScreenQuad"]->setUniform("magMulti", 10.0f);

				quad.renderMulti(frame[rgbd::FRAME::CURRENT].getDepthMap(), frame[rgbd::FRAME::VIRTUAL].getNormalMap(), frame[rgbd::FRAME::CURRENT].getColorAlignedToDepthMap(), frame[rgbd::FRAME::CURRENT].getInfraMap(), frame[rgbd::FRAME::CURRENT].getMappingC2DMap(), disflow.getFlowMap());

				//quad.renderMulti(frame[rgbd::FRAME::CURRENT].getDepthMap(), frame[rgbd::FRAME::VIRTUAL].getNormalMap(), frame[rgbd::FRAME::CURRENT].getColorAlignedToDepthMap(), frame[rgbd::FRAME::CURRENT].getInfraMap(), frame[rgbd::FRAME::CURRENT].getMappingC2DMap(), gflow.getFlowTextureFrame());
				progs["ScreenQuad"]->disuse();

				glEnable(GL_DEPTH_TEST);
			}


		if (cameraRunning)
		{
			renderOpts = getRenderOptions(0, showNormalFlag, showColorFlag, 0, showFlowFlag);

			glDisable(GL_DEPTH_TEST);
			glViewport(display3DWindow.x, display_h - display3DWindow.y - display3DWindow.h, display3DWindow.w, display3DWindow.h);
			progs["ScreenQuad"]->use();
			progs["ScreenQuad"]->setUniform("mapType", int(rgbd::MAP_TYPE::NORMAL));
			progs["ScreenQuad"]->setUniform("renderOptions", renderOpts);
			progs["ScreenQuad"]->setUniform("maxDepth", 10.0f);
			progs["ScreenQuad"]->setUniform("depthRange", glm::vec2(depthMin, depthMax));
			progs["ScreenQuad"]->setUniform("renderType", 0);
			progs["ScreenQuad"]->setUniform("flowType", 0);
			progs["ScreenQuad"]->setUniform("level", texLevel);
			progs["ScreenQuad"]->setUniform("magMulti", 1.0f);

			//quad.renderMulti(frame[rgbd::FRAME::GLOBAL].getDepthMap(), frame[rgbd::FRAME::GLOBAL].getNormalMap(), useSharp == 1 ? frame[rgbd::FRAME::CURRENT].getColorFilteredMap() : frame[rgbd::FRAME::CURRENT].getColorMap(), frame[rgbd::FRAME::CURRENT].getInfraMap(), frame[rgbd::FRAME::CURRENT].getMappingMap(), gflow.getFlowTextureFrame());

			quad.renderMulti(frame[rgbd::FRAME::GLOBAL].getDepthMap(), frame[rgbd::FRAME::VIRTUAL].getVertexMap(), useSharp == 1 ? frame[rgbd::FRAME::CURRENT].getColorFilteredMap() : frame[rgbd::FRAME::CURRENT].getColorMap(), frame[rgbd::FRAME::CURRENT].getInfraMap(), frame[rgbd::FRAME::CURRENT].getMappingC2DMap(), gradFilter.getGradientMap());
			progs["ScreenQuad"]->disuse();
			glEnable(GL_DEPTH_TEST);
		}

		//if (poseFound)
		//{
		//	glDisable(GL_DEPTH_TEST);
		//	glViewport(display3DWindow.x, display_h - display3DWindow.y - display3DWindow.h, display3DWindow.w, display3DWindow.h);
		//	progs["ScreenQuad"]->use();
		//	progs["ScreenQuad"]->setUniform("maxDepth", 10.0f);
		//	progs["ScreenQuad"]->setUniform("renderType", 1);


		//	lines.render();
		//	progs["ScreenQuad"]->disuse();
		//	glEnable(GL_DEPTH_TEST);
		//}




		glfwSwapBuffers(window);

	}



	// Cleanup
	// close file writer
	outputFile.close();

	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();
	//kcamera.stop();
	//cameraInterface.stopDevice(cameraDevice);
	cameraInterface.stopDevices();
	glfwTerminate();


}