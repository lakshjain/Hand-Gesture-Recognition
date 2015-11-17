#pragma once
#include "opencv2\opencv.hpp"


class HandGestureController
{
	class Swipe
	{
		static int sFrameNumber;
		static cv::Point sRefPoint;
		public:
			float Velocity(cv::Point Start, cv::Point End, int TimeSpan = 1);
			void Direction(cv::Point Start, cv::Point End, cv::Mat& LiveFrame);
			void SwipeInit(cv::Point COM, cv::Mat& LiveFrame);
			void Reset();
	};
	public:
		Swipe mSwipeGesture;
		bool mSwipeON;
		
		std::vector<char> mSwipeCombo;

		HandGestureController();
		
	
};