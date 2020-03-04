#pragma once

#include "ComputeShader.h"
#include "glhelper.h"
#include "GLCore/Framebuffer.h"
#include "GLCore/Buffer.h"
#include "Frame.h"

#include "opencv2/core/utility.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/optflow.hpp"
//#include "opencv2/video/tracking.hpp"

namespace rgbd
{
	
	class DisFlow : public ComputeShader
	{
	private:
		std::map<std::string, const gl::Shader::Ptr> progs;

		gl::Texture::Ptr sparseFlowMap;

		gl::Texture::Ptr lastFlowMap;
		gl::Texture::Ptr nextFlowMap;

		gl::Texture::Ptr meanI_Map;
		gl::Texture::Ptr I_t_Map;
		gl::Texture::Ptr pixelBaseFlow;
		gl::Texture::Ptr I_x_y_Map;
		gl::Texture::Ptr pixelFlowMap;
		std::vector<gl::Texture::Ptr> pixelDiffFlowMap;
		gl::Texture::Ptr diffusivityMap;

		gl::ShaderStorageBuffer<float> ssboSOR;



		int maxLevels;

		std::vector<gl::Texture::Ptr> densificationFlowMap;

		std::vector<gl::Framebuffer> densificationFBO;


		//int variational_refinement_iter;
		//float variational_refinement_alpha;
		//float variational_refinement_gamma;
		//float variational_refinement_delta;
		//int getVariationalRefinementIterations() const { return variational_refinement_iter; }
		//void setVariationalRefinementIterations(int val) { variational_refinement_iter = val; }
		//float getVariationalRefinementAlpha() const { return variational_refinement_alpha; }
		//void setVariationalRefinementAlpha(float val) { variational_refinement_alpha = val; }
		//float getVariationalRefinementDelta() const { return variational_refinement_delta; }
		//void setVariationalRefinementDelta(float val) { variational_refinement_delta = val; }
		//float getVariationalRefinementGamma() const { return variational_refinement_gamma; }
		//void setVariationalRefinementGamma(float val) { variational_refinement_gamma = val; }
		//std::vector<cv::Ptr<cv::VariationalRefinement>> variational_refinement_processors;


	public:
		DisFlow();
		~DisFlow();

		void DisFlow::loadShaders(
			std::map<std::string, const gl::Shader::Ptr> &progs,
			const std::string &folderPath
		);

		
		void init(
			int numLevels,
			int width,
			int height,
			const std::map<std::string, const gl::Shader::Ptr> &programs
		);

		void execute(
			const rgbd::Frame &currentFrame,
			gl::Texture::Ptr nextGradientMap
		);

		gl::Texture::Ptr getFlowMap() const;

	};
}