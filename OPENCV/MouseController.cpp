#include "MouseController.h"
#include <Windows.h>
#include <math.h>
using namespace std;
using namespace cv;

MouseController::MouseController()
{
	mMouseControl = false;
}

void MouseController::InitMouseControl(Mat& LiveFrame, vector<Point> FingerTips, Point2f COM, int TiltAngle)
{
	bool indexfin = false;
	bool midFin = false;
	String MouseStatus;
	if (FingerTips.size() > 1)
	{
		for (int i = 0; i < FingerTips.size(); i++)
		{
			int Angle = CalculateAngle(COM,FingerTips[i],TiltAngle);
			ShowAngle(LiveFrame,COM,FingerTips[i],Angle);
			line(LiveFrame,COM,FingerTips[i],CV_RGB(0,0,0),2);
			if (Angle >= 110 && Angle < 140)
				indexfin = true;
			if (Angle >= 85 && Angle <= 105)
				midFin = true;
		}

	}
	
	//finger check ends here
	if (indexfin && midFin)
		MouseStatus = "Move";
	else if (indexfin)
		MouseStatus = "Right Click";
	else if (midFin)
		MouseStatus = "Left Click";
	else MouseStatus = "No Movement";

	//putText(frame,position,Point(400,50),1,2,CV_RGB(255,0,0),2,8);
   
	
	INPUT ip;
	ip.type = INPUT_MOUSE;
	static bool buttonPressed = false;
	if (MouseStatus == "Move")
	{
		buttonPressed = false;
		putText(LiveFrame, MouseStatus, Point(50, 50), 1, 2, CV_RGB(0, 0, 255),2);
		SetCursorPos((1366/640)*(COM.x)*1.5,(768/480)*(COM.y));
	}
	else if (MouseStatus == "Left Click" && !buttonPressed)
	{
			ip.type = INPUT_MOUSE;
			buttonPressed = true;
			ip.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
			SendInput(1, &ip, sizeof(INPUT));
			ZeroMemory(&ip, sizeof(INPUT));
			ip.type = INPUT_MOUSE;
			ip.mi.dwFlags = MOUSEEVENTF_LEFTUP;
			SendInput(1, &ip, sizeof(INPUT));
			ZeroMemory(&ip, sizeof(INPUT));
			putText(LiveFrame, MouseStatus, Point(50, 50), 1, 2, CV_RGB(0, 0, 255), 2);
	}
	else if (MouseStatus == "Right Click" && !buttonPressed)
	{
				ip.type = INPUT_MOUSE;
				buttonPressed = true;
				ip.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
				SendInput(1, &ip, sizeof(INPUT));
				ZeroMemory(&ip, sizeof(INPUT));
				ip.type = INPUT_MOUSE;
				ip.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
				SendInput(1, &ip, sizeof(INPUT));
				ZeroMemory(&ip, sizeof(INPUT));
				ZeroMemory(&ip, sizeof(INPUT));
				putText(LiveFrame, MouseStatus, Point(50, 50), 1, 2, CV_RGB(0, 0, 255), 2, 8);
		
	}
	
}

int MouseController::CalculateAngle(const Point &Finger, const Point &Center, int TiltAngle) {
    float y_angle = Center.y - Finger.y; 
    float x_angle = Finger.x - Center.x;
    float theta = atan(y_angle/ x_angle);
    int Angle = (int) ( theta * 180 / CV_PI);
	
	if (Angle < 0)
		return Angle + 180;
	else
		return Angle;
	
}

void MouseController::ShowAngle(Mat &LiveFrame, Point p1, Point p2, int Angle)
{
	putText(LiveFrame, ToString(Angle), Point((p1.x + p2.x) / 2, (p1.y + p2.y) / 2),1,1,CV_RGB(255,0,0),2);
}

string MouseController::ToString(int var) { 
   ostringstream OutputString; 
   OutputString << var; 
   return OutputString.str(); 
} 

void MouseController::ToggleMouseControl()
{
	mMouseControl = !mMouseControl;
}


