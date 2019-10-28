#include "trackp2v.h"

namespace rgbd
{
	p2vICP::p2vICP(
		int width,
		int height,
		float distThresh,
		float normThresh,
		const glm::mat4 &K,
		const std::map<std::string, const gl::Shader::Ptr> &progs
	) : progs{ { "p2vTrack", progs.at("p2vTrack") },
			   { "p2vReduce", progs.at("p2vReduce") } }
	{





	}

	p2vICP::~p2vICP()
	{

	}

	void p2vICP::track(
		GLuint gVolID,
		const rgbd::Frame &currentFrame,
		const rgbd::Frame &virtualFrame,
		gl::ShaderStorageBuffer<rgbd::BufferReductionP2V> &ssboRed,
		glm::vec3 volDim,
		glm::vec3 volSize,
		glm::mat4 &T,
		int level
	)
	{
		glm::mat4 invT = glm::inverse(T);

		progs["p2vTrack"]->use();
		progs["p2vTrack"]->setUniform("T", T);
		progs["p2vTrack"]->setUniform("mip", level);

		progs["p2vTrack"]->setUniform("volDim", volDim);
		progs["p2vTrack"]->setUniform("volSize", volSize);

		//progs["p2vTrack"]->setUniform("dMin", dMin);
		//progs["p2vTrack"]->setUniform("dMax", dMax);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, gVolID);

		//glBindImageTexture(5, gVolID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG16F);


		currentFrame.getVertexMap(0)->bindImage(0, level, GL_READ_ONLY);
		currentFrame.getNormalMap(0)->bindImage(1, level, GL_READ_ONLY);

		virtualFrame.getNormalMap(0)->bindImage(2, level, GL_WRITE_ONLY);

		currentFrame.getTrackMap()->bindImage(3, level, GL_WRITE_ONLY);

		ssboRed.bindBase(0);

		glDispatchCompute(divup(currentFrame.getVertexMap(0)->getWidth() >> level, 32), divup(currentFrame.getVertexMap(0)->getHeight() >> level, 32), 1);

		progs["p2vTrack"]->disuse();
	}

	void p2vICP::reduce(
		gl::ShaderStorageBuffer<rgbd::BufferReductionP2V> &ssboRed,
		gl::ShaderStorageBuffer<float> &ssboRedOut,
		const glm::ivec2 &imSize
	)
	{
		progs["p2vReduce"]->use();
		progs["p2vReduce"]->setUniform("imSize", imSize);

		ssboRed.bindBase(0);
		ssboRedOut.bindBase(1);


		glDispatchCompute(8, 1, 1);

		progs["p2vReduce"]->disuse();

	}

	std::vector<float> p2vICP::makeJTJ(
		std::vector<float> v
	)
	{
		// C is a 6 x 6 matrix (essentially)
		// here we copy the triangluar matrix v into C corner, but we do the mirror in here at the same time, because we can??
		std::vector<float> C;
		C.resize(6 * 6);

		C[0] = v[0];	C[1] = v[1];	C[2] = v[2];	C[3] = v[3];	C[4] = v[4];	C[5] = v[5];
		C[6] = v[1];	C[7] = v[6];	C[8] = v[7];	C[9] = v[8];	C[10] = v[9];	C[11] = v[10];
		C[12] = v[2];	C[13] = v[7];	C[14] = v[11];	C[15] = v[12];	C[16] = v[13];	C[17] = v[14];
		C[18] = v[3];	C[19] = v[8];	C[20] = v[12];	C[21] = v[15];	C[22] = v[16];	C[23] = v[17];
		C[24] = v[4];	C[25] = v[9];	C[26] = v[13];	C[27] = v[16];	C[28] = v[18];	C[29] = v[19];
		C[30] = v[5];	C[31] = v[10];	C[32] = v[14];	C[33] = v[17];	C[34] = v[19];	C[35] = v[20];

		return C;
	}

	void p2vICP::getReduction(
		gl::ShaderStorageBuffer<float> &ssboRedOut,
		std::vector<float> &b,
		std::vector<float> &C,
		float &AE,
		uint32_t &icpCount
	)
	{
		outputReductionData.resize(32 * 8);
		ssboRedOut.read(outputReductionData.data(), 0, 32 * 8);

		for (int row = 1; row < 8; row++)
		{
			for (int col = 0; col < 32; col++)
			{
				outputReductionData[col + 0 * 32] = outputReductionData[col + row * 32];
			}
		}


		std::vector<float>::const_iterator first0 = outputReductionData.begin() + 1;
		std::vector<float>::const_iterator last0 = outputReductionData.begin() + 28;

		std::vector<float> vals(first0, last0);

		std::vector<float>::const_iterator first1 = vals.begin();
		std::vector<float>::const_iterator last1 = vals.begin() + 6;
		std::vector<float>::const_iterator last2 = vals.begin() + 6 + 21;

		std::vector<float> bee(first1, last1);
		std::vector<float> Cee = makeJTJ(std::vector<float>(last1, last2));

		b = bee;
		C = Cee;

		AE = sqrt(outputReductionData[0] / outputReductionData[28]);
		icpCount = outputReductionData[28];


	}



	bool p2vICP::calc(
		const int level,
		GLuint gVolID,
		const rgbd::Frame &currentFrame,
		const rgbd::Frame &virtualFrame,
		gl::ShaderStorageBuffer<rgbd::BufferReductionP2V> &ssboRed,
		gl::ShaderStorageBuffer<float> &ssboRedOut,
		Eigen::Matrix4f &T_eigPrev,
		float &AE, 
		uint32_t &icpCount,
		glm::vec3 volDim,
		glm::vec3 volSize,
		Eigen::Matrix<double, 6, 1> &result,
		Eigen::Matrix<double, 6, 1> &resultPrev,
		const float finThresh
	)
	{

		bool tracked = false;
		glm::mat4 dT(1.0f);

		for (int loop = 0; loop < ICPConstParam::MAX_ITR_NUM[level]; ++loop)
		{
			std::vector<float> b, C;
			Eigen::Matrix4f tempT = Twist(result).exp() * T_eigPrev;
			glm::mat4 currT;
			std::memcpy(glm::value_ptr(currT), tempT.data(), 16 * sizeof(float));

			// track
			track(gVolID, currentFrame, virtualFrame, ssboRed, volDim, volSize, currT, level);
			// then reduce
			reduce(ssboRed, ssboRedOut, glm::ivec2( currentFrame.getWidth(level), currentFrame.getHeight(level)));
			// get reduction
			getReduction(ssboRedOut, b, C, AE, icpCount);

			// now solve
			Eigen::Matrix<float, 6, 1> b_icp(b.data());
			Eigen::Matrix<float, 6, 6, Eigen::RowMajor> C_icp(C.data());

			//Eigen::Matrix<double, 6, 1> result;
			Eigen::Matrix<double, 6, 6, Eigen::RowMajor> dC_icp = C_icp.cast<double>();
			Eigen::Matrix<double, 6, 1> db_icp = b_icp.cast<double>();

			double scaling = 1.0 / (dC_icp.maxCoeff() > 0 ? dC_icp.maxCoeff() : 1.0);

			dC_icp *= scaling;
			db_icp *= scaling;

			dC_icp = dC_icp + (double(loop)*1.0)*Eigen::MatrixXd::Identity(6, 6);

			result = result - dC_icp.ldlt().solve(db_icp);

			Eigen::Matrix<double, 6, 1> Change = result - resultPrev;
			auto Cnorm = Change.norm();

			resultPrev = result;

			//std::cout << "cnorms : " << Cnorm << std::endl;

			if (Cnorm < 1e-4 && AE != 0)
			{
				tracked = true;
				break;
			}
			else
			{
				tracked = false;
			}
		}

		return tracked;
	}

}