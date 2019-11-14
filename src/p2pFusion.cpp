#include "p2pFusion.h"

namespace rgbd
{
	p2pFusion::p2pFusion()
	{

	}

	p2pFusion::~p2pFusion()
	{

	}

	void p2pFusion::loadShaders(
		std::map<std::string, const gl::Shader::Ptr> &progs,
		const std::string &folderPath
	)
	{
		progs.insert(std::make_pair("Integrate", std::make_shared<gl::Shader>(folderPath + "integrate.comp")));
		progs.insert(std::make_pair("Raycast", std::make_shared<gl::Shader>(folderPath + "raycast.comp")));
		progs.insert(std::make_pair("BilateralFilter", std::make_shared<gl::Shader>(folderPath + "BilateralFilter.comp")));
		progs.insert(std::make_pair("CASFilter", std::make_shared<gl::Shader>(folderPath + "contrastAdaptiveSharpening.comp")));
		progs.insert(std::make_pair("alignDepthColor", std::make_shared<gl::Shader>(folderPath + "alignDepthColor.comp")));
		progs.insert(std::make_pair("CalcVertexMap", std::make_shared<gl::Shader>(folderPath + "CalcVertexMap.comp")));
		progs.insert(std::make_pair("CalcNormalMap", std::make_shared<gl::Shader>(folderPath + "CalcNormalMap.comp")));
		progs.insert(std::make_pair("DownSamplingC", std::make_shared<gl::Shader>(folderPath + "DownSamplingC.comp")));
		progs.insert(std::make_pair("DownSamplingD", std::make_shared<gl::Shader>(folderPath + "DownSamplingD.comp")));
		progs.insert(std::make_pair("DownSamplingV", std::make_shared<gl::Shader>(folderPath + "DownSamplingV.comp")));
		progs.insert(std::make_pair("DownSamplingN", std::make_shared<gl::Shader>(folderPath + "DownSamplingN.comp")));
		progs.insert(std::make_pair("p2pTrack", std::make_shared<gl::Shader>(folderPath + "p2pTrack.comp")));
		progs.insert(std::make_pair("p2pReduce", std::make_shared<gl::Shader>(folderPath + "p2pReduce.comp")));
		progs.insert(std::make_pair("rgbOdometry", std::make_shared<gl::Shader>(folderPath + "rgbOdometry.comp")));
		progs.insert(std::make_pair("rgbOdometryReduce", std::make_shared<gl::Shader>(folderPath + "rgbOdometryReduce.comp")));
		//progs.insert(std::make_pair("rgbStep", std::make_shared<gl::Shader>(folderPath + "rgbStep.comp")));
		//progs.insert(std::make_pair("rgbStepReduce", std::make_shared<gl::Shader>(folderPath + "rgbStepReduce.comp")));

	}
	void p2pFusion::init(
		rgbd::GlobalVolume::Ptr volume,
		const rgbd::Frame &currentFrame,
		const rgbd::Frame &virtualFrame,
		glm::vec3 &dim,
		glm::vec3 &size,
		const glm::mat4 &initPose,
		const glm::mat4 &K,
		const float maxWeight,
		const float distThresh,
		const float normThresh,
		const float largeStep,
		const float step,
		const float nearPlane,
		const float farPlane,
		const std::map<std::string, const gl::Shader::Ptr> &progs
	)
	{
		int width = currentFrame.getWidth();
		int height = currentFrame.getHeight();
		volSize = size;
		vT.resize(1, initPose);
		T = initPose;


		icp = std::make_shared<rgbd::PyramidricalICP>(width, height, K, rgbd::FUSIONTYPE::P2P, progs, dim, size, distThresh, normThresh);
		
		gVol = volume; // std::make_shared<rgbd::GlobalVolume>(width, height, dim, size, dMin, dMax, K, maxWeight, largeStep, step, nearPlane, farPlane, progs);

		integrate(currentFrame);

		raycast(virtualFrame);

	}

	glm::mat4 p2pFusion::calcDevicePose(
		const rgbd::Frame &currentFrame,
		const rgbd::Frame &virtualFrame
	)
	{
		//GLuint query;
		//glGenQueries(1, &query);
		//glBeginQuery(GL_TIME_ELAPSED, query);

		icp->calcP2P(currentFrame, virtualFrame, T);
		//vT.push_back(vT.back() * T);
		vT.push_back(T);

		//glEndQuery(GL_TIME_ELAPSED);
		//GLuint available = 0;
		//while (!available) {
		//	glGetQueryObjectuiv(query, GL_QUERY_RESULT_AVAILABLE, &available);
		//}
		// elapsed time in nanoseconds
		//GLuint64 elapsed;
		//glGetQueryObjectui64vEXT(query, GL_QUERY_RESULT, &elapsed);
		//std::cout << "p2p time : " << elapsed / 1000000.0 << std::endl;

		return vT.back();
	}

	void p2pFusion::integrate(
		const rgbd::Frame &currentFrame
	)
	{
		// rgbd::FUSIONTYPE::P2P == 1

		gVol->integrate(0, currentFrame, vT.back());
	}

	void p2pFusion::raycast(
		const rgbd::Frame &virtualFrame
	)
	{
		//glm::mat4 invT = glm::inverse(vT.back());
		gVol->raycast(virtualFrame, vT.back());
	}

	GLuint p2pFusion::getVolumeID()
	{
		return gVol->getID();
	}

	void p2pFusion::clear(const glm::mat4 & resetPose, glm::vec3 volumeSize)
	{
		volSize = volumeSize;
		gVol->reset();
		vT.resize(1, resetPose);
		T = resetPose;
	}

	void p2pFusion::setT(
		glm::mat4 pose
	)
	{
		T = pose;
		vT.push_back(T);

	}

} // namespace rgbd
