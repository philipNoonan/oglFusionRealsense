#include "rgbDTAM.h"

namespace rgbd
{
	RGBDtam::RGBDtam() {};
	RGBDtam::~RGBDtam() {};

	void RGBDtam::loadShaders(
		std::map<std::string, const gl::Shader::Ptr> &progs,
		const std::string &folderPath
	)
	{
		progs.insert(std::make_pair("rgbDTAM", std::make_shared<gl::Shader>(folderPath + "rgbDTAM.comp")));
		progs.insert(std::make_pair("rgbDTAMReduce", std::make_shared<gl::Shader>(folderPath + "rgbDTAMReduce.comp")));

	}

	void RGBDtam::init(
		int width,
		int height,
		const std::map<std::string, const gl::Shader::Ptr> &programs
	)
	{


		//: progs{ { "rgbDTAM", progs.at("rgbDTAM") },
		//		  { "rgbDTAMReduce", progs.at("rgbDTAMReduce") } }

		progs = programs;

		std::vector<BufferReductionDTAM> tmpMapData(width * height);
		std::vector<float> tempOutputData(8 * 11);



		ssboSO3.bind();
		ssboSO3.create(tmpMapData.data(), width * height, GL_DYNAMIC_DRAW);
		ssboSO3.bindBase(0);
		ssboSO3.unbind();

		ssboSO3Output.bind();
		ssboSO3Output.create(tempOutputData.data(), 8 * 11, GL_DYNAMIC_DRAW);
		ssboSO3Output.bindBase(1);
		ssboSO3Output.unbind();

	}

	Eigen::Matrix<double, 3, 3, Eigen::RowMajor> RGBDtam::rodrigues(
		const Eigen::Vector3d & src
	)
	{
		Eigen::Matrix<double, 3, 3, Eigen::RowMajor> dst = Eigen::Matrix<double, 3, 3, Eigen::RowMajor>::Identity();

		double rx, ry, rz, theta;

		rx = src(0);
		ry = src(1);
		rz = src(2);

		theta = src.norm();

		if (theta >= DBL_EPSILON)
		{
			const double I[] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };

			double c = cos(theta);
			double s = sin(theta);
			double c1 = 1. - c;
			double itheta = theta ? 1. / theta : 0.;

			rx *= itheta; ry *= itheta; rz *= itheta;

			double rrt[] = { rx*rx, rx*ry, rx*rz, rx*ry, ry*ry, ry*rz, rx*rz, ry*rz, rz*rz };
			double _r_x_[] = { 0, -rz, ry, rz, 0, -rx, -ry, rx, 0 };
			double R[9];

			for (int k = 0; k < 9; k++)
			{
				R[k] = c * I[k] + c1 * rrt[k] + s * _r_x_[k];
			}

			memcpy(dst.data(), &R[0], sizeof(Eigen::Matrix<double, 3, 3, Eigen::RowMajor>));
		}

		return dst;
	}

	glm::mat4 RGBDtam::calcDevicePose(
		const rgbd::Frame &currentFrame,
		const glm::vec4 cam,
		glm::mat4 colorPose,
		bool &tracked
	)
	{
		Eigen::Matrix<double, 3, 3, Eigen::RowMajor> resultR = Eigen::Matrix<double, 3, 3, Eigen::RowMajor>::Identity();

		int pyramidLevel = 0;

		Eigen::Matrix<float, 3, 3, Eigen::RowMajor> R_lr = Eigen::Matrix<float, 3, 3, Eigen::RowMajor>::Identity();

		Eigen::Matrix<double, 3, 3, Eigen::RowMajor> K = Eigen::Matrix<double, 3, 3, Eigen::RowMajor>::Zero();

		K(0, 0) = cam.z / (std::pow(pyramidLevel + 1, 2));
		K(1, 1) = cam.w / (std::pow(pyramidLevel + 1, 2));
		K(0, 2) = cam.x / (std::pow(pyramidLevel + 1, 2));
		K(1, 2) = cam.y / (std::pow(pyramidLevel + 1, 2));
		K(2, 2) = 1;

		float lastError = std::numeric_limits<float>::max() / 2;
		float lastCount = std::numeric_limits<float>::max() / 2;

		Eigen::Matrix<double, 3, 3, Eigen::RowMajor> lastResultR = Eigen::Matrix<double, 3, 3, Eigen::RowMajor>::Identity();

		for (int i = 0; i < 10; i++)
		{
			Eigen::Matrix<float, 3, 3, Eigen::RowMajor> jtj;
			Eigen::Matrix<float, 3, 1> jtr;

			Eigen::Matrix<double, 3, 3, Eigen::RowMajor> homography = K * resultR * K.inverse();

			glm::mat3 imageBasis;

			for (int i = 0; i < 3; ++i)
			{
				for (int j = 0; j < 3; ++j)
				{
					imageBasis[j][i] = homography(i, j);
				}
			}

			Eigen::Matrix<double, 3, 3, Eigen::RowMajor> K_inv = K.inverse();
			glm::mat3 kinv;

			for (int i = 0; i < 3; ++i)
			{
				for (int j = 0; j < 3; ++j)
				{
					kinv[j][i] = K_inv(i, j);
				}
			}

			Eigen::Matrix<double, 3, 3, Eigen::RowMajor> K_R_lr = K * resultR;
			glm::mat3 krlr;

			for (int i = 0; i < 3; ++i)
			{
				for (int j = 0; j < 3; ++j)
				{
					krlr[j][i] = K_R_lr(i, j);
				}
			}



			progs["rgbDTAM"]->use();

			progs["rgbDTAM"]->setUniform("imageBasis", imageBasis);
			progs["rgbDTAM"]->setUniform("kinv", kinv);
			progs["rgbDTAM"]->setUniform("krlr", krlr);

			currentFrame.getColorFilteredMap()->bindImage(0, pyramidLevel, GL_READ_ONLY);
			currentFrame.getColorPreviousMap()->bindImage(1, pyramidLevel, GL_READ_ONLY);

			ssboSO3.bindBase(0);

			glDispatchCompute(GLHelper::divup(currentFrame.getColorFilteredMap()->getWidth(), 32), GLHelper::divup(currentFrame.getColorFilteredMap()->getHeight(), 32), 1);

			progs["rgbDTAM"]->disuse();



			progs["rgbDTAMReduce"]->use();

			progs["rgbDTAMReduce"]->setUniform("imSize", glm::ivec2(currentFrame.getColorFilteredMap()->getWidth(), currentFrame.getColorFilteredMap()->getHeight()));

			ssboSO3.bindBase(0);
			ssboSO3Output.bindBase(1);

			glDispatchCompute(8, 1, 1);

			progs["rgbDTAMReduce"]->disuse();

			// Get Reduction

			std::vector<float> tmpOutputData(8 * 11);

			ssboSO3Output.read(tmpOutputData.data(), 0, 8 * 11);

			for (int row = 1; row < 8; row++)
			{
				for (int col = 0; col < 11; col++)
				{
					tmpOutputData[col + 0 * 11] += tmpOutputData[col + row * 11];
				}
			}

			float vectorB_host[3];
			float matrixA_host[3 * 3];


			/*
			vector b

			| 3  |
			| 6 |
			| 8 |

			and
			matrix a

			| 0  | 1  | 2  |
			| 1  | 4  | 5  |
			| 2  | 5  | 7 |

			*/

			int shift = 0;
			for (int i = 0; i < 3; ++i)
			{
				for (int j = i; j < 4; ++j)
				{
					float value = tmpOutputData[shift++];
					if (j == 3)
						vectorB_host[i] = value;
					else
						matrixA_host[j * 3 + i] = matrixA_host[i * 3 + j] = value;
				}
			}

			std::memcpy(jtj.data(), &matrixA_host[0], 3 * 3 * sizeof(float));
			std::memcpy(jtr.data(), &vectorB_host[0], 1 * 3 * sizeof(float));


			float lastSO3Error = sqrt(tmpOutputData[9] / tmpOutputData[10]);
			float lastSO3Count = tmpOutputData[10];

			//Converged
			if (lastSO3Error < lastError && lastCount == lastSO3Count)
			{
				break;
			}
			else if (lastSO3Error > lastError + 0.001) //Diverging
			{
				lastSO3Error = lastError;
				lastSO3Count = lastCount;
				resultR = lastResultR;
				break;
			}

			lastError = lastSO3Error;
			lastCount = lastSO3Count;
			lastResultR = resultR;

			Eigen::Vector3f delta = jtj.ldlt().solve(jtr);

			Eigen::Matrix<double, 3, 3, Eigen::RowMajor> rotUpdate = rodrigues(delta.cast<double>());

			R_lr = rotUpdate.cast<float>() * R_lr;

			for (int x = 0; x < 3; x++)
			{
				for (int y = 0; y < 3; y++)
				{
					resultR(x, y) = R_lr(x, y);
				}
			}


		}

		glm::mat4 outPose(1.0f);

		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				outPose[j][i] = resultR(i, j);
			}
		}

		return colorPose * outPose;

	}

}