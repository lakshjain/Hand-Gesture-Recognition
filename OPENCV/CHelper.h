#pragma once
#include "opencv2\opencv.hpp"
#include <iostream>

class CHelper
{
	public:
		void SaveGestures(int GID, cv::Mat ROI);
		void ReadGestures();
		std::vector<cv::Point> FindContour(cv::Mat image);
		std::vector<std::vector<cv::Point>> mContours;

};