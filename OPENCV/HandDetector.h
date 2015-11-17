#pragma once
#include "MouseController.h"
#include "HandGestureController.h"
#include "Paint.h"
#include <iostream>

using namespace std;
using namespace cv;

class HandDetector : public MouseController, public HandGestureController, public Paint
{
		protected :

			void FindConvexityDefects(vector<Vec4i> ConvexityDefectSet, vector<Point> HandContour, Mat& LiveFrame, Point2f COM, int TiltAngle);
			//void kCurvature(vector<vector<Point>> contours, vector<vector<Point>> hull,vector< vector<int> >inthull, Mat& LiveFrame);
			int CalculateHandTilt(double m11, double m20, double m02);
			double GetOrientation(const vector<Point> &pts, Mat &img);
			void DrawAxis(Mat& img, Point p, Point q, Scalar colour, const float scale = 0.2);
			
		public :
			HandDetector();
			void FindConvexHull(Mat& LiveFrame, Mat SkinThreshold);
			bool mObjectTrackFlag;
			
};