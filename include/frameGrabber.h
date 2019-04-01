#ifndef GRABBER_H
#define GRABBER_H


#include <iostream>
#include <string>
#include <thread>
#include <condition_variable> 
#include <mutex>

#include <librealsense2/rs.hpp>

#include "opencv2/core/utility.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <chrono>
#include <time.h>
#include <ctime>   // localtime



class FrameGrabber
{
public:
	FrameGrabber(const std::string& window_title)
		: m_thread(std::thread(&FrameGrabber::run, this))
		, m_window_title(window_title)
		, ready(false)
	{
	}
	~FrameGrabber() 
	{
		if (m_thread.joinable())
		{
			m_thread.join();
		}
	}
	void wait()
	{
		if (m_thread.joinable())
		{
			m_thread.join();
		}
	}
	void operator()(rs2::frame f)
	{	
		auto eTime = epchTime0();
		std::cout << eTime - previousTime << std::endl;
		previousTime = eTime;
		m_frames.enqueue(f);
		if (!ready)
		{
			std::unique_lock<std::mutex> lck(mtx);
			ready = true;
			condVar.notify_all();
		}

	}

private:
	void run()
	{
		//std::this_thread::sleep_for(std::chrono::seconds(5));
		std::cout << "ready : " << ready << std::endl;
		std::unique_lock<std::mutex> lck(mtx);

		condVar.wait(lck);

		while (ready)
		{
			//std::cout << "ready : " << ready << std::endl;

			rs2::frame frame;
			if (!m_frames.poll_for_frame(&frame))
			{
				frame = m_lastFrame;
			}
			m_lastFrame = frame;

			if (frame)
			{
				cv::Mat dFrame(480, 848, CV_16SC1, (void*)frame.get_data());
				if (!dFrame.empty())
				{
					cv::imshow(m_window_title, dFrame);
					cv::waitKey(1);
				}
			}

		}
		

	}

	const double epchTime0() {
		//DWORD ms = GetTickCount();
		//std::ostringstream stream;
		//stream << ms;
		//return stream.str();
		using namespace std::chrono;
		milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		std::ostringstream stream;
		return ms.count();

	}


	std::thread m_thread;
	rs2::frame_queue m_frames;
	rs2::frame m_lastFrame;
	bool ready;
	std::string m_window_title;
	std::condition_variable condVar;
	std::mutex mtx;
	double previousTime = 0;
};




#endif