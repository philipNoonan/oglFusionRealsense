#include "trackp2p.h"

namespace rgbd
{
	p2pICP::p2pICP() {};
	p2pICP::~p2pICP() {};

	void p2pICP::loadShaders(
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
	}



	void p2pICP::init(
		int width,
		int height,
		float distThresh,
		float normThresh,
		const glm::mat4 &K,
		const std::map<std::string, const gl::Shader::Ptr> &programs
	)
	{
		progs = programs;

		std::vector<BufferReduction> tmpMapData(width * height);
		std::vector<float> tmpOutputData(32 * 8);

		ssboReduction.bind();
		ssboReduction.create(tmpMapData.data(), width * height, GL_DYNAMIC_DRAW);
		ssboReduction.bindBase(0);
		ssboReduction.unbind();

		ssboReductionOutput.bind();
		ssboReductionOutput.create(tmpOutputData.data(), 32 * 8, GL_DYNAMIC_DRAW);
		ssboReductionOutput.bindBase(1);
		ssboReductionOutput.unbind();

		


		glm::mat4 invK = glm::inverse(K);
		this->progs["p2pTrack"]->setUniform("K", K);
		this->progs["p2pTrack"]->setUniform("invK", invK);
		this->progs["p2pTrack"]->setUniform("distThresh", distThresh);
		this->progs["p2pTrack"]->setUniform("normThresh", normThresh);
	}

	void p2pICP::track(
		const rgbd::Frame &currentFrame,
		const rgbd::Frame &virtualFrame,
		glm::mat4 &T,
		int layer
	)
	{
		glm::mat4 invT = glm::inverse(T);

		progs["p2pTrack"]->use();
		progs["p2pTrack"]->setUniform("T", T);
		progs["p2pTrack"]->setUniform("invT", invT);
		progs["p2pTrack"]->setUniform("mip", layer);

		currentFrame.getVertexMap(0)->bindImage(0, layer, GL_READ_ONLY);
		currentFrame.getNormalMap(0)->bindImage(1, layer, GL_READ_ONLY);

		virtualFrame.getVertexMap(0)->bindImage(2, 0, GL_READ_ONLY);
		virtualFrame.getNormalMap(0)->bindImage(3, 0, GL_READ_ONLY);

		currentFrame.getTrackMap()->bindImage(4, 0, GL_WRITE_ONLY);
		currentFrame.getTestMap()->bindImage(5, 0, GL_WRITE_ONLY);

		ssboReduction.bindBase(0);

		glDispatchCompute((currentFrame.getVertexMap(0)->getWidth() >> layer) / 32, (currentFrame.getVertexMap(0)->getHeight() >> layer) / 32, 1);

		progs["p2pTrack"]->disuse();
	}

	void p2pICP::reduce(
		const glm::ivec2 &imSize
	)
	{
		progs["p2pReduce"]->use();
		progs["p2pReduce"]->setUniform("imSize", imSize);

		ssboReduction.bindBase(0);
		ssboReductionOutput.bindBase(1);


		glDispatchCompute(8, 1, 1);

		progs["p2pReduce"]->disuse();

	}

	std::vector<float> p2pICP::makeJTJ(
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

	void p2pICP::getReduction(
		std::vector<float> &b,
		std::vector<float> &C,
		float &AE,
		uint32_t &icpCount
	)
	{
		outputReductionData.resize(32 * 8);
		ssboReductionOutput.read(outputReductionData.data(), 0, 32 * 8);

		for (int row = 1; row < 8; row++)
		{
			for (int col = 0; col < 32; col++)
			{
				outputReductionData[col + 0 * 32] += outputReductionData[col + row * 32];
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

	void p2pICP::getReduction(
		float *matrixA_host,
		float *vectorB_host,
		float &AE,
		uint32_t &icpCount
	)
	{
		outputReductionData.resize(32 * 8);
		ssboReductionOutput.read(outputReductionData.data(), 0, 32 * 8);

		for (int row = 1; row < 8; row++)
		{
			for (int col = 0; col < 32; col++)
			{
				outputReductionData[col + 0 * 32] += outputReductionData[col + row * 32];
			}
		}

		/*
		vector b
		| 1 |
		| 2 |
		| 3 |
		| 4 |
		| 5 |
		| 6 |

		and
		matrix a
		| 7  | 8  | 9  | 10 | 11 | 12 |
		| 8  | 13 | 14 | 15 | 16 | 17 |
		| 9  | 14 | 18 | 19 | 20 | 21 |
		| 10 | 15 | 19 | 22 | 23 | 24 |
		| 11 | 16 | 20 | 23 | 25 | 26 |
		| 12 | 17 | 21 | 24 | 26 | 27 |

		AE = sqrt( [0] / [28] )
		count = [28]

		*/

		for (int i = 1; i <= 6; i++)
		{
			vectorB_host[i - 1] = outputReductionData[i];
		}

		int shift = 7;
		for (int i = 0; i < 6; ++i)
		{
			for (int j = i; j < 6; ++j)
			{
				float value = outputReductionData[shift++];

				matrixA_host[j * 6 + i] = matrixA_host[i * 6 + j] = value;
			}
		}

		AE = sqrt(outputReductionData[0] / outputReductionData[28]);
		icpCount = outputReductionData[28];

	}

	void p2pICP::calc(
		const int level,
		const rgbd::Frame &currentFrame,
		const rgbd::Frame &virtualFrame,
		glm::mat4 &T,
		float &AE, 
		uint32_t &icpCount,
		const float finThresh
	)
	{


		glm::mat4 dT(1.0f);

		for (int loop = 0; loop < ICPConstParam::MAX_ITR_NUM[level]; ++loop)
		{
			std::vector<float> b, C;

			// track
			track(currentFrame, virtualFrame, T, level);


			// then reduce
			reduce(glm::ivec2(currentFrame.getWidth(level), currentFrame.getHeight(level)));
			// get reduction
			getReduction(b, C, AE, icpCount);

			// now solve
			Eigen::Matrix<float, 6, 1> b_icp(b.data());
			Eigen::Matrix<float, 6, 6, Eigen::RowMajor> C_icp(C.data());
			Eigen::Matrix<double, 6, 1> result;
			Eigen::Matrix<double, 6, 6, Eigen::RowMajor> dC_icp = C_icp.cast<double>();
			Eigen::Matrix<double, 6, 1> db_icp = b_icp.cast<double>();

			//Eigen::JacobiSVD<Eigen::MatrixXd> svd(dC_icp, Eigen::ComputeFullU | Eigen::ComputeFullV);
			//result = svd.solve(db_icp); // TODO CHECK THIS WORKS, SHOULD WE MAKE A BACK SUB SOLVER?

			result = dC_icp.ldlt().solve(db_icp);

			glm::mat4 delta = glm::eulerAngleXYZ(result(3), result(4), result(5));
			delta[3][0] = result(0);
			delta[3][1] = result(1);
			delta[3][2] = result(2);

			T = delta * T;

			if (result.norm() < 1e-5 && result.norm() != 0)
				break;
		}
	}

}