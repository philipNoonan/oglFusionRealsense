#include "interface.h"



rs2::device_list Interface::getDeviceList()
{
	return m_devices;
}

void Interface::searchForCameras()
{
	rs2::context ctx;
	m_devices = ctx.query_devices();

	if (m_devices.size() == 0)
	{
		std::cerr << "No device connected, please connect a RealSense device" << std::endl;

		//To help with the boilerplate code of waiting for a device to connect
		//The SDK provides the rs2::device_hub class
		//rs2::device_hub device_hub(ctx);

		//Using the device_hub we can block the program until a device connects
		//dev = device_hub.wait_for_device();
	}
	else
	{
		int index = 0;
		for (rs2::device device : m_devices)
		{
			std::cout << "  " << index++ << " : " << getDeviceName(device) << std::endl;
		}
		m_threads.resize(index);
		m_cameras.resize(index, Realsense2Camera());
		
		m_depthFrames.resize(index);
		m_colorFrames.resize(index);

		m_depthProps.resize(index);
		m_colorProps.resize(index);

		m_depthIntrinsics.resize(index);
		m_colorIntrinsics.resize(index);

		m_depthQueues.resize(index);
		m_colorQueues.resize(index);

	}

}

void Interface::setDepthProperties(int devNumber, int w, int h, int r)
{
	m_depthProps[devNumber].width = w;
	m_depthProps[devNumber].height = h;
	m_depthProps[devNumber].rate = r;
}

void Interface::setColorProperties(int devNumber, int w, int h, int r)
{
	m_colorProps[devNumber].width = w;
	m_colorProps[devNumber].height = h;
	m_colorProps[devNumber].rate = r;
}




void Interface::startDevice(int devNumber)
{
	//Realsense2Camera camera;
	m_cameras[devNumber].setDev(m_devices[devNumber]);
	m_cameras[devNumber].setDepthProperties(m_depthProps[devNumber].width, m_depthProps[devNumber].height, m_depthProps[devNumber].rate);
	m_cameras[devNumber].setColorProperties(m_colorProps[devNumber].width, m_colorProps[devNumber].height, m_colorProps[devNumber].rate);

	m_cameras[devNumber].start();

	m_threads[devNumber] = std::thread(&Realsense2Camera::capture, m_cameras[devNumber]);
}

void Interface::stopDevice(int devNumber)
{
	m_cameras[devNumber].stop();
	if (m_threads[devNumber].joinable())
	{
		m_threads[devNumber].join();
	}
}

FrameIntrinsics Interface::getDepthIntrinsics(int devnumber)
{
	return m_depthIntrinsics[devnumber];
}

FrameIntrinsics Interface::getColorIntrinsics(int devnumber)
{
	return m_colorIntrinsics[devnumber];
}

uint32_t Interface::getDepthUnit(int devNumber)
{
	return m_cameras[devNumber].getDepthUnit();
}

void Interface::getColorFrame(int devNumber, std::vector<uint16_t> &colorArray)
{
	if (colorArray.data() != NULL)
	{
		memcpy(colorArray.data(), m_colorFrames[devNumber].get_data(), m_colorProps[devNumber].width * m_colorProps[devNumber].height * sizeof(uint16_t));
	}
}

void Interface::getDepthFrame(int devNumber, std::vector<uint16_t> &depthArray)
{
	if (depthArray.data() != NULL)
	{
		memcpy(depthArray.data(), m_depthFrames[devNumber].get_data(), m_depthProps[devNumber].width * m_depthProps[devNumber].height * sizeof(uint16_t));
	}
}

std::vector<rs2::frame_queue> Interface::getDepthQueues()
{
	return m_depthQueues;
}

std::vector<rs2::frame_queue> Interface::getColorQueues()
{
	return m_colorQueues;
}

bool Interface::collateFrames()
{
	bool frameReady = false;
	for (int i = 0; i < m_cameras.size(); i++)
	{
		m_cameras[i].getFrames(m_depthQueues[i], m_colorQueues[i]);
		//rs2::frame frmD;
		//rs2::frame frmC;

		//if (depQ.poll_for_frame(&frmD))
		//{
		//	std::vector<uint16_t> dframe(848 * 480, 0);
		//	memcpy(dframe.data(), frmD.get_data(), 848 * 480 * 2);
		//	cv::Mat colMat = cv::Mat(480, 848, CV_16SC1, dframe.data());
		//	std::string imname = "im" + std::to_string(i);
		//	cv::imshow(imname, colMat);
		//	cv::waitKey(1);

		//}

		//if (colQ.poll_for_frame(&frmC))
		//{
		//	std::vector<uint16_t> dframe(848 * 480, 0);
		//	memcpy(dframe.data(), frmC.get_data(), 848 * 480 * 2);
		//	cv::Mat colMat = cv::Mat(480, 848, CV_16SC1, dframe.data());
		//	std::string imname = "imC" + std::to_string(i);
		//	cv::imshow(imname, colMat);
		//	cv::waitKey(1);

		//}
		frameReady = true;

	}

	return frameReady;
}












std::string Interface::getDeviceName(const rs2::device& dev)
{
	// Each device provides some information on itself, such as name:
	std::string name = "Unknown Device";
	if (dev.supports(RS2_CAMERA_INFO_NAME))
		name = dev.get_info(RS2_CAMERA_INFO_NAME);

	// and the serial number of the device:
	std::string sn = "########";
	if (dev.supports(RS2_CAMERA_INFO_SERIAL_NUMBER))
		sn = std::string("#") + dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);

	return name + " " + sn;
}

void Interface::setDepthIntrinsics(int devNumber)
{
	auto i = m_cameras[devNumber].getDepthIntrinsics();

	m_depthIntrinsics[devNumber].cx = i.ppx;
	m_depthIntrinsics[devNumber].cy = i.ppy;
	m_depthIntrinsics[devNumber].fx = i.fx;
	m_depthIntrinsics[devNumber].fy = i.fy;
}

void Interface::setColorIntrinsics(int devNumber)
{
	auto i = m_cameras[devNumber].getColorIntrinsics();

	m_colorIntrinsics[devNumber].cx = 0;
	m_colorIntrinsics[devNumber].cy = 0;
	m_colorIntrinsics[devNumber].fx = 0;
	m_colorIntrinsics[devNumber].fy = 0;
}


















//void Realsense2Camera::setPreset(int rate, int res)
//{
//	m_depthRate = rate;
//	if (res == 0)
//	{
//		m_depthframe_height = 480;
//		m_depthframe_width = 848;
//	}
//	else if (res == 1)
//	{
//		m_depthframe_height = 720;
//		m_depthframe_width = 1280;
//	}
//
//}
//
//std::string get_device_name(const rs2::device& dev)
//{
//	// Each device provides some information on itself, such as name:
//	std::string name = "Unknown Device";
//	if (dev.supports(RS2_CAMERA_INFO_NAME))
//		name = dev.get_info(RS2_CAMERA_INFO_NAME);
//
//	// and the serial number of the device:
//	std::string sn = "########";
//	if (dev.supports(RS2_CAMERA_INFO_SERIAL_NUMBER))
//		sn = std::string("#") + dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);
//
//	return name + " " + sn;
//}
//
//void Realsense2Camera::start()
//{
//	if (m_status == STOPPED)
//	{
//
//		rs2::context ctx;
//		rs2::device_list devices = ctx.query_devices();
//
//
//		if (devices.size() == 0)
//		{
//			std::cerr << "No device connected, please connect a RealSense device" << std::endl;
//
//			//To help with the boilerplate code of waiting for a device to connect
//			//The SDK provides the rs2::device_hub class
//			rs2::device_hub device_hub(ctx);
//
//			//Using the device_hub we can block the program until a device connects
//			dev = device_hub.wait_for_device();
//		}
//		else
//		{
//			std::cout << "Found the following devices:\n" << std::endl;
//
//			// device_list is a "lazy" container of devices which allows
//			//The device list provides 2 ways of iterating it
//			//The first way is using an iterator (in this case hidden in the Range-based for loop)
//			int index = 0;
//			for (rs2::device device : devices)
//			{
//				std::cout << "  " << index++ << " : " << get_device_name(device) << std::endl;
//			}
//
//			//uint32_t selected_device_index = get_user_selection("Select a device by index: ");
//
//			//// The second way is using the subscript ("[]") operator:
//			//if (selected_device_index >= devices.size())
//			//{
//			//	throw std::out_of_range("Selected device index is out of range");
//			//}
//
//			// Update the selected device
//			dev = devices[1];
//		}
//
//		std::string serial_number(dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
//
//		// Declare RealSense pipeline, encapsulating the actual device and sensors
//
//		//Create a configuration for configuring the pipeline with a non default profile
//		rs2::config cfg;
//
//		cfg.enable_device(serial_number);
//
//
//		//Add desired streams to configuration // 
//		cfg.enable_stream(RS2_STREAM_COLOR, m_colorframe_width, m_colorframe_height, RS2_FORMAT_BGRA8, 60);
//		cfg.enable_stream(RS2_STREAM_DEPTH, m_depthframe_width, m_depthframe_height, RS2_FORMAT_Z16, 60);
//
//
//		// Start streaming with default recommended configuration
//		selection = pipe.start(cfg);
//		
//
//
//
//
//
//
//
//
//
//		//auto sensor = selection.get_device().first<rs2::depth_sensor>();
//		//sensor.set_option(RS2_OPTION_ENABLE_AUTO_EXPOSURE, 1);
//
//		//sensor.set_option(RS2_OPTION_EMITTER_ENABLED, 1);
//		//sensor.set_option(RS2_OPTION_LASER_POWER, 30.0f);
//
//
//		auto depth_stream = selection.get_stream(RS2_STREAM_DEPTH).as<rs2::video_stream_profile>();
//
//		auto i = depth_stream.get_intrinsics();
//		m_depth_ppx = i.ppx;
//		m_depth_ppy = i.ppy;
//		m_depth_fx = i.fx;
//		m_depth_fy = i.fy;
//
//		m_status = CAPTURING;
//		m_thread = new std::thread(&Realsense2Camera::captureLoop, this);
//	}
//}
//
//void Realsense2Camera::stop()
//{
//	if (m_status == CAPTURING)
//	{
//		m_status = STOPPED;
//
//		if (m_thread->joinable())
//		{
//			m_thread->join();
//		}
//
//		m_thread = nullptr;
//	}
//}
//
//void Realsense2Camera::frames(unsigned char * colorAray, float * bigDepthArray)
//{
//	//m_mtx.lock();
//
//	//memcpy_S the arrays to the pointers passed in here
//
//	memcpy_s(colorAray, 1920 * 1080 * 4, m_rawColor, 1920 * 1080 * 4);
//	memcpy_s(bigDepthArray, 1920 * 1082 * 4, m_rawBigDepth, 1920 * 1082 * 4);
//
//
//
//	//m_mtx.unlock();
//	m_frames_ready = false;
//}
//
//// get all the frames available, pass a null pointer in for each array you dont want back
//bool Realsense2Camera::frames(uint64_t &frameTime, unsigned char * colorArray, std::vector<uint16_t> &depthArray, float * infraredArray, float * bigDepthArray, int * colorDepthMapping)
//{
//	//m_mtx.lock();
//
//	//memcpy_S the arrays to the pointers passed in here
//	bool newFrame = false;
//
//	if (frameTime < m_frameArrivalTime)
//	{
//		if (colorArray != NULL)
//		{
//			memcpy_s(colorArray, m_colorframe_width * m_colorframe_height * 4, m_color_frame, m_colorframe_height * m_colorframe_width * 4);
//		}
//
//		if (depthArray.data() != NULL)
//		{
//			memcpy_s(depthArray.data(), m_depthframe_width * m_depthframe_height * 2, m_depth_frame, m_depthframe_width * m_depthframe_height * 2);
//		}
//
//		if (infraredArray != NULL)
//		{
//			//memcpy_s(infraredArray, 512 * 424 * 4, m_infra_frame, 512 * 424 * 4);
//		}
//
//		if (bigDepthArray != NULL)
//		{
//			//memcpy_s(bigDepthArray, 1920 * 1082 * 4, m_rawBigDepth, 1920 * 1082 * 4);
//		}
//		if (colorDepthMapping != NULL)
//		{
//			//memcpy_s(colorDepthMapping, 512 * 424 * 4, m_color_Depth_Map, 512 * 424 * 4);
//		}
//
//		frameTime = m_frameArrivalTime;
//
//		newFrame = true;
//	}
//
//		
//	return newFrame;
//
//
//
//
//	//m_mtx.unlock();
//	//m_frames_ready = false;
//}
//
//
//std::vector<float> Realsense2Camera::getColorCameraParameters()
//{
//	std::vector<float> camPams;
//
//	/*camPams.push_back(m_colorCamPams.fx);
//	camPams.push_back(m_colorCamPams.fy);
//	camPams.push_back(m_colorCamPams.cx);
//	camPams.push_back(m_colorCamPams.cy);
//	camPams.push_back(m_colorCamPams.mx_x3y0);
//	camPams.push_back(m_colorCamPams.mx_x0y3);
//	camPams.push_back(m_colorCamPams.mx_x2y1);
//	camPams.push_back(m_colorCamPams.mx_x1y2);
//	camPams.push_back(m_colorCamPams.mx_x2y0);
//	camPams.push_back(m_colorCamPams.mx_x0y2);
//	camPams.push_back(m_colorCamPams.mx_x1y1);
//	camPams.push_back(m_colorCamPams.mx_x1y0);
//	camPams.push_back(m_colorCamPams.mx_x0y1);
//	camPams.push_back(m_colorCamPams.mx_x0y0);
//	camPams.push_back(m_colorCamPams.my_x3y0);
//	camPams.push_back(m_colorCamPams.my_x0y3);
//	camPams.push_back(m_colorCamPams.my_x2y1);
//	camPams.push_back(m_colorCamPams.my_x1y2);
//	camPams.push_back(m_colorCamPams.my_x2y0);
//	camPams.push_back(m_colorCamPams.my_x0y2);
//	camPams.push_back(m_colorCamPams.my_x1y1);
//	camPams.push_back(m_colorCamPams.my_x1y0);
//	camPams.push_back(m_colorCamPams.my_x0y1);
//	camPams.push_back(m_colorCamPams.my_x0y0);
//	camPams.push_back(m_colorCamPams.shift_d);
//	camPams.push_back(m_colorCamPams.shift_m);*/
//
//	return camPams;
//}
//
//void Realsense2Camera::captureLoop()
//{
//
//	auto advanced = dev.as<rs400::advanced_mode>(); // NOTE rs400 namespace!
//
//	if (advanced.is_enabled())
//	{
//		std::string f = "./resources/nearmode415.json";
//	    //std::string f = "./resources/standard415.json";
//		//std::string f = "./resources/nearmode435.json"; 
//		//std::string f = "./resources/standard435.json";
//
//		std::ifstream file(f, std::ifstream::in);
//		if (!file.good())
//		{
//			std::cout << "file not good" << std::endl;
//		}
//		else
//		{
//			std::cout << "config file good" << std::endl;
//		}
//		std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
//
//		advanced.load_json(str);
//
//		m_ctrl_curr = advanced.get_depth_table(0);
//		m_ctrl_curr.disparityShift = 0;
//
//		advanced.set_depth_table(m_ctrl_curr);
//
//	}
//
//	rs2_stream align_to = RS2_STREAM_COLOR;
//	rs2::align align(align_to);
//
//	rs2::frame depth;
//	rs2::frame color;
//
//	m_color_frame = new float[m_colorframe_width * m_colorframe_height * 4];
//	m_depth_frame = new uint16_t[m_depthframe_width * m_depthframe_height];
//	//rs2::frame_queue fq_depth;
//	uint64_t previousTime = 0;
//	while (m_status == CAPTURING)
//	{
//		if (m_valuesChanged)
//		{
//			advanced.set_depth_table(m_ctrl_curr);
//			m_valuesChanged = false;
//		}
//
//		//fq_depth.enqueue(std::move(depth));
//		bool frameArrived = pipe.try_wait_for_frames(&data); // Wait for next set of frames from the camera
//
//		//bool frameArrived = pipe.poll_for_frames(&data);
//
//		if (frameArrived)
//		{
//			processed = align.process(data);
//
//			// Trying to get both other and aligned depth frames
//			//rs2::video_frame other_frame = processed.first(align_to);
//			rs2::depth_frame aligned_depth_frame = processed.get_depth_frame();
//
//			//depth = data.get_depth_frame(); // Find and colorize the depth data
//			color = processed.get_color_frame();            // Find the color data
//
//			//if (color.supports_frame_metadata(RS2_FRAME_METADATA_TIME_OF_ARRIVAL))
//			//{
//			//	double currentTime = color.get_frame_metadata(RS2_FRAME_METADATA_TIME_OF_ARRIVAL);
//			//	double deltaTime = currentTime - previousTime;
//			//	std::cout << deltaTime << std::endl;
//			//	previousTime = currentTime;
//
//			//	//std::cout << color.get_frame_metadata(RS2_FRAME_METADATA_TIME_OF_ARRIVAL) << std::endl;
//			//}
//			//std::cout << " expos: " << color.get_frame_metadata(RS2_FRAME_METADATA_ACTUAL_EXPOSURE) << std::endl;
//
//			if (aligned_depth_frame.supports_frame_metadata(RS2_FRAME_METADATA_FRAME_TIMESTAMP))
//			{
//				m_frameArrivalTime = aligned_depth_frame.get_frame_metadata(RS2_FRAME_METADATA_FRAME_TIMESTAMP);
//				uint64_t currentTime = aligned_depth_frame.get_frame_metadata(RS2_FRAME_METADATA_FRAME_TIMESTAMP);
//				uint64_t deltaTime = currentTime - previousTime;
//				if (deltaTime > 20000)
//				{
//					std::cout << deltaTime << std::endl;
//				}
//				previousTime = currentTime;
//			}
//
//			//if (color.supports_frame_metadata(RS2_FRAME_METADATA_FRAME_TIMESTAMP))
//			//{
//			//	double currentTime = color.get_frame_metadata(RS2_FRAME_METADATA_FRAME_TIMESTAMP);
//			//	double deltaTime = currentTime - previousTime;
//			//	std::cout << deltaTime << std::endl;
//			//	previousTime = currentTime;
//			//}
//
//			auto dbg = selection.get_device().as<rs2::debug_protocol>();
//			std::vector<uint8_t> cmd = { 0x14, 0, 0xab, 0xcd, 0x2a, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
//
//			// Query frame size (width and height)
//			const int w = color.as<rs2::video_frame>().get_width();
//			const int h = color.as<rs2::video_frame>().get_height();
//
//			const int wD = aligned_depth_frame.as<rs2::video_frame>().get_width();
//			const int hD = aligned_depth_frame.as<rs2::video_frame>().get_height();
//
//			//std::cout <<  "frame ready? : " << m_frames_ready << std::endl;
//
//			//cv::Mat colMat = cv::Mat(hD, wD, CV_16SC1, (void*)depth.get_data());
//
//			//cv::imshow("cv wsrtyindow", colMat);
//			//cv::waitKey(1);
//
//			memcpy_s(m_color_frame, w * h * 4, color.get_data(), w * h * 4);
//			memcpy_s(m_depth_frame, wD * hD * 2, aligned_depth_frame.get_data(), wD * hD * 2);
//
//			auto res = dbg.send_and_receive_raw_data(cmd);
//			m_temperature = res[4];
//
//			m_frames_ready = true;
//		}
//		else
//		{
//			//std::this_thread::sleep_for(std::chrono::milliseconds(2));
//
//
//		}
//		
//
//
//
//	}
//
//	pipe.stop();
//
//	delete m_rawColor;
//	delete m_rawBigDepth;
//
//	delete m_color_frame;
//	delete m_depth_frame;
//	delete m_infra_frame;
//
//	delete m_color_Depth_Map;
//
//}
