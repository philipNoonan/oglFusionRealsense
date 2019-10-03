#include "p2vFusion.h"

namespace rgbd
{
	p2vFusion::p2vFusion()
	{

	}

	p2vFusion::~p2vFusion()
	{

	}

	void p2vFusion::loadShaders(
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
		progs.insert(std::make_pair("p2vTrack", std::make_shared<gl::Shader>(folderPath + "p2vTrack.comp")));
		progs.insert(std::make_pair("p2vReduce", std::make_shared<gl::Shader>(folderPath + "p2vReduce.comp")));

	}
	void p2vFusion::init(
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


		icp = std::make_shared<rgbd::PyramidricalICP>(width, height, K, rgbd::FUSIONTYPE::P2V, progs, dim, size, dMin, dMax, distThresh, normThresh);
		gVol = std::make_shared<rgbd::GlobalVolume>(width, height, dim, size, dMin, dMax, K, maxWeight, largeStep, step, nearPlane, farPlane, progs);
		
		integrate(currentFrame);

		//raycast(virtualFrame);

	}

	glm::mat4 p2vFusion::calcDevicePose(
		const rgbd::Frame &currentFrame,
		const rgbd::Frame &virtualFrame
	)
	{
		icp->calcP2V(gVol->getID(), currentFrame, virtualFrame, T);
		//vT.push_back(vT.back() * T);
		vT.push_back(T);

		return vT.back();
	}

	void p2vFusion::integrate(
		const rgbd::Frame &currentFrame
	)
	{
		gVol->integrate(currentFrame, vT.back());
	}

	void p2vFusion::raycast(
		const rgbd::Frame &virtualFrame
	)
	{
		//glm::mat4 invT = glm::inverse(vT.back());
		gVol->raycast(virtualFrame, vT.back());
	}

	GLuint p2vFusion::getVolumeID()
	{
		return gVol->getID();
	}

	void p2vFusion::clear(const glm::mat4 & resetPose)
	{
		gVol->reset();
		vT.resize(1, resetPose);
		T = resetPose;
	}

} // namespace rgbd
