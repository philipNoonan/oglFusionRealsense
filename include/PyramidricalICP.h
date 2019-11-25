#pragma once

#include "glhelper.h"

#include "ConstantParameters.h"
#include "IterativeClosestPoint.h"
#include "trackp2p.h" // RENAME ME
#include "trackp2v.h"
#include "GlobalVolume.h"

#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Geometry> 
#include <unsupported/Eigen/MatrixFunctions>
#include "eigen_utils.h"

namespace rgbd
{
	enum FUSIONTYPE
	{
		P2P,
		P2V,
		SPLATTER
	};



	class PyramidricalICP
	{
	private:

		gl::ShaderStorageBuffer<BufferReductionP2V> ssboReduction;
		gl::ShaderStorageBuffer<float> ssboReductionOutput;

		std::vector<rgbd::splatterICP::Ptr> icp;
		//std::vector<rgbd::p2pICP::Ptr> p2picp;
		std::vector<rgbd::p2vICP::Ptr> p2vicp;



		glm::vec3 volDim;
		glm::vec3 volSize;

		Eigen::Matrix4f Twist(const Eigen::Matrix<double, 6, 1> &xi)
		{
			Eigen::Matrix4f M;

			M << 0.0, -xi(2), xi(1), xi(3),
				xi(2), 0.0, -xi(0), xi(4),
				-xi(1), xi(0), 0.0, xi(5),
				0.0, 0.0, 0.0, 0.0;

			return M;
		};

	public:
		PyramidricalICP(
			int width,
			int height,
			const glm::mat4 &K,
			const rgbd::FUSIONTYPE fType,
			const std::map<std::string, const gl::Shader::Ptr> &progs,
			glm::vec3 &volDim = glm::vec3(128.0f),
			glm::vec3 &volSize = glm::vec3(1.0f),
			float distThresh = 0.05f,
			float normThresh = 0.9f
		);
		~PyramidricalICP();

		void calc(
			const rgbd::Frame &prevFrame,
			const rgbd::Frame &currFrame,
			glm::mat4 &T,
			bool &tracked
	);

		void calcP2P(
			const rgbd::Frame &prevFrame,
			const rgbd::Frame &currFrame,
			glm::mat4 &T
		);

		void calcP2V(
			GLuint gVolID,
			const rgbd::Frame &prevFrame,
			const rgbd::Frame &currFrame,
			glm::mat4 &T
		);

		void reset(
			glm::vec3 vDim,
			glm::vec3 vSize
		);



		typedef std::shared_ptr<rgbd::PyramidricalICP> Ptr;
	};
}