#include "rgbDTAM.h"

namespace rgbd
{
	RGBDtam::RGBDtam() {};
	RGBDtam::~RGBDtam() {};

	void RGBDtam::init(
		int width, 
		int height,
		const std::map<std::string, const gl::Shader::Ptr> &progs
		)
	{


		//: progs{ { "rgbDTAM", progs.at("rgbDTAM") },
		//		  { "rgbDTAMReduce", progs.at("rgbDTAMReduce") } }


		std::vector<BufferReductionDTAM> tmpMapData(width * height);
		std::vector<float> tempOutputData(8 * 32);



		ssboDTAMReduction.bind();
		ssboDTAMReduction.create(tmpMapData.data(), width * height, GL_DYNAMIC_DRAW);
		ssboDTAMReduction.bindBase(0);
		ssboDTAMReduction.unbind();
			
		ssboDTAMReductionOutput.bind();
		ssboDTAMReductionOutput.create(tempOutputData.data(), 8 * 32, GL_DYNAMIC_DRAW);
		ssboDTAMReductionOutput.bindBase(1);
		ssboDTAMReductionOutput.unbind();

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
		bool &tracked
	)
	{
		return glm::mat4(1.0f);
	}
}