#pragma once

#include "opencv2\opencv.hpp"
#include <string>

using namespace cv;

class MouseController
{
	protected:
		bool mMouseControl;
		int CalculateAngle(const cv::Point &Finger, const cv::Point &Center, int TiltAngle); 
		void ShowAngle(cv::Mat &LiveFrame, cv::Point p1, cv::Point p2, int Angle);
		void InitMouseControl(cv::Mat& LiveFrame, std::vector<cv::Point> FingerTips, cv::Point2f COM, int TiltAngle);
	
	public:
		MouseController();
		std::string ToString(int var);
		void ToggleMouseControl();

};