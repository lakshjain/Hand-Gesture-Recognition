#include "SkinDetector.h"
#include "HandDetector.h"
#include "MouseController.h"
#include "Socket.h"
#include "CHelper.h"

#define WIDTH 640
#define HEIGHT 480

using namespace cv;
using namespace std;

void ShiftROI(int count, SkinDetector &obj)
{
	switch(count)
	{
		case 0: obj.CreateROI(WIDTH/2 ,HEIGHT/2-60,60,60);
				break;
		case 1: obj.CreateROI(580,180,60,60);
				break;
		case 2: obj.CreateROI(320,0,60,60);
				break;
		case 3: obj.CreateROI(0,180,60,60);
				break;	
		case 4: obj.CreateROI(320,420,60,60);
			break;
	}
}


int StartInteractiveMode()
{
	Socket* socketInstance = Socket::GetInstance();
	socketInstance->Init();

	VideoCapture camera(0);
	camera.set(CV_CAP_PROP_FRAME_WIDTH,WIDTH);
	camera.set(CV_CAP_PROP_FRAME_HEIGHT,HEIGHT);

	SkinDetector skinDetector;
	HandDetector handDetector;
	skinDetector.SelectModel();
	
	cv::BackgroundSubtractorMOG2 backgroundSubtractor(5,13,false);
	backgroundSubtractor.nmixtures = 3;	
	
	Mat liveFrame, backgroundFrame, foregroundFrame, erode, dilate, threshold, colorFrame, resultFrame, equalizerFrame;
	Mat roi;
	vector<Mat> channels;//for spliiting channels for equalizing histogram values of a color	

	bool backgroundFlag = true, sampleFlag = false, thresholdFlag = false, debugTools = false, resultFlag = false, normalizeFlag = false;
	int key;
	
	//For Finding ROI Position at Realtime*/
	//int roiX = 0,roiY = 0;
	//namedWindow("ROI Test");
	//createTrackbar("X","ROI Test",&roiX,WIDTH-1);
	//createTrackbar("Y","ROI Test",&roiY,HEIGHT-1);

	skinDetector.CreateROI(WIDTH/2 ,HEIGHT/2-60,60,60);
	
	int count = 0;
	while(1)
	{
		//cam >> LiveFrame;
		camera >> liveFrame;
		flip(liveFrame,liveFrame,1);
		cv::medianBlur(liveFrame,liveFrame,3);
		///for ROI Position 
		//cv::rectangle(liveFrame,cv::Rect(roiX,roiY,60,60),CV_RGB(255,0,0),2);


		//histogram equalization for stable illuminance factor
		if(normalizeFlag)
			{
				skinDetector.Normalize(liveFrame);
				cv::medianBlur(liveFrame,liveFrame,5);
			}
				
			
		if(backgroundFlag)
			{
				
				backgroundSubtractor.operator()(liveFrame,foregroundFrame);
				backgroundSubtractor.getBackgroundImage(backgroundFrame);
				
			}
		else
			{
				backgroundSubtractor.operator()(liveFrame,foregroundFrame,0.00001);
			}
		
		//for taking realtime samples
		if(sampleFlag && !debugTools)
		{
			skinDetector.TakeSample(liveFrame);
			sampleFlag = !sampleFlag;
		}
		if(thresholdFlag && !debugTools)
		{
			threshold = skinDetector.SkinThresholding(liveFrame);
			skinDetector.ErodeAndDilate(threshold,threshold,5,8);
			imshow("Threshold", threshold);
		}
		
		if(debugTools)
		{
			skinDetector.ShowTrackbars();
			threshold = skinDetector.TrackBarThreshold(liveFrame);
			//skinDetector.ErodeAndDilate(threshold,threshold,5,8);
			thresholdFlag = false;
			imshow("Threshold",threshold);
		}


		//morphological operations
		//skinDetector.ErodeAndDilate(foregroundFrame,foregroundFrame,5,8);
		cv::erode(foregroundFrame,foregroundFrame,getStructuringElement(MORPH_RECT,Size(3,3)));

		if(resultFlag && (thresholdFlag || debugTools))
		{
			//and operation on foreground and the skin segmented threshold
			cv::bitwise_and(foregroundFrame, threshold, resultFrame);
			skinDetector.ErodeAndDilate(resultFrame,resultFrame,5,8);
			imshow("Result",resultFrame);
			handDetector.FindConvexHull(liveFrame,resultFrame);
		}
		else if(!debugTools)
			skinDetector.ShowROI(liveFrame);
			
		
		
		imshow("LiveFrame",liveFrame);
		imshow("Background",backgroundFrame);
		imshow("Foreground",foregroundFrame);
		socketInstance->Run();
		
		//Various Keys For Various Modes
		key = waitKey(30);
		if((char)key == 'b')
			backgroundFlag = !backgroundFlag ;
		else if((char)key == 's' && !resultFlag)
			sampleFlag = true;
		else if((char)key == 't')
			thresholdFlag = !thresholdFlag; 
		else if ((char) key == 'd')
			debugTools = !debugTools;
		else if((char) key == 'r' && (thresholdFlag || debugTools)) 
			resultFlag = !resultFlag;
		else if((char)key == 'q')
		{
			cv::destroyAllWindows();
			camera.release();
			break;
		}
		else if((char)key == 'm')
			handDetector.ToggleMouseControl();
		else if((char)key == 'p')
		{
			handDetector.mSwipeON = !handDetector.mSwipeON;
			handDetector.mSwipeGesture.Reset();
		}
		else if((char)key == 'n')
			normalizeFlag = !normalizeFlag;
		else if((char)key == 'x')
			skinDetector.ResetLastSample();
		else if((char)key == 'c')
		{
			count++;
			skinDetector.DeleteLastROI();
			ShiftROI(count%5, skinDetector);
			if(count == 5)
			{	// for avoiding large values
				count = 0;
			}
		
		}
		else if((char)key == 'o')
		{
			handDetector.mObjectTrackFlag = !handDetector.mObjectTrackFlag;
			if(handDetector.mObjectTrackFlag)
				handDetector.InitPaint();
			else
				handDetector.ClosePaint();
		}
		else if((char)key == 'e' && handDetector.mObjectTrackFlag)
			handDetector.Erase();

		


	}
	socketInstance->Dispose();
	return 0;
}


int StartGamingMode()
{
	CHelper helper;
	helper.ReadGestures();

	bool isGestureStart = false, isSaving = false;
	int GID=1;
	Socket* socketInstance = Socket::GetInstance();
	socketInstance->Init();
	vector<Point> contour2;
	
	VideoCapture cam(0);
	cam.set(CV_CAP_PROP_FRAME_WIDTH,WIDTH);
	cam.set(CV_CAP_PROP_FRAME_HEIGHT,HEIGHT);
	Mat LiveFrame, TemplateROI;
	Rect roiRect = Rect(350,70,250,250);
	BackgroundSubtractorMOG2 bg(15,10,false);
	bg.nmixtures = 3;
	bool bgON = true;
	int key;
	vector<Point> temp;
	while(1)
	{
		cam >> LiveFrame;
		flip(LiveFrame,LiveFrame,1);
		
		TemplateROI = LiveFrame.clone().operator()(roiRect);
		

		cv::rectangle(LiveFrame,roiRect,Scalar(255,0,0),2);

		if(bgON)
		{
			bg.operator()(TemplateROI,TemplateROI);
			bg.setInt("history",15);
		}
		else
		{
			bg.operator()(TemplateROI,TemplateROI,0.0001);
			//bg.setInt("history",400);
		}
		cv::medianBlur(TemplateROI,TemplateROI,5);

		cv::erode(TemplateROI,TemplateROI,getStructuringElement(MORPH_RECT,Size(3,3)));
		cv::dilate(TemplateROI,TemplateROI,getStructuringElement(MORPH_RECT,Size(2,2)));

		//cv::dilate(TemplateROI,TemplateROI,getStructuringElement(MORPH_RECT,Size(3,3)));
		//cv::erode(TemplateROI,TemplateROI,getStructuringElement(MORPH_RECT,Size(4,4)));
		
		//cv::Canny(TemplateROI,TemplateROI,20,255);
		contour2.clear();
		contour2.operator=(helper.FindContour(TemplateROI.clone()));
		
		//contour2.push_back(findContour(sample));
		if(contour2.size() > 0 && !isSaving && contourArea(contour2) > 6000 && contourArea(contour2) < 50000)
		{
			
			//cout<<"\nsize : "<<contour2.size();
			//cout<<"\n"<<cv::matchShapes(contour1,contour2,CV_CONTOURS_MATCH_I1,0)<<" I2 : "<<cv::matchShapes(contour1,contour2,CV_CONTOURS_MATCH_I2,0)
				//<<" I3 : "<<cv::matchShapes(contour1,contour2,CV_CONTOURS_MATCH_I3,0);
			float gestureMatchI3 = 10,ID = 0;//max amount
			for(int i=0;i<helper.mContours.size();++i)
			{
				float I3;
				I3 = matchShapes(helper.mContours[i],contour2,CV_CONTOURS_MATCH_I3,0);
				if (I3 < gestureMatchI3)
				{
					gestureMatchI3 = I3;
					ID = i+1;
				}
			}
			//float gestureMatchI1 = matchShapes(contour1,contour2,CV_CONTOURS_MATCH_I2,0);
			cout<<"\n"<<gestureMatchI3<<" "<<ID;
			if(gestureMatchI3 < 0.23f && gestureMatchI3 != 0.00 ) 
			{
				string gesture;
				if(!isGestureStart)
				{
					isGestureStart = true;
					if(ID == 1)
					{
						gesture = "GID_B_PRESSED";
					}
					else if(ID == 2)
					{
						gesture = "GID_R_PRESSED";
					}
					socketInstance->Broadcast(gesture.c_str(),gesture.length());
				}
			}
			else if(isGestureStart)
			{
				isGestureStart = false;
				string gesture;
				if(ID == 1)
					{
						gesture = "GID_B_RELEASED";
					}
					else if(ID == 2)
					{
						gesture = "GID_R_RELEASED";
					}
				socketInstance->Broadcast(gesture.c_str(),gesture.length());
				
			}
				
		}
		else
			cout<<"\n contour not found";
		//contour2.clear();
		Mat t;
		bg.getBackgroundImage(t);
		imshow("LiveFrame",LiveFrame);
		imshow("ROI",TemplateROI);
		imshow("Background",t);

		socketInstance->Run();

		key = waitKey(20);
		if((char)key == 'b')
			bgON = !bgON;
		else if((char)key == 'q')
		{
			cv::destroyAllWindows();
			break;
		}
		else if((char)key == 's')
		{
			isSaving = true;
			helper.SaveGestures(GID++,TemplateROI);
		}
		if(GID > 2)
		{
			isSaving = false;
			GID = 1;
			helper.ReadGestures();
		}
			
	}
	socketInstance->Dispose();
	return 0;

}

int main()
{
	int option;
	do
	{
		system("cls");
		cout<<"Select Mode : \n";
		cout<<"1. Start Interactive Mode\n";
		cout<<"2. Start Gesture Gaming Mode\n";
		cout<<"3. Exit\n";
		cin>>option;

		if(option == 1)
			StartInteractiveMode();
		else if(option == 2)
			StartGamingMode();
		else
			option = 3;



	}while(option != 3);
	return 0;
}