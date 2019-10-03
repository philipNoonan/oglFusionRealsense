#pragma once

#include <opencv2/opencv.hpp>

#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Geometry> 
#include <unsupported/Eigen/MatrixFunctions>
#include "eigen_utils.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <GLCore/Shader.h>
#include "ConstantParameters.h"
#include "VirtualFrameRenderer.h"
#include "ProjectiveDataAssoc.h"

// Ref: "Linear Least - squares Optimization for Point-to-plane ICP Surface Registration"
namespace rgbd
{
	class PointToPlaneICP
	{
	private:
		VirtualFrameRenderer virtualFrameRenderer;
		ProjectiveDataAssoc dataAssoc;

	public:
		PointToPlaneICP(
			int width,
			int height,
			const glm::mat4 &K,
			const std::map<std::string, const gl::Shader::Ptr> &progs
		);
		~PointToPlaneICP();
		
		void paramToMat(
			const cv::Mat &params,	// parameters (6 x 1 matrix)
			glm::mat4 &T			// 4x4 matrix in glm::mat4
		);
		
		void calc(
			const int level,
			const rgbd::Frame &prevFrame,
			const rgbd::Frame &currFrame,
			glm::mat4 &T,
			Eigen::Matrix<double, 6, 6, Eigen::RowMajor> &lastA,
			const float finThresh = 1.0e-4F
		);

		typedef std::shared_ptr<rgbd::PointToPlaneICP> Ptr;
	};
}
