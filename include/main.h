#include <imgui.h>
#include "imgui_impl_glfw_gl3.h"
#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

#include <stdio.h>
#include <iostream>
#define GLUT_NO_LIB_PRAGMA
//##### OpenGL ######
#include <GL/glew.h>
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>
#include <deque>
#include <thread>
#include <chrono>
#include <fstream>



#include "kRender.h"
#include "renderHelpers.h"

#include "openPoseWrapper.h"

#include "interface.h"
//#include "openCVStuff.h"
#include "gFusion.h"
//#include "gDisOptFlow.h"
#include "mcubes.h"

#include "ImGuiFileDialog.h"

#include "flood.h"
#include "flow.h"

#include "opencv2/core/utility.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/optflow.hpp"

#include "markerTracking.h"


#include <chrono>
#include <time.h>
#include <ctime>   // localtime
#include <sstream> // stringstream
#include <iomanip> 



//const std::string currentDateTime();
//const std::string epochTime();
//
//const std::string currentDateTime() {
//	timeval curTime;
//	DWORD ms = GetTickCount();
//	time_t	now = time(0);
//	struct tm tstruct;
//	char buf[80];
//	tstruct = *localtime(&now);
//	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
//	return buf;
//}

std::string return_current_time_and_date()
{
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d_%H-%M-%S");
	return ss.str();
}

const std::string epochTime() {
	//DWORD ms = GetTickCount();
	//std::ostringstream stream;
	//stream << ms;
	//return stream.str();
	using namespace std::chrono;
	milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	std::ostringstream stream;
	stream << ms.count();
	return stream.str();
}

const double epchTime() {
	//DWORD ms = GetTickCount();
	//std::ostringstream stream;
	//stream << ms;
	//return stream.str();
	using namespace std::chrono;
	milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	std::ostringstream stream;
	return ms.count();

}

inline void writeTrackDataToFile();


GLFWwindow *window;

kRender krender;

//Realsense2Camera kcamera;
Realsense2Interface cameraInterface;
int cameraDevice = 0;
std::vector<std::tuple<int, int, int, rs2_format>> depthProfiles;
std::vector<std::tuple<int, int, int, rs2_format>> colorProfiles;
std::vector<std::tuple<int, int, int, rs2_format>> infraProfiles;


static bool cameraRunning = false;

bool usingDataFromFile = false;

MCubes mcubes;

gFlood gflood;

renderWindow navigationWindow;
renderWindow display2DWindow;
renderWindow display3DWindow;
renderWindow graphWindow;
anchorPoint controlPoint0;


//openCVStuff OCVStuff;

gFusion gfusion;
gFusionConfig gconfig;
mCubeConfig mcconfig;
//gDisOptFlow gdisoptflow;
gFlow gflow;


void setUpGPU();


//cv::Mat flow;// = cv::Mat(424, 512, CV_8UC3);
//cv::Mat tFlow;

//INTERFACE REALSENSE STUFF
int dispShift = 0;

float depthMin = 0.0f;
float depthMax = 1.5f;

static int eRate = 90;
static int eRes = 0;

/////////////////////////
// KINECT STUFF

const int screenWidth = 1920;
const int screenHeight = 1080;

std::vector<glm::ivec2> depthFrameSize;
std::vector<glm::ivec2> colorFrameSize;
std::vector<glm::ivec2> infraFrameSize;



//unsigned char colorArray[4 * 848 * 480];

//float previousColorArray[depthWidth * depthHeight];
//float bigDepthArray[colorWidth * (colorHeight + 2)]; // 1082 is not a typo
							
std::vector<uint16_t> colorArray;//float color[512 * 424];
std::vector<uint16_t> depthArray;
//float infraredArray[depthWidth * depthHeight];
//int colorDepthMap[depthWidth * depthHeight];

// depth color points picking
bool select_color_points_mode = false;
bool select_depth_points_mode = false;

//std::vector<cv::Point3f> depthPoints;
//std::vector<cv::Point2f> colorPoints;
//cv::Mat newColor;

bool showDepthFlag = true;
bool showBigDepthFlag = false;
bool showInfraFlag = false;
bool showColorFlag = true;
bool showLightFlag = false;
bool showPointFlag = false;

bool showFlowFlag = false;
bool showEdgesFlag = false;
bool showNormalFlag = true;
bool showVolumeFlag = false;
bool showTrackFlag = false;
bool showSDFVolume = false;
bool showMarkerFlag = false;

float irBrightness = 1.0;
float irLow = 0.0f;
float irHigh = 65536.0f;
float vertFov = 40.0f;

bool emitterStatus = true;


float xRot = 0.0f;
float zRot = 0.0f;
float yRot = 0.0f;
float xTran = 0.0f;
float yTran = 0.0f;
float zTran = 2000.0f;
void resetSliders() 
{
	xRot = 0.0f;
	zRot = 0.0f;
	yRot = 0.0f;
	xTran = 0.0f;
	yTran = 0.0f;
	zTran = 2000.0f;
}

float zModelPC_offset = 0.0f;

//cv::Mat infraGrey;

bool calibratingFlag = false;

//////////////////////////////////////////////////
// SAVING IMAGES

// FUSION STUFF
bool trackDepthToPoint = false;
bool trackDepthToVolume = true;
bool useMultipleFusion = false;
bool performFlood = false;
bool performFlow = false;
bool performAruco = false;

int counter = 0;
bool reset = true;
bool integratingFlag = true;
bool selectInitialPoseFlag = false;

const char* sizes[] = { "32", "64", "128", "256", "384", "512", "768", "1024" };

static int sizeX = 2;
static int sizeY = 2;
static int sizeZ = 2;
float dimension = 1.0f;
float volSlice = 0.0f;

glm::vec3 iOff;

glm::vec3 initOffset(int devNumber, int pixX, int pixY)
{
	int pointX = float(pixX) * (float(depthFrameSize[devNumber].x) / float(display2DWindow.w));
	int pointY = depthFrameSize[devNumber].y - float(pixY) * (float(depthFrameSize[devNumber].y) / float(display2DWindow.h));
	//std::cout << std::endl;
	//std::cout << "depth width " << depthWidth << " px " << pointX << " py " << pointY << " size " << depthArray.size() << " valu " << pointY * depthWidth + pointX << std::endl;
	//float z = float(depthArray[pointY * depthWidth + pointX]) * (float)cameraInterface.getDepthUnit(cameraDevice) / 1000000.0f;


	rs2::frame depthFrame;
	float z = 0.0f;

	// bug be here
	depthFrame = cameraInterface.getDepthQueues()[cameraDevice].wait_for_frame();


	const uint16_t* p_depth_frame = reinterpret_cast<const uint16_t*>(depthFrame.get_data());
	//float z = float(depthArray[pointY * depthWidth + pointX]) * (float)cameraInterface.getDepthUnit(cameraDevice) / 1000000.0f;
	int depth_pixel_index = (pointY * depthFrameSize[devNumber].x + pointX);
	z = p_depth_frame[depth_pixel_index] * (float)cameraInterface.getDepthUnit(cameraDevice) / 1000000.0f;
	//std::cout << z << std::endl;



	//kcamera.fx(), kcamera.fx(), kcamera.ppx(), kcamera.ppy()
	//std::cout << z << std::endl;

	float x = (pointX - cameraInterface.getDepthIntrinsics(cameraDevice).cx) * (1.0f / cameraInterface.getDepthIntrinsics(cameraDevice).fx) * z;
	float y = (pointY - cameraInterface.getDepthIntrinsics(cameraDevice).cy) * (1.0f / cameraInterface.getDepthIntrinsics(cameraDevice).fy) * z;

	//std::cout << "HAVE I BEEN SET CORRECTLY FROM DEPTH UNITS CFROM SENSOR??? x " << x << " y " << y << " z " << z << std::endl;


	return glm::vec3(x, y, z);

}

// GRAPHING STUFF
float arrayX[3600];
float arrayY[3600];
float arrayZ[3600];

std::pair<float, float> minmaxX = std::make_pair<float, float>(-0.1f, 0.1f);
std::pair<float, float> minmaxY = std::make_pair<float, float>(-0.1f, 0.1f);;
std::pair<float, float> minmaxZ = std::make_pair<float, float>(-0.1f, 0.1f);;


std::deque<glm::vec4> graphPoints;


// FLOW STUFF

//std::vector<cv::Mat> imagesFromFile;
//std::vector<cv::VideoCapture> videosFromFile;


int imageNumber = 0;

bool imguiFocus2D = false;


float mouseX = 0;
float mouseY = 0;

glm::vec2 mousePos = glm::vec2(0,0);

int numberOfCameras;

/// MARKER TRACKING
MarkerTracker mTracker;
std::vector<glm::mat4> markerMats;

glm::mat4 cam2camTrans(1.0f);

std::vector<glm::mat4> colorToDepth;
std::vector<glm::mat4> depthToColor;

glm::mat4 depthToDepth;

bool performStereo = false;
bool pairFromFile = false;
// GEM STUFF

int desiredWidth = 848;
int desiredHeight = 480;
int desiredRate = 90;

int desiredColorWidth = 1920;
int desiredColorHeight = 1080;
int desiredColorRate = 6;