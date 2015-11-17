#include "Paint.h"
using namespace cv;
void Paint::Draw(int x, int y)
{
	cv::circle(mDrawArea,Point(x,y),3,Scalar(255,0,0),CV_FILLED);
	imshow("Drawing",mDrawArea);
}
void Paint::InitPaint()
{
	mDrawArea = Mat(480,640,CV_8UC3,Scalar(255,255,255));
	imshow("Drawing", mDrawArea);
}
void Paint::ClosePaint()
{
	cv::destroyWindow("Drawing");
}
void Paint::Erase()
{
	mDrawArea = Mat(480,640,CV_8UC3,Scalar(255,255,255));
}
