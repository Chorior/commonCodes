#include <iostream>
#include <future>
#include <Windows.h>

#include "opencv2/opencv.hpp"

using namespace cv;

std::promise<Mat*> promise_left;
std::promise<Mat*> promise_right;

std::mutex g_mtx_io;
std::condition_variable g_cv_wait;

Mat* obstacle_left;
Mat* obstacle_right;

// 左右摄像头获取图像时的时间点
auto epoch_time = std::chrono::time_point<std::chrono::system_clock>();
auto left_time = epoch_time;
auto right_time = epoch_time;

int exposure_left = 0;
int exposure_right = 0;
int left_cam = 0;
int right_cam = 1;

void fun1()
{
	VideoCapture cap1(left_cam);
	if (!cap1.isOpened())
	{
		std::lock_guard<std::mutex> lk(g_mtx_io);
		std::cout << "cap1(0) is not opened" << std::endl;
		return;
	}
	else
	{
		std::lock_guard<std::mutex> lk(g_mtx_io);
		std::cout << "cap1(0) is opened " << std::endl;
	}

	cap1.set(CAP_PROP_FRAME_HEIGHT, 360);
	cap1.set(CAP_PROP_FRAME_WIDTH, 640);

	if (!exposure_left)
		cap1.set(CV_CAP_PROP_AUTO_EXPOSURE, 1);
	else
		cap1.set(CV_CAP_PROP_EXPOSURE, exposure_left);

	cap1.set(CV_CAP_PROP_FPS, 15);

	std::mutex mut_left;
	while (1)
	{
		Mat left;
		do {
			cap1 >> left;

		} while (left.empty());

		left_time = std::chrono::system_clock::now();
		promise_left.set_value(&left);

		std::unique_lock<std::mutex> lk(mut_left);
		g_cv_wait.wait(lk);
	}
}

void fun2()
{
	VideoCapture cap2(right_cam);

	if (!cap2.isOpened())
	{
		std::lock_guard<std::mutex> lk(g_mtx_io);
		std::cout << "cap2(1) is not opened" << std::endl;
		return;
	}
	else
	{
		std::lock_guard<std::mutex> lk(g_mtx_io);
		std::cout << "cap2(1) is opened " << std::endl;
	}

	cap2.set(CAP_PROP_FRAME_HEIGHT, 360);
	cap2.set(CAP_PROP_FRAME_WIDTH, 640);

	if (!exposure_right)
		cap2.set(CV_CAP_PROP_AUTO_EXPOSURE, 1);
	else
		cap2.set(CV_CAP_PROP_EXPOSURE, exposure_right);

	cap2.set(CV_CAP_PROP_FPS, 15);

	std::mutex mut_right;
	while (1)
	{
		Mat right;
		do {
			cap2 >> right;

		} while (right.empty());

		right_time = std::chrono::system_clock::now();
		promise_right.set_value(&right);

		std::unique_lock<std::mutex> lk(mut_right);
		g_cv_wait.wait(lk);
	}
}

int main()
{
	unsigned int total_count = 0;
	unsigned int drop_count = 0;

	const DWORD sleep_time_min = 10;
	const DWORD sleep_time_max = 200;
	DWORD sleep_time = sleep_time_min;

	std::thread t1(fun1);
	std::thread t2(fun2);

	while (1)
	{
		Sleep(sleep_time);

		obstacle_left = promise_left.get_future().get();
		obstacle_right = promise_right.get_future().get();

		promise_left = std::promise<Mat*>();
		promise_right = std::promise<Mat*>();

		auto timediff = fabs(std::chrono::duration<double, std::milli>(left_time - right_time).count());
		std::cout << "timediff: " << timediff << std::endl;

		++total_count;
		if (timediff > 10)
		{
			std::cout << "\t\t\t\t\t\t\t\tdrop frames" << std::endl;

			if (drop_count == 0)
			{
				drop_count = total_count;
			}
			else
			{
				if (3 > (total_count - drop_count))
				{
					sleep_time = (sleep_time < sleep_time_max) ? (sleep_time + 2) : sleep_time_min;
					std::cout << "\t\t\tsleep_time = " << sleep_time << std::endl;
				}
				drop_count = 0;
				total_count = 0;
			}
			g_cv_wait.notify_all();
			continue;
		}

		imshow("left", *obstacle_left);
		imshow("right", *obstacle_right);
		waitKey(1);

		g_cv_wait.notify_all();
	}

	t1.join();
	t2.join();
	return 0;
}
