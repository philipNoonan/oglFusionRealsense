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

#include "interface.h"
//#include "openCVStuff.h"
#include "gFusion.h"
#include "gDisOptFlow.h"
#include "mcubes.h"

#include "ImGuiFileDialog.h"

#include "flood.h"

#include "opencv2/core/utility.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/optflow.hpp"


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


GLFWwindow *window;

kRender krender;

Realsense2Camera kcamera;
static bool cameraRunning = false;


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
gDisOptFlow gdisoptflow;

void setUpGPU();


//cv::Mat flow;// = cv::Mat(424, 512, CV_8UC3);
//cv::Mat tFlow;

//INTERFACE REALSENSE STUFF
int dispShift = 0;

float depthMin = 0.0f;
float depthMax = 4.0f;
/////////////////////////
// KINECT STUFF

const int screenWidth = 1920;
const int screenHeight = 1080;

const int colorWidth = 848;
const int colorHeight = 480;

int depthWidth = 640;
int depthHeight = 480;

float *mainColor[colorWidth * colorHeight];

unsigned char colorArray[4 * 848 * 480];

//float previousColorArray[depthWidth * depthHeight];
//float bigDepthArray[colorWidth * (colorHeight + 2)]; // 1082 is not a typo
													 //float color[512 * 424];
std::vector<uint16_t> depthArray;
//float infraredArray[depthWidth * depthHeight];
//int colorDepthMap[depthWidth * depthHeight];

// depth color points picking
bool select_color_points_mode = false;
bool select_depth_points_mode = false;

//std::vector<cv::Point3f> depthPoints;
//std::vector<cv::Point2f> colorPoints;
//cv::Mat newColor;

bool showDepthFlag = false;
bool showBigDepthFlag = false;
bool showInfraFlag = false;
bool showColorFlag = true;
bool showLightFlag = false;
bool showPointFlag = false;

bool showFlowFlag = false;
bool showEdgesFlag = false;
bool showNormalFlag = true;
bool showVolumeFlag = false;
bool showTrackFlag = true;
bool showSDFVolume = false;

float irBrightness = 1.0;
float irLow = 0.0f;
float irHigh = 65536.0f;
float vertFov = 40.0f;



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
bool performFlood = false;

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

glm::vec3 initOffset(int pixX, int pixY)
{
	int pointX = float(pixX) * (float(depthWidth) / float(display2DWindow.w));
	int pointY = depthHeight - float(pixY) * (float(depthHeight) / float(display2DWindow.h));
	//std::cout << std::endl;
	//std::cout << "depth width " << depthWidth << " px " << pointX << " py " << pointY << " size " << depthArray.size() << " valu " << pointY * depthWidth + pointX << std::endl;
	float z = float(depthArray[pointY * depthWidth + pointX]) * kcamera.getDepthUnit() / 1000000.0f;
	//kcamera.fx(), kcamera.fx(), kcamera.ppx(), kcamera.ppy()
	std::cout << z << std::endl;

	float x = (pointX - kcamera.ppx()) * (1.0f / kcamera.fx()) * z;
	float y = (pointY - kcamera.ppy()) * (1.0f / kcamera.fx()) * z;

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


std::deque<std::vector<float> > graphPoints;


// FLOW STUFF

//std::vector<cv::Mat> imagesFromFile;
//std::vector<cv::VideoCapture> videosFromFile;


int imageNumber = 0;

bool imguiFocus2D = false;


float mouseX = 0;
float mouseY = 0;

glm::vec2 mousePos = glm::vec2(0,0);
