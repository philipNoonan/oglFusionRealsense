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
#include <glm/gtx/string_cast.hpp>


#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API

//#include "shader.hpp"


#include "opencv2/core/utility.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"




#include <iostream>
#include <fstream>

#include <vector>
#include <list>
#include <numeric>
#include <valarray>

#include "glutils.h"
#include "glslprogram.h"
#include "glhelper.h"

#define _USE_MATH_DEFINES
#include <math.h>

class kRender
{
public:
	kRender()
		: m_window()
		, m_show_imgui(true)
		, m_screen_height(1080)
		, m_screen_width(1920)
		, m_depth_height(480)
		, m_depth_width(848)
		, m_color_height(480)
		, m_color_width(848)
		, m_big_depth_height(1082)
		, m_big_depth_width(1920)
		, m_VAO()
		, m_VBO_Color()
		, m_VBO_Depth()
		, m_EBO()
		, m_gui_padding(std::make_pair<int, int>(50, 50))
		, m_render_scale_height(1.0f)
		, m_render_scale_width(1.0f)
		, m_colorToDepth(glm::mat4(1.0f))
		//, m_graph_points_x()
		//, m_graph_points_y()
		//, m_graph_points_z()
		//, m_graph_points_long_x()
		//, m_graph_points_long_y()
		//, m_graph_points_long_z()
		//, m_graph_vector_x()
	{}
	~kRender();

	GLFWwindow * window()
	{
		return m_window;
	}

	void setWindow(GLFWwindow* win)
	{
		m_window = win;
	}

	void SetCallbackFunctions();

	bool showImgui()
	{
		return m_show_imgui;
	}

	float renderScaleHeight()
	{
		return m_render_scale_height;
	}
	void renderScaleHeight(float scale)
	{
		m_render_scale_height = scale;
	}
	float renderScaleWidth()
	{
		return m_render_scale_width;
	}
	void renderScaleWidth(float scale)
	{
		m_render_scale_width = scale;
	}
	
	void setDepthMinMax(float min, float max)
	{
		m_depthMin = min;
		m_depthMax = max;
	}
	void setDepthHeight(int h)
	{
		m_depth_height = h;
	}
	void setDepthWidth(int w)
	{
		m_depth_width = w;
	}
	void setDisplayOriSize(int x, int y, int w, int h)
	{
		m_display2DPos = glm::vec2(x, y);
		m_display2DSize = glm::vec2(w, h);
	}
	
	void set3DDisplayOriSize(int x, int y, int w, int h)
	{
		m_display3DPos = glm::vec2(x, y);
		m_display3DSize = glm::vec2(w, h);
	}

	void setColorDisplayOriSize(int x, int y, glm::ivec2 fSize)
	{
		m_displayColorPos = glm::vec2(x, y);
		m_displayColorSize = fSize;
	}
	//std::vector<float> graphPointsX()
	//{
	//	return m_graph_vector_x;
	//}

	void setBodyPosePoints(std::vector<std::valarray<float>> bpp)
	{
		m_bodyPosePoints.resize(bpp.size());
		for (int person = 0; person < bpp.size(); person++)
		{
			m_bodyPosePoints[person].assign(std::begin(bpp[person]), std::end(bpp[person]));
		}
	}

	std::pair<int, int> guiPadding()
	{
		return std::make_pair<int, int>(m_gui_padding.first * m_render_scale_width, m_gui_padding.second * m_render_scale_height);
	}

	std::vector<float> getDepthPoints()
	{
		return m_depthPointsFromBuffer;
	}

	//void getDepthPoints3D();

	void getMouseClickPositionsDepth();

	void setMarkerData(std::vector<glm::mat4> tMat);

	void anchorMW(std::pair<int, int> anchor)
	{
		m_anchorMW = std::make_pair<int, int>((float)anchor.first * m_render_scale_width, (float)anchor.second * m_render_scale_height);
	}
	void anchorSG(std::pair<int, int> anchor)
	{
		m_anchorSG = anchor;
	}
	void anchorAS(std::pair<int, int> anchor)
	{
		m_anchorAS = anchor;
	}
	GLFWwindow * loadGLFWWindow();

	void compileAndLinkShader();
	void requestShaderInfo();
	void setLocations();
	void setVertPositions();
	void allocateTextures();
	void allocateBuffers();
	void setDepthFrameSize(int width, int height)
	{
		m_depth_width = width;
		m_depth_height = height;
	}
	void setColorFrameSize(int width, int height)
	{
		m_color_width = width;
		m_color_height = height;
	}

	void setColorToDepth(glm::mat4 col2Dep)
	{
		m_colorToDepth = col2Dep;
	}
	void setDepthToColor(glm::mat4 dep2Col)
	{
		m_depthToColor = dep2Col;
	}
	void setColorFrame(std::vector<uint16_t> imageArray);
	void setColorFrame(std::vector<rs2::frame_queue> colorQ, int devNumber, cv::Mat &colorMat);

	void setInfraFrame(std::vector<rs2::frame_queue> infraQ, int devNumber, cv::Mat &infraMat);

	void setTextures(GLuint depthTex, GLuint colorTex, GLuint vertexTex, GLuint normalTex, GLuint volumeTex, GLuint trackTex, GLuint pvpNormTex, GLuint pvdNormTex, GLuint splatterDepth, GLuint splatterNormal);
	void setFlowTexture(GLuint flowTex);
	void setBuffersFromMarchingCubes(GLuint posBuf, GLuint normBuf, size_t numVerts)
	{
		m_posBufMC = posBuf;
		m_normBufMC = normBuf;
		m_numVerts = numVerts;
	}
	void setTrackedPointsBuffer(GLuint trackPointsBuf)
	{
		GLint size = 100 * 100 * 2 * sizeof(float);
		glBindBuffer(GL_COPY_READ_BUFFER, trackPointsBuf);
		glGetBufferParameteriv(GL_COPY_READ_BUFFER, GL_BUFFER_SIZE, &size);

		glBindBuffer(GL_COPY_WRITE_BUFFER, m_bufferTrackedPoints);
		glBufferData(GL_COPY_WRITE_BUFFER, size, nullptr, GL_STATIC_DRAW);

		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, size);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);



		std::vector<float> outputSDFData;
		outputSDFData.resize(100 * 100 * 2);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferTrackedPoints);
		void *ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		memcpy_s(outputSDFData.data(), outputSDFData.size() * sizeof(float), ptr, outputSDFData.size() * sizeof(float));
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);



	}
	void bindBuffersForRendering();

	//void setWindowPositions();
	//void setWindowPositions(std::pair<int, int> anchorMW, std::pair<int, int> anchorAS, std::pair<int, int> anchorSG);
	void setWindowLayout();
	void setupComputeFBO();

	// The correcter way 
	void setRenderingOptions(bool showDepthFlag, bool showBigDepthFlag, bool showInfraFlag, bool showColorFlag, bool showLightFlag, bool showPointFlag, bool showFlowFlag, bool showEdgesFlag, bool showNormalFlag, bool showVolumeSDFFlag, bool showTrackFlag, bool showSFFlag, bool showMarkerFlag, bool showDepthSplat, bool showNormalSplat);
	void setBuffersForRendering(float * depthArray, float * bigDepthArray, float * colorArray, float * infraArray, unsigned char * flowPtr);
	void setDepthImageRenderPosition(float vertFov);
	void setNormalImageRenderPosition();
	void setRayNormImageRenderPosition(float vertFov);
	void setColorImageRenderPosition(float vertFov);
	void setInfraImageRenderPosition();
	void setTrackImageRenderPosition(float vertFov);
	void setFlowImageRenderPosition(float vertFov);
	void setPointCloudRenderPosition(float modelZ);
	void setLightModelRenderPosition();
	void setMarchingCubesRenderPosition(float modelZ);
	void setViewMatrix(float xRot, float yRot, float zRot, float xTran, float yTran, float zTran);
	void setProjectionMatrix(int camDevice);
	void setDepthTextureProjectionMatrix();
	void setColorTextureProjectionMatrix();
	void setViewport(int x, int y, int w, int h);

	void setColorDepthMapping(int* colorDepthMap);

	void setVolumeSize(glm::vec3 size)
	{
		m_volume_size = size;
	}
	void updateVerts(float w, float h);

	void bindTexturesForRendering();

	void setVolumeSDFRenderPosition(float slice);

	void Render(bool useInfrared, int devNumber);

	void renderLiveVideoWindow(bool useInfrared, int devNumber);

	void setComputeWindowPosition(int x, int y, int w, int h);

	void setNumberOfCameras(int numberOfCameras)
	{
		m_cameraParams.resize(numberOfCameras);
		m_cameraParams_color.resize(numberOfCameras);
	}

	void setCameraParams(int devNumber, glm::vec4 camPams, glm::vec4 camPamsColor)
	{
		m_cameraParams[devNumber] = camPams;
		m_cameraParams_color[devNumber] = camPamsColor;
	}


	void setExportPly(bool opt)
	{
		m_export_ply = opt;
	}

	/*
	void setRVec(
	Mat rvec)
	{
	rotation_vector = rvec;
	}
	void setTVec(cv::Mat tvec)
	{
	translation_vector = tvec;
	}*/
	void setIrBrightness(float irL, float irH)
	{
		m_ir_low = irL;
		m_ir_high = irH;
	}

	void setFov(float fov)
	{
		m_vertFov = fov;
	}

	GLuint getColorTexture()
	{
		return m_textureColor;
	}


	//void exportPointCloud();

	// compute shader time
	void renderPointCloud(bool useBigDepth = false);
	void renderTSDFPointCloud();

	// FLOOD STUFF
	void setFloodTexture(GLuint floodTex)
	{
		m_textureFlood = floodTex;
	}


	// TSDF STUFF

	void genTexCoordOffsets(GLuint width, GLuint height, GLfloat step);

	void setSelectInitPose(bool flag)
	{
		m_selectInitialPoseFlag = flag;
	}
	float getCenterPixX()
	{
		return m_center_pixX;
	}
	float getCenterPixY()
	{
		return m_center_pixY;
	}

	void setFusionType(bool usePVP, bool usePVD, bool useSplatter)
	{
		m_usePVP = usePVP;
		m_usePVD = usePVD;
		m_useSplatter = useSplatter;
	}

	void setDepthToDepth(glm::mat4 dep2dep)
	{
		m_depthToDepth = dep2dep;
	}
	void setOtherMarkerData(std::vector<glm::mat4> tMat);

private:

	GLSLProgram renderProg;

	GLFWwindow * m_window;
	bool m_show_imgui;

	GLuint m_VAO, m_EBO;
	GLuint m_VBO_Standard, m_VBO_Color, m_VBO_Depth, m_VBO_Markers;
	std::vector<float> m_standard_verts;
	std::vector<float> m_color_vert;
	std::vector<float> m_depth_vert;
	std::vector<float> m_vertices;
	std::vector<unsigned int> m_indices;

	GLuint m_VAO_MC;
	GLuint m_VBO_Vert_MC;

	GLuint m_VAO_Markers;

	GLuint m_poseVAO;
	GLuint m_poseVBO;
	GLuint m_poseEBO;

	GLuint m_programID;
	GLuint m_ProjectionID;
	GLuint m_MvpID;
	GLuint m_ModelID;
	GLuint m_ViewProjectionID;
	GLuint m_sliceID;
	GLuint m_imSizeID;
	GLuint m_depthScaleID; // remember to set me from main
	GLuint m_depthRangeID;
	GLuint m_cameraDeviceID;

	GLuint m_renderOptionsID;

	GLuint m_fromStandardFragmentID;

	GLuint m_getPositionSubroutineID;
	GLuint m_fromTextureID;
	GLuint m_fromPosition4DID;
	GLuint m_fromPosition2DID;
	GLuint m_fromStandardTextureID;
	GLuint m_colorSelectionRoutineID;
	GLuint m_fromDepthID;
	GLuint m_fromColorID;
	GLuint m_fromRayNormID;
	GLuint m_fromRayVertID;
	GLuint m_fromPointsID;
	GLuint m_fromVolumeID;
	GLuint m_fromVolumeSDFID;
	GLuint m_fromTrackID;
	GLuint m_fromFlowID;
	GLuint m_fromPosePoints2DID;


	GLuint m_fromMarkersVerticesID;
	GLuint m_fromMarkersID;

	GLuint m_ambientID; 
	GLuint m_lightID;


	GLuint m_testTextureFragOut;

	//GLuint m_irLowID;
	//GLuint m_irHighID;


	//textures
	GLuint m_textureDepth;
	GLuint m_textureInfra;
	GLuint m_textureColor;


	GLuint m_textureVertex;
	GLuint m_textureNormal;
	GLuint m_textureVolume;
	GLuint m_textureTrack;
	GLuint m_textureFlow;
	GLuint m_texturePVPNormal;
	GLuint m_texturePVDNormal;
	GLuint m_textureSplatterDepth;
	GLuint m_textureSplatterNormal;

	GLuint m_textureFlood;

	int m_screen_height;
	int m_screen_width;
	int m_depth_height;
	int m_depth_width;
	int m_color_height;
	int m_color_width;
	int m_big_depth_height;
	int m_big_depth_width;

	float m_depthMin = 0.0f;
	float m_depthMax = 10.0f;

	float m_render_scale_height;
	float m_render_scale_width;

	std::pair<int, int> m_anchorMW;
	std::pair<int, int> m_anchorAS;
	std::pair<int, int> m_anchorSG;

	std::pair<int, int> m_gui_padding;

	std::vector<float> m_trackedPoints;
	GLuint m_bufferTrackedPoints;

	glm::mat4 ColorView = glm::translate(glm::mat4(1.0f), glm::vec3(-0.f, -0.f, -0.0f));

	std::vector<glm::vec4> m_cameraParams;
	std::vector<glm::vec4> m_cameraParams_color;

	// k.x = fx, k.y = fy, k.z = cx, k.w = cy, skew = 1
	glm::mat4 getInverseCameraMatrix(const glm::vec4 & k) { // [col][row]
		glm::mat4 invK;
		invK[0][0] = 1.0f / k.x;	invK[1][0] = 0;				invK[2][0] = -k.z / k.x;	invK[3][0] = 0;
		invK[0][1] = 0;				invK[1][1] = 1.0f / k.y;	invK[2][1] = -k.w / k.y;	invK[3][1] = 0;
		invK[0][2] = 0;				invK[1][2] = 0;				invK[2][2] = 1;				invK[3][2] = 0;
		invK[0][3] = 0;				invK[1][3] = 0;				invK[2][3] = 0;				invK[3][3] = 1;

		return invK;
	}

	glm::mat4 getCameraMatrix(const glm::vec4 & k) {
		glm::mat4 K;

		K[0][0] = k.x;	K[1][0] = 0;	K[2][0] = k.z;	K[3][0] = 0;
		K[0][1] = 0;	K[1][1] = k.y;	K[2][1] = k.w;	K[3][1] = 0;
		K[0][2] = 0;	K[1][2] = 0;	K[2][2] = 1;	K[3][2] = 0;
		K[0][3] = 0;	K[1][3] = 0;	K[2][3] = 0;	K[3][3] = 1;

		return K;
	}



	inline int divup(int a, int b) { return (a % b != 0) ? (a / b + 1) : (a / b); }

	void writePLYFloat(std::vector<float> PC, std::vector<float> NC, const char* FileName);




	bool m_export_ply = false;

	float m_mouse_pos_x;
	float m_mouse_pos_y;
	glm::mat4 m_registration_matrix = glm::mat4(1.0f);

	// this static wrapped clas was taken from BIC comment on https://stackoverflow.com/questions/7676971/pointing-to-a-function-that-is-a-class-member-glfw-setkeycallback
	void MousePositionCallback(GLFWwindow* window, double positionX, double positionY);
	void KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

	class GLFWCallbackWrapper
	{
	public:
		GLFWCallbackWrapper() = delete;
		GLFWCallbackWrapper(const GLFWCallbackWrapper&) = delete;
		GLFWCallbackWrapper(GLFWCallbackWrapper&&) = delete;
		~GLFWCallbackWrapper() = delete;

		static void MousePositionCallback(GLFWwindow* window, double positionX, double positionY);
		static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void SetApplication(kRender *application);
	private:
		static kRender* s_application;
	};

	std::vector<float> m_depthPointsFromBuffer;
	std::vector<std::pair<int, int>> m_depthPixelPoints2D;

	//cv::Mat rotation_vector; // Rotation in axis-angle form
	//cv::Mat translation_vector;


	float m_ir_low = 0.0f;
	float m_ir_high = 65536.0f;
	float m_vertFov = 35.0f;
	//std::vector<cv::Point2f> m_detected_points_color;
	//std::vector<cv::Point2f> m_detected_points_infra;

	glm::mat4 m_model_depth = glm::mat4(1.0);
	glm::mat4 m_model_normal = glm::mat4(1.0f);
	glm::mat4 m_model_color = glm::mat4(1.0);
	glm::mat4 m_model_infra = glm::mat4(1.0);
	glm::mat4 m_model_track = glm::mat4(1.0);
	glm::mat4 m_model_flow = glm::mat4(1.0f);
	glm::mat4 m_model_pointcloud = glm::mat4(1.0f);
	glm::mat4 m_model_lightmodel = glm::mat4(1.0f);
	glm::mat4 m_model_MC = glm::mat4(1.0f);
	glm::mat4 m_model_volume = glm::mat4(1.0f);
	glm::mat4 m_model_raynorm = glm::mat4(1.0f);
	glm::mat4 m_view = glm::mat4(1.0f);
	glm::mat4 m_projection = glm::mat4(0.0f); // some default matrix
    glm::mat4 m_projectionColor = glm::mat4(0.0f); // some default matrix
	glm::vec3 m_volume_size = glm::vec3(128.0f, 128.0f, 128.0f);


	int getRenderOptions(bool depth, bool infra, bool color, bool norm, bool track, bool flood, bool volume, bool splatterDepth, bool splatterNormal);

	bool m_showDepthFlag = false;
	bool m_showNormalFlag = false;
	bool m_showInfraFlag = false;
	bool m_showColorFlag = false;
	bool m_showBigDepthFlag = false;
	bool m_showLightFlag = false;
	bool m_showPointFlag = false;
	bool m_showFlowFlag = false;
	bool m_showEdgesFlag = false;
	bool m_showVolumeSDFFlag = false;
	bool m_showTrackFlag = false;
	bool m_showSplatterDepth = false;
	bool m_showSplatterNormal = false;
	bool m_showVolumeFlag = false;
	bool m_showMarkersFlag = false;

	uint32_t m_renderOptions = 0;

	bool m_usePVP;
	bool m_usePVD;
	bool m_useSplatter;


	const GLint tcOffsetColumns = 5;
	const GLint tcOffsetRows = 5;
	GLint filterNumberCode = 0;
	GLint texCoordOffsets[5 * 5 * 2];
	GLuint m_tcOffsetID;
	GLuint m_contrastID;

	GLuint m_posBufMC;
	GLuint m_normBufMC;
	GLuint m_numVerts;

	bool m_selectInitialPoseFlag;
	float m_center_pixX;
	float m_center_pixY;

	float m_volumeSDFRenderSlice = 0;

	glm::vec2 m_display2DPos;
	glm::vec2 m_display2DSize;

	glm::vec2 m_display3DPos;
	glm::vec2 m_display3DSize;

	glm::vec2 m_displayColorPos;
	glm::vec2 m_displayColorSize;

	glm::mat4 m_tMat[256];
	glm::mat4 m_tDMat[256];
	glm::mat4 m_tOtherDMat[256];
	int m_numMarkers = 0;
	int m_numMarkersOther = 0;
	glm::mat4 m_colorToDepth;
	glm::mat4 m_depthToColor;
	glm::mat4 m_depthToDepth;


	std::vector<std::vector<float>> m_bodyPosePoints;

};