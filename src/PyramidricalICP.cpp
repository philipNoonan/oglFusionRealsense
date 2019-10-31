#include "PyramidricalICP.h"

namespace rgbd
{
	PyramidricalICP::PyramidricalICP(
		int width,
		int height,
		const glm::mat4 &K,
		const rgbd::FUSIONTYPE fType,
		const std::map<std::string, const gl::Shader::Ptr> &progs, 
		glm::vec3 &vDim,
		glm::vec3 &vSize,
		float distThresh,
		float normThresh
	)
	{
		volDim = vDim;
		volSize = vSize;


		std::vector<BufferReductionP2V> tmpMapData(width * height);
		std::vector<float> tmpOutputData(32 * 8);

		ssboReduction.bind();
		ssboReduction.create(tmpMapData.data(), width * height, GL_DYNAMIC_DRAW);
		ssboReduction.bindBase(0);
		ssboReduction.unbind();

		ssboReductionOutput.bind();
		ssboReductionOutput.create(tmpOutputData.data(), 32 * 8, GL_DYNAMIC_DRAW);
		ssboReductionOutput.bindBase(1);
		ssboReductionOutput.unbind();

		icp.resize(ICPConstParam::MAX_LEVEL);
		p2picp.resize(ICPConstParam::MAX_LEVEL);
		p2vicp.resize(ICPConstParam::MAX_LEVEL);

		for (int lv = 0; lv < ICPConstParam::MAX_LEVEL; ++lv)
		{
			int bias(int(pow(2, lv)));

			glm::mat4 _K(1.0f);
			_K[0][0] = K[0][0] / bias; _K[1][1] = K[1][1] / bias;
			_K[2][0] = (K[2][0] + 0.5f) / bias - 0.5f; _K[2][1] = (K[2][1] + 0.5f) / bias - 0.5f;

			switch (fType)
			{
			case FUSIONTYPE::P2P:
				p2picp[lv] = std::make_shared<rgbd::p2pICP>(width / bias, height / bias, distThresh, normThresh, _K, progs);
				break;
			case FUSIONTYPE::P2V:
				p2vicp[lv] = std::make_shared<rgbd::p2vICP>(width / bias, height / bias, distThresh, normThresh, _K, progs);
				break;
			case FUSIONTYPE::SPLATTER:
				icp[lv] = std::make_shared<rgbd::PointToPlaneICP>(width / bias, height / bias, _K, progs);
				break;
			}
		}
	}

	PyramidricalICP::~PyramidricalICP()
	{
	}

	void PyramidricalICP::calc(
		const rgbd::Frame &prevFrame,
		const rgbd::Frame &currFrame,
		glm::mat4 &T,
		bool &tracked
	)
	{
		Eigen::Matrix<double, 6, 6, Eigen::RowMajor> lastA;
		float ae, icpCount;

		for (int lv = ICPConstParam::MAX_LEVEL - 1; lv >= 0; --lv)
		{


			icp[lv]->calc(lv, prevFrame, currFrame, T, ae, icpCount);


		}
		// || ((icpCount / (currFrame.getWidth() * currFrame.getHeight())) < 0.15)
		//std::cout << "ae : " << ae << " : count : " << icpCount << std::endl;
		if (ae > 2e-2 || isnan(ae) || ae == 0)
		{
		  	tracked = false;
			//std::cout << "tracking lost" << std::endl;
		}

	}

	void PyramidricalICP::calcP2P(
		const rgbd::Frame &currentFrame,
		const rgbd::Frame &virtualFrame,
		glm::mat4 &T
	)
	{
		float AE;
		uint32_t icpCount;

		glm::mat4 oldT = T;



		for (int lv = ICPConstParam::MAX_LEVEL - 1; lv >= 0; --lv)
		{
			p2picp[lv]->calc(lv, currentFrame, virtualFrame, T, AE, icpCount);
		}





		//std::cout << " AE: " << AE << std::endl;

		//T = oldT;

	}



	void PyramidricalICP::calcP2V(
		GLuint gVolID,
		const rgbd::Frame &currentFrame,
		const rgbd::Frame &virtualFrame,
		glm::mat4 &T
	)
	{
		Eigen::Matrix<float, 4, 4, Eigen::ColMajor> T_eig, T_eigPrev;
		std::memcpy(T_eig.data(), glm::value_ptr(T), 16 * sizeof(float));
		T_eigPrev = T_eig;
		float AE;
		uint32_t icpCount;

		glm::mat4 oldT = T;

		Eigen::Matrix<double, 6, 1> result;
		result << 0.0, 0.0, 0.0, 0.0, 0.0, 0.0;
		//glm::mat4 twistMat = Twist(result);
		Eigen::Matrix<double, 6, 1> result_prev = result;

		bool tracked;

		for (int lv = ICPConstParam::MAX_LEVEL - 1; lv >= 0; --lv)
		{
			tracked = p2vicp[lv]->calc(lv, gVolID, currentFrame, virtualFrame, ssboReduction, ssboReductionOutput, T_eig, AE, icpCount, volDim, volSize, result, result_prev);
		}

		std::cout << " p2v AE: " << AE << std::endl;

		if (std::isnan(result.sum())) result << 0.0, 0.0, 0.0, 0.0, 0.0, 0.0;

		//if (tracked)
		//{
			T_eig = Twist(result).exp() * T_eigPrev;

			std::memcpy(glm::value_ptr(T), T_eig.data(), 16 * sizeof(float));



			//updatePoseFinder();

			//m_cumTwist += result;
		//}



		//T = oldT;

	}

	void PyramidricalICP::reset(
		glm::vec3 vDim,
		glm::vec3 vSize
	)
	{
		volDim = vDim;
		volSize = vSize;
	}
}