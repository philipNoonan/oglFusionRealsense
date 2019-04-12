#include "main.h"

//#define USEINFRARED
//#define USEWEBCAM
//#define USEIMAGES
//#define USEVIDEO
//#define USETESTIMAGE

//
//#include "opencv2/core/utility.hpp"
//#include "opencv2/opencv.hpp"
//#include "opencv2/imgproc/imgproc.hpp"
//#include "opencv2/highgui/highgui.hpp"

// To split this up we will have just rendering calls in 
// kRender - rendering calls
// gFusion - raycasting, integrating, set, reset, getPose, setPose, depth to vert, vert to normals, 

// memory bank
// volume - 3D Texture short2 RG16I, 256x256x256
// depth - 2D Texture float RED, 512x424 mipmapped
// color - 2D Texture unsigned byte RGBA, 1920x1080
// vertex - 2D Texture float RGBA32F, vector of textures mipmapped
// normal - 2D Texture float RGBA32F, vector of textures mipmapped



// Doesnt work :( cant seem to easily get laptop to use integrated GPU from openGL context. 
//#include <windows.h>
//extern "C" {
//	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000000;
//}







static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error %d: %s\n", error, description);
}

void kRenderInit()
{
	krender.SetCallbackFunctions();
	krender.compileAndLinkShader();
	krender.setCameraParams(glm::vec4(cameraInterface.getDepthIntrinsics(cameraDevice).fx,
									  cameraInterface.getDepthIntrinsics(cameraDevice).fy, 
								      cameraInterface.getDepthIntrinsics(cameraDevice).cx, 
									  cameraInterface.getDepthIntrinsics(cameraDevice).cy), 
						    glm::vec4(cameraInterface.getColorIntrinsics(cameraDevice).fx, 
									  cameraInterface.getColorIntrinsics(cameraDevice).fy,
									  cameraInterface.getColorIntrinsics(cameraDevice).cx,
								      cameraInterface.getColorIntrinsics(cameraDevice).cy));

	// Set locations
	krender.setLocations();
	krender.setVertPositions();
	krender.allocateBuffers();
	krender.setTextures(gfusion.getDepthImage(), gflow.getColorTexture(), gfusion.getVerts(), gfusion.getNorms(), gfusion.getVolume(), gfusion.getTrackImage(), gfusion.getPVPNorms(), gfusion.getPVDNorms()); //needs texture uints from gfusion init
	krender.anchorMW(std::make_pair<int, int>(1920 - 512 - krender.guiPadding().first, krender.guiPadding().second));

	krender.setFusionType(trackDepthToPoint, trackDepthToVolume);

	gflow.setColorTexture(krender.getColorTexture());
	krender.setDepthMinMax(depthMin, depthMax);

}

void gFusionInit()
{
	int devNumber = 0;
	depthArray.resize(depthFrameSize[devNumber].x * depthFrameSize[devNumber].y, 1);
	colorArray.resize(depthFrameSize[devNumber].x * depthFrameSize[devNumber].y, 0);
	//gfusion.queryDeviceLimits();
	gfusion.compileAndLinkShader();
	gfusion.setLocations();

	gconfig.volumeSize = glm::vec3(128);
	gconfig.volumeDimensions = glm::vec3(1.0f);
	gconfig.depthFrameSize = glm::vec2(depthFrameSize[devNumber]);
	gconfig.mu = 0.05f;
	gconfig.maxWeight = 100.0f;
	gconfig.iterations[0] = 2;
	gconfig.iterations[1] = 4;
	gconfig.iterations[2] = 6;
	
	gfusion.setCameraParams(glm::vec4(cameraInterface.getDepthIntrinsics(cameraDevice).fx,
									  cameraInterface.getDepthIntrinsics(cameraDevice).fy,
									  cameraInterface.getDepthIntrinsics(cameraDevice).cx,
									  cameraInterface.getDepthIntrinsics(cameraDevice).cy),
							glm::vec4(cameraInterface.getColorIntrinsics(cameraDevice).fx,
									  cameraInterface.getColorIntrinsics(cameraDevice).fy,
									  cameraInterface.getColorIntrinsics(cameraDevice).cx,
									  cameraInterface.getColorIntrinsics(cameraDevice).cy));

	glm::mat4 initPose = glm::translate(glm::mat4(1.0f), glm::vec3(gconfig.volumeDimensions.x / 2.0f, gconfig.volumeDimensions.y / 2.0f, 0.0f));

	gfusion.setConfig(gconfig);
	gfusion.setPose(initPose);
	gfusion.setPoseP2V(initPose);

	gfusion.initTextures();
	gfusion.initVolume();
	gfusion.allocateBuffers();


	gfusion.setUsingDepthFloat(false);
	gfusion.setDepthUnit((float)cameraInterface.getDepthUnit(cameraDevice));



	volSlice = gconfig.volumeSize.z / 2.0f;


}

void gDisOptFlowInit()
{
	int devNumber = 0;
	gflow.compileAndLinkShader();
	gflow.setLocations();
#ifdef USEINFRARED
	gdisoptflow.setNumLevels(depthWidth);
	gdisoptflow.setTextureParameters(depthWidth, depthHeight);
	gdisoptflow.allocateTextures(true);
#else
	gflow.setNumLevels(colorFrameSize[devNumber].x);
	gflow.setTextureParameters(colorFrameSize[devNumber].x, colorFrameSize[devNumber].y);
	gflow.allocateTextures(false);

	krender.setFlowTexture(gflow.getFlowTexture());

#endif

	gflow.allocateBuffers();

}

void gFloodInit()
{
	gflood.setCameraParams(glm::vec4(cameraInterface.getDepthIntrinsics(cameraDevice).fx,
									 cameraInterface.getDepthIntrinsics(cameraDevice).fy,
									 cameraInterface.getDepthIntrinsics(cameraDevice).cx,
									 cameraInterface.getDepthIntrinsics(cameraDevice).cy));

	gflood.compileAndLinkShader();
	gflood.setLocations();


}

void mCubeInit()
{
	//mcconfig.gridSize = gconfig.volumeSize;
	//mcconfig.gridSizeMask = glm::uvec3(mcconfig.gridSize.x - 1, mcconfig.gridSize.y - 1, mcconfig.gridSize.z - 1);
	//mcconfig.gridSizeShift = glm::uvec3(0, log2(mcconfig.gridSize.x), log2(mcconfig.gridSize.y) + log2(mcconfig.gridSize.z));
	//mcconfig.numVoxels = mcconfig.gridSize.x * mcconfig.gridSize.y * mcconfig.gridSize.z;
	//mcconfig.voxelSize = glm::vec3(gconfig.volumeDimensions.x * 1000.0f / gconfig.volumeSize.x, gconfig.volumeDimensions.y * 1000.0f / gconfig.volumeSize.y, gconfig.volumeDimensions.z * 1000.0f / gconfig.volumeSize.z);
	//mcconfig.maxVerts = std::min(mcconfig.gridSize.x * mcconfig.gridSize.y * 128, uint32_t(128 * 128 * 128));

	//gfusion.setMcConfig(mcconfig);

}

void resetVolume()
{
	int devNumber = 0;
	//glm::mat4 initPose = glm::translate(glm::mat4(1.0f), glm::vec3(gconfig.volumeDimensions.x / 2.0f, gconfig.volumeDimensions.y / 2.0f, 0.0f));
	gflow.wipeFlow();

	bool deleteFlag = false;

	if (glm::vec3(std::stoi(sizes[sizeX]), std::stoi(sizes[sizeY]), std::stoi(sizes[sizeZ])) != gconfig.volumeSize)
	{
		deleteFlag = true;
	}

	gconfig.volumeSize = glm::vec3(std::stoi(sizes[sizeX]), std::stoi(sizes[sizeY]), std::stoi(sizes[sizeZ]));
	gconfig.volumeDimensions = glm::vec3(dimension);

	gfusion.setConfig(gconfig);

	iOff = initOffset(devNumber, mousePos.x - controlPoint0.x, controlPoint0.y - mousePos.y);

	//std::cout << "lalala " << mousePos.x - controlPoint0.x << " " << controlPoint0.y - mousePos.y<< std::endl;

	glm::mat4 initPose = glm::translate(glm::mat4(1.0f), glm::vec3(-iOff.x + gconfig.volumeDimensions.x / 2.0f, -iOff.y + gconfig.volumeDimensions.y / 2.0f, -iOff.z + dimension / 2.0));

	volSlice = gconfig.volumeSize.z / 2.0f;

	krender.setVolumeSize(gconfig.volumeSize);

	gfusion.Reset(initPose, deleteFlag);
	reset = true;
	if (trackDepthToPoint)
	{
		gfusion.raycast();
	}

	counter = 0;
	if (performFlood)
	{
		gflood.setVolumeConfig(gconfig.volumeSize.x, gconfig.volumeDimensions.x);
		gflood.allocateTextures();
	}
}

void saveSTL()
{
	mcconfig.gridSize = glm::uvec3(gconfig.volumeSize.x, gconfig.volumeSize.y, gconfig.volumeSize.z);
	mcubes.setConfig(mcconfig);
	mcubes.setVolumeTexture(gfusion.getVolume());
	//mcubes.setVolumeTexture(gflood.getFloodSDFTexture());
	mcubes.init();
	mcubes.setIsolevel(0.0f);
	mcubes.generateMarchingCubes();
	mcubes.exportMesh();
}

void startRealsense()
{


	int prevERate = eRate;
	int prevERes = eRes;

	if (cameraRunning) eRate = prevERate;
	if (eRate == 90) eRes = 0;

	if (cameraRunning) eRes = prevERes;
	if (eRes == 1) eRate = 30;


	if (cameraRunning == false)
	{
		cameraRunning = true;
		//kcamera.setPreset(eRate, eRes);
		int numberOfCameras = cameraInterface.searchForCameras();

		if (numberOfCameras > 0)
		{
			
			depthProfiles.resize(numberOfCameras, 71);
			colorProfiles.resize(numberOfCameras, 13); // 0 1920x1080x30 rgb8 // 13 1920x1080x6 rgb8

			depthFrameSize.resize(numberOfCameras);
			colorFrameSize.resize(numberOfCameras);

			gfusion.setNumberOfCameras(numberOfCameras);

			for (int camera = 0; camera < numberOfCameras; camera++)
			{
				cameraInterface.startDevice(camera, depthProfiles[camera], colorProfiles[camera]);
				int wd, hd, rd;
				int wc, hc, rc;
				cameraInterface.getDepthProperties(camera, wd, hd, rd);
				cameraInterface.getColorProperties(camera, wc, hc, rc);

				gfusion.setDepthToColorExtrinsics(cameraInterface.getDepthToColorIntrinsics(camera), camera);

				depthFrameSize[camera].x = wd;
				depthFrameSize[camera].y = hd;

				colorFrameSize[camera].x = wc;
				colorFrameSize[camera].y = hc;
			}

			krender.setColorFrameSize(colorFrameSize[0].x, colorFrameSize[0].y);
			krender.allocateTextures();

		}

		//if (eRes == 0)
		//{
		//	depthWidth = 848;
		//	depthHeight = 480;
		//}
		//else if (eRes == 1)
		//{
		//	depthWidth = 1280;
		//	depthHeight = 720;
		//}

		//kcamera.start();

		setUpGPU();

	}


}

void setImguiWindows()
{
	int width;
	int height;
	int topBarHeight = 128;

	glfwGetFramebufferSize(window, &width, &height);

	controlPoint0.x = width / 4.0f;
	controlPoint0.y = height - height / 3.0f;

	navigationWindow.set(0, topBarHeight, controlPoint0.x, height - topBarHeight, true, true, "navi");

	graphWindow.set(controlPoint0.x, controlPoint0.y, width - controlPoint0.x, height - controlPoint0.y, true, true, "graphs"); // 420 is its height

	display2DWindow.set(controlPoint0.x, topBarHeight, (width - controlPoint0.x) / 2, controlPoint0.y - topBarHeight, true, true, "2d");
	
	display3DWindow.set(controlPoint0.x + (width - controlPoint0.x) / 2, topBarHeight, (width - controlPoint0.x) / 2, controlPoint0.y - topBarHeight, true, true, "3d");

}

void setUIStyle()
{
	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();

	style.Alpha = 1.0f;
	
	style.WindowBorderSize = 1.0f;
	style.WindowRounding = 0.0f;

	style.FrameBorderSize = 1.0f;
	style.FrameRounding = 12.0f;

	style.PopupBorderSize = 0.0f;

	style.ScrollbarRounding = 12.0f;
	style.GrabRounding = 12.0f;
	style.GrabMinSize = 20.0f;


	// spacings
	style.ItemSpacing = ImVec2(8.0f, 8.0f);
	style.FramePadding = ImVec2(8.0f, 8.0f);
	style.WindowPadding = ImVec2(8.0f, 8.0f);



}

void setUI()
{
	setImguiWindows();
	// graphs
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

		ImGui::SetNextWindowSize(ImVec2(graphWindow.w, graphWindow.h), ImGuiSetCond_Always);
		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		//window_flags |= ImGuiWindowFlags_ShowBorders;
		//if (graphWindow.resize == false) window_flags |= ImGuiWindowFlags_NoResize;
		//window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoCollapse;

		ImGui::Begin("Slider Graph", &graphWindow.visible, window_flags);
		//ImGui::PushItemWidth(-krender.guiPadding().first);
		ImGui::SetWindowPos(ImVec2(graphWindow.x, graphWindow.y));
		ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(1.0, 0.0, 0.0, 1.0));
		ImGui::PlotLines("X", &arrayX[0], graphWindow.w, 0, "", minmaxX.first, minmaxX.second, ImVec2(graphWindow.w, graphWindow.h / 3));
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.0, 1.0, 0.0, 1.0));
		ImGui::PlotLines("Y", &arrayY[0], graphWindow.w, 0, "", minmaxY.first, minmaxY.second, ImVec2(graphWindow.w, graphWindow.h / 3));
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.0, 0.0, 1.0, 1.0));
		ImGui::PlotLines("Z", &arrayZ[0], graphWindow.w, 0, "", minmaxZ.first, minmaxZ.second, ImVec2(graphWindow.w, graphWindow.h / 3));
		ImGui::PopStyleColor();

		ImGui::End();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();

	}

	// 2d data
	{
		ImGui::SetNextWindowPos(ImVec2(display2DWindow.x, display2DWindow.y));
		ImGui::SetNextWindowSize(ImVec2(display2DWindow.w, display2DWindow.h), ImGuiSetCond_Always);

		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		//window_flags |= ImGuiWindowFlags_ShowBorders;
		//window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoCollapse;

		ImGui::Begin("Video ", &display2DWindow.visible, window_flags);
		imguiFocus2D = ImGui::IsWindowHovered();
		ImGui::End();
	}

	//3d data
	{
		ImGui::SetNextWindowPos(ImVec2(display3DWindow.x, display3DWindow.y));
		ImGui::SetNextWindowSize(ImVec2(display3DWindow.w, display3DWindow.h), ImGuiSetCond_Always);

		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		//window_flags |= ImGuiWindowFlags_ShowBorders;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoCollapse;

		ImGui::Begin("Video Sources", &display3DWindow.visible, window_flags);
		//ImGui::ShowDemoWindow();

		ImGui::End();
	}
	// navigation
	{
		ImGui::SetNextWindowPos(ImVec2(navigationWindow.x, navigationWindow.y));
		ImGui::SetNextWindowSize(ImVec2(navigationWindow.w, navigationWindow.h), ImGuiSetCond_Always);
		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		//window_flags |= ImGuiWindowFlags_ShowBorders;
		//window_flags |= ImGuiWindowFlags_NoResize;
		//window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoCollapse;

		float arr[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		static float maxArr[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		static int frameCounter = 0;

		arr[0] = gflow.getTimeElapsed();
		gfusion.getTimes(arr);
		arr[8] = arr[0] + arr[1] + arr[2] + arr[3] + arr[4] + arr[5] + arr[6] + arr[7];

		for (int i = 0; i < 9; i++)
		{
			if (arr[i] > maxArr[i])
			{
				maxArr[i] = arr[i];
			}
		}
		frameCounter++;
		if (frameCounter > 100)
		{
			frameCounter = 0;
			float maxArr[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		}
		ImGui::Begin("Menu", &navigationWindow.visible, window_flags);
		//ImGui::Text("Timing");
		//ImGui::Separator();

		ImGui::Text("Framerate %.3f ms/frame (%.1f FPS)", arr[8], 1000.0f / arr[8]);
		if (ImGui::BeginPopupContextItem("item context menu"))
		{
			ImGui::Text("Jump Flooding %.3f ms/frame (max : %.3f)", arr[0], maxArr[0]);
			ImGui::Text("Raycasting %.3f ms/frame (max : %.3f)", arr[1], maxArr[1]);
			ImGui::Text("TBC %.3f ms/frame (max : %.3f)", arr[2], maxArr[2]);
			ImGui::Text("Track P2P %.3f ms/frame (max : %.3f)", arr[3], maxArr[3]);
			ImGui::Text("TBC %.3f ms/frame (max : %.3f)", arr[4], maxArr[4]);
			ImGui::Text("Integration %.3f ms/frame (max : %.3f)", arr[5], maxArr[5]);
			ImGui::Text("Track P2V %.3f ms/frame (max : %.3f)", arr[6], maxArr[6]);
			ImGui::Text("TBC %.3f ms/frame (max : %.3f)", arr[7], maxArr[7]);
			ImGui::EndPopup();
		}

		//ImGui::PushItemWidth(-krender.guiPadding().first);
		//ImGui::SetWindowPos(ImVec2(display_w - (display_w / 4) - krender.guiPadding().first, ((krender.guiPadding().second) + (0))));
		ImGui::PushItemWidth(-1);
		ImGui::PlotHistogram("", arr, IM_ARRAYSIZE(arr), 0, NULL, 0.0f, 33.0f, ImVec2(0, 80));
		ImGui::PopItemWidth();

		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.33f, 1.0f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.33f, 0.7f, 0.2f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.33f, 0.8f, 0.3f));
		if (ImGui::BeginPopupContextItem("plot context menu"))
		{
			ImGui::Text("Jump Flooding %.3f ms/frame (max : %.3f)", arr[0], maxArr[0]);
			ImGui::Text("Raycasting %.3f ms/frame (max : %.3f)", arr[1], maxArr[1]);
			ImGui::Text("TBC %.3f ms/frame (max : %.3f)", arr[2], maxArr[2]);
			ImGui::Text("Track P2P %.3f ms/frame (max : %.3f)", arr[3], maxArr[3]);
			ImGui::Text("TBC %.3f ms/frame (max : %.3f)", arr[4], maxArr[4]);
			ImGui::Text("Integration %.3f ms/frame (max : %.3f)", arr[5], maxArr[5]);
			ImGui::Text("Track P2V %.3f ms/frame (max : %.3f)", arr[6], maxArr[6]);
			ImGui::Text("TBC %.3f ms/frame (max : %.3f)", arr[7], maxArr[7]);
			ImGui::EndPopup();
		}

		if (ImGui::Button("Start Realsense"))
		{
			startRealsense();
		}
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		if (ImGui::Button("Camera 0"))
		{
			cameraDevice = 0;
		}
		ImGui::SameLine();
		if (ImGui::Button("Camera 1"))
		{
			cameraDevice = 1;
		}

		ImGui::Separator();
		if (ImGui::CollapsingHeader("Camera"))
		{
			


			ImGui::RadioButton("30 Hz", &eRate, 30); ImGui::SameLine();
			ImGui::RadioButton("90 Hz", &eRate, 30); ImGui::SameLine();

			ImGui::RadioButton("848x480", &eRes, 0); ImGui::SameLine();
			ImGui::RadioButton("1280x720", &eRes, 1);

			int prevShift = dispShift;
			ImGui::Text("Disparity");
			ImGui::PushItemWidth(-1);
			ImGui::SliderInt("Disparity", &dispShift, 0, 300);
			if (prevShift != dispShift)
			{
				//////////////////////////kcamera.setDepthControlGroupValues(0, 0, 0, 0, (uint32_t)dispShift); // TODO make this work with depth min and depth max
			}
			ImGui::PopItemWidth();


		}


		static bool openFileDialog = false;

		/*if (ImGui::Button("Open File Dialog"))
		{
			openFileDialog = true;
		}

		static std::string filePathName = "";
		static std::string path = "";
		static std::string fileName = "";
		static std::string filter = "";

		if (openFileDialog)
		{
			if (ImGuiFileDialog::Instance()->FileDialog("Choose File", ".cpp\0.h\0.hpp\0\0", ".", ""))
			{
				if (ImGuiFileDialog::Instance()->IsOk == true)
				{
					filePathName = ImGuiFileDialog::Instance()->GetFilepathName();
					path = ImGuiFileDialog::Instance()->GetCurrentPath();
					fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();
					filter = ImGuiFileDialog::Instance()->GetCurrentFilter();
				}
				else
				{
					filePathName = "";
					path = "";
					fileName = "";
					filter = "";
				}
				openFileDialog = false;
			}
		}

		if (filePathName.size() > 0) ImGui::Text("Choosed File Path Name : %s", filePathName.c_str());
		if (path.size() > 0) ImGui::Text("Choosed Path Name : %s", path.c_str());
		if (fileName.size() > 0) ImGui::Text("Choosed File Name : %s", fileName.c_str());
		if (filter.size() > 0) ImGui::Text("Choosed Filter : %s", filter.c_str());*/



		ImGui::Separator();
		if (ImGui::CollapsingHeader("Fusion"))
		{
			ImGui::Text("Mode");

			if (ImGui::Button("P2P")) trackDepthToPoint ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &trackDepthToPoint); ImGui::SameLine();
			if (ImGui::Button("P2V")) trackDepthToVolume ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &trackDepthToVolume);

			ImGui::Text("Resolution");
			ImGui::PushItemWidth(-1);
			float avail_width = ImGui::CalcItemWidth();
			float label_width = ImGui::CalcTextSize(" X ").x;
			ImGui::PopItemWidth();
			ImGui::PushItemWidth((avail_width / 3) - label_width);
			ImGui::Combo("X  ", &sizeX, sizes, IM_ARRAYSIZE(sizes)); ImGui::SameLine(0.0f, 0.0f);
			ImGui::Combo("Y  ", &sizeX, sizes, IM_ARRAYSIZE(sizes)); ImGui::SameLine(0.0f, 0.0f);
			ImGui::Combo("Z  ", &sizeX, sizes, IM_ARRAYSIZE(sizes));
			sizeY = sizeX;
			sizeZ = sizeX;
			ImGui::PopItemWidth();

			ImGui::Text("Length");
			ImGui::PushItemWidth(-1);
			ImGui::SliderFloat("Length", &dimension, 0.005f, 2.0f);
			ImGui::PopItemWidth();

			ImGui::Text("Volume Options");

			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 1.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.7f, 0.7f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.8f, 0.8f));

			if (ImGui::Button("Reset Volume"))
			{
				resetVolume();
			}
			ImGui::PopStyleColor(3);


			if (ImGui::Button("Integrate")) integratingFlag ^= 1; ImGui::SameLine();
			ImGui::Checkbox("", &integratingFlag);
			krender.setSelectInitPose(integratingFlag);

			if (ImGui::Button("save stl"))
			{
				saveSTL();
			}
		}
		ImGui::Separator();
		if (ImGui::CollapsingHeader("Image"))
		{
			ImGui::Text("Image Processing");

			if (ImGui::Button("Flood"))
			{
				performFlood ^= 1;
				if (performFlood)
				{
					gflood.setVolumeConfig(gconfig.volumeSize.x, gconfig.volumeDimensions.x);

					gflood.allocateTextures();

					krender.setFloodTexture(gflood.getFloodOutputTexture());
					//krender.setFloodTexture(gflood.getFloodSDFTexture());

					gflood.setVertices(gfusion.getVerts());
					gflood.setNormals(gfusion.getNorms());

				}
			} ImGui::SameLine(); ImGui::Checkbox("", &performFlood); ImGui::SameLine();
			if (ImGui::Button("Flow"))
			{
				performFlow ^= 1;
				gflow.resetTimeElapsed();

			}ImGui::SameLine(); ImGui::Checkbox("", &performFlow);

			if (ImGui::Button("Aruco"))
			{
				performAruco ^= 1;
				if (performAruco)
				{
					mTracker.configGEM();
					mTracker.startTracking();
				}
				else
				{
					mTracker.stopTracking();
				}

			}
			ImGui::SameLine(); ImGui::Checkbox("", &performAruco);
			static int gemOption = gemStatus::STOPPED;
			//static int gemOpt = 0;
			if (performAruco)
			{
				ImGui::RadioButton("Stopped", &gemOption, 0); ImGui::SameLine();
				ImGui::RadioButton("Collect", &gemOption, 1); ImGui::SameLine();
				ImGui::RadioButton("AutoCalibrate", &gemOption, 2); ImGui::SameLine();
				ImGui::RadioButton("Track", &gemOption, 3);

				mTracker.setGemOption(gemOption);

				if (ImGui::Button("Calibrate"))
				{
					mTracker.calibrate();
					gemOption = gemStatus::TRACKING;
				}
				ImGui::SameLine();
				if (ImGui::Button("Clear Calibration"))
				{
					mTracker.clearCalibration();
					gemOption = gemStatus::STOPPED;
				}
				ImGui::SameLine();
				
				if (ImGui::Button("Export Calibration")) mTracker.exportCalibration();

			}



		}








		ImGui::Separator();
		if (ImGui::CollapsingHeader("Display"))
		{
			ImGui::Text("View Options");

			if (ImGui::Button("Depth")) showDepthFlag ^= 1; ImGui::SameLine();	ImGui::Checkbox("", &showDepthFlag); 
			ImGui::SameLine(); 
			if (ImGui::Button("Color")) showColorFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showColorFlag);

			if (ImGui::Button("Flood##show")) showSDFVolume ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showSDFVolume);
			ImGui::SameLine(); 
			if (ImGui::Button("Flow##show")) showFlowFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showFlowFlag);


			if (ImGui::Button("Model")) showNormalFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showNormalFlag);
			ImGui::SameLine(); 
			if (ImGui::Button("Volume")) showVolumeFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showVolumeFlag);

			if (ImGui::Button("Track")) showTrackFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showTrackFlag);
			ImGui::SameLine();
			if (ImGui::Button("Marker")) showMarkerFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showMarkerFlag);

			ImGui::Text("slice");
			ImGui::PushItemWidth(-1);
			ImGui::SliderFloat("slice", &volSlice, 0, gconfig.volumeSize.z - 1);
			ImGui::PopItemWidth();

		}

		ImGui::Separator();
		if (ImGui::CollapsingHeader("Misc"))
		{

			ImGui::SliderFloat("vFOV", &vertFov, 1.0f, 90.0f);
			krender.setFov(vertFov);


			ImGui::SliderFloat("xRot", &xRot, 0.0f, 90.0f);
			ImGui::SliderFloat("yRot", &yRot, 0.0f, 90.0f);
			ImGui::SliderFloat("zRot", &zRot, 0.0f, 90.0f);




			ImGui::Separator();
			ImGui::Text("Infrared Adj.");

			//cv::imshow("irg", infraGrey);

			//if (ImGui::Button("Save Infra")) OCVStuff.saveImage(1);  // saving infra image (flag == 1)
			ImGui::SliderFloat("depthMin", &depthMin, 0.0f, 10.0f);
			/*if (irLow > (irHigh - 255.0f))
			{
			irHigh = irLow + 255.0f;
			}*/
			ImGui::SliderFloat("depthMax", &depthMax, 0.0f, 10.0f);
			//if (irHigh < (irLow + 255.0f))
			//{
			//	irLow = irHigh - 255.0f;
			//}
			krender.setDepthMinMax(depthMin, depthMax);

		}


		ImGui::End();

	}

	ImGuiIO& io = ImGui::GetIO();
	if (ImGui::IsMouseClicked(0) && imguiFocus2D == true)
	{
		int devNumber = 0;
		ImVec2 mPos = ImGui::GetMousePos();
		mousePos.x = mPos.x;
		mousePos.y = mPos.y;
		//std::cout << "from imgui " << mousePos.x << " " << mousePos.y << std::endl;
		//std::cout << "lalala " << mousePos.x - controlPoint0.x << " " << controlPoint0.y - mousePos.y << std::endl;
		iOff = initOffset(devNumber, mousePos.x - controlPoint0.x, controlPoint0.y - mousePos.y);

	}
}

void setUpGPU()
{
	gFusionInit();
	//mCubeInit();
	//krender.setBuffersFromMarchingCubes(gfusion.getVertsMC(), gfusion.getNormsMC(), gfusion.getNumVerts());

	gDisOptFlowInit();

	kRenderInit();

	gFloodInit();

}



int main(int, char**)
{
	


	int display_w, display_h;
	// load openGL window
	window = krender.loadGLFWWindow();

	glfwGetFramebufferSize(window, &display_w, &display_h);

	controlPoint0.x = display_w / 4.0f;
	controlPoint0.y = display_h - display_h / 3.0f;

	// Setup ImGui binding
	ImGui::CreateContext();
	ImGui_ImplGlfwGL3_Init(window, true);
	ImVec4 clear_color = ImColor(114, 144, 154);

	setUIStyle();
	setImguiWindows();


	//cv::Ptr<cv::DenseOpticalFlow> algorithm = cv::optflow::createOptFlow_DIS(cv::optflow::DISOpticalFlow::PRESET_MEDIUM);
	
	//cv::Mat prevgray, gray, graySmall, rgb, frame;
	//cv::Mat I0x = cv::Mat(1080, 1920, CV_16SC1);
	//cv::Mat I0y = cv::Mat(1080, 1920, CV_16SC1);
	//cv::Mat flow, flow_uv[2];
	//cv::Mat mag, ang;
	//cv::Mat hsv_split[3], hsv;
	//char ret;

	//cv::namedWindow("flow", 1);
	//cv::namedWindow("orig", 1);

	const int samples = 50;
	float time[samples];
	int index = 0;

	bool newFrame = false;
	bool show_slider_graph = true;

	float midDepth1 = 0.0f;
	float midDepth2 = 0.0f;
	float midDepth3 = 0.0f;
	
	std::stringstream filenameSS;
	filenameSS << "data/motion/motion_" << return_current_time_and_date() << ".txt";
	std::ofstream outputFile(filenameSS.str(), std::ios::out | std::ios::app);

	uint64_t previousTime = 0;// ();

	bool frameReady = false;

	cv::Mat colMat;

	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		glfwGetFramebufferSize(window, &display_w, &display_h);

		//// Rendering
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);


		frameReady = cameraInterface.collateFrames();

		//std::this_thread::sleep_for(std::chrono::milliseconds(30));

		if (frameReady)
		{
			gfusion.setCameraParams(glm::vec4(cameraInterface.getDepthIntrinsics(cameraDevice).fx,
				cameraInterface.getDepthIntrinsics(cameraDevice).fy,
				cameraInterface.getDepthIntrinsics(cameraDevice).cx,
				cameraInterface.getDepthIntrinsics(cameraDevice).cy),
				glm::vec4(cameraInterface.getColorIntrinsics(cameraDevice).fx,
					cameraInterface.getColorIntrinsics(cameraDevice).fy,
					cameraInterface.getColorIntrinsics(cameraDevice).cx,
					cameraInterface.getColorIntrinsics(cameraDevice).cy));

			gfusion.resetTimes();
			gfusion.setDepthUnit((float)cameraInterface.getDepthUnit(cameraDevice));

			mTracker.setCamPams(cameraInterface.getColorIntrinsics(cameraDevice).fx,
				cameraInterface.getColorIntrinsics(cameraDevice).fy,
				cameraInterface.getColorIntrinsics(cameraDevice).cx,
				cameraInterface.getColorIntrinsics(cameraDevice).cy,
				colorFrameSize[cameraDevice].x,
				colorFrameSize[cameraDevice].y);

			//auto eTime = epchTime();


			// THIS SHOULD ONLY BE COPIED WHENEVER THERES A NEW COLOR FRAME, NOT EVERY DEPTH FRAME
			krender.setColorFrame(cameraInterface.getColorQueues(), cameraDevice, colMat);

			if (!colMat.empty() && performAruco)
			{
				//cv::imshow("!", colMat);
				//cv::waitKey(1);
				mTracker.setMat(colMat);
				mTracker.detect();
				//mTracker.draw();
			}

			if (performAruco)
			{
				mTracker.useGEM();

				std::vector<glm::mat4> tMat;
				mTracker.getMarkerData(tMat);
				
				if (tMat.size() > 0)
				{
					//if (mTracker.gemStatus == gemStatus::TRACKING)
					//{
					//	krender.setMarkerData(tMat);

					//}
					//else
					//{
						krender.setMarkerData(tMat);
					//}
				}
				
			}

			double currentTime = epchTime();
			double deltaTime = currentTime - previousTime;
			previousTime = currentTime;
			//std::cout << (int)(deltaTime) << std::endl;

			if (performFlow)
			{
				gflow.setTexture();

				gflow.calc(false);
			}		   

			gfusion.depthToVertex(cameraInterface.getDepthQueues(), cameraDevice);

			gfusion.vertexToNormal();
			if (performFlood)
			{
				gflood.setPose(gfusion.getPose());
				gflood.jumpFloodCalc();
			}

			bool tracked = false;

			if (trackDepthToPoint)
			{
				tracked = gfusion.Track(); 
				gfusion.raycast();
			}

			if (trackDepthToVolume)
			{
				tracked = gfusion.TrackSDF();
			}


			if (tracked && integratingFlag && ((counter % 1) == 0) || reset)
			{
				gfusion.integrate();
				if (counter > 2)
					reset = false;
			}

			if (!tracked)
			{
				//gfusion.recoverPose();
			}

			if (counter <= 2)
			{
				counter++;
			}

			//outputFile << std::to_string(previousTime) << " " << gfusion.alignmentEnergy() << " " << kcamera.getTemperature();
			outputFile << " " << gfusion.getPose()[0].x << " " << gfusion.getPose()[0].y << " " << gfusion.getPose()[0].z << " " << gfusion.getPose()[0].w << \
				" " << gfusion.getPose()[1].x << " " << gfusion.getPose()[1].y << " " << gfusion.getPose()[1].z << " " << gfusion.getPose()[1].w << \
				" " << gfusion.getPose()[2].x << " " << gfusion.getPose()[2].y << " " << gfusion.getPose()[2].z << " " << gfusion.getPose()[2].w << \
				" " << gfusion.getPose()[3].x << " " << gfusion.getPose()[3].y << " " << gfusion.getPose()[3].z << " " << gfusion.getPose()[3].w << std::endl;

			graphPoints.push_back(gfusion.getTransPose());

			if (graphPoints.size() > graphWindow.w)
			{
				graphPoints.pop_front();
			}

			minmaxX = std::make_pair<float, float>(1000, -1000.f);
			minmaxY = std::make_pair<float, float>(1000, -1000.f);;
			minmaxZ = std::make_pair<float, float>(1000, -1000.f);;

			int idx = 0;
			for (auto i : graphPoints)
			{
				if (i[0] < minmaxX.first) minmaxX.first = i[0];
				if (i[0] > minmaxX.second) minmaxX.second = i[0];

				if (i[1] < minmaxY.first) minmaxY.first = i[1];
				if (i[1] > minmaxY.second) minmaxY.second = i[1];

				if (i[2] < minmaxZ.first) minmaxZ.first = i[2];
				if (i[2] > minmaxZ.second) minmaxZ.second = i[2];

				arrayX[idx] = i[0];
				arrayY[idx] = i[1];
				arrayZ[idx] = i[2];

				idx++;
			}

		}

		

		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();

		setUI();


		krender.setRenderingOptions(showDepthFlag, showBigDepthFlag, showInfraFlag, showColorFlag, showLightFlag, showPointFlag, showFlowFlag, showEdgesFlag, showNormalFlag, showVolumeFlag, showTrackFlag, showSDFVolume, showMarkerFlag);
		krender.setFusionType(trackDepthToPoint, trackDepthToVolume);


		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());





		krender.setDepthImageRenderPosition(vertFov);
		krender.setRayNormImageRenderPosition(vertFov);
		krender.setTrackImageRenderPosition(vertFov);

		//krender.setInfraImageRenderPosition();
		krender.setColorImageRenderPosition(vertFov);

		krender.setFlowImageRenderPosition(vertFov);

		krender.setVolumeSDFRenderPosition(volSlice);

		krender.setMarchingCubesRenderPosition(zModelPC_offset);
		krender.setViewMatrix(xRot, yRot, zRot, xTran, yTran, zTran);
		krender.setDepthTextureProjectionMatrix();




		if (cameraRunning)
		{
			krender.setDisplayOriSize(display2DWindow.x, display_h - display2DWindow.y - display2DWindow.h, display2DWindow.w, display2DWindow.h);
			krender.set3DDisplayOriSize(display3DWindow.x, display_h - display3DWindow.y - display3DWindow.h, display3DWindow.w, display3DWindow.h);
			krender.setColorDisplayOriSize(display3DWindow.x, display_h - display3DWindow.y - display3DWindow.h, colorFrameSize[cameraDevice]);

			krender.Render(false);
		}

		glfwSwapBuffers(window);

	}

	

	// Cleanup
	// close file writer
	outputFile.close();

	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();
	//kcamera.stop();
	cameraInterface.stopDevice(cameraDevice);
	glfwTerminate();

	//krender.cleanUp();

	return 0;
}