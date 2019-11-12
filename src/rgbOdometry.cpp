#include "rgbOdometry.h"

namespace rgbd
{

	RGBOdometry::RGBOdometry() {};
	RGBOdometry::~RGBOdometry() {};

	void RGBOdometry::loadShaders(
		std::map<std::string, const gl::Shader::Ptr> &progs,
		const std::string &folderPath
	)
	{
		progs.insert(std::make_pair("rgbOdometry", std::make_shared<gl::Shader>(folderPath + "rgbOdometry.comp")));
		progs.insert(std::make_pair("rgbOdometryReduce", std::make_shared<gl::Shader>(folderPath + "rgbOdometryReduce.comp")));
		progs.insert(std::make_pair("rgbOdometryStep", std::make_shared<gl::Shader>(folderPath + "rgbOdometryStep.comp")));
		progs.insert(std::make_pair("rgbOdometryStepReduce", std::make_shared<gl::Shader>(folderPath + "rgbOdometryStepReduce.comp")));

	}

	void RGBOdometry::init(
		int width, 
		int height,
		const std::map<std::string, const gl::Shader::Ptr> &programs
		)
	{
		progs = programs;



		std::vector<BufferReductionRGB> tmpMapData(width * height);
		std::vector<float> tempOutputData(8 * 2);
		std::vector<float> tempJtJData(8 * width * height);
		std::vector<float> tempJtJOutputData(8 * 32);


		ssboRGBReduction.bind();
		ssboRGBReduction.create(tmpMapData.data(), width * height, GL_DYNAMIC_DRAW);
		ssboRGBReduction.bindBase(0);
		ssboRGBReduction.unbind();

		ssboRGBReductionOutput.bind();
		ssboRGBReductionOutput.create(tempOutputData.data(), 8 * 2, GL_DYNAMIC_DRAW);
		ssboRGBReductionOutput.bindBase(1);
		ssboRGBReductionOutput.unbind();

		ssboRGBRGBJtJJtrSE3.bind();
		ssboRGBRGBJtJJtrSE3.create(tempJtJData.data(), 8 * width * height, GL_DYNAMIC_DRAW); // 7 data float 1 inlier float potentially per pixel
		ssboRGBRGBJtJJtrSE3.bindBase(2);
		ssboRGBRGBJtJJtrSE3.unbind();

		ssboRGBRGBJtJJtrSE3Output.bind();
		ssboRGBRGBJtJJtrSE3Output.create(tempJtJOutputData.data(), 8 * 32, GL_DYNAMIC_DRAW);
		ssboRGBRGBJtJJtrSE3Output.bindBase(3);
		ssboRGBRGBJtJJtrSE3Output.unbind();


	}

	Eigen::Matrix<double, 3, 3, Eigen::RowMajor> RGBOdometry::rodrigues(
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

	void RGBOdometry::computeUpdateSE3(
		Eigen::Matrix<double, 4, 4, Eigen::RowMajor> & resultRt,
		const Eigen::Matrix<double, 6, 1> & result,
		Eigen::Isometry3f & rgbOdom
	)
	{
		// for infinitesimal transformation
		Eigen::Matrix<double, 4, 4, Eigen::RowMajor> Rt = Eigen::Matrix<double, 4, 4, Eigen::RowMajor>::Identity();

		Eigen::Vector3d rvec(result(3), result(4), result(5));

		Eigen::Matrix<double, 3, 3, Eigen::RowMajor> R = rodrigues(rvec);

		Rt.topLeftCorner(3, 3) = R;
		Rt(0, 3) = result(0);
		Rt(1, 3) = result(1);
		Rt(2, 3) = result(2);

		resultRt = Rt * resultRt;

		Eigen::Matrix<double, 3, 3, Eigen::RowMajor> rotation = resultRt.topLeftCorner(3, 3);
		rgbOdom.setIdentity();
		rgbOdom.rotate(rotation.cast<float>().eval());
		rgbOdom.translation() = resultRt.cast<float>().eval().topRightCorner(3, 1);
	}

	void RGBOdometry::computeDerivativeImages(
		gl::Texture::Ptr srcNextImage,
		gl::Texture::Ptr dstNextIdxy
	)
	{
		progs["rgbOdometry"]->setUniform("functionID", 0);

		progs["rgbOdometry"]->use();
		srcNextImage->bindImage(0, 0, GL_READ_ONLY);
		dstNextIdxy->bindImage(2, 0, GL_WRITE_ONLY);

		glDispatchCompute(GLHelper::divup(srcNextImage->getWidth(), 32), GLHelper::divup(srcNextImage->getHeight(), 32), 1);
		progs["rgbOdometry"]->disuse();
	}

	void RGBOdometry::computeResiduals(
		const rgbd::Frame &currentFrame,
		const gl::Texture::Ptr & gradientMap,
		const glm::vec3 kT,
		const glm::mat3 krkinv,
		float &sigmaVal,
		float &rgbError
	)
	{
		progs["rgbOdometry"]->setUniform("minScale", 0.05f); // THINK ON THIS ONE!!!!
		progs["rgbOdometry"]->setUniform("maxDepthDelta", 0.07f);
		progs["rgbOdometry"]->setUniform("kt", kT);
		progs["rgbOdometry"]->setUniform("krkinv", krkinv);

		progs["rgbOdometry"]->use();
		currentFrame.getColorPreviousMap()->bindImage(0, 0, GL_READ_ONLY);
		currentFrame.getColorFilteredMap()->bindImage(1, 0, GL_READ_ONLY);

		gradientMap->bindImage(2, 0, GL_READ_ONLY);
		currentFrame.getMappingC2DMap()->bindImage(3, 0, GL_READ_ONLY);
		currentFrame.getMappingD2CMap()->bindImage(4, 0, GL_READ_ONLY);

		currentFrame.getDepthPreviousMap()->bindImage(5, 0, GL_READ_ONLY);
		currentFrame.getDepthMap()->bindImage(6, 0, GL_READ_ONLY);

		currentFrame.getTestMap()->bindImage(7, 0, GL_WRITE_ONLY);

		ssboRGBReduction.bindBase(0);


		glDispatchCompute(GLHelper::divup(currentFrame.getColorMap()->getWidth(), 32), GLHelper::divup(currentFrame.getColorMap()->getHeight(), 32), 1);
		progs["rgbOdometry"]->disuse();

		progs["rgbOdometryReduce"]->use();
		progs["rgbOdometryReduce"]->setUniform("imSize", glm::ivec2(currentFrame.getColorMap()->getWidth(), currentFrame.getColorMap()->getHeight()));

		ssboRGBReduction.bindBase(0);
		ssboRGBReductionOutput.bindBase(1);

		glDispatchCompute(8, 1, 1);
		progs["rgbOdometryReduce"]->disuse();

		std::vector<float> tmpOutputData(8 * 2);
		//std::vector<BufferReductionRGB> tmpMapData(currentFrame.getColorMap()->getWidth() * currentFrame.getColorMap()->getHeight());

		ssboRGBReductionOutput.read(tmpOutputData.data(), 0, 8 * 2);
		//ssboRGBReduction.read(tmpMapData.data(), 0, currentFrame.getColorMap()->getWidth() * currentFrame.getColorMap()->getHeight());

		float sigma = 0;
		float count = 0;

		for (int i = 0; i < 16; i+= 2)
		{
			count += tmpOutputData[i];
			sigma += tmpOutputData[i + 1];
		}

		sigmaVal = std::sqrt((float)sigma / count == 0 ? 1 : count);
		rgbError = std::sqrt(sigma) / (count == 0 ? 1 : count);

		//std::cout << sigmaVal << " " << rgbError << std::endl;
	}

	void RGBOdometry::computeStep(
		const rgbd::Frame &currentFrame,
		const gl::Texture::Ptr &gradientMap,
		const glm::vec4 &cam,
		float sigmaVal,
		float rgbError,
		glm::mat4 &resultRt,
		Eigen::Isometry3f &rgbodomiso3f
		)
	{
		progs["rgbOdometryStep"]->setUniform("sigma", -1);
		progs["rgbOdometryStep"]->setUniform("sobelScale", 0.125f);
		progs["rgbOdometryStep"]->setUniform("cam", cam);

		progs["rgbOdometryStep"]->use();
		currentFrame.getVertexPreviousMap()->bindImage(0, 0, GL_READ_ONLY);

		gradientMap->bindImage(1, 0, GL_READ_ONLY);

		currentFrame.getMappingC2DMap()->bindImage(2, 0, GL_READ_ONLY);
		currentFrame.getMappingD2CMap()->bindImage(3, 0, GL_READ_ONLY);

		currentFrame.getTestMap()->bindImage(4, 0, GL_WRITE_ONLY);

		ssboRGBReduction.bindBase(0);
		ssboRGBRGBJtJJtrSE3.bindBase(1);
		glDispatchCompute(GLHelper::divup(currentFrame.getColorMap()->getWidth(), 32), GLHelper::divup(currentFrame.getColorMap()->getHeight(), 32), 1);

		progs["rgbOdometryStep"]->disuse();

		//std::vector<float> tmpData(8 * currentFrame.getColorMap()->getWidth() * currentFrame.getColorMap()->getHeight());

		//ssboRGBRGBJtJJtrSE3.read(tmpData.data(), 0, 8 * currentFrame.getColorMap()->getWidth() * currentFrame.getColorMap()->getHeight());

		progs["rgbOdometryStepReduce"]->use();
		progs["rgbOdometryStepReduce"]->setUniform("imSize", glm::ivec2(currentFrame.getColorMap()->getWidth(), currentFrame.getColorMap()->getHeight()));

		ssboRGBRGBJtJJtrSE3.bindBase(0);
		ssboRGBRGBJtJJtrSE3Output.bindBase(1);

		glDispatchCompute(8, 1, 1);
		progs["rgbOdometryStepReduce"]->disuse();


		std::vector<float> tmpOutputData(8 * 32);

		ssboRGBRGBJtJJtrSE3Output.read(tmpOutputData.data(), 0, 8 * 32);

		for (int row = 1; row < 8; row++)
		{
			for (int col = 0; col < 32; col++)
			{
				tmpOutputData[col + 0 * 32] += tmpOutputData[col + row * 32];
			}
		}

		float vectorB_host[6];
		float matrixA_host[6 * 6];


		/*
		vector b
		| 6  |
		| 12 |
		| 17 |
		| 21 |
		| 24 |
		| 26 |
		
		and
		matrix a
		| 0  | 1  | 2  | 3  | 4  | 5  |
		| 1  | 7  | 8  | 9  | 10 | 11 |
		| 2  | 8  | 13 | 14 | 15 | 16 |
		| 3  | 9  | 14 | 18 | 19 | 20 |
		| 4  | 10 | 15 | 19 | 22 | 23 |
		| 5  | 11 | 16 | 20 | 23 | 25 |
		
		*/
		
		int shift = 0;
		for (int i = 0; i < 6; ++i)
		{
			for (int j = i; j < 7; ++j)
			{
				float value = tmpOutputData[shift++];
				if (j == 6)
					vectorB_host[i] = value;
				else
					matrixA_host[j * 6 + i] = matrixA_host[i * 6 + j] = value;
			}
		}



		Eigen::Matrix<float, 6, 6, Eigen::RowMajor> A_rgbd;
		Eigen::Matrix<float, 6, 1> b_rgbd;

		std::memcpy(A_rgbd.data(), &matrixA_host[0], 6 * 6 * sizeof(float));
		std::memcpy(b_rgbd.data(), &vectorB_host[0], 1 * 6 * sizeof(float));

		Eigen::Matrix<double, 6, 1> result;

		Eigen::Matrix<double, 6, 6, Eigen::RowMajor> dA_rgbd = A_rgbd.cast<double>();
		Eigen::Matrix<double, 6, 1> db_rgbd = b_rgbd.cast<double>();

		Eigen::Matrix<double, 6, 6, Eigen::RowMajor> lastA;
		Eigen::Matrix<double, 6, 1> lastb;

		lastA = dA_rgbd;
		lastb = db_rgbd;
		result = lastA.ldlt().solve(lastb);

		//std::cout << "A_rgbd" << std::endl;
		//std::cout << A_rgbd << std::endl;

		Eigen::Matrix<double, 4, 4, Eigen::RowMajor> resultRt_eig = Eigen::Matrix<double, 4, 4, Eigen::RowMajor>::Identity();

		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				resultRt_eig(i, j) = resultRt[j][i];
			}
		}


		computeUpdateSE3(resultRt_eig, result, rgbodomiso3f);

		//Eigen::Isometry3f currentT;
		//currentT.setIdentity();
		//currentT.rotate(Rprev);
		//currentT.translation() = tprev;

		//currentT = currentT * rgbOdom.inverse();

		//tcurr = currentT.translation();
		//Rcurr = currentT.rotation();


		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				resultRt[j][i] = resultRt_eig(i, j);
			}
		}


	}

	void RGBOdometry::performColorTracking(
		const rgbd::Frame &currentFrame,
		const rgbd::Frame &virtualFrame,
		const gl::Texture::Ptr &gradientMap,
		glm::mat4 &pose,
		glm::vec4 cam // cx, cy, fx, fy
	)
	{

		float sigma;
		float rgbError;

		glm::mat3 K = glm::mat3(1.0f);
		K[0][0] = cam.z;
		K[1][1] = cam.w;
		K[2][0] = cam.x;
		K[2][1] = cam.y;

		Eigen::Matrix<double, 3, 3, Eigen::RowMajor> K_eig = Eigen::Matrix<double, 3, 3, Eigen::RowMajor>::Zero();

		K_eig(0, 0) = cam.z;
		K_eig(1, 1) = cam.w;
		K_eig(0, 2) = cam.x;
		K_eig(1, 2) = cam.y;
		K_eig(2, 2) = 1;

		glm::mat4 resultRt = glm::mat4(1.0f);
		Eigen::Matrix<double, 4, 4, Eigen::RowMajor> resultRt_eig = Eigen::Matrix<double, 4, 4, Eigen::RowMajor>::Identity();


		Eigen::Matrix<float, 3, 3, Eigen::RowMajor> Rprev;
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				Rprev(i, j) = pose[j][i];
			}
		}

		Eigen::Vector3f tprev;
		tprev(0) = pose[3][0];
		tprev(1) = pose[3][1];
		tprev(2) = pose[3][2];

		Eigen::Matrix<float, 3, 3, Eigen::RowMajor> Rcurr = Rprev;
		Eigen::Vector3f tcurr = tprev;

		for (int iter = 0; iter < 5; iter++)
		{
			Eigen::Matrix<double, 4, 4, Eigen::RowMajor> Rt_eig = resultRt_eig.inverse();

			Eigen::Matrix<double, 3, 3, Eigen::RowMajor> R_eig = Rt_eig.topLeftCorner(3, 3);

			Eigen::Matrix<double, 3, 3, Eigen::RowMajor> KRK_inv_eig = K_eig * R_eig * K_eig.inverse();

			glm::mat3 KRK_inv;// = K * R  * glm::inverse(K);

			std::cout << " iter : " << iter << " krk_inv \n" << KRK_inv_eig << std::endl;

			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					KRK_inv[i][j] = KRK_inv_eig(j, i);
				}
			}

			Eigen::Vector3d Kt_eig = Rt_eig.topRightCorner(3, 1);
			Kt_eig = K_eig * Kt_eig;

			glm::vec3 kT(Kt_eig(0), Kt_eig(1), Kt_eig(2));

			//glm::mat4 Rt = glm::inverse(resultRt);
			//glm::mat3 R = glm::mat3(Rt);

			////glm::mat3 KRK_inv = K * R  * glm::inverse(K);

			//glm::vec3 kT = glm::vec3(Rt[3][0], Rt[3][1], Rt[3][2]);

			//kT = K * kT;

			Eigen::Isometry3f rgbodomiso3f;

			computeResiduals(
				currentFrame,
				gradientMap,
				kT,
				KRK_inv,
				sigma,
				rgbError
			);

			computeStep(
				currentFrame,
				gradientMap,
				cam,
				sigma,
				rgbError,
				resultRt,
				rgbodomiso3f
			);

			Eigen::Isometry3f currentT;
			currentT.setIdentity();
			currentT.rotate(Rprev);
			currentT.translation() = tprev;

			currentT = currentT * rgbodomiso3f.inverse();

			tcurr = currentT.translation();
			Rcurr = currentT.rotation();

			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					resultRt_eig(i, j) = resultRt[j][i];
				}
			}

		}

		if (sigma == 0 || (tcurr - tprev).norm() > 0.3 || isnan(tcurr(0)))
		{
			Rcurr = Rprev;
			tcurr = tprev;
		}

		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				pose[i][j] = Rcurr(j, i);
			}
		}

		pose[3][0] = tcurr(0);
		pose[3][1] = tcurr(1);
		pose[3][2] = tcurr(2);

		//std::cout << tcurr << std::endl;
		//std::cout << glm::to_string(glm::transpose(pose)) << std::endl;


	}
}