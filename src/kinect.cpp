#include "kinect.h"

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
	krender.setCameraParams(glm::vec4(kcamera.fx(), kcamera.fx(), kcamera.ppx(), kcamera.ppy()), glm::vec4(kcamera.fx_col(), kcamera.fx_col(), kcamera.ppx_col(), kcamera.ppy_col())); // FIX ME

	// Set locations
	krender.setLocations();
	krender.setVertPositions();
	krender.allocateBuffers();
	krender.setTextures(gfusion.getDepthImage(), gdisoptflow.getColorTexture(), gfusion.getVerts(), gfusion.getNorms(), gfusion.getVolume(), gfusion.getTrackImage(), gfusion.getPVPNorms(), gfusion.getPVDNorms()); //needs texture uints from gfusion init
	krender.anchorMW(std::make_pair<int, int>(1920 - 512 - krender.guiPadding().first, krender.guiPadding().second));
	//krender.genTexCoordOffsets(1, 1, 1.0f);
	krender.setFusionType(trackDepthToPoint, trackDepthToVolume);
}

void gFusionInit()
{
	depthArray.resize(depthHeight * depthWidth, 1);
	gfusion.queryDeviceLimits();
	gfusion.compileAndLinkShader();
	gfusion.setLocations();

	gconfig.volumeSize = glm::vec3(128);
	gconfig.volumeDimensions = glm::vec3(1.0f);
	gconfig.depthFrameSize = glm::vec2(depthWidth, depthHeight);
	gconfig.mu = 0.05f;
	gconfig.maxWeight = 100.0f;
	gconfig.iterations[0] = 2;
	gconfig.iterations[1] = 4;
	gconfig.iterations[2] = 6;
	
	gfusion.setCameraParams(glm::vec4(kcamera.fx(), kcamera.fx(), kcamera.ppx(), kcamera.ppy()), glm::vec4(kcamera.fx_col(), kcamera.fx_col(), kcamera.ppx_col(), kcamera.ppy_col())); // FIX ME

	glm::mat4 initPose = glm::translate(glm::mat4(1.0f), glm::vec3(gconfig.volumeDimensions.x / 2.0f, gconfig.volumeDimensions.y / 2.0f, 0.0f));

	gfusion.setConfig(gconfig);
	gfusion.setPose(initPose);
	gfusion.setPoseP2V(initPose);

	gfusion.initTextures();
	gfusion.initVolume();
	gfusion.allocateBuffers();


	gfusion.setUsingDepthFloat(false);
	gfusion.setDepthUnit(kcamera.getDepthUnit());

}

void gDisOptFlowInit()
{
	gdisoptflow.compileAndLinkShader();
	gdisoptflow.setLocations();
#ifdef USEINFRARED
	gdisoptflow.setNumLevels(depthWidth);
	gdisoptflow.setTextureParameters(depthWidth, depthHeight);
	gdisoptflow.allocateTextures(true);
#else
	gdisoptflow.setNumLevels(colorWidth);
	gdisoptflow.setTextureParameters(colorWidth, colorHeight);
	gdisoptflow.allocateTextures(false);


#endif

	gdisoptflow.allocateBuffers();

}

void gFloodInit()
{
	gflood.setCameraParams(glm::vec4(kcamera.fx(), kcamera.fx(), kcamera.ppx(), kcamera.ppy())); // FIX ME

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
		arr[0] = gdisoptflow.getTimeElapsed();
		gfusion.getTimes(arr);
		arr[8] = arr[0] + arr[1] + arr[2] + arr[3] + arr[4] + arr[5] + arr[6] + arr[7];

		ImGui::Begin("Menu", &navigationWindow.visible, window_flags);
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", arr[8], 1000.0f / arr[8]);

		//ImGui::PushItemWidth(-krender.guiPadding().first);
		//ImGui::SetWindowPos(ImVec2(display_w - (display_w / 4) - krender.guiPadding().first, ((krender.guiPadding().second) + (0))));
		ImGui::Text("Help menu - press 'H' to hide");

		static int eRate = 90;
		static int eRes = 0;

		int prevERate = eRate;
		int prevERes = eRes;


		ImGui::RadioButton("30 Hz", &eRate, 30); ImGui::SameLine();
		ImGui::RadioButton("90 Hz", &eRate, 90); ImGui::SameLine();
		if (cameraRunning) eRate = prevERate;
		if (eRate == 90) eRes = 0;

		
		ImGui::RadioButton("848x480", &eRes, 0); ImGui::SameLine();
		ImGui::RadioButton("1280x720", &eRes, 1); ImGui::SameLine();
		
		if (cameraRunning) eRes = prevERes;
		if (eRes == 1) eRate = 30;


		if (ImGui::Button("Start Realsense"))
		{
			if (cameraRunning == false)
			{
				cameraRunning = true;
				kcamera.setPreset(eRate, eRes);
				if (eRes == 0)
				{
					depthWidth = 848;
					depthHeight = 480;
				}
				else if (eRes == 1)
				{
					depthWidth = 1280;
					depthHeight = 720;
				}

				kcamera.start();

				setUpGPU();

			}

		}

		
		
		static bool openFileDialog = false;

		if (ImGui::Button("Open File Dialog"))
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
		if (filter.size() > 0) ImGui::Text("Choosed Filter : %s", filter.c_str());
		ImGui::Separator();
		ImGui::Text("Fusion Options");

		if (ImGui::Button("P2P")) trackDepthToPoint ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &trackDepthToPoint); ImGui::SameLine();
		if (ImGui::Button("P2V")) trackDepthToVolume ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &trackDepthToVolume);
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
		}

		ImGui::SameLine(); ImGui::Checkbox("", &performFlood);


		if (ImGui::Button("Reset Volume"))
		{	// update config
			//m_center_pixX

			//glm::mat4 initPose = glm::translate(glm::mat4(1.0f), glm::vec3(gconfig.volumeDimensions.x / 2.0f, gconfig.volumeDimensions.y / 2.0f, 0.0f));
			gdisoptflow.wipeFlow();

			bool deleteFlag = false;

			if (glm::vec3(std::stoi(sizes[sizeX]), std::stoi(sizes[sizeY]), std::stoi(sizes[sizeZ])) != gconfig.volumeSize)
			{
				deleteFlag = true;
			}

			gconfig.volumeSize = glm::vec3(std::stoi(sizes[sizeX]), std::stoi(sizes[sizeY]), std::stoi(sizes[sizeZ]));
			gconfig.volumeDimensions = glm::vec3(dimension);
			
			gfusion.setConfig(gconfig);

			iOff = initOffset(mousePos.x - controlPoint0.x, controlPoint0.y - mousePos.y);

			//std::cout << "lalala " << mousePos.x - controlPoint0.x << " " << controlPoint0.y - mousePos.y<< std::endl;

			glm::mat4 initPose = glm::translate(glm::mat4(1.0f), glm::vec3(-iOff.x + gconfig.volumeDimensions.x / 2.0f, -iOff.y + gconfig.volumeDimensions.y / 2.0f, -iOff.z + dimension / 2.0));


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

		if (ImGui::Button("Integrate")) integratingFlag ^= 1; ImGui::SameLine();
		ImGui::Checkbox("", &integratingFlag);
		krender.setSelectInitPose(integratingFlag);

		if (ImGui::Button("ROI")) selectInitialPoseFlag ^= 1; ImGui::SameLine();
		ImGui::Checkbox("", &selectInitialPoseFlag);


		//if (ImGui::Button("DO SUM")) gfusion.testPrefixSum();
		if (ImGui::Button("save stl"))
		{
			//gfusion.marchingCubes();
			//gfusion.exportSurfaceAsStlBinary();

			mcconfig.gridSize = glm::uvec3(gconfig.volumeSize.x, gconfig.volumeSize.y, gconfig.volumeSize.z);
			//mcconfig.numVoxels = mcconfig.gridSize.x * mcconfig.gridSize.y * mcconfig.gridSize.z;
			//mcconfig.maxVerts = std::min(mcconfig.gridSize.x * mcconfig.gridSize.y * 128, uint32_t(128 * 128 * 128));

			mcubes.setConfig(mcconfig);

			mcubes.setVolumeTexture(gfusion.getVolume());
			//mcubes.setVolumeTexture(gflood.getFloodSDFTexture());


			mcubes.init();

			mcubes.setIsolevel(0.0f);

			mcubes.generateMarchingCubes();
			mcubes.exportMesh();

		}

		ImGui::PlotHistogram("Timing", arr, IM_ARRAYSIZE(arr), 0, NULL, 0.0f, 33.0f, ImVec2(0, 80));


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

		ImGui::SliderFloat("dim", &dimension, 0.005f, 2.0f);

		ImGui::SliderFloat("slice", &volSlice, 0, gconfig.volumeSize.z - 1);

		ImGui::Separator();
		ImGui::Text("View Options");

		ImGui::Separator();
		ImGui::Text("Realsense Options");
		int prevShift = dispShift;
		ImGui::SliderInt("disparity shift", &dispShift, 0, 300);
		if (prevShift != dispShift)
		{
			kcamera.setDepthControlGroupValues(0, 0, 0, 0, (uint32_t)dispShift); // TODO make this work with depth min and depth max
		}


		if (ImGui::Button("Show Depth")) showDepthFlag ^= 1; ImGui::SameLine();	ImGui::Checkbox("", &showDepthFlag); ImGui::SameLine(); if (ImGui::Button("Show Big Depth")) showBigDepthFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showBigDepthFlag);
		if (ImGui::Button("Show Infra")) showInfraFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showInfraFlag); ImGui::SameLine(); if (ImGui::Button("Show Flow")) showFlowFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showFlowFlag);
		if (ImGui::Button("Show Color")) showColorFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showColorFlag); ImGui::SameLine(); if (ImGui::Button("Show Edges")) showEdgesFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showEdgesFlag);
		if (ImGui::Button("Show Light")) showLightFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showLightFlag); ImGui::SameLine(); if (ImGui::Button("Show RayNorm")) showNormalFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showNormalFlag);
		if (ImGui::Button("Show Point")) showPointFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showPointFlag); ImGui::SameLine(); if (ImGui::Button("Show Volume")) showVolumeFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showVolumeFlag);
		if (ImGui::Button("Show Track")) showTrackFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showTrackFlag); ImGui::SameLine(); if (ImGui::Button("Show SDF")) showSDFVolume ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showSDFVolume);

		ImGui::Separator();
		ImGui::Text("Other Options");

		if (ImGui::Button("Select color points")) select_color_points_mode ^= 1; ImGui::SameLine();
		//if (ImGui::Button("Reset")) OCVStuff.resetColorPoints();

		if (ImGui::Button("Select depth points")) select_depth_points_mode ^= 1; ImGui::SameLine();
		//if (ImGui::Button("Reset Depth")) krender.resetRegistrationMatrix();

		//if (ImGui::Button("Export PLY")) krender.setExportPly(true);
		//if (ImGui::Button("Export PLY")) krender.exportPointCloud();
		//if (ImGui::Button("Save Color")) OCVStuff.saveImage(0); // saving color image (flag == 0)


		ImGui::Separator();
		ImGui::Text("View Transforms");
		ImGui::SliderFloat("vFOV", &vertFov, 1.0f, 90.0f);
		krender.setFov(vertFov);


		ImGui::SliderFloat("xRot", &xRot, 0.0f, 90.0f);
		ImGui::SliderFloat("yRot", &yRot, 0.0f, 90.0f);
		ImGui::SliderFloat("zRot", &zRot, 0.0f, 90.0f);

		ImGui::SliderFloat("xTran", &xTran, -2000.0f, 2000.0f);
		ImGui::SliderFloat("yTran", &yTran, -2000.0f, 2000.0f);
		ImGui::SliderFloat("zTran", &zTran, 0.0f, 4000.0f);

		ImGui::SliderFloat("model z", &zModelPC_offset, -4000.0f, 4000.0f);
		if (ImGui::Button("Reset Sliders")) resetSliders();

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

		ImGui::SliderFloat("irLow", &irLow, 0.0f, 65536.0f - 255.0f);
		if (irLow > (irHigh - 255.0f))
		{
			irHigh = irLow + 255.0f;
		}
		ImGui::SliderFloat("irHigh", &irHigh, 255.0f, 65536.0f);
		if (irHigh < (irLow + 255.0f))
		{
			irLow = irHigh - 255.0f;
		}
		krender.setIrBrightness(irLow, irHigh);

		ImGui::Separator();
		ImGui::Text("Calibration Misc.");
		if (ImGui::Button("Calibrate")) calibratingFlag ^= 1; ImGui::SameLine();
		ImGui::Checkbox("", &calibratingFlag);


		ImGui::End();

	}

	ImGuiIO& io = ImGui::GetIO();
	if (ImGui::IsMouseClicked(0) && imguiFocus2D == true)
	{
		ImVec2 mPos = ImGui::GetMousePos();
		mousePos.x = mPos.x;
		mousePos.y = mPos.y;
		std::cout << "from imgui " << mousePos.x << " " << mousePos.y << std::endl;
		std::cout << "lalala " << mousePos.x - controlPoint0.x << " " << controlPoint0.y - mousePos.y << std::endl;
		iOff = initOffset(mousePos.x - controlPoint0.x, controlPoint0.y - mousePos.y);

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

	// Main loop
	while (!glfwWindowShouldClose(window))
	{

		glfwGetFramebufferSize(window, &display_w, &display_h);

		//// Rendering
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);


		//gfusion.update(float(glfwGetTime()));

		if (kcamera.ready())
		{
			gfusion.resetTimes();
			gfusion.setDepthUnit(kcamera.getDepthUnit());

			kcamera.frames(colorArray, depthArray, NULL, NULL, NULL);

			//cv::Mat colFrame(480, 848, CV_8UC4, colorArray);
			//cv::imshow("c", colFrame);
			//cv::waitKey(1);

#ifdef USEINFRARED
			//		gdisoptflow.setTexture(infraredArray);
#else
			gdisoptflow.setTexture(colorArray);
#endif


#ifdef USEINFRARED
		//	gdisoptflow.calc(true);
#else
			//gdisoptflow.calc(false);
#endif



			//gdisoptflow.track();

		//	gfusion.trackPoints3D(gdisoptflow.getTrackedPointsBuffer());

		//	krender.setTrackedPointsBuffer(gdisoptflow.getTrackedPointsBuffer());

			//krender.setFlowTexture(gdisoptflow.getFlowTexture());

			//cv::Mat totflow = cv::Mat(1080 >> 0, 1920 >> 0, CV_32FC2);

			//glActiveTexture(GL_TEXTURE0);
			//glBindTexture(GL_TEXTURE_2D, gdisoptflow.getFlowTexture());
			//glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, totflow.data);// this is importnant, you are using GL_RED_INTEGETER!!!!
			//glBindTexture(GL_TEXTURE_2D, 0);
			// 

			//cv::Mat tofl[2];
			//cv::split(totflow, tofl);  

			////cv::imshow("wwee", tofl[0] - tofl[1]);
			////cv::imshow("dwerwev", tofl[1]); 

			//  
			//cv::Mat mag, ang; 
			//cv::Mat hsv_split[3], hsv;
			//cv::Mat rgb; 
			//cv::cartToPolar(tofl[0], tofl[1], mag, ang, true);
			//cv::normalize(mag, mag, 0, 1, cv::NORM_MINMAX);
			//hsv_split[0] = ang;
			//hsv_split[1] = mag;
			//hsv_split[2] = cv::Mat::ones(ang.size(), ang.type()); 
			//cv::merge(hsv_split, 3, hsv);
			//cv::cvtColor(hsv, rgb, cv::COLOR_HSV2BGR);
			//cv::Mat outrgb, outmix;
			//rgb.convertTo(outrgb, CV_8UC4, 255);
			//cv::addWeighted(outrgb, 0.7, col, 0.3, 1.0, outmix);
			//cv::imshow("totflowrgb", outrgb);


			//outWriter << outmix;



			gfusion.depthToVertex(depthArray.data());

			gfusion.vertexToNormal();
			if (performFlood)
			{
				GLenum theError;
				theError = glGetError();

				gflood.setPose(gfusion.getPose());
				gflood.jumpFloodCalc();
			}


			gfusion.showNormals();

			bool tracked = false;

			if (trackDepthToPoint)
			{
				tracked = gfusion.Track(); // this should return a bool for successful track
				gfusion.raycast();
			}

			if (trackDepthToVolume)
			{
				tracked = gfusion.TrackSDF();
			}


			//gfusion.testLargeUpload();

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


			counter++;



			// log tracking data to file



			//std::string motionTrackingFile = "data/motion/motion-test.txt";


			std::ofstream out(filenameSS.str(), std::ios::out | std::ios::app);
			//iOff = initOffset(krender.getCenterPixX(), krender.getCenterPixY());

			glm::vec4 position = glm::vec4(iOff, 1.0f);

			glm::vec4 transformedPosition = gfusion.getPose() * position;

			out << epochTime() << " " << gfusion.alignmentEnergy() << " " << kcamera.getTemperature();
			out << " " << gfusion.getPose()[0].x << " " << gfusion.getPose()[0].y << " " << gfusion.getPose()[0].z << " " << gfusion.getPose()[0].w << \
				" " << gfusion.getPose()[1].x << " " << gfusion.getPose()[1].y << " " << gfusion.getPose()[1].z << " " << gfusion.getPose()[1].w << \
				" " << gfusion.getPose()[2].x << " " << gfusion.getPose()[2].y << " " << gfusion.getPose()[2].z << " " << gfusion.getPose()[2].w << \
				" " << gfusion.getPose()[3].x << " " << gfusion.getPose()[3].y << " " << gfusion.getPose()[3].z << " " << gfusion.getPose()[3].w << std::endl;
			out.close();


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

			//gfusion.intensityProjection();
			//gfusion.marchingCubes();


			//gfusion.exportSurfaceAsStlBinary();

			//krender.setBuffersFromMarchingCubes(gfusion.getVertsMC(), gfusion.getNormsMC(), gfusion.getNumVerts());

			//gfusion.showDifference();

			newFrame = true; // make this a return bool on the kcamera.frames()


		}
		else
		{
			newFrame = false;
		}

		bool show_test_window = true;

		//gfusion.printTimes();
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();

		setUI();


		krender.setRenderingOptions(showDepthFlag, showBigDepthFlag, showInfraFlag, showColorFlag, showLightFlag, showPointFlag, showFlowFlag, showEdgesFlag, showNormalFlag, showVolumeFlag, showTrackFlag, showSDFVolume);
		krender.setFusionType(trackDepthToPoint, trackDepthToVolume);


		//ImGui::ShowTestWindow();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());


		if (newFrame)
		{
			
			//krender.setBuffersForRendering(depthArray, bigDepthArray, infraredArray, colorArray, flow.ptr());
			krender.setDepthImageRenderPosition(vertFov);
			krender.setRayNormImageRenderPosition(vertFov);
			krender.setTrackImageRenderPosition(vertFov);

			//krender.setInfraImageRenderPosition();
			krender.setColorImageRenderPosition(vertFov);

#ifdef USEINFRARED
			krender.setFlowImageRenderPosition(depthHeight, depthWidth, vertFov);
#else
			krender.setFlowImageRenderPosition(colorHeight, colorWidth, vertFov);

#endif


			//krender.setPointCloudRenderPosition(zModelPC_offset);
			//krender.setLightModelRenderPosition();
			krender.setVolumeSDFRenderPosition(volSlice);

			krender.setMarchingCubesRenderPosition(zModelPC_offset);
			krender.setViewMatrix(xRot, yRot, zRot, xTran, yTran, zTran);
			krender.setDepthTextureProjectionMatrix();



		}


		if (cameraRunning)
		{
			krender.setDisplayOriSize(display2DWindow.x, display_h - display2DWindow.y - display2DWindow.h, display2DWindow.w, display2DWindow.h);
			krender.set3DDisplayOriSize(display3DWindow.x, display_h - display3DWindow.y - display3DWindow.h, display3DWindow.w, display3DWindow.h);

#ifdef USEINFRARED
			krender.Render(true);
#else
			krender.Render(false);
#endif
		}


		glfwSwapBuffers(window);


		//std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}

	

	// Cleanup
	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();
	kcamera.stop();
	glfwTerminate();

	//krender.cleanUp();

	return 0;
}