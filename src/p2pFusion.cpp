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
		progs.insert(std::make_pair("CalcVertexMap", std::make_shared<gl::Shader>(folderPath + "CalcVertexMap.comp")));
		progs.insert(std::make_pair("CalcNormalMap", std::make_shared<gl::Shader>(folderPath + "CalcNormalMap.comp")));
		progs.insert(std::make_pair("DownSamplingC", std::make_shared<gl::Shader>(folderPath + "DownSamplingC.comp")));
		progs.insert(std::make_pair("DownSamplingD", std::make_shared<gl::Shader>(folderPath + "DownSamplingD.comp")));
		progs.insert(std::make_pair("DownSamplingV", std::make_shared<gl::Shader>(folderPath + "DownSamplingV.comp")));
		progs.insert(std::make_pair("DownSamplingN", std::make_shared<gl::Shader>(folderPath + "DownSamplingN.comp")));
		progs.insert(std::make_pair("p2pTrack", std::make_shared<gl::Shader>(folderPath + "p2pTrack.comp")));
		progs.insert(std::make_pair("p2pReduce", std::make_shared<gl::Shader>(folderPath + "p2pReduce.comp")));

	}
	void p2pFusion::init(
		const rgbd::Frame &currentFrame,
		const rgbd::Frame &virtualFrame,
		glm::vec3 &dim,
		glm::vec3 &size,
		const float dMin,
		const float dMax,
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


		icp = std::make_shared<rgbd::PyramidricalICP>(width, height, K, rgbd::FUSIONTYPE::P2P, progs, dim, size, dMin, dMax, distThresh, normThresh);
		gVol = std::make_shared<rgbd::GlobalVolume>(width, height, dim, size, dMin, dMax, K, maxWeight, largeStep, step, nearPlane, farPlane, progs);

		integrate(currentFrame);

		raycast(virtualFrame);

	}

	glm::mat4 p2pFusion::calcDevicePose(
		const rgbd::Frame &currentFrame,
		const rgbd::Frame &virtualFrame
	)
	{
		icp->calcP2P(currentFrame, virtualFrame, T);
		//vT.push_back(vT.back() * T);
		vT.push_back(T);

		return vT.back();
	}

	void p2pFusion::integrate(
		const rgbd::Frame &currentFrame
	)
	{
		gVol->integrate(currentFrame, vT.back());
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

	void p2pFusion::clear(const glm::mat4 & resetPose)
	{
		gVol->reset();
		vT.resize(1, resetPose);
		T = resetPose;
	}

} // namespace rgbd
