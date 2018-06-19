#include "interface.h"



void Realsense2Camera::setPreset(int rate, int res)
{
	m_depthRate = rate;
	if (res == 0)
	{
		m_depthframe_height = 480;
		m_depthframe_width = 848;
	}
	else if (res == 1)
	{
		m_depthframe_height = 720;
		m_depthframe_width = 1280;
	}

}

void Realsense2Camera::start()
{
	if (m_status == STOPPED)
	{

		rs2::context ctx;
		rs2::device_list devices = ctx.query_devices();


		if (devices.size() == 0)
		{
			std::cerr << "No device connected, please connect a RealSense device" << std::endl;

			//To help with the boilerplate code of waiting for a device to connect
			//The SDK provides the rs2::device_hub class
			rs2::device_hub device_hub(ctx);

			//Using the device_hub we can block the program until a device connects
			dev = device_hub.wait_for_device();
		}
		else
		{
			std::cout << "Found the following devices:\n" << std::endl;

			// device_list is a "lazy" container of devices which allows
			//The device list provides 2 ways of iterating it
			//The first way is using an iterator (in this case hidden in the Range-based for loop)
			//int index = 0;
			//for (rs2::device device : devices)
			//{
			//	std::cout << "  " << index++ << " : " << get_device_name(device) << std::endl;
			//}

			//uint32_t selected_device_index = get_user_selection("Select a device by index: ");

			//// The second way is using the subscript ("[]") operator:
			//if (selected_device_index >= devices.size())
			//{
			//	throw std::out_of_range("Selected device index is out of range");
			//}

			// Update the selected device
			dev = devices[0];
		}

		std::string serial_number(dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));

		// Declare RealSense pipeline, encapsulating the actual device and sensors

		//Create a configuration for configuring the pipeline with a non default profile
		rs2::config cfg;

		cfg.enable_device(serial_number);


		//Add desired streams to configuration // 
		cfg.enable_stream(RS2_STREAM_COLOR, m_colorframe_width, m_colorframe_height, RS2_FORMAT_BGRA8, 30);
		cfg.enable_stream(RS2_STREAM_DEPTH, m_depthframe_width, m_depthframe_height, RS2_FORMAT_Z16, m_depthRate);


		// Start streaming with default recommended configuration
		selection = pipe.start(cfg);










		//auto sensor = selection.get_device().first<rs2::depth_sensor>();
		//sensor.set_option(RS2_OPTION_ENABLE_AUTO_EXPOSURE, 1);

		//sensor.set_option(RS2_OPTION_EMITTER_ENABLED, 1);
		//sensor.set_option(RS2_OPTION_LASER_POWER, 30.0f);


		auto depth_stream = selection.get_stream(RS2_STREAM_DEPTH).as<rs2::video_stream_profile>();

		auto i = depth_stream.get_intrinsics();
		m_depth_ppx = i.ppx;
		m_depth_ppy = i.ppy;
		m_depth_fx = i.fx;
		m_depth_fy = i.fy;

		m_status = CAPTURING;
		m_thread = new std::thread(&Realsense2Camera::captureLoop, this);
	}
}

void Realsense2Camera::stop()
{
	if (m_status == CAPTURING)
	{
		m_status = STOPPED;

		if (m_thread->joinable())
		{
			m_thread->join();
		}

		m_thread = nullptr;
	}
}

void Realsense2Camera::frames(unsigned char * colorAray, float * bigDepthArray)
{
	//m_mtx.lock();

	//memcpy_S the arrays to the pointers passed in here

	memcpy_s(colorAray, 1920 * 1080 * 4, m_rawColor, 1920 * 1080 * 4);
	memcpy_s(bigDepthArray, 1920 * 1082 * 4, m_rawBigDepth, 1920 * 1082 * 4);



	//m_mtx.unlock();
	m_frames_ready = false;
}

// get all the frames available, pass a null pointer in for each array you dont want back
void Realsense2Camera::frames(unsigned char * colorArray, std::vector<uint16_t> &depthArray, float * infraredArray, float * bigDepthArray, int * colorDepthMapping)
{
	//m_mtx.lock();

	//memcpy_S the arrays to the pointers passed in here

		if (colorArray != NULL)
		{
			memcpy_s(colorArray, m_colorframe_width * m_colorframe_height * 4, m_color_frame, m_colorframe_width * m_colorframe_height * 4);
		}

		if (depthArray.data() != NULL)
		{
			memcpy_s(depthArray.data(), m_depthframe_width * m_depthframe_height * 2, m_depth_frame, m_depthframe_width * m_depthframe_height * 2);
		}

		if (infraredArray != NULL)
		{
			//memcpy_s(infraredArray, 512 * 424 * 4, m_infra_frame, 512 * 424 * 4);
		}

		if (bigDepthArray != NULL)
		{
			//memcpy_s(bigDepthArray, 1920 * 1082 * 4, m_rawBigDepth, 1920 * 1082 * 4);
		}
		if (colorDepthMapping != NULL)
		{
			//memcpy_s(colorDepthMapping, 512 * 424 * 4, m_color_Depth_Map, 512 * 424 * 4);
		}






	//m_mtx.unlock();
	m_frames_ready = false;
}


std::vector<float> Realsense2Camera::getColorCameraParameters()
{
	std::vector<float> camPams;

	/*camPams.push_back(m_colorCamPams.fx);
	camPams.push_back(m_colorCamPams.fy);
	camPams.push_back(m_colorCamPams.cx);
	camPams.push_back(m_colorCamPams.cy);
	camPams.push_back(m_colorCamPams.mx_x3y0);
	camPams.push_back(m_colorCamPams.mx_x0y3);
	camPams.push_back(m_colorCamPams.mx_x2y1);
	camPams.push_back(m_colorCamPams.mx_x1y2);
	camPams.push_back(m_colorCamPams.mx_x2y0);
	camPams.push_back(m_colorCamPams.mx_x0y2);
	camPams.push_back(m_colorCamPams.mx_x1y1);
	camPams.push_back(m_colorCamPams.mx_x1y0);
	camPams.push_back(m_colorCamPams.mx_x0y1);
	camPams.push_back(m_colorCamPams.mx_x0y0);
	camPams.push_back(m_colorCamPams.my_x3y0);
	camPams.push_back(m_colorCamPams.my_x0y3);
	camPams.push_back(m_colorCamPams.my_x2y1);
	camPams.push_back(m_colorCamPams.my_x1y2);
	camPams.push_back(m_colorCamPams.my_x2y0);
	camPams.push_back(m_colorCamPams.my_x0y2);
	camPams.push_back(m_colorCamPams.my_x1y1);
	camPams.push_back(m_colorCamPams.my_x1y0);
	camPams.push_back(m_colorCamPams.my_x0y1);
	camPams.push_back(m_colorCamPams.my_x0y0);
	camPams.push_back(m_colorCamPams.shift_d);
	camPams.push_back(m_colorCamPams.shift_m);*/

	return camPams;
}

void Realsense2Camera::captureLoop()
{

	auto advanced = dev.as<rs400::advanced_mode>(); // NOTE rs400 namespace!

	if (advanced.is_enabled())
	{
		std::string f = "./resources/nearmode.json";
	    //std::string f = "./resources/standard.json";
		//std::string f = "./resources/nearmode435.json"; 
		//std::string f = "./resources/standard435.json";

		std::ifstream file(f, std::ifstream::in);
		if (!file.good())
		{
			std::cout << "file not good" << std::endl;
		}
		else
		{
			std::cout << "config file good" << std::endl;
		}
		std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

		advanced.load_json(str);

		m_ctrl_curr = advanced.get_depth_table(0);
		advanced.set_depth_table(m_ctrl_curr);

	}


	rs2::frame depth;
	rs2::frame color;

	m_color_frame = new float[m_colorframe_width * m_colorframe_height * 3];
	m_depth_frame = new uint16_t[m_depthframe_width * m_depthframe_height];

	while (m_status == CAPTURING)
	{
		if (m_valuesChanged)
		{
			advanced.set_depth_table(m_ctrl_curr);
			m_valuesChanged = false;
		}
		data = pipe.wait_for_frames(); // Wait for next set of frames from the camera

		depth = data.get_depth_frame(); // Find and colorize the depth data
		color = data.get_color_frame();            // Find the color data

		auto dbg = selection.get_device().as<rs2::debug_protocol>();
		std::vector<uint8_t> cmd = { 0x14, 0, 0xab, 0xcd, 0x2a, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

												   // Query frame size (width and height)
		const int w = color.as<rs2::video_frame>().get_width();
		const int h = color.as<rs2::video_frame>().get_height();

		const int wD = depth.as<rs2::video_frame>().get_width();
		const int hD = depth.as<rs2::video_frame>().get_height();

		//std::cout <<  "frame ready? : " << m_frames_ready << std::endl;

		//cv::Mat colMat = cv::Mat(hD, wD, CV_16SC1, (void*)depth.get_data());

		//cv::imshow("cv wsrtyindow", colMat);
		//cv::waitKey(1);

		memcpy_s(m_color_frame, w * h * 4, color.get_data(), w * h * 4);
		memcpy_s(m_depth_frame, wD * hD * 2, depth.get_data(), wD * hD * 2);

		auto res = dbg.send_and_receive_raw_data(cmd);
		m_temperature = res[4];

		m_frames_ready = true;



	}

	pipe.stop();

	delete m_rawColor;
	delete m_rawBigDepth;

	delete m_color_frame;
	delete m_depth_frame;
	delete m_infra_frame;

	delete m_color_Depth_Map;

}
