#include "HandGestureController.h"
#include "Socket.h"
#include <Windows.h>
#define MOVEMENTFILTER 190.00f
#define FPS 5

using namespace std;
using namespace cv;

int HandGestureController::Swipe::sFrameNumber = 0;
Point HandGestureController::Swipe::sRefPoint = Point(0,0);

float HandGestureController::Swipe::Velocity(Point Start, Point End, int TimeSpan)
{
	
	float Velocity = 0;
	Point diff = End - Start;
    return sqrt((float)(diff.x*diff.x) + (float)(diff.y*diff.y))/TimeSpan;
	
}

void HandGestureController::Swipe::SwipeInit(Point COM, Mat& LiveFrame)
{
	if(Swipe::sFrameNumber > FPS)
	{
		Swipe::sFrameNumber = 0;
		Swipe::sRefPoint = COM;
	}
	else if(Swipe::sFrameNumber == FPS)
	{
		Point Start = Swipe::sRefPoint;
		int Speed = Velocity(Start,COM);
		//cout<<"\n"<<COM;
		if(Speed>120)
		{			
			Direction(Start,COM,LiveFrame);
		}
	}
	else if(Swipe::sFrameNumber == 0)
	{
		Swipe::sRefPoint = COM;
	}else if(sFrameNumber == 2)
	{
		
		Point Start = Swipe::sRefPoint;
		int Speed = Velocity(Start,COM);
		//For filtering had movements
		if(Velocity(Start,COM) < MOVEMENTFILTER)
		{
			cout<<"\n===============MISS GESTURE================";
			Swipe::sFrameNumber = 0;
		}
		else	
			cout<<"\n Speed : "<<Speed;
		
	}
	Swipe::sFrameNumber++;

}

void HandGestureController::Swipe::Direction(Point Start, Point End, Mat& LiveFrame)
{
	Point diff = End - Start;
	//cv::line(LiveFrame,Start,End,CV_RGB(0,255,255),2,8);
	if(abs(diff.x) >= abs(diff.y))
	{
		if(diff.x < 0)
		{
			
			putText(LiveFrame,"Left Swipe",Point(400,50),1,1,CV_RGB(255,0,0));
			cout<<"\nLEFT";
			keybd_event(VK_LEFT,0,KEYEVENTF_EXTENDEDKEY|0,0);
			keybd_event(VK_LEFT,0,KEYEVENTF_EXTENDEDKEY|KEYEVENTF_KEYUP,0);
			keybd_event(VK_NEXT,0,KEYEVENTF_EXTENDEDKEY|0,0);
			keybd_event(VK_NEXT,0,KEYEVENTF_EXTENDEDKEY|KEYEVENTF_KEYUP,0);
		}
		else
		{
			cout<<"\nRIGHT";
			string gesture = "GID_V_PRESSED";
			Socket::GetInstance()->Broadcast(gesture.c_str(),gesture.length());
			putText(LiveFrame,"Right Swipe",Point(400,50),1,1,CV_RGB(255,0,0));
			keybd_event(VK_RIGHT,0,KEYEVENTF_EXTENDEDKEY|0,0);
			keybd_event(VK_RIGHT,0,KEYEVENTF_EXTENDEDKEY|KEYEVENTF_KEYUP,0);
			keybd_event(VK_PRIOR,0,KEYEVENTF_EXTENDEDKEY|0,0);
			keybd_event(VK_PRIOR,0,KEYEVENTF_EXTENDEDKEY|KEYEVENTF_KEYUP,0);

		}
			
	}
	else
	{
		if(diff.y < 0)
		{
			cout<<"\nUP";
			putText(LiveFrame,"Up Swipe",Point(400,50),1,1,CV_RGB(255,0,0));
		}
		else
		{
			cout<<"\nDOWN";
			putText(LiveFrame,"Down Swipe",Point(400,50),1,1,CV_RGB(255,0,0));
		}
	}
	
}

void HandGestureController::Swipe::Reset()
{
	Swipe::sFrameNumber = 0;
	Swipe::sRefPoint = Point(0,0);
}

HandGestureController::HandGestureController()
{
	mSwipeON = false;
}