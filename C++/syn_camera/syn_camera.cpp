#include <iostream>
#include <unistd.h>
#include <cmath>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "opencv2/opencv.hpp"

using namespace cv;

std::condition_variable cond_left;
std::condition_variable cond_right;
std::condition_variable cond_main;

Mat mat_left;
Mat mat_right;

int64 left_time = 0;
int64 right_time = 0;

bool need_left_frame = false;
bool need_right_frame = false;

std::mutex mtx_left;
std::mutex mtx_right;
std::mutex mtx_main;
std::mutex mtx_cout;

void camera0()
{
	VideoCapture cap(0);
	if(!cap.isOpened()){
		std::lock_guard<std::mutex> lg(mtx_cout);
		std::cout << "cap(0) is not opened.\n";
		return;
	}else{
		std::lock_guard<std::mutex> lg(mtx_cout);
		std::cout << "cap(0) is opened.\n";
	}

	cap.set(CV_CAP_PROP_AUTO_EXPOSURE,1);
	cap.set(CV_CAP_PROP_FPS,30);

	std::unique_lock<std::mutex> lck(mtx_left);
	while(1){
		cond_left.wait(lck,[]{return need_left_frame;});
		need_left_frame = false;

		do{
			cap >> mat_left;
		}while(mat_left.empty());

		left_time = getTickCount();

		cond_main.notify_one();
	}
}

void camera1()
{
	VideoCapture cap(1);
	if(!cap.isOpened()){
		std::lock_guard<std::mutex> lg(mtx_cout);
		std::cout << "cap(1) is not opened.\n";
		return;
	}else{
		std::lock_guard<std::mutex> lg(mtx_cout);
		std::cout << "cap(1) is opened.\n";
	}

	cap.set(CV_CAP_PROP_AUTO_EXPOSURE,1);
	cap.set(CV_CAP_PROP_FPS,30);

	std::unique_lock<std::mutex> lck(mtx_right);
	while(1){
		cond_right.wait(lck,[]{return need_right_frame;});
		need_right_frame = false;

		do{
			cap >> mat_right;
		}while(mat_right.empty());

		right_time = getTickCount();

		cond_main.notify_one();
	}
}

int main()
{
	unsigned int total_count = 0;
	unsigned int drop_time = 0;

	const useconds_t sleep_time_min = 10 * 1000;
	const useconds_t sleep_time_max = 200 * 1000;
	useconds_t sleep_time = sleep_time_min;

	std::thread t1(camera0);
	std::thread t2(camera1);

	while(1)
	{
		usleep(sleep_time);

		need_left_frame = true;
		need_right_frame = true;
		cond_left.notify_one();
		cond_right.notify_one();

		std::unique_lock<std::mutex> lck(mtx_main);
		cond_main.wait(lck,[]{return left_time && right_time;});

		double timediff =
			fabs((left_time - right_time)) / getTickFrequency() * 1000;
		{
			std::lock_guard<std::mutex> lg(mtx_cout);
			std::cout << "timediff: "
					  << timediff << std::endl;
		}

		++ total_count;

		if(5 < timediff)
		{
			{
				std::lock_guard<std::mutex> lg(mtx_cout);
				std::cout << "\t\t\t\t\t\tdrop frame\n";
			}

			if(!drop_time){
				drop_time = total_count;
			}else{
				if(3 > total_count - drop_time)
				{
					sleep_time =
						(sleep_time < sleep_time_max)?(sleep_time + 2*1000):sleep_time_max;

					std::lock_guard<std::mutex> lg(mtx_cout);
					std::cout << "\t\t\tsleep_time = "
							  << sleep_time << std::endl;
				}
				drop_time = 0;
				total_count = 0;
			}

			{
				mat_left.release();
				mat_right.release();
				left_time = 0;
				right_time = 0;
			}

			continue;
		}

		{
			std::lock_guard<std::mutex> lg(mtx_cout);
			std::cout << "here is the synchronous pictures: "
					  << "mat_left.data, and mat_right.data\n";
		}

		{
			mat_left.release();
			mat_right.release();
			left_time = 0;
			right_time = 0;
		}
	}

	t1.join();
	t2.join();

	exit(EXIT_SUCCESS);
}
