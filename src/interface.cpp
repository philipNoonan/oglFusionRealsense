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
			std::cout << "  " << index << " : " << getDeviceName(device) << std::endl;
			index++;
		}
		m_threads.resize(index);
		m_cameras.resize(index);
		
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
	m_cameras[devNumber].setStreams();
	m_cameras[devNumber].setDepthProperties(m_depthProps[devNumber].width, m_depthProps[devNumber].height, m_depthProps[devNumber].rate);
	m_cameras[devNumber].setColorProperties(m_colorProps[devNumber].width, m_colorProps[devNumber].height, m_colorProps[devNumber].rate);

	//m_cameras[devNumber].start();

	m_threads[devNumber] = std::thread(&Realsense2Camera::start, &m_cameras[devNumber]);

	//std::cout << "  " << devNumber << " : Setting intrinsics" << std::endl;
	setDepthIntrinsics(devNumber);
	//setColorIntrinsics(devNumber);
	//// this need references to the threads
	//m_threads[devNumber] = std::thread(&Realsense2Camera::capture, &m_cameras[devNumber]);
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
		m_cameras[i].getStatus();

		m_cameras[i].getFrames(m_depthQueues[i], m_colorQueues[i]);

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

	m_colorIntrinsics[devNumber].cx = i.ppx;
	m_colorIntrinsics[devNumber].cy = i.ppy;
	m_colorIntrinsics[devNumber].fx = i.fx;
	m_colorIntrinsics[devNumber].fy = i.fy;
}