#define GLUT_NO_LIB_PRAGMA
//##### OpenGL ######
#include <GL/glew.h>
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

#include "glutils.h"
#include "glslprogram.h"

#include "glhelper.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <fstream>
#include <valarray>

#include "opencv2/core/utility.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <opencv2/optflow.hpp>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/tracking.hpp"

#include <librealsense2/rs.hpp>

#include "GLCore/Texture.h"



class gFlow
{
public:
	gFlow() {};
	~gFlow() {};
	void setupEKF();
	void compileAndLinkShader();
	void setLocations();
	void allocateBuffers();
	void allocateTextures(int nChn);
	void allocateOffscreenRendering();

	void setTextureParameters(int width, int height) { m_texture_width = width;  m_texture_height = height; }
	void setNumLevels(int width)
	{
		m_numLevels = (int)(log((2 * width) / (4.0 * m_patch_size)) / log(2.0) + 0.5) + 1;
	}
	void setCutoff(int cOff)
	{
		m_cutoff = (uint32_t)cOff;
	}
	void setCameraDevice(int dev)
	{
		m_cameraDevice = dev;
	}

	void setFrameTexture(gl::Texture::Ptr inTex);

	void setTexture(unsigned char * imageArray, int nChn);
	void setTexture(std::vector<rs2::frame_queue> colorQ, cv::Mat &colorMat);
	void setTexture(float * imageArray);
	void setDepthTexture(std::vector<rs2::frame_queue> depthQ);
	GLuint getDepthTexture()
	{
		return m_textureDepth;
	}
	void setColorTexture(std::vector<rs2::frame_queue> colorQ, cv::Mat &colorMat);
	void setInfraTexture(std::vector<rs2::frame_queue> infraQ, cv::Mat &infraMat);
	void computeSobel(int level, bool useInfrared);
	void makePatches(int level);
	//bool precomputeStructureTensor();
	bool calc(bool useInfrared);
	void track();
	void track(GLuint bufferToTrack, int numPointsInBuffer);
	bool densification(int level);
	void medianFilter(int level);
	void calcStandardDeviation(int level);
	void wipeFlow();
	void wipeSumFlow();
	void clearPoints();

	void smoothPoints(std::vector<std::valarray<float>> RA);

	//void resizeFlow(int level);
	bool patchInverseSearch(int level, bool useInfrared);
	void variationalRefinement(int level);
	void variRef(int level);
	void sumFlowTexture();
	void swapTextures();
	void jumpFloodCalc();

	// QUADTREE STUFF
	void buildQuadtree();


	GLuint getFlowTexture()
	{
		//return m_sumFlow;

		return m_textureU_x_y;
		return mTextureFlowXY->getID();
	}
	gl::Texture::Ptr getFlowTextureFrame()
	{
		return mTextureFlowXY;
	}

	GLuint getColorTexture()
	{
		return m_textureI1;
	}
	GLuint getEdgesTexture()
	{
		return m_textureI0_grad_x_y;
	}

	GLuint getTrackedPointsBuffer()
	{
		return m_trackedPointsBuffer;
	}
	GLuint getFlowMinusMeanFlowTexture()
	{
		return m_textureFlowMinusMeanFlow;
	}
	void setVals(float a, float b)
	{
		m_valA = a;
		m_valB = b;
	}

	double getTimeElapsed()
	{
		return m_timeElapsed;
	}
	void resetTimeElapsed()
	{
		m_timeElapsed = 0.0;
	}
	void setTrackedPoint(float x, float y)
	{
		m_trackedPoint = cv::Point2f(x, y);
	}

	GLuint getQuadlist()
	{
		return m_bufferQuadlist;
	}
	uint32_t getQuadlistCount()
	{
		return m_quadlistCount;
	}
	GLuint getQuadlistMeanTemp()
	{
		return m_bufferQuadlistMeanTemp;
	}

	bool firstFrame = true;

	void setCurrentLevel(int frameNumber)
	{
		m_currentLevel = frameNumber % m_flowHistoryLevels;
	}
	int getCurrentLevel()
	{
		return m_currentLevel;
	}
	void setOpLevel(int frameNumber)
	{
		m_opLevel = frameNumber;
		m_newPointsData = true;
	}

private:


	cv::Mat I0im, I1im;

	cv::Point2f m_trackedPoint = cv::Point2f(1920 >> 1, 1080 >> 1);
	std::vector<float> m_trackedPoints;

	GLuint m_trackedPointsBuffer;

	int variational_refinement_iter;
	float variational_refinement_alpha;
	float variational_refinement_gamma;
	float variational_refinement_delta;
	int getVariationalRefinementIterations() const { return variational_refinement_iter; }
	void setVariationalRefinementIterations(int val) { variational_refinement_iter = val; }
	float getVariationalRefinementAlpha() const { return variational_refinement_alpha; }
	void setVariationalRefinementAlpha(float val) { variational_refinement_alpha = val; }
	float getVariationalRefinementDelta() const { return variational_refinement_delta; }
	void setVariationalRefinementDelta(float val) { variational_refinement_delta = val; }
	float getVariationalRefinementGamma() const { return variational_refinement_gamma; }
	void setVariationalRefinementGamma(float val) { variational_refinement_gamma = val; }
	//std::vector<cv::Ptr<cv::optflow::VariationalRefinement> > variational_refinement_processors;

	GLuint timeQuery[1];
	double m_timeElapsed = 0.0;

	inline int divup(int a, int b) { return (a % b != 0) ? (a / b + 1) : (a / b); }

	GLSLProgram disFlowProg;
	GLSLProgram sobelProg;
	GLSLProgram variRefineProg;
	GLSLProgram extKalmanProg;
	GLSLProgram jumpFloodProg;
	GLSLProgram hpQuadtreeProg;
	GLSLProgram hpQuadListProg;
	GLSLProgram prefixSumProg;
	GLSLProgram stdDevProg;
	GLSLProgram renderOffscreenProg;

	GLSLProgram densifyRasterProg;

	//Locations
	/* subroutines */
	GLuint m_subroutine_SobelID;
	GLuint m_getGradientsID;
	GLuint m_getSmoothnessID;
	GLuint m_subroutine_jumpFloodID;

	GLuint m_subroutine_DISflowID;
	GLuint m_makePatchesID;
	GLuint m_makePatchesHorID;
	GLuint m_makePatchesVerID;

	GLuint m_patchInverseSearchID;
	GLuint m_patchInverseSearchDescentID;
	GLuint m_densificationID;
	GLuint m_medianFilterID;
	GLuint m_resizeID;
	GLuint m_trackID;
	GLuint m_trackPoseID;
	GLuint m_prefixSum2D_HorID, m_prefixSum2D_VerID;
	GLuint m_patch_sizeID;
	GLuint m_patch_strideID;
	GLuint m_trackWidthID;
	GLuint m_getLivePointsID;

	GLuint m_sumFlowTextureID;

	GLuint m_subroutine_variRefineID;
	GLuint m_prepareBuffersID;
	GLuint m_computeDataTermID;
	GLuint m_computeSmoothnessTermID;
	GLuint m_computeSORID;

	// quadtree
	GLuint m_subroutine_hpQuadtreeID;
	GLuint m_hpDiscriminatorID;
	GLuint m_hpBuilderID;
	GLuint m_bufferQuadlist;
	GLuint m_bufferQuadlistMeanTemp;

	GLuint m_subroutine_hpQuadlistID;
	GLuint m_traverseHPLevelID;
	GLuint m_totalSumID;
	GLuint m_cutoffID;

	GLuint m_hpLevelID;
	GLuint m_quadThreshID;

	// std dev
	GLuint m_subroutine_stdDevID;
	GLuint m_stdFirstID;
	GLuint m_stdSecondID;
	GLuint m_quadListCountID;

	//prefixsum
	GLuint m_useRGBAID;

	//offscreen
	GLuint m_imSizeID;
	GLuint m_texLevelID;

	/* uniforms */
	GLuint m_level_cov_ID;
	GLuint m_imageType_cov_ID;
	GLuint m_imageType_dis_ID;
	GLuint m_level_dis_ID;
	GLuint m_iter_dis_ID;
	GLuint m_level_var_ID;
	GLuint m_flipflopID;
	GLuint m_iter_var_ID;
	GLuint m_texSizeID;

	GLuint m_valAID;
	GLuint m_valBID;

	GLuint m_currentLevelID;
	GLuint m_opLevelID;

	GLuint m_jfaInitID;
	GLuint m_jfaUpdateID;
	GLuint m_jumpID;


	//Buffers
	std::vector<float> m_refinementDataTerms;
	GLuint m_buffer_refinement_data_terms;
	GLuint m_bufferLivePoints;

	//GLuint m_bufferU; //!< a buffer for the merged flow

	//GLuint m_textureSx; //!< intermediate sparse flow representation (x component)
	//GLuint m_textureSy; //!< intermediate sparse flow representation (y component)
	GLuint m_textureS_x_y;
	GLuint m_textureI1_s_ext;

	GLuint m_textureTest;



	// variational refineing textures
	GLuint m_textureI_mix_diff_warp; //  averageI, Iz, warpedI
	GLuint m_textureI_grads_mix_diff_x_y; // Ix, Iy, Ixz, Iyz
	GLuint m_textureI_second_grads_mix_diff_x_y; // Ixx, Ixy, Iyy
	GLuint m_texture_dup_dvp;
	GLuint m_texture_total_flow;
	GLuint m_texture_smoothness_terms;
	GLuint m_texture_smoothness_weight;

	//Textures
	GLuint createTexture(GLuint ID, GLenum target, int levels, int w, int h, int d, GLuint internalformat);

	GLuint m_textureDepth;


	GLuint m_textureI0_prod_xx_yy_xy;
	GLuint m_textureI0_sum_x_y;
	GLuint m_textureI0_grad_x_y;
	GLuint m_textureI0_prod_xx_yy_xy_aux;
	GLuint m_textureI0_sum_x_y_aux;


	GLuint m_textureI0;
	GLuint m_textureI1;
	//GLuint m_textureU;

	GLuint m_textureWarp_I1;
	//GLuint m_textureI0s;
	//GLuint m_textureI1s;
	GLuint m_textureI0s_I1s;

	GLuint m_textureI0_xs_ys;
	//GLuint m_textureI0ys;

	gl::Texture::Ptr mTextureFlowXY;




	GLuint m_textureU_x_y;
	GLuint m_texture_init_U_x_y;
	GLuint m_texture_previous_U_x_y;
	GLuint m_texture_prefixSumTemp;
	GLuint m_texture_prefixSum;

	GLuint m_texture_prefixSumSecondPass;
	GLuint m_texture_prefixSumTempSecondPass;

	GLuint m_textureFlowArray;

	//GLuint m_textureUy;

	GLuint m_textureUx_initial;
	GLuint m_textureUy_initial;

	GLuint m_texture_temp;
	GLuint m_texture_temp1;

	GLuint m_sumFlow;

	//JFA
	GLuint m_texture_jfa_0;
	GLuint m_texture_jfa_1;

	// quadtree
	GLuint m_texture_hpOriginalData;
	GLuint m_texture_hpQuadtree;
	GLuint m_textureBLANKFLOW;


	// OFFSCREEN RENDERING STUFF
	GLuint m_FBO;
	GLuint m_VAO;
	GLuint m_RBO;
	GLuint m_textureFlowMinusMeanFlow;

	// parameters
	int m_texture_width;
	int m_texture_height;
	int m_patch_size = 8;
	int m_patch_stride = 4;
	int m_border_size = 16;
	int m_ws; // 1 + (width - patch_size) / patch_stride
	float m_valA = 0.01f;
	float m_valB = 0.01f;

	int swapCounter = 0;
	bool flipflop = false;

	uint32_t m_cutoff;

	uint32_t m_quadlistCount;

	uint32_t m_numberHPLevels;

	int m_numLevels;// = (int)(log((2 * 1920) / (4.0 * m_patch_size)) / log(2.0) + 0.5) - 1;



	std::vector<float> zeroValues;// = std::vector<float>(1920 * 1080 * 4, 0.0f);
	std::vector<float> oneValues;// = std::vector<float>(1920 * 1080 * 2, 1.0f);

	std::vector<int> zeroValuesInt = std::vector<int>(1920 * 1080 * 4, 0);


	int m_flowHistoryLevels = 90;
	int m_currentLevel;
	int m_opLevel;

	cv::Mat tempMat;

	std::vector<glm::vec4> m_oldPoints;
	std::vector<glm::vec4> m_currentPoints;

	bool m_newPointsData = false;
	int m_cameraDevice;

};