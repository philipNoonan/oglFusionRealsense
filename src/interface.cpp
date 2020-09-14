#include "interface.h"



rs2::device_list Realsense2Interface::getDeviceList()
{
	return m_devices;
}

static std::string get_sensor_name(const rs2::sensor& sensor)
{
	// Sensors support additional information, such as a human readable name
	if (sensor.supports(RS2_CAMERA_INFO_NAME))
		return sensor.get_info(RS2_CAMERA_INFO_NAME);
	else
		return "Unknown Sensor";
}


int Realsense2Interface::searchForCameras()
{
	int numberOfRealsense = 0;
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
			std::cout << "  " << index << " : " << getDeviceName(device) << std::endl;
			index++;
		}
		m_threads.resize(index);
		m_cameras.resize(index);
		
		m_depthTables.resize(index);

		m_depthFrames.resize(index);
		m_colorFrames.resize(index);
		m_infraFrames.resize(index);

		m_depthProps.resize(index);
		m_colorProps.resize(index);

		m_depthIntrinsics.resize(index);
		m_colorIntrinsics.resize(index);

		m_depthQueues.resize(index);
		m_colorQueues.resize(index);
		m_infraQueues.resize(index);

		numberOfRealsense = index;

	}

	return numberOfRealsense;

}

void Realsense2Interface::setDepthProperties(int devNumber, int w, int h, int r)
{
	m_depthProps[devNumber].width = w;
	m_depthProps[devNumber].height = h;
	m_depthProps[devNumber].rate = r;
}

void Realsense2Interface::setColorProperties(int devNumber, int w, int h, int r)
{
	m_colorProps[devNumber].width = w;
	m_colorProps[devNumber].height = h;
	m_colorProps[devNumber].rate = r;
}

void Realsense2Interface::getDepthProperties(int devNumber, int &w, int &h, int &r)
{
	m_cameras[devNumber].getDepthProperties(w, h, r);
}

void Realsense2Interface::getColorProperties(int devNumber, int &w, int &h, int &r)
{
	m_cameras[devNumber].getColorProperties(w, h, r);
}

void Realsense2Interface::getDepthPropertiesFromFile(int &w, int &h, int &r)
{
	w = m_depthProps[0].width;
	h = m_depthProps[0].height;
	r = m_depthProps[0].rate;
}

void Realsense2Interface::getColorPropertiesFromFile(int &w, int &h, int &r)
{
	w = m_colorProps[0].width;
	h = m_colorProps[0].height;
	r = m_colorProps[0].rate;
}

void Realsense2Interface::setDepthTable(int devNumber, int depthMax, int depthMin, int depthUnits, int disparityMode, int disparityShift)
{
	m_depthTables[devNumber].depthClampMax = depthMax;
	m_depthTables[devNumber].depthClampMin = depthMin;
	m_depthTables[devNumber].depthUnits = depthUnits;
	m_depthTables[devNumber].disparityMode = disparityMode;
	m_depthTables[devNumber].disparityShift = disparityShift;

	m_cameras[devNumber].setDepthTable(m_depthTables[devNumber]);

}

void Realsense2Interface::startDevice(int devNumber, std::tuple<int, int, int, rs2_format> depthProfile, std::tuple<int, int, int, rs2_format> infraProfile, std::tuple<int, int, int, rs2_format> colorProfile)
{
	//Realsense2Camera camera;
	m_cameras[devNumber].setDev(m_devices[devNumber]);
	m_cameras[devNumber].setDepthProperties(depthProfile);
	m_cameras[devNumber].setInfraProperties(infraProfile);
	m_cameras[devNumber].setColorProperties(colorProfile);
	m_cameras[devNumber].setStreams();
	m_cameras[devNumber].setSensorOptions();


	//m_cameras[devNumber].start();

	m_threads[devNumber] = std::thread(&Realsense2Camera::start, &m_cameras[devNumber]);

	//std::cout << "  " << devNumber << " : Setting intrinsics" << std::endl;
	setDepthIntrinsics(devNumber);

	setColorIntrinsics(devNumber);

	//// this need references to the threads
	//m_threads[devNumber] = std::thread(&Realsense2Camera::capture, &m_cameras[devNumber]);
}

void Realsense2Interface::startDeviceFromFile(std::string filename, int useDepth, int useColor)
{
	int numberOfRealsense = 0;
	//rs2::context ctx;
	rs2::config cfg;

	cfg.enable_device_from_file(filename);
	rs2::pipeline_profile selection = m_pipe.start(cfg);
	auto playbackDevice = m_pipe.get_active_profile().get_device();

	//auto playbackDevice = ctx.load_device(filename);

	//m_devices.front() = device;

	m_threads.resize(1);
	m_cameras.resize(1);

	m_depthFrames.resize(1);
	m_colorFrames.resize(1);

	m_depthProps.resize(1);
	m_colorProps.resize(1);

	m_depthIntrinsics.resize(1);
	m_colorIntrinsics.resize(1);

	m_depthQueues.resize(1);
	m_colorQueues.resize(1);


	//m_cameras[0].setDev(playbackDevice);
	//m_cameras[0].setStreams();
	//m_cameras[0].setSensorOptions();
	//m_cameras[0].setDepthProperties(depthProfile);
	//m_cameras[0].setColorProperties(colorProfile);

	//m_threads[0] = std::thread(&Realsense2Camera::start, &m_cameras[0]);

	//m_cameras[0].startFromFile();

	auto depth_stream = selection.get_stream(RS2_STREAM_DEPTH)
		.as<rs2::video_stream_profile>();



	m_depthIntrinsics[0].cx = depth_stream.get_intrinsics().ppx;
	m_depthIntrinsics[0].cy = depth_stream.get_intrinsics().ppy;
	m_depthIntrinsics[0].fx = depth_stream.get_intrinsics().fx;
	m_depthIntrinsics[0].fy = depth_stream.get_intrinsics().fy;

	m_depthProps[0].width = depth_stream.width();
	m_depthProps[0].height = depth_stream.height();
	m_depthProps[0].rate = 90;

	if (useColor)
	{
		auto color_stream = selection.get_stream(RS2_STREAM_COLOR)
			.as<rs2::video_stream_profile>();
	
		m_colorIntrinsics[0].cx = color_stream.get_intrinsics().ppx;
		m_colorIntrinsics[0].cy = color_stream.get_intrinsics().ppy;
		m_colorIntrinsics[0].fx = color_stream.get_intrinsics().fx;
		m_colorIntrinsics[0].fy = color_stream.get_intrinsics().fy;

		m_colorProps[0].width = color_stream.width();
		m_colorProps[0].height = color_stream.height();
		m_colorProps[0].rate = 6;
	}
	else
	{
		m_colorIntrinsics[0].cx = depth_stream.get_intrinsics().ppx;
		m_colorIntrinsics[0].cy = depth_stream.get_intrinsics().ppy;
		m_colorIntrinsics[0].fx = depth_stream.get_intrinsics().fx;
		m_colorIntrinsics[0].fy = depth_stream.get_intrinsics().fy;

		m_colorProps[0].width = depth_stream.width();
		m_colorProps[0].height = depth_stream.height();
		m_colorProps[0].rate = 6;


	}
	

	auto sensor = selection.get_device().first<rs2::depth_sensor>();
	m_depthUnitFromFile = sensor.get_depth_scale() * 1000000.0f;

	
	//return numberOfRealsense;

}

void Realsense2Interface::stopDevice(int devNumber)
{
	m_cameras[devNumber].stop();
	if (m_threads[devNumber].joinable())
	{
		m_threads[devNumber].join();
	}
}

void Realsense2Interface::stopDevices()
{
	for (int i = 0; i < m_cameras.size(); i++)
	{
		if (m_threads[i].joinable())
		{
			m_threads[i].join();
		}
		m_cameras[i].stop();

	}
}

FrameIntrinsics Realsense2Interface::getDepthIntrinsics(int devnumber)
{
	return m_depthIntrinsics[devnumber];
}

FrameIntrinsics Realsense2Interface::getColorIntrinsics(int devnumber)
{
	return m_colorIntrinsics[devnumber];
}

uint32_t Realsense2Interface::getDepthUnit(int devNumber)
{
	return m_cameras[devNumber].getDepthUnit();
}

float Realsense2Interface::getDepthUnitFromFile()
{

	return m_depthUnitFromFile;
}

glm::mat4 Realsense2Interface::getDepthToColorExtrinsics(int devNumber)
{
	glm::mat4 dep2Col = glm::mat4(1.0f);

	if (true) { // HACK FOR 515
		glm::mat4 d2c{ 0.999978, 0.00326805, -0.00569149, 0.0f,
					 -0.00309894, 0.999561, 0.0294727, 0.0f,
					  0.00578531, -0.0294545, 0.999549, 0.0f,
					 -9.05889e-05, 0.0137201, -0.00740386, 1.0f };

		dep2Col = d2c;
	}
	else {

		rs2_extrinsics extrin = m_cameras[devNumber].getDepthToColorExtrinsics();

		dep2Col[0] = glm::vec4(extrin.rotation[0], extrin.rotation[1], extrin.rotation[2], 0.0f);
		dep2Col[1] = glm::vec4(extrin.rotation[3], extrin.rotation[4], extrin.rotation[5], 0.0f);
		dep2Col[2] = glm::vec4(extrin.rotation[6], extrin.rotation[7], extrin.rotation[8], 0.0f);
		dep2Col[3] = glm::vec4(extrin.translation[0], extrin.translation[1], extrin.translation[2], 1.0f);

	}

	return dep2Col;
}

glm::mat4 Realsense2Interface::getColorToDepthExtrinsics(int devNumber)
{
	glm::mat4 col2Dep = glm::mat4(1.0f);



	if (true) { // HACK FOR 515

		glm::mat4 c2d{ 0.999978, -0.00309894, 0.00578531, 0.0f,
			 0.00326805, 0.999561, -0.0294545, 0.0f,
			  -0.00569149, 0.0294727, 0.999549, 0.0f,
			 0.000175938, -0.0139319, 0.00699563, 1.0f };

		col2Dep = c2d;
		
		col2Dep[0] = glm::vec4(0.999978, -0.00309894, 0.00578531, 0.0f);
		col2Dep[1] = glm::vec4(0.00326805, 0.999561, -0.0294545, 0.0f);
		col2Dep[2] = glm::vec4(-0.00569149, 0.0294727, 0.999549, 0.0f);
		col2Dep[3] = glm::vec4(0.000175938, -0.0139319, 0.00699563, 1.0f);
	}
	else {

		rs2_extrinsics extrin = m_cameras[devNumber].getColorToDepthExtrinsics();

		col2Dep[0] = glm::vec4(extrin.rotation[0], extrin.rotation[1], extrin.rotation[2], 0.0f);
		col2Dep[1] = glm::vec4(extrin.rotation[3], extrin.rotation[4], extrin.rotation[5], 0.0f);
		col2Dep[2] = glm::vec4(extrin.rotation[6], extrin.rotation[7], extrin.rotation[8], 0.0f);
		col2Dep[3] = glm::vec4(extrin.translation[0], extrin.translation[1], extrin.translation[2], 1.0f);
	}



	return col2Dep;
}

void Realsense2Interface::getColorFrame(int devNumber, std::vector<uint16_t> &colorArray)
{
	if (colorArray.data() != NULL)
	{
		memcpy(colorArray.data(), m_colorFrames[devNumber].get_data(), m_colorProps[devNumber].width * m_colorProps[devNumber].height * sizeof(uint16_t));
	}
}

void Realsense2Interface::getDepthFrame(int devNumber, std::vector<uint16_t> &depthArray)
{
	if (depthArray.data() != NULL)
	{
		memcpy(depthArray.data(), m_depthFrames[devNumber].get_data(), m_depthProps[devNumber].width * m_depthProps[devNumber].height * sizeof(uint16_t));
	}
}

std::vector<rs2::frame_queue> Realsense2Interface::getDepthQueues()
{
	return m_depthQueues;
}

std::vector<rs2::frame_queue> Realsense2Interface::getColorQueues()
{
	return m_colorQueues;
}

std::vector<rs2::frame_queue> Realsense2Interface::getInfraQueues()
{
	return m_infraQueues;
}

std::vector<int> Realsense2Interface::getTemperature()
{
	std::vector<int> temps(m_cameras.size());
	for (int i = 0; i < m_cameras.size(); i++)
	{
		temps[i] = m_cameras[i].readTemperature();
	}
	return temps;
}

bool Realsense2Interface::collateFrames()
{
	bool frameReady = false;
	for (int i = 0; i < m_cameras.size(); i++)
	{
		//m_cameras[i].getStatus();

		m_cameras[i].getFrames(m_depthQueues[i], m_colorQueues[i], m_infraQueues[i]);

		frameReady = true;

	}

	return frameReady;
}

bool Realsense2Interface::collateFramesFromFile()
{
	bool frameReady = false;
	rs2::frameset frames;
	rs2::frame depth;
	rs2::frame color;

	if (m_pipe.poll_for_frames(&frames)) // Check if new frames are ready
	{
		depth = frames.get_depth_frame(); 
		m_depthQueues[0].enqueue(depth);

#ifdef USE_COLOR
		color = frames.get_color_frame();
		m_colorQueues[0].enqueue(color);
#endif
	}
	
	//m_cameras[0].getFramesFromFile(m_depthQueues[0], m_colorQueues[0]);
	frameReady = true;

	return frameReady;
}











std::string Realsense2Interface::getDeviceName(const rs2::device& dev)
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

void Realsense2Interface::setDepthIntrinsics(int devNumber)
{
	auto i = m_cameras[devNumber].getDepthIntrinsics();

	float fov[2];
	rs2_fov(&i, fov);

	//std::cout << " depth fov : " << fov[0] << " " << fov[1] << std::endl;

	m_depthIntrinsics[devNumber].cx = i.ppx;
	m_depthIntrinsics[devNumber].cy = i.ppy;
	m_depthIntrinsics[devNumber].fx = i.fx;
	m_depthIntrinsics[devNumber].fy = i.fy;
}

void Realsense2Interface::setColorIntrinsics(int devNumber)
{
	auto i = m_cameras[devNumber].getColorIntrinsics();

	m_colorIntrinsics[devNumber].cx = i.ppx;
	m_colorIntrinsics[devNumber].cy = i.ppy;
	m_colorIntrinsics[devNumber].fx = i.fx;
	m_colorIntrinsics[devNumber].fy = i.fy;

	float fov[2];
	rs2_fov(&i, fov);

	//std::cout << " color fov : " << fov[0] << " " << fov[1] << std::endl;

	if (i.model == RS2_DISTORTION_BROWN_CONRADY)
	{
		m_colorIntrinsics[devNumber].k1 = i.coeffs[0];
		m_colorIntrinsics[devNumber].k2 = i.coeffs[1];
		m_colorIntrinsics[devNumber].p1 = i.coeffs[2];
		m_colorIntrinsics[devNumber].p2 = i.coeffs[3];
		m_colorIntrinsics[devNumber].k3 = i.coeffs[4];



	}
}

void Realsense2Interface::setEmitterOptions(int devNumber, bool status, float power)
{
	m_cameras[devNumber].setEmitterOptions(status ? 1.0f : 0.0f, power);
}