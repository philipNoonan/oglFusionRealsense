#include "device.h"
#include "helper.h"

void Realsense2Camera::setDev(rs2::device dev)
{
	m_dev = dev;

	std::string name = "Unknown Device";
	if (dev.supports(RS2_CAMERA_INFO_NAME))
		name = dev.get_info(RS2_CAMERA_INFO_NAME);

	std::vector<std::string> tokens = Helper::split(name, " ");

	m_configFilename = "./resources/nearmode" + tokens[2] + ".json";
}

static std::string get_sensor_name(const rs2::sensor& sensor)
{
	// Sensors support additional information, such as a human readable name
	if (sensor.supports(RS2_CAMERA_INFO_NAME))
		return sensor.get_info(RS2_CAMERA_INFO_NAME);
	else
		return "Unknown Sensor";
}

void Realsense2Camera::setDepthProperties(std::tuple<int, int, int, rs2_format> choice)
{
	m_depthStreamChoice = choice;
}

void Realsense2Camera::setInfraProperties(std::tuple<int, int, int, rs2_format> choice)
{
	m_infraStreamChoice = choice;
}

void Realsense2Camera::setColorProperties(std::tuple<int, int, int, rs2_format> choice)
{
	m_colorStreamChoice = choice;
}

void Realsense2Camera::getDepthProperties(int &width, int &height, int &rate)
{
	width = m_stream_profiles_depthIR[m_depthStreamID].as<rs2::video_stream_profile>().width();
	height = m_stream_profiles_depthIR[m_depthStreamID].as<rs2::video_stream_profile>().height();
	rate = m_stream_profiles_depthIR[m_depthStreamID].as<rs2::video_stream_profile>().fps();
}

void Realsense2Camera::getInfraProperties(int &width, int &height, int &rate)
{
	width = m_stream_profiles_depthIR[m_infraStreamID].as<rs2::video_stream_profile>().width();
	height = m_stream_profiles_depthIR[m_infraStreamID].as<rs2::video_stream_profile>().height();
	rate = m_stream_profiles_depthIR[m_infraStreamID].as<rs2::video_stream_profile>().fps();
}

void Realsense2Camera::getColorProperties(int &width, int &height, int &rate)
{
	width = m_stream_profiles_color[m_colorStreamID].as<rs2::video_stream_profile>().width();
	height = m_stream_profiles_color[m_colorStreamID].as<rs2::video_stream_profile>().height();
	rate = m_stream_profiles_color[m_colorStreamID].as<rs2::video_stream_profile>().fps();
}

void Realsense2Camera::setDepthTable(STDepthTableControl dTable)
{
	auto advanced = m_dev.as<rs400::advanced_mode>(); // NOTE rs400 namespace!
	//STDepthTableControl depthTableOri = advanced.get_depth_table();
	//STDepthTableControl depthTable;
	//depthTable.depthClampMax = 10000;
	//depthTable.depthClampMin = 0;
	//depthTable.depthUnits = 100;
	//depthTable.disparityMode = 0;
	//depthTable.disparityShift = 0;
	advanced.set_depth_table(dTable);
}

void Realsense2Camera::setStreams()
{
	m_sensors = m_dev.query_sensors();

	int index = 0;
	for (rs2::sensor sensor : m_sensors)
	{
		std::cout << "  " << index++ << " : " << get_sensor_name(sensor) << std::endl;
	}

	// for 4xx
	// 0 : stereo
	// 1 : rgb
	// 2 : motion (if available)

	m_stream_profiles_depthIR = m_sensors[0].get_stream_profiles();
	m_stream_profiles_color = m_sensors[1].get_stream_profiles();

	std::map<std::pair<rs2_stream, int>, int> unique_streams;
	for (auto&& sp : m_stream_profiles_depthIR)
	{
		unique_streams[std::make_pair(sp.stream_type(), sp.stream_index())]++;
	}
	std::cout << "Sensor consists of " << unique_streams.size() << " streams: " << std::endl;
	for (size_t i = 0; i < unique_streams.size(); i++)
	{
		auto it = unique_streams.begin();
		std::advance(it, i);
		std::cout << "  - " << it->first.first << " #" << it->first.second << std::endl;
	}


	std::map<std::pair<rs2_stream, int>, int> unique_streams_color;
	for (auto&& sp : m_stream_profiles_color)
	{
		unique_streams_color[std::make_pair(sp.stream_type(), sp.stream_index())]++;
	}
	std::cout << "Sensor consists of " << unique_streams_color.size() << " streams: " << std::endl;
	for (size_t i = 0; i < unique_streams_color.size(); i++)
	{
		auto it = unique_streams_color.begin();
		std::advance(it, i);
		std::cout << "  - " << it->first.first << " #" << it->first.second << std::endl;
	}

	int id;
	//////Next, we go over all the stream profiles and print the details of each one
	int profile_num = 0;
	for (rs2::stream_profile stream_profile : m_stream_profiles_depthIR)
	{
		rs2_stream stream_data_type = stream_profile.stream_type();
		int stream_index = stream_profile.stream_index();
		std::string stream_name = stream_profile.stream_name();
		int unique_stream_id = stream_profile.unique_id(); // The unique identifier can be used for comparing two streams
		if (stream_profile.is<rs2::video_stream_profile>()) //"Is" will test if the type tested is of the type given
		{
			rs2::video_stream_profile video_stream_profile = stream_profile.as<rs2::video_stream_profile>();
					
			if (stream_index == 0 && video_stream_profile.format() == std::get<3>(m_depthStreamChoice) 
				                   && video_stream_profile.width() == std::get<0>(m_depthStreamChoice) 
				                  && video_stream_profile.height() == std::get<1>(m_depthStreamChoice)
				                     && video_stream_profile.fps() == std::get<2>(m_depthStreamChoice))
			{
				m_depthStreamID = profile_num;
				std::cout << std::setw(3) << profile_num << ": " << stream_data_type << " #" << stream_index;
				std::cout << " (Video Stream: " << video_stream_profile.format() << " " <<
					video_stream_profile.width() << "x" << video_stream_profile.height() << "@ " << video_stream_profile.fps() << "Hz)" << std::endl;

			}

			if (stream_index == 1 && video_stream_profile.format() == std::get<3>(m_infraStreamChoice)
				                   && video_stream_profile.width() == std::get<0>(m_infraStreamChoice)
				                  && video_stream_profile.height() == std::get<1>(m_infraStreamChoice)
				                     && video_stream_profile.fps() == std::get<2>(m_infraStreamChoice))
			{
				m_infraStreamID = profile_num;
				std::cout << std::setw(3) << profile_num << ": " << stream_data_type << " #" << stream_index;
				std::cout << " (Video Stream: " << video_stream_profile.format() << " " <<
					video_stream_profile.width() << "x" << video_stream_profile.height() << "@ " << video_stream_profile.fps() << "Hz)" << std::endl;;

			}
		
		}
		//std::cout << std::endl;
		profile_num++;
	}


	////Next, we go over all the stream profiles and print the details of each one
	int colprofile_num = 0;
	for (rs2::stream_profile stream_profile : m_stream_profiles_color)
	{
		rs2_stream stream_data_type = stream_profile.stream_type();
		int stream_index = stream_profile.stream_index();
		std::string stream_name = stream_profile.stream_name();
		int unique_stream_id = stream_profile.unique_id(); // The unique identifier can be used for comparing two streams
		if (stream_profile.is<rs2::video_stream_profile>()) //"Is" will test if the type tested is of the type given
		{
			rs2::video_stream_profile video_stream_profile = stream_profile.as<rs2::video_stream_profile>();
			
			if (stream_index == 0 && video_stream_profile.format() == std::get<3>(m_colorStreamChoice)
				                   && video_stream_profile.width() == std::get<0>(m_colorStreamChoice)
				                  && video_stream_profile.height() == std::get<1>(m_colorStreamChoice)
				                     && video_stream_profile.fps() == std::get<2>(m_colorStreamChoice))
			{
				m_colorStreamID = colprofile_num;
				std::cout << std::setw(3) << colprofile_num << ": " << stream_data_type << " #" << stream_index;
				std::cout << " (Video Stream: " << video_stream_profile.format() << " " <<
					video_stream_profile.width() << "x" << video_stream_profile.height() << "@ " << video_stream_profile.fps() << "Hz)" << std::endl;

			}
	
		}
		//std::cout << std::endl;
		colprofile_num++;
	}

}

void Realsense2Camera::setSensorOptions()
{
	std::string name = "Unknown Device";
	if (m_dev.supports(RS2_CAMERA_INFO_NAME))
		name = m_dev.get_info(RS2_CAMERA_INFO_NAME);

	std::vector<std::string> tokens = Helper::split(name, " ");

	if (tokens[2] == "D415")
	{
		// depth
		m_sensors[0].set_option(RS2_OPTION_EXPOSURE, 20000);
		m_sensors[0].set_option(RS2_OPTION_ENABLE_AUTO_EXPOSURE, 0);
		m_sensors[0].set_option(RS2_OPTION_GAIN, 70);
	}
	else if (tokens[2] == "D435" || tokens[2] == "D435i")
	{
		// depth
		m_sensors[0].set_option(RS2_OPTION_EXPOSURE, 4000);
		m_sensors[0].set_option(RS2_OPTION_ENABLE_AUTO_EXPOSURE, 0);
	}
	else
	{
		// depth
		m_sensors[0].set_option(RS2_OPTION_ENABLE_AUTO_EXPOSURE, 1);
	}


	// rcolor
	m_sensors[1].set_option(RS2_OPTION_EXPOSURE, 200);
	m_sensors[1].set_option(RS2_OPTION_ENABLE_AUTO_EXPOSURE, 0);
}

void Realsense2Camera::setEmitterOptions(float status, float power)
{
	if (m_sensors[0].supports(RS2_OPTION_EMITTER_ENABLED))
	{
		m_sensors[0].set_option(RS2_OPTION_EMITTER_ENABLED, status);
	}

	//if (m_sensors[devNumber].supports(RS2_OPTION_LASER_POWER))
	//{
	//	// Query min and max values:
	//	m_sensors[devNumber].set_option(RS2_OPTION_LASER_POWER, power); // Set max power
	//}

}

bool Realsense2Camera::start()
{
		m_sensors[1].open(m_stream_profiles_color[m_colorStreamID]); //848 480 60fps rgb8
		std::thread colThread = std::thread(&Realsense2Camera::colorThread, this, std::ref(m_sensors[1]));
		colThread.detach();

		std::vector<rs2::stream_profile> sp = { m_stream_profiles_depthIR[m_infraStreamID], m_stream_profiles_depthIR[m_depthStreamID] };
		//std::vector<rs2::stream_profile> sp = { m_stream_profiles_depthIR[35], m_stream_profiles_depthIR[m_depthStreamChoice] };

		m_sensors[0].open(sp); // depth 848*480@90
		std::thread depThread = std::thread(&Realsense2Camera::depthThread, this, std::ref(m_sensors[0]));
		depThread.detach();

		
	return false;
}

bool Realsense2Camera::startFromFile()
{

	m_sensors[1].open(m_stream_profiles_color[m_colorStreamID]); //848 480 60fps rgb8
	
	m_sensors[0].open(m_stream_profiles_depthIR[m_depthStreamID]); // depth 848*480@90


	return false;
}

void Realsense2Camera::capturingDepth(rs2::frame &f)
{
	if (f.get_profile().stream_type() == RS2_STREAM_DEPTH)
	{
		m_depthQueue.enqueue(f);
	}
	else if (f.get_profile().stream_type() == RS2_STREAM_INFRARED)
	{
		m_infraQueue.enqueue(f);
	}
}
void Realsense2Camera::capturingInfra(rs2::frame &f)
{
	m_infraQueue.enqueue(f);
}
void Realsense2Camera::capturingColor(rs2::frame &f)
{
	m_colorQueue.enqueue(f);
}
void Realsense2Camera::depthThread(rs2::sensor& sens)
{
	sens.start([&](rs2::frame f) {this->capturingDepth(f); });
}
void Realsense2Camera::infraThread(rs2::sensor& sens)
{
	sens.start([&](rs2::frame f) {this->capturingInfra(f); });
}
void Realsense2Camera::colorThread(rs2::sensor &sens)
{
	sens.start([&](rs2::frame f) {this->capturingColor(f); });
}

bool Realsense2Camera::stop()
{
	// stop the sensors

	// close the sensors
	for (int i = 0; i < 2; i++)
	{
		m_sensors[i].stop();
		m_sensors[i].close();
	}


	return false;
}

Realsense2Camera::Status Realsense2Camera::getStatus()
{
	return m_status;
}

int Realsense2Camera::readTemperature()
{
	auto dbg0 = m_dev.as<rs2::debug_protocol>();
	std::vector<uint8_t> cmd = { 0x14, 0, 0xab, 0xcd, 0x2a, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	auto res = dbg0.send_and_receive_raw_data(cmd);
	return res[4];
}

void Realsense2Camera::capture()
{
	auto advanced = m_dev.as<rs400::advanced_mode>(); // NOTE rs400 namespace!

	if (advanced.is_enabled())
	{
		std::ifstream file(m_configFilename, std::ifstream::in);
		if (!file.good())
		{
			std::cout << "config file not good" << std::endl;
		}
		else
		{
			std::cout << "config file good" << std::endl;
		}
		std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

		advanced.load_json(str);

		m_ctrl_curr = advanced.get_depth_table(0);
		m_ctrl_curr.disparityShift = 0;

		advanced.set_depth_table(m_ctrl_curr);
	}

	uint64_t previousTime = 0;

	//rs2_stream align_to = RS2_STREAM_COLOR;
	//rs2::align align(align_to);

	//m_depthQueue = rs2::frame_queue(5);
	//m_colorQueue = rs2::frame_queue(5);



	while (m_status == CAPTURING)
	{
		if (m_valuesChanged)
		{
			advanced.set_depth_table(m_ctrl_curr);
			m_valuesChanged = false;
		}

		bool frameArrived = m_pipe.try_wait_for_frames(&m_data); // Wait for next set of frames from the camera

		if (frameArrived)
		{
			//m_dataProcessed = align.process(m_data);

			// Trying to get both other and aligned depth frames
			//rs2::video_frame other_frame = processed.first(align_to);
			//rs2::depth_frame aligned_depth_frame = m_dataProcessed.get_depth_frame();
			
			rs2::frame depth = m_data.get_depth_frame(); // Find and colorize the depth data
			rs2::frame color = m_data.get_color_frame();            // Find the color data
			//if (depth.supports_frame_metadata(RS2_FRAME_METADATA_FRAME_TIMESTAMP))

			
			if (depth.supports_frame_metadata(RS2_FRAME_METADATA_TIME_OF_ARRIVAL))
			{
				m_frameArrivalTime = depth.get_frame_metadata(RS2_FRAME_METADATA_TIME_OF_ARRIVAL);
				uint64_t currentTime = depth.get_frame_metadata(RS2_FRAME_METADATA_TIME_OF_ARRIVAL);
				uint64_t deltaTime = currentTime - previousTime;
				//if (deltaTime > (1e6f / (float)m_depthRate) * 1.5f) // if greater than 1.5 frame duration in microseconds
				//{
					std::cout << deltaTime << std::endl;
				//}
				previousTime = currentTime;
			}


			auto dbg = m_selection.get_device().as<rs2::debug_protocol>();
			std::vector<uint8_t> cmd = { 0x14, 0, 0xab, 0xcd, 0x2a, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

			// Query frame size (width and height)
			/*const int w = color.as<rs2::video_frame>().get_width();
			const int h = color.as<rs2::video_frame>().get_height();

			const int wD = depth.as<rs2::video_frame>().get_width();
			const int hD = depth.as<rs2::video_frame>().get_height();*/

			m_depthQueue.enqueue(depth);
			m_colorQueue.enqueue(color);

			//std::cout <<  "frame ready? : " << m_frames_ready << std::endl;

			//cv::Mat colMat = cv::Mat(hD, wD, CV_16SC1, (void*)m_depth.get_data());

			//cv::imshow("cv wsrtyindow", colMat);
			//cv::waitKey(1);

			//memcpy_s(m_color_frame, w * h * 4, color.get_data(), w * h * 4);
			//memcpy_s(m_depth_frame, wD * hD * 2, aligned_depth_frame.get_data(), wD * hD * 2);

			auto res = dbg.send_and_receive_raw_data(cmd);
			m_temperature = res[4];

		}
		else
		{
			//std::this_thread::sleep_for(std::chrono::milliseconds(2));


		}





	}

	m_pipe.stop();


}
bool Realsense2Camera::getFrames(rs2::frame_queue &depthQ, rs2::frame_queue &colorQ, rs2::frame_queue & infraQ)
{
	depthQ = m_depthQueue;
	colorQ = m_colorQueue;
	infraQ = m_infraQueue;
	return false;
}

bool Realsense2Camera::getFramesFromFile(rs2::frame_queue &depthQ, rs2::frame_queue &colorQ)
{
	
	

	//m_depthQueue.enqueue(dep);

	//m_colorQueue.enqueue(col);

	//depthQ = m_depthQueue;
	//colorQ = m_colorQueue;

	return false;

}
rs2_intrinsics Realsense2Camera::getDepthIntrinsics()
{
	auto video_stream = m_stream_profiles_depthIR[m_depthStreamID].as<rs2::video_stream_profile>();
	return video_stream.get_intrinsics();
	
	//If the stream is indeed a video stream, we can now simply call get_intrinsics()
	//if (rs2::depth_sensor dpt_sensor = m_depthSensor.as<rs2::depth_sensor>())
	//{
	//	auto depthScale = dpt_sensor.get_depth_scale();
	//}
	//auto depth_stream = m_selection.get_stream(RS2_STREAM_DEPTH).as<rs2::video_stream_profile>();
	//return depth_stream.get_intrinsics();
}

rs2_intrinsics Realsense2Camera::getColorIntrinsics()
{
	auto video_stream = m_stream_profiles_color[m_colorStreamID].as<rs2::video_stream_profile>();
	return video_stream.get_intrinsics();

	//auto color_stream = m_selection.get_stream(RS2_STREAM_COLOR).as<rs2::video_stream_profile>();
	//return color_stream.get_intrinsics();
}

uint32_t Realsense2Camera::getDepthUnit()
{

	float depthScale = m_sensors[0].as<rs2::depth_sensor>().get_depth_scale();

	return 100;

	//std::cout << "depth scale : " << depthScale * 1e6 << std::endl;
	return (uint32_t)(depthScale * 1000000.0f);
	//return m_ctrl_curr.depthUnits;
}

rs2_extrinsics Realsense2Camera::getDepthToColorExtrinsics()
{
	rs2_extrinsics outExtrin;
	try 
	{
		outExtrin = m_stream_profiles_depthIR[m_depthStreamID].get_extrinsics_to(m_stream_profiles_color[m_colorStreamID]);
		//outExtrin = m_stream_profiles_color[m_colorStreamChoice].get_extrinsics_to(m_stream_profiles_depthIR[m_depthStreamChoice]);

		std::cout << "Translation Vector : [" << outExtrin.translation[0] << "," << outExtrin.translation[1] << "," << outExtrin.translation[2] << "]\n";
		std::cout << "Rotation Matrix    : [" << outExtrin.rotation[0] << "," << outExtrin.rotation[3] << "," << outExtrin.rotation[6] << "]\n";
		std::cout << "                   : [" << outExtrin.rotation[1] << "," << outExtrin.rotation[4] << "," << outExtrin.rotation[7] << "]\n";
		std::cout << "                   : [" << outExtrin.rotation[2] << "," << outExtrin.rotation[5] << "," << outExtrin.rotation[8] << "]" << std::endl;

	}
	catch (const std::exception& e)
	{
		std::cerr << "Failed to get extrinsics for the given streams. " << e.what() << std::endl;

	}
	return outExtrin;
}

rs2_extrinsics Realsense2Camera::getColorToDepthExtrinsics()
{
	rs2_extrinsics outExtrin;
	try
	{
		//outExtrin = m_stream_profiles_depthIR[m_depthStreamChoice].get_extrinsics_to(m_stream_profiles_color[m_colorStreamChoice]);
		outExtrin = m_stream_profiles_color[m_colorStreamID].get_extrinsics_to(m_stream_profiles_depthIR[m_depthStreamID]);

		std::cout << "Translation Vector : [" << outExtrin.translation[0] << "," << outExtrin.translation[1] << "," << outExtrin.translation[2] << "]\n";
		std::cout << "Rotation Matrix    : [" << outExtrin.rotation[0] << "," << outExtrin.rotation[3] << "," << outExtrin.rotation[6] << "]\n";
		std::cout << "                   : [" << outExtrin.rotation[1] << "," << outExtrin.rotation[4] << "," << outExtrin.rotation[7] << "]\n";
		std::cout << "                   : [" << outExtrin.rotation[2] << "," << outExtrin.rotation[5] << "," << outExtrin.rotation[8] << "]" << std::endl;

	}
	catch (const std::exception& e)
	{
		std::cerr << "Failed to get extrinsics for the given streams. " << e.what() << std::endl;

	}
	return outExtrin;
}


