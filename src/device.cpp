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

void Realsense2Camera::setDepthProperties(int width, int height, int rate)
{
	m_depthWidth = width;
	m_depthHeight = height;
	m_depthRate = rate;
}

void Realsense2Camera::setColorProperties(int width, int height, int rate)
{
	m_colorWidth = width;
	m_colorHeight = height;
	m_colorRate = rate;
}

bool Realsense2Camera::start()
{
	if (m_status == STOPPED)
	{
		rs2::config cfg;

		std::string serial_number(m_dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));

		cfg.enable_device(serial_number);

		cfg.enable_stream(RS2_STREAM_COLOR, m_colorWidth, m_colorHeight, RS2_FORMAT_Y16, m_colorRate);
		cfg.enable_stream(RS2_STREAM_DEPTH, m_depthWidth, m_depthHeight, RS2_FORMAT_Z16, m_colorRate);

		m_selection = m_pipe.start(cfg);

		m_status = CAPTURING;
	}
	return false;
}

bool Realsense2Camera::stop()
{
	if (m_status == CAPTURING)
	{
		m_status = STOPPED;
	}



	return false;
}

Realsense2Camera::Status Realsense2Camera::getStatus()
{
	return m_status;
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

			
			if (depth.supports_frame_metadata(RS2_FRAME_METADATA_FRAME_TIMESTAMP))
			{
				m_frameArrivalTime = depth.get_frame_metadata(RS2_FRAME_METADATA_FRAME_TIMESTAMP);
				uint64_t currentTime = depth.get_frame_metadata(RS2_FRAME_METADATA_FRAME_TIMESTAMP);
				uint64_t deltaTime = currentTime - previousTime;
				if (deltaTime > (1e6f / (float)m_depthRate) * 1.5f) // if greater than 1.5 frame duration in microseconds
				{
					std::cout << deltaTime << std::endl;
				}
				previousTime = currentTime;
			}


			auto dbg = m_selection.get_device().as<rs2::debug_protocol>();
			std::vector<uint8_t> cmd = { 0x14, 0, 0xab, 0xcd, 0x2a, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

			// Query frame size (width and height)
			const int w = color.as<rs2::video_frame>().get_width();
			const int h = color.as<rs2::video_frame>().get_height();

			const int wD = depth.as<rs2::video_frame>().get_width();
			const int hD = depth.as<rs2::video_frame>().get_height();

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
bool Realsense2Camera::getFrames(rs2::frame_queue &depthQ, rs2::frame_queue &colorQ)
{
	depthQ = m_depthQueue;
	colorQ = m_colorQueue;


	return false;
}

rs2_intrinsics Realsense2Camera::getDepthIntrinsics()
{
	auto depth_stream = m_selection.get_stream(RS2_STREAM_DEPTH).as<rs2::video_stream_profile>();
	return depth_stream.get_intrinsics();

}

rs2_intrinsics Realsense2Camera::getColorIntrinsics()
{
	auto color_stream = m_selection.get_stream(RS2_STREAM_COLOR).as<rs2::video_stream_profile>();
	return color_stream.get_intrinsics();

}

uint32_t Realsense2Camera::getDepthUnit()
{
	return m_ctrl_curr.depthUnits;
}