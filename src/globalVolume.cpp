#include "GlobalVolume.h"

namespace rgbd
{
	GlobalVolume::GlobalVolume(
		const int width,
		const int height,
		glm::vec3 &dim,
		glm::vec3 &size,
		const glm::mat4 &K,
		const float maxWeight,
		const float largeStep,
		const float step,
		const float nearPlane,
		const float farPlane,
		const std::map<std::string, const gl::Shader::Ptr> &progs
	) : width(width), height(height), 
		size(size), dim(dim), K(K),
		progs{ { "Integrate", progs.at("Integrate") },
		{ "Raycast", progs.at("Raycast") } }
	{

		glm::mat4 P = rgbd::calibratedPerspective(
			ICPConstParam::MIN_DEPTH, ICPConstParam::MAX_DEPTH,
			width, height, K[2][0], K[2][1], K[0][0], K[1][1], 0.0f
		);

		volumeTex = std::make_shared<gl::Texture3D>();
		volumeTex->createStorage(1, size.x, size.y, size.z, 2, GL_RG16F, gl::Texture3DType::FLOAT16, true);

		//volumeTex->create(0, size.x, size.y, size.z, 2, gl::Texture3DType::FLOAT16);
		volumeTex->setFiltering(gl::Texture3DFilter::NEAREST);
		volumeTex->setWarp(gl::Texture3DWarp::CLAMP_TO_EDGE);
		glm::mat4 invK = glm::inverse(K);

		this->progs["Integrate"]->setUniform("maxWeight", maxWeight);
		this->progs["Integrate"]->setUniform("K", K);
		this->progs["Integrate"]->setUniform("invK", invK);

	
		this->progs["Raycast"]->setUniform("largeStep", largeStep);
		this->progs["Raycast"]->setUniform("step", step);
		this->progs["Raycast"]->setUniform("nearPlane", nearPlane);
		this->progs["Raycast"]->setUniform("farPlane", farPlane);


	}

	GlobalVolume::~GlobalVolume()
	{
	}

	void GlobalVolume::integrate(
		const int fType,
		const rgbd::Frame &srcFrame,
		const glm::mat4 &T		
	)
	{
		glm::mat4 invT = glm::inverse(T);
		progs["Integrate"]->use();
		progs["Integrate"]->setUniform("T", T);
		progs["Integrate"]->setUniform("invT", invT);
		progs["Integrate"]->setUniform("p2p", fType == 0 ? 1 : 0);
		progs["Integrate"]->setUniform("p2v", fType == 1 ? 1 : 0);
		progs["Integrate"]->setUniform("integrateFlag", 1);
		progs["Integrate"]->setUniform("resetFlag", 0);
		progs["Integrate"]->setUniform("volDim", dim.x);
		progs["Integrate"]->setUniform("volSize", size.x);

		volumeTex->bindImage(0, 0, GL_READ_WRITE, GL_RG16F);

		srcFrame.getVertexMap(0)->bindImage(1, 0, GL_READ_ONLY);
		srcFrame.getTrackMap()->bindImage(2, 0, GL_READ_ONLY);

		glDispatchCompute(volumeTex->getWidth() / 32, volumeTex->getHeight() / 32, 1);

		progs["Integrate"]->disuse();
	}

	void GlobalVolume::raycast(
		const rgbd::Frame & dstFrame,
		const glm::mat4 T
	)
	{
		glm::mat4 view = T * glm::inverse(K);
		progs["Raycast"]->use();
		
		progs["Raycast"]->setUniform("view", view);
		progs["Raycast"]->setUniform("volDim", dim);
		progs["Raycast"]->setUniform("volSize", size);
		
		volumeTex->bindImage(0, 0, GL_READ_ONLY, GL_RG16F);

		dstFrame.getVertexMap(0)->bindImage(1, 0, GL_WRITE_ONLY);
		dstFrame.getNormalMap(0)->bindImage(2, 0, GL_WRITE_ONLY);

		glDispatchCompute(width / 32, height / 32, 1);

		progs["Raycast"]->disuse();
	}


	GLuint GlobalVolume::getID()
	{
		return volumeTex->getID();
	}

	void GlobalVolume::exportVertexPointCloud(std::vector<glm::vec3> &outputVertexData)
	{


	}

	void GlobalVolume::setVolDim(glm::vec3 vDim)
	{
		dim = vDim;
	}

	void GlobalVolume::reset()
	{
		progs["Integrate"]->use();
		progs["Integrate"]->setUniform("integrateFlag", 0);
		progs["Integrate"]->setUniform("resetFlag", 1);

		volumeTex->bindImage(0, 0, GL_READ_WRITE, GL_RG16F);

		glDispatchCompute(volumeTex->getWidth() / 32, volumeTex->getHeight() / 32, 1);

		progs["Integrate"]->disuse();
	}

	void GlobalVolume::resize(glm::vec3 vSize)
	{
		size = vSize;
		volumeTex->deleteTexture();
		volumeTex->createStorage(1, size.x, size.y, size.z, 2, GL_RG16F, gl::Texture3DType::FLOAT16, true);
	}



}