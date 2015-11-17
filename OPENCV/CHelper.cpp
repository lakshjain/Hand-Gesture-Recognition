#include "CHelper.h"


using namespace std;
using namespace cv;

void CHelper::SaveGestures(int GID, Mat ROI)
{
	mContours.clear();
	if(GID == 1)
		imwrite("GestureSamples//Blue.jpg",ROI);
	else
		imwrite("GestureSamples//Red.jpg",ROI);
		
}

void CHelper::ReadGestures()
{
	Mat gestureImg; 
	gestureImg = imread("GestureSamples//Blue.jpg");
	cvtColor(gestureImg, gestureImg, COLOR_BGR2GRAY);
	threshold(gestureImg,gestureImg,0,255,THRESH_BINARY);
	cv::erode(gestureImg,gestureImg,getStructuringElement(MORPH_RECT,Size(2,2)));
	cv::dilate(gestureImg,gestureImg,getStructuringElement(MORPH_RECT,Size(2,2)));
	mContours.push_back(FindContour(gestureImg));
	imshow("Blue",gestureImg);

	gestureImg = imread("GestureSamples//Red.jpg");
	cvtColor(gestureImg, gestureImg, COLOR_BGR2GRAY);
	threshold(gestureImg,gestureImg,0,255,THRESH_BINARY);
	cv::erode(gestureImg,gestureImg,getStructuringElement(MORPH_RECT,Size(2,2)));
	cv::dilate(gestureImg,gestureImg,getStructuringElement(MORPH_RECT,Size(2,2)));
	mContours.push_back(FindContour(gestureImg));
	imshow("Red",gestureImg);

}

vector<Point> CHelper::FindContour(Mat image)
{
	int largest_area = 0, id = -1;
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	cv::findContours(image,contours,CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
	
	for(int i=0;i<contours.size();++i)
	{
		double area = cv::contourArea(contours[i],false);
		if(area > largest_area)
			
			{
				largest_area = area;
				id = i;
			}
	}
	if(contours.size() > 0 && id!= -1)
	{
		//cout<<"\nid : "<<id;
		return contours[id];
	}
	vector<Point> test;
	test.push_back(Point(0,0));
	contours.push_back(test);
	return contours[0];
}