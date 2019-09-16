#define GLUT_NO_LIB_PRAGMA
//##### OpenGL ######
#include <GL/glew.h>
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API


#define USE_OPENCV

#ifdef USE_OPENCV

#include "opencv2/core/utility.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#endif


#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Geometry> 
#include <unsupported/Eigen/MatrixFunctions>
#include "eigen_utils.h"



#include <iostream>
#include <fstream>

#include <vector>
#include <list>
#include <numeric>

#include "glutils.h"
#include "glslprogram.h"
#include "glhelper.h"

#define _USE_MATH_DEFINES
#include <math.h>

struct gFusionConfig
{
	glm::vec3 volumeSize; // size in voxels
	glm::vec3 volumeDimensions; // size in meters
	float mu;
	float maxWeight;
	float nearPlane;
	float farPlane;
	float dist_threshold;
	float normal_threshold;
	std::vector<int> iterations;
	float track_threshold;
	glm::vec2 depthFrameSize;
	float dMax;
	float dMin;

	gFusionConfig() 
	{
		volumeSize = glm::vec3(256);
		volumeDimensions = glm::vec3(1.0f);
		//cameraParameters = glm::vec4(256.0f, 212.0f, );
		mu = 0.5f;
		maxWeight = 100.0f;
		nearPlane = 0.1f;
		farPlane = 3.0f;
		dist_threshold = 0.05f;
		normal_threshold = 0.8f;
		iterations.push_back(2);
		iterations.push_back(4);
		iterations.push_back(6);
		track_threshold = 0.5f;
		depthFrameSize = glm::vec2(848, 480);
		dMax = 0.01f;
		dMin = -0.004f;
	}

	float stepSize()
	{
		float minxy = glm::min(volumeDimensions.x, volumeDimensions.y);
		float minxyz = glm::min(minxy, volumeDimensions.z);

		float maxxy = glm::max(volumeSize.x, volumeSize.y);
		float maxxyz = glm::max(maxxy, volumeSize.z);

		return minxyz / maxxyz;
	}
};

struct integrateShaderConfigs
{
	int numberOfCameras;
	int d2p;
	int d2v;
	float dMax;
	float dMin;
	float maxWeight;
	float depthScale;
	float volDim; // vol dim real world span of the volume in meters
	float volSize; // voxel grid size of the volume 
};

//
//struct mCubeConfig
//{
//	glm::uvec3 gridSize;
//	glm::uvec3 gridSizeMask;
//	glm::uvec3 gridSizeShift;
//	GLuint numVoxels;
//	glm::vec3 voxelSize;
//	float isoValue;
//	GLuint maxVerts;
//	float activeVoxels;
//
//	mCubeConfig()
//	{
//		gridSize = glm::uvec3(256, 256, 256);
//		gridSizeMask = glm::uvec3(gridSize.x - 1, gridSize.y - 1, gridSize.z - 1);
//		gridSizeShift = glm::uvec3(0, log2(gridSize.x), log2(gridSize.y) + log2(gridSize.z));
//		numVoxels = gridSize.x * gridSize.y * gridSize.z;
//		voxelSize = glm::vec3(1.0f * 1000.0f / 128.0f, 1.0f * 1000.0f / 128.0f, 1.0f * 1000.0f / 128.0f);
//		isoValue = 0.0f;
//		maxVerts = 256 * 256 * 256;
//		
//	}
//
//};

struct gPosePair
{
	GLuint textureID;
	glm::mat4 pose;
};


struct gTrackData
{
	GLint result;
	GLfloat error;
	GLfloat J[6];
};


class gFusion
{
public:
	gFusion() {}
	~gFusion();

	gFusionConfig configuration;
	//mCubeConfig mcubeConfiguration;
	glm::mat4 pose, raycastPose;

	void queryDeviceLimits();
	// load and link shaders
	void compileAndLinkShader();
	// set shader locations
	void setLocations();
	// texture setup
	void initTextures();

	void setColorToColor(glm::mat4 cam2cam)
	{
		m_colorToColor = cam2cam;

		//m_depthToDepth = glm::inverse(m_extrinsics[0]) * (m_colorToColor) * m_extrinsics[1];
	}

	void setDepthToDepth(glm::mat4 dep2dep)
	{
		m_depthToDepth = dep2dep;
	}

	void setUsingDepthFloat(bool useFloat)
	{
		m_usingFloatDepth = useFloat;
	}
	void setDepthUnit(float value)
	{
		m_depthUnit = value;
	}
	void setDepthTexture(GLuint depthTex)
	{

		/*GLenum theerror;
		theerror = glGetError();

		glCopyImageSubData(depthTex, GL_TEXTURE_2D, 0, 0, 0, 0, m_textureDepth0, GL_TEXTURE_2D, 0, 0, 0, 0, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1);

		theerror = glGetError();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_textureDepth0);
		glGenerateMipmap(GL_TEXTURE_2D);

		theerror = glGetError();*/


	}
	void setClickedPoint(int x, int y)
	{
		m_clickedPoint = true;
		m_pointX = x;
		m_pointY = y;
	}
	// buffers setup
	void initVolume();
	// reset functions
	void Reset(glm::mat4 pose, bool deleteFlag);
	void resetVolume();
	void resetPose(glm::mat4 pose);
	void allocateBuffers();
	void initSplatterVAOs();
	void initSplatterFBOs();
	// depth functions
	// combine multiple depth into one buffer of verts
	//void depthToVertex(std::vector<rs2::frame_queue> depthQ, glm::vec3 &point);
	// backproject depth frame into vertex image
	void uploadDepth(std::vector<rs2::frame_queue> depthQ, int devNumber, glm::vec3 &point);
	void uploadDepthToBuffer(std::vector<rs2::frame_queue> depthQ, int devNumber, glm::vec3 &point, uint32_t counter);
	void allocateTransformFeedbackBuffers();
	void initSplatterFusion();
	void combinedPredict();
	//void makeImagePyramids();
	void predictIndices();
	void fuse();
	void splatterDepth();
	void splatterModel();
	void makeImagePyramids();
	bool TrackSplat();
	void trackSplat(int level);
	void reduceSplat(int level);
	void depthToVertex();
	void depthToVertex(float * depthArray);
	void depthToVertex(uint16_t * depthArray);

	void vertexToNormal(int devNumber);
	void vertexToNormal();
	bool Track();
	bool TrackSDF();

	void setInitUnstable(int val)
	{
		m_initUnstable = val;
	}

	std::vector<uint64_t> getFrameTime()
	{
		return m_sensorsTimestamps;
	}

	// volume functions
	void integrate(bool forceIntegrate);
	void integrate(int devNumber);
	void raycast(int devNumber);
	//void marchingCubes();
	void intensityProjection();

	// fusion functions
	void track(int devNumber, int layer);
	void reduce(int devNumber, int layer);
	void getReduction(std::vector<float>& b, std::vector<float>& C, float & alignmentEnergy, float &lastICPCount);

	// fusion sdf functions
	void trackSDF(int devNumber, int layer, Eigen::Matrix4f camToWorld);
	void reduceSDF(int layer);
	void getSDFReduction(std::vector<float>& b, std::vector<float>& C, float &alignmentEnergy);
	void trackSDF(int layer, Eigen::Matrix4f camToWorld);

	// tracking lost functions
	void updatePoseFinder();
	void addPoseToLibrary();
	bool recoverPose(glm::mat4 recoveryPose);
	void checkDepthToDepth();

	void trackPoints3D(GLuint trackedPoints2D);

	float alignmentEnergy()
	{
		return m_alignmentEnergy;
	}

	// tracking sets and gets
	glm::mat4 getPose()
	{
		return m_pose;
	}
	void setPose(glm::mat4 pose)
	{
		m_pose = pose;
	}
	void setPoseP2V(glm::mat4 pose)
	{
		std::memcpy(m_pose_eig.data(), glm::value_ptr(pose), 16 * sizeof(float));
	}
	// camera parameters
	void setCameraParams(int devNumber, glm::vec4 camPams, glm::vec4 camPamsColor)
	{
		m_camPamsDepth[devNumber] = camPams;
		m_camPamsColor[devNumber] = camPamsColor;
	}
	void setNumberOfCameras(int numCams)
	{
		m_numberOfCameras = numCams;
		m_extrinsics.resize(numCams);
		m_camPamsDepth.resize(numCams);
		m_camPamsColor.resize(numCams);
	}
	void setDepthToColorExtrinsics(glm::mat4 extrin, int devNumber)
	{
		m_extrinsics[devNumber] = extrin;

	}
	void setConfig(gFusionConfig config)
	{
		configuration = config;
	}
	//void setMcConfig(mCubeConfig config)
	//{
	//	mcubeConfiguration = config;
	//}
	GLuint getDepthImage()
	{
		return m_textureDepthArray;
	}
	GLuint getColorImage()
	{
		return m_textureColor;
	}
	GLuint getVerts()
	{
		return m_textureVertex;

		//return m_textureReferenceVertex;

	}
	GLuint getSplatterDepth()
	{
		return m_textureSplatteredDepth;
	}
	GLuint getSplatterModel()
	{
		return m_textureSplatteredModel;
	}
	GLuint getNorms()
	{
		return m_textureNormalArray;
		//return m_textureNormal[devNumber];
	}
	GLuint getPVPNorms()
	{
		return m_textureReferenceNormalArray;
	}
	GLuint getPVDNorms()
	{
		return m_textureSDFImage;
	}
	GLuint getVertsMC()
	{
		return m_bufferPos;
	}
	GLuint getNormsMC()
	{
		return m_bufferNorm;
	}
	GLuint getVolume()
	{
		return m_textureVolume;
	}
	GLuint getTrackImage()
	{
		return m_textureTrackImage;
	}
	size_t getNumVerts()
	{
		return m_totalVerts;

	}
	void setFloodSDF(GLuint floodTex)
	{
		m_textureFloodSDF = floodTex;
	}


	void showNormals(int devNumber);
	void showRaycast();
	void showDifference();
	void showTrack();

	void testPrefixSum();
	void exportSurfaceAsStlBinary();
	//void exportMeshPly();

	void printTimes();
	void resetTimes();
	void getTimes(float arr[]);

	glm::vec3 getInitialPose(float pixX, float pixY);

	void testLargeUpload();

	std::vector<float> getCumTwist()
	{
		Eigen::MatrixXf f = m_cumTwist.cast <float>();
		std::vector<float> vec(f.data(), f.data() + f.rows() * f.cols());
		return vec;
	}

	std::vector<float> getTransPose()
	{
		std::vector<float> vec(3);
		vec[0] = m_pose[3][0];
		vec[1] = m_pose[3][1];
		vec[2] = m_pose[3][2];
		return vec;
	}




private:

	// PROGRAMS
	GLSLProgram depthToBufferProg;
	GLSLProgram updateGlobalModelProg;
	GLSLProgram dataProg;
	GLSLProgram cleanGlobalProg;
	GLSLProgram initUnstableProg;
	GLSLProgram splatterProg;
	GLSLProgram splatterGlobalProg;
	GLSLProgram createIndexMapProg;
	GLSLProgram combinedPredictProg;

	GLSLProgram depthToVertProg;
	GLSLProgram vertToNormProg;
	GLSLProgram raycastProg;
	GLSLProgram integrateProg;
	GLSLProgram trackProg;
	GLSLProgram reduceProg;
	GLSLProgram helpersProg;
	GLSLProgram prefixSumProg;
	GLSLProgram marchingCubesProg;
	GLSLProgram trackSDFProg;
	GLSLProgram reduceSDFProg;
	GLSLProgram mipProg;

	GLSLProgram trackSplatProg;
	GLSLProgram reduceSplatProg;

	// LOCATIONS ID
	// depthtovert
	GLuint m_invkID;
	GLuint m_colorKID;
	GLuint m_extrinsicsID;
	GLuint m_camPamsDepthID;
	GLuint m_camPamsColorID;
	GLuint m_imageTypeID;
	GLuint m_depthScaleID;
	GLuint m_numberOfCamerasID_d;
	// vert to norm
	GLuint m_numberOfCamerasID_v;

	// track
	GLuint m_viewID_t;
	GLuint m_TtrackID;
	GLuint m_distThresh_t;
	GLuint m_normThresh_t;
	GLuint m_dMaxID_t;
	GLuint m_dMinID_t;
	GLuint m_numberOfCamerasID_tr;
	GLuint m_cameraPosesID_tr;
	GLuint m_inverseCameraPosesID_tr;
	
	// reduce
	GLuint m_imageSizeID;


	GLuint m_viewID_tsp;
	GLuint m_TtrackID_tsp;
	GLuint m_distThresh_tsp;
	GLuint m_normThresh_tsp;
	GLuint m_numberOfCamerasID_tsp;
	GLuint m_cameraPosesID_tsp;
	GLuint m_camPamID_tsp;
	GLuint m_inverseVPID_tsp;
	
	GLuint m_imageSizeID_rsp;



	// integrate
	GLuint m_integrateSubroutineID;
	GLuint m_integrateStandardID;
	GLuint m_integrateMultipleID;
	GLuint m_integrateExperimentalID;
	GLuint m_invTrackID;
	GLuint m_KID;
	GLuint m_invKID_i;
	GLuint m_muID;
	GLuint m_maxWeightID;
	GLuint m_volDimID;
	GLuint m_volSizeID;
	GLuint m_imageTypeID_i;
	GLuint m_depthScaleID_i;
	GLuint m_dMaxID_i;
	GLuint m_dMinID_i;
	GLuint m_cameraDeviceID_i;
	GLuint m_numberOfCamerasID_i;
	GLuint m_forceIntegrateID;
	GLuint m_cameraPosesID_i;
	GLuint m_cameraIntrinsicsID_i;
	GLuint m_inverseCameraIntrinsicsID_i;

	GLuint m_d2pID_i;
	GLuint m_d2vID_i;

	integrateShaderConfigs shaderConfigs;



	// INTEGRTATE UB
	unsigned int m_ub_integrateID;
	GLuint m_ub_integrate;

	// raycast
	GLuint m_viewID_r;
	GLuint m_nearPlaneID;
	GLuint m_farPlaneID;
	GLuint m_stepID;
	GLuint m_largeStepID;
	GLuint m_volDimID_r;
	GLuint m_volSizeID_r;
	GLuint m_helpersSubroutineID;
	GLuint m_cameraPosesID_r;
	GLuint m_numberOfCamerasID_r;

	// Helpers
	GLuint m_resetVolumeID;
	GLuint m_trackPointsToVertsID;
	GLuint m_volSizeID_h;
	GLuint m_buffer2DWidthID;
	GLuint m_invKID_h;

	// prefix sum
	GLuint m_prefixSumSubroutineID;
	GLuint m_resetSumsArrayID;
	GLuint m_forEachGroupID;
	GLuint m_forEveryGroupID;
	GLuint m_forFinalIncrementalSumID;

	// marching cubes
	GLuint m_marchingCubesSubroutineID;
	GLuint m_classifyVoxelID;
	GLuint m_compactVoxelsID;
	GLuint m_generateTrianglesID;
	
	GLuint m_gridSizeID;
	GLuint m_gridSizeShiftID;
	GLuint m_gridSizeMaskID;
	GLuint m_isoValueID;
	GLuint m_numVoxelsID;
	GLuint m_activeVoxelsID;
	GLuint m_maxVertsID;
	GLuint m_voxelSizeID;

	// track sdf
	GLuint m_TtrackID_t;
	GLuint m_volDimID_t;
	GLuint m_volSizeID_t;
	GLuint m_cID;
	GLuint m_epsID;
	GLuint m_numberOfCamerasID_t;
	GLuint m_mipLayerID_t;
	// track sdf
	GLuint m_imageSizeID_t_sdf;
	//reduce sdf
	GLuint m_imageSizeID_sdf; 
	GLuint m_cameraPosesID_tsdf;

	// intensity propjection
	GLuint m_viewID_m;
	GLuint m_nearPlaneID_m;
	GLuint m_farPlaneID_m;
	GLuint m_stepID_m;
	GLuint m_largeStepID_m;
	GLuint m_volDimID_m;
	GLuint m_volSizeID_m;

	// depth to buffer 
	GLuint m_camPamID_d2b;
	GLuint m_depthScaleID_d2b;
	GLuint m_invKID_d2b;
	GLuint m_initUnstableID_d2b;
	GLuint m_frameCountID_d2b;

	// index map
	GLuint m_indexInversePoseID;
	GLuint m_indexCamPamID; 
	GLuint m_indexImSizeID;
	GLuint m_indexMaxDepthID;
	GLuint m_indexTimeID;

	// init unstable 
	GLuint m_InitUnstableMaxNumVertsID;

	// combined predict
	GLuint m_cpMaxNumVertsID;
	GLuint m_cpInversePoseID;
	GLuint m_cpCamPamID;
	GLuint m_cpImSizeID;
	GLuint m_cpMaxDepthID;
	GLuint m_cpConfThresholdID;
	GLuint m_cpTimeID;
	GLuint m_cpMaxTimeID;
	GLuint m_cpTimeDeltaID;
	GLuint m_cpKMatID;

	// data fusion
	GLuint m_dpCamPamID;
	GLuint m_dpImSizeID;
	GLuint m_dpScaleID;
	GLuint m_dpTexDimID;
	GLuint m_dpPoseID;
	GLuint m_dpMaxDepthID;
	GLuint m_dpTimeID;
	GLuint m_dpTimeDeltaID;
	GLuint m_dpWeightingID;

	// update global model
	GLuint m_ugmCamPamID;
	GLuint m_ugmTimeID;
	GLuint m_ugmTimeDeltaID;
	GLuint m_ugmTexDimID;
	GLuint m_ugmCurrentGlobalNumberID;
	GLuint m_ugmCurrentNewUnstableNumberID;

	// clean global model
	GLuint m_cgTimeID;
	GLuint m_cgTimeDeltaID;
	GLuint m_cgInversePoseID;
	GLuint m_cgCamPamID;
	GLuint m_cgConfThresholdID;
	GLuint m_cgCurrentGlobalNumberID;
	GLuint m_cgCurrentNewUnstableNumberID;
	// TEXTURES
	GLuint createTexture(GLenum target, int levels, int w, int h, int d, GLint internalformat, GLenum magFilter, GLenum minFilter);
	GLuint m_textureColor;
	GLuint m_textureColorArray; // EMPTY AT THE MOMENT
	GLuint m_textureDepthArray;
	GLuint m_textureVertexArray;
	GLuint m_textureNormalArray;
	std::vector<GLuint> m_textureDepth;
	GLuint m_textureVertex;
	std::vector<GLuint> m_textureNormal;
	GLuint m_textureSDFImage;
	GLuint m_textureTrackImage;

	GLuint m_textureReferenceVertexArray;
	GLuint m_textureReferenceNormalArray;
	GLuint m_textureOutputData;
	GLuint m_textureDifferenceVertex;
	GLuint m_textureTestImage;
	GLuint m_textureFloodSDF;

	GLuint m_textureDist;

	GLuint m_atomicCounterTest;


	//GLuint m_textureEdgeTable;
	//GLuint m_textureTriTex;
	//GLuint m_textureNumVertsTable;

	GLuint m_textureVolume;

	GLuint m_textureTest;

	GLuint m_textureMip;
	// BUFFERS
	std::vector<short> m_volume;
	std::vector<gTrackData> m_reduction;
	GLuint m_buffer_reduction;
	GLuint m_bufferSDFReduction;
	GLuint m_bufferSDFoutputdata;

	std::vector<float> m_outputdata;
	GLuint m_buffer_outputdata;

	GLuint m_bufferVoxelVerts;
	GLuint m_bufferVoxelVertsScan;
	GLuint m_bufferVoxelOccupied;
	GLuint m_bufferVoxelOccupiedScan;
	GLuint m_bufferCompactedVoxelArray;
	GLuint m_bufferPos;
	GLuint m_bufferNorm;

	GLuint m_uboIntegrationConfig;

	GLuint m_bufferCameraData;
	GLuint m_bufferCameraData_i;
	GLuint m_bufferCameraIntrinsics;
	GLuint m_bufferInverseCameraIntrinsics;

	GLuint m_buffer_testInput;
	GLuint m_buffer_testOutput;
	GLuint m_buffer_testMidput;

	GLuint m_bufferPrefixSumByGroup;

	GLuint m_devNumberTrackSdfID;
	GLuint m_devNumberReduceSdfID;

	// TRACKED POINTS
	std::vector<float> m_trackedPoints3D;
	GLuint m_trackedPoints3DBuffer;


	// SPLATTERING
	GLuint m_VAO;
	GLuint m_global_FBO;
	GLuint m_global_RBO;

	GLuint m_depth_FBO;
	GLuint m_depth_RBO;

	GLuint m_combined_FBO;
	GLuint m_combined_RBO;

	GLuint m_updateMapIndex_FBO;
	GLuint m_updateMapIndex_RBO;

	GLuint m_modelVBO;
	GLuint m_textureSplatteredModel;
	GLuint m_textureSplatteredDepth;

	GLuint m_textureGlobalIndexVertConf;
	GLuint m_textureGlobalIndexNormRadi;
	GLuint m_textureGlobalIndexColTimDev;
	GLuint m_textureVertexID;

	GLuint m_textureDepthIndexVertConf;
	GLuint m_textureDepthIndexNormRadi;
	GLuint m_textureDepthIndexColTimDev;
	GLuint m_textureDepthTime;

	GLuint m_textureCombinedIndexVertex;
	GLuint m_textureCombinedIndexNormal;
	GLuint m_textureCombinedIndexColTim;
	GLuint m_textureCombinedIndexConRadDev;

	GLuint m_textureUpdateMapIndexVertConf;
	GLuint m_textureUpdateMapIndexNormRadi;
	GLuint m_textureUpdateMapIndexColTimDev;

	GLuint m_splatterMVPID;
	GLuint m_splatterModelID;
	GLuint m_splatterCamPamID;
	GLuint m_splatterMaxDepthID;
	GLuint m_splatterImSizeID;

	// Depth to Vertex Buffering
	GLuint m_depth_VAO;
	GLuint m_global_VAO;

	GLuint m_depth_VBO;
	GLuint m_depth_TFO;

	GLuint m_data_VAO;


	GLuint m_globalTarget_VBO;
	GLuint m_globalTarget_TFO;

	GLuint m_globalRender_VBO;
	GLuint m_globalRender_TFO;

	GLuint m_unstable_VBO;
	GLuint m_unstable_TFO;

	GLuint m_updateIndex_VBO;

	GLuint m_newUnstable_TFO;
	GLuint m_newUnstableIndex_VBO;

	GLuint countQuery;
	GLuint count;


	// POSE RECOVERY
	std::vector<gPosePair> poseLibrary;

	Eigen::Matrix<double, 6, 1> m_cumTwist;

	float m_alignmentEnergy = 0.0f;


	//int m_depth_height = 0; // set these properly from main?
	//int m_depth_width = 0;
	int m_color_height = 1080;
	int m_color_width = 1920;

	// CAMERA DATA
	std::vector<glm::mat4> m_extrinsics;

	// TRACKING DATA
	// glm::vec3 m_volSize = glm::vec3(256,256,256);
	glm::mat4 m_K;
	glm::mat4 m_invK;
	std::vector<glm::vec4> m_camPamsDepth;
	std::vector<glm::vec4> m_camPamsColor;
	glm::mat4 m_pose;




	Eigen::Matrix<float, 4, 4, Eigen::ColMajor> m_pose_eig;// = Eigen::MatrixXf::Identity(4, 4);







	std::vector<float> makeJTJ(std::vector<float> v)
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

	// NOT USED
	static inline Eigen::Matrix<double, 3, 3, Eigen::RowMajor> rodrigues(const Eigen::Vector3d & src)
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
				R[k] = c*I[k] + c1*rrt[k] + s*_r_x_[k];
			}

			memcpy(dst.data(), &R[0], sizeof(Eigen::Matrix<double, 3, 3, Eigen::RowMajor>));
		}

		return dst;
	}

	// NOT USED
	static inline void computeUpdateSE3(Eigen::Matrix<double, 4, 4, Eigen::RowMajor> & resultRt, const Eigen::Matrix<double, 6, 1> & result, Eigen::Isometry3f & rgbOdom)
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

	inline int divup(int a, int b) { return (a % b != 0) ? (a / b + 1) : (a / b); }




	std::vector<float> edgeTable;
	std::vector<float> triTex;
	std::vector<float> numVertsTable;
	
	uint32_t m_totalVerts;

	
	std::vector<uint32_t> debugArray;

	//GLuint  prefixSum(GLuint inputBuffer, GLuint outputBuffer);


	double raycastTime;
	double marchingCubesTime;
	double trackTime;
	double reduceTime;
	double integrateTime;
	double trackSDFTime;
	double reduceSDFTime;

	// output from get reudction
	std::vector<float> outputData;

	GLuint query[7];

	void getPreRedu(Eigen::Matrix<double, 6, 6> &A, Eigen::Matrix<double, 6, 1> &b);

	//cv::Mat testIm = cv::Mat(424, 512, CV_32FC4);
	//cv::Mat testIm2 = cv::Mat(424, 512, CV_32FC4);

	
	GLuint m_lmbuff_0;

	GLuint m_lmbuff_1;

	std::vector<int> listmode;
	//cv::Mat mipMat = cv::Mat(424,512, CV_32FC4);


	bool m_usingFloatDepth;
	float m_depthUnit;

	bool m_clickedPoint = false;
	float m_pointX = 0.0f;
	float m_pointY = 0.0f;

	glm::mat4 m_colorToColor;
	glm::mat4 m_depthToDepth = glm::mat4(1.0f);

	std::vector<uint64_t> m_previousTime;
	std::vector<uint64_t> m_sensorsTimestamps;
	int m_numberOfCameras;
	uint64_t timeShiftOffsets = 0;
	float indexMapScaleFactor = 4.0f;
	uint32_t textureDimension = 3072; // but why?
	GLuint inputDepthCount;
	GLuint fuseCount;
	GLuint newUnstableCount;
	GLuint globalVertCount = 0;

	int m_initUnstable;
	uint32_t m_frameCount;
	uint32_t m_timeDelta = 200;
	float m_depthMax = 0.5f;

};
