#include <iostream>
#include <future>

#include "opencv2/opencv.hpp"

using namespace cv;

std::promise<Mat*> promise_left;
std::promise<Mat*> promise_right;

std::mutex g_mtx_io;
std::condition_variable g_cv_wait;

Mat* obstacle_left;
Mat* obstacle_right;

// 左右摄像头获取图像时的时间点
auto epoch_time = std::chrono::time_point<std::chrono::steady_clock>();
auto left_time = epoch_time;
auto right_time = epoch_time;

bool b_restart1 = false;
bool b_restart2 = false;

int exposure_left = 0;
int exposure_right = 0;
int left_cam = 0;
int right_cam = 1;

void fun1()
{
restart1:
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
		if (b_restart1) { cap1.release(); b_restart1 = false; goto restart1; }

		Mat left;
		do {
			cap1 >> left;

		} while (left.empty());

		left_time = std::chrono::steady_clock::now();
		promise_left.set_value(&left);

		std::unique_lock<std::mutex> lk(mut_left);
		g_cv_wait.wait(lk);
	}
}

void fun2()
{
restart2:
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
		if (b_restart2) { cap2.release(); b_restart2 = false; goto restart2; }

		Mat right;
		do {
			cap2 >> right;

		} while (right.empty());

		right_time = std::chrono::steady_clock::now();
		promise_right.set_value(&right);

		std::unique_lock<std::mutex> lk(mut_right);
		g_cv_wait.wait(lk);
	}
}

int main()
{
	unsigned int total_count = 0;
	unsigned int drop_count = 0;

	std::thread t1(fun1);
	std::thread t2(fun2);

	while (1)
	{
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
					b_restart1 = true;
					b_restart2 = true;
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
