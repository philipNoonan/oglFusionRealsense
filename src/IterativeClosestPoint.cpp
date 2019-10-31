#include "IterativeClosestPoint.h"

namespace rgbd
{
	PointToPlaneICP::PointToPlaneICP(
		int width,
		int height,
		const glm::mat4 &K,
		const std::map<std::string, const gl::Shader::Ptr> &progs
	) : virtualFrameRenderer(width, height, K, progs.at("VirtualMapGeneration")),
		dataAssoc(width, height, progs),
		progs{ { "p2pTrack", progs.at("p2pTrack") },
			   { "p2pReduce", progs.at("p2pReduce") } }
	{
		glm::mat4 invK = glm::inverse(K);
		this->progs["p2pTrack"]->setUniform("K", K);
		this->progs["p2pTrack"]->setUniform("invK", invK);
		this->progs["p2pTrack"]->setUniform("distThresh", 0.01f); // SET ME PROPERLY
		this->progs["p2pTrack"]->setUniform("normThresh", 0.9f);
		this->progs["p2pTrack"]->setUniform("cam", glm::vec4(K[2][0], K[2][1], K[0][0], K[1][1]));

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

	}

	PointToPlaneICP::~PointToPlaneICP()
	{
	}

	void PointToPlaneICP::track(
		const rgbd::Frame &currentFrame,
		const rgbd::Frame &virtualFrame,
		glm::mat4 &T,
		int level
	)
	{
		glm::mat4 invT = glm::inverse(T);

		progs["p2pTrack"]->use();
		progs["p2pTrack"]->setUniform("T", T);
		progs["p2pTrack"]->setUniform("invT", invT);
		progs["p2pTrack"]->setUniform("mip", level);

		currentFrame.getVertexMap(0)->bindImage(0, level, GL_READ_ONLY);
		currentFrame.getNormalMap(0)->bindImage(1, level, GL_READ_ONLY);

		virtualFrame.getVertexMap(0)->bindImage(2, 0, GL_READ_ONLY);
		virtualFrame.getNormalMap(0)->bindImage(3, 0, GL_READ_ONLY);

		currentFrame.getTrackMap()->bindImage(4, level, GL_WRITE_ONLY);
		currentFrame.getTestMap()->bindImage(5, level, GL_WRITE_ONLY);

		ssboReduction.bindBase(0);

		glDispatchCompute(GLHelper::divup(currentFrame.getVertexMap(0)->getWidth() >> level, 32), GLHelper::divup(currentFrame.getVertexMap(0)->getHeight() >> level, 32), 1);

		progs["p2pTrack"]->disuse();
	}

	void PointToPlaneICP::reduce(
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

	std::vector<float> PointToPlaneICP::makeJTJ(
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

	void PointToPlaneICP::getReduction(
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









	void PointToPlaneICP::paramToMat(
		const cv::Mat &params,
		glm::mat4 &T
	)
	{
		const float a(params.ptr<float>(0)[0]);	// alpha (yaw)
		const float b(params.ptr<float>(1)[0]);	// beta (pitch)
		const float g(params.ptr<float>(2)[0]);	// gamma (roll)
		const float cos_a(cos(a));
		const float cos_b(cos(b));
		const float cos_g(cos(g));
		const float sin_a(sin(a));
		const float sin_b(sin(b));
		const float sin_g(sin(g));

		T[0][0] = cos_g * cos_b;
		T[1][0] = -sin_g * cos_a + cos_g * sin_b * sin_a;
		T[2][0] = sin_g * sin_a + cos_g * sin_b * cos_a;
		T[3][0] = params.ptr<float>(3)[0];
		T[0][1] = sin_g * cos_b;
		T[1][1] = cos_g * cos_a + sin_g * sin_b * sin_a;
		T[2][1] = -cos_g * sin_a + sin_g * sin_b * cos_a;
		T[3][1] = params.ptr<float>(4)[0];
		T[0][2] = -sin_b;
		T[1][2] = cos_b * sin_a;
		T[2][2] = cos_b * cos_a;
		T[3][2] = params.ptr<float>(5)[0];

		// <-- An alternative
		//const float a(params.ptr<float>(0)[0]);	// alpha (yaw)
		//const float b(params.ptr<float>(1)[0]);	// beta (pitch)
		//const float g(params.ptr<float>(2)[0]);	// gamma (roll)
		//glm::mat4 Rx, Ry, Rz;
		//Rx = glm::rotate(a, glm::vec3(1.0f, 0.0f, 0.0f));
		//Ry = glm::rotate(b, glm::vec3(0.0f, 1.0f, 0.0f));
		//Rz = glm::rotate(g, glm::vec3(0.0f, 0.0f, 1.0f));
		//T = Rz * Ry * Rx;
		//T[3][0] = params.ptr<float>(3)[0];
		//T[3][1] = params.ptr<float>(4)[0];
		//T[3][2] = params.ptr<float>(5)[0];
	}

	void PointToPlaneICP::calc(
		const int level,
		const rgbd::Frame &currentFrame,
		const rgbd::Frame &virtualFrame,
		glm::mat4 &T,
		float &alignmentEnergy,
		float &lastICPcount,
		const float finThresh
	)
	{
		glm::mat4 dT(1.0f);
		float AE;
		uint32_t icpCount;
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

			Eigen::JacobiSVD<Eigen::MatrixXd> svd(dC_icp, Eigen::ComputeFullU | Eigen::ComputeFullV);
			result = svd.solve(db_icp); // TODO CHECK THIS WORKS, SHOULD WE MAKE A BACK SUB SOLVER?

			glm::mat4 delta = glm::eulerAngleXYZ(result(3), result(4), result(5));
			delta[3][0] = result(0);
			delta[3][1] = result(1);
			delta[3][2] = result(2);

			T = delta * T;

			//lastA = dC_icp;

			if (result.norm() < 1e-5 && result.norm() != 0)
				break;
		}

		alignmentEnergy = AE;
		lastICPcount = icpCount;

		//std::cout << glm::to_string(glm::transpose(T)) << std::endl;

		//std::cout << "AE : " << AE << " count : " << icpCount << std::endl;

		//glm::mat4 dT(1.0f);
		//for (int loop = 0; loop < ICPConstParam::MAX_ITR_NUM[level]; ++loop)
		//{
		//	virtualFrameRenderer.execute(level, currFrame, T); // do we need this?

		//	cv::Mat assocData(7, 6, CV_32F);
		//	int elemNum = dataAssoc.execute(
		//		level, prevFrame,
		//		virtualFrameRenderer.getVirtualVertexMap(),
		//		virtualFrameRenderer.getVirtualNormalMap(),
		//		(float *)assocData.data
		//	);

		//	if (elemNum >= 6)
		//	{
		//		cv::Mat A(assocData, cv::Rect(0, 0, 6, 6));
		//		cv::Mat b(assocData, cv::Rect(0, 6, 6, 1));
		//		cv::Mat x;
		//		bool bSolved = cv::solve(A, b.t(), x, cv::DECOMP_QR);

		//		paramToMat(x, dT);
		//		T = dT * T;

		//		
		//		for (int i = 0; i < 6; i++)
		//		{
		//			for (int j = 0; j < 6; j++)
		//			{
		//				lastA(i, j) = A.at<float>(i, j);
		//			}
		//		}



		//		float diff = (float)cv::norm(x, cv::NORM_L2);
		//		//std::cout << "diff : " << diff << std::endl;
		//		if (diff < finThresh) break;

		//	}
		//	else
		//	{
		//		break;
		//	}
		//}
	}
}