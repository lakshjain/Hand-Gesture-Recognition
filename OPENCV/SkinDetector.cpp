#include "SkinDetector.h"
//Color Codes
#define B 1
#define G 2
#define R 3
#define H 4
#define S 5
#define V 6
#define Y 7
#define Cb 8
#define Cr 9

using namespace std;
using namespace cv;

SkinDetector::SkinDetector()
{
	mTrackMinH = mTrackMinS = mTrackMinV = mTrackMinY = mTrackMinCr = mTrackMinCb = 0;
	mTrackMaxS = mTrackMaxV = mTrackMaxY = mTrackMaxCr = mTrackMaxCb = 255;
	mTrackMaxH = 180;	//As H in cylinderical Coordinate System having max range upto 180
	mTrackMinB = mTrackMinG = mTrackMinR = mTrackMaxB = mTrackMaxG = mTrackMaxR = 0;

	//Map for mixing different channels as per user demand
	mMixChannels[B] = false;
	mMixChannels[G] = false;
	mMixChannels[R] = false;
	
	mMixChannels[H] = false;
	mMixChannels[S] = false;
	mMixChannels[V] = false;
	
	mMixChannels[Y] = false;
	mMixChannels[Cr] = false;
	mMixChannels[Cb] = false;

}

//For Selecting Model
void SkinDetector::SelectModel()
{
	system("cls");
	cout<<"===================================================\n";
	cout<<"1. YCrCb Model\n";
	cout<<"2. HSV Model\n";
	cout<<"3. Make your own Model\n";
	cout<<"===================================================\n";
	cin>>mModel;
	if(mModel == 3)
	{
		ShowChannels();
	}
	if(mModel != 1 && mModel !=2 && mModel != 3)
			mModel = 1; //taking YCrCb model as default Model for skin colour segmentation

	
}

void SkinDetector::ShowChannels()
{
	int color=-1;
	do{
		system("cls");
		cout<<"===================================================\n";
		cout<<"1. B\n";
		cout<<"2. G\n";
		cout<<"3. R\n\n";
		cout<<"4. H\n";
		cout<<"5. S\n";
		cout<<"6. V\n\n";
		cout<<"7. Y\n";
		cout<<"8. Cr\n";
		cout<<"9. Cb\n\n";
		cout<<"0. Exit\n";
		cout<<"===================================================\n";
		cout<<"Select option (Example For Colour H Press 1) : ";
		cin>>color;
		if (mMixChannels.find(color) != mMixChannels.end())
			mMixChannels[color] = true;

		
	}while(color);
	InitMinMaxValues();
		
}

//Take samples of the ROI of the current Frame
void SkinDetector::TakeSample(Mat LiveFrame)
{
	for(int i=0;i<mRoiRect.size();++i)
	{
		mROI = LiveFrame.clone()(mRoiRect[i]);
		Mat tempROI;
		switch(mModel)
		{
			case 1: cv::cvtColor(mROI,tempROI,COLOR_BGR2YCrCb);//YCrCb Model
					CalculateRange(tempROI,1);
					break;
			case 2: cv::cvtColor(mROI,tempROI,COLOR_BGR2HSV);//HSV Model
					CalculateRange(tempROI,2);
					break;
			case 3: NewModel_Range(mROI);
					break;
		
		}
	}
	
	
}

//creates ROI rectangle on current Frame
void SkinDetector::CreateROI(int x, int y, int width, int height)
{
	mRoiRect.push_back(cv::Rect(cv::Rect(cv::Point(x,y),cv::Point(x+width,y+height))));
}

void SkinDetector::ShowROI(Mat &LiveFrame)
{
	for(int i=0;i<mRoiRect.size();++i)
		rectangle(LiveFrame,mRoiRect[i],Scalar(0,255,0),2);

}

void SkinDetector::DeleteLastROI()
{
	mRoiRect.pop_back();
}

void SkinDetector::CalculateRange(Mat ROI,int ModelOption)
{
	vector<Mat> colors;
	cv::split(mROI,colors);
	double min, max;
	
	switch (ModelOption)
	{
		//YCrCb Model
	case 1:	
			minMaxIdx(colors[0], &min, &max);// function for finding Min and Max value of the colour 
			mMinY.push_back(min);
			mMaxY.push_back(max);

			minMaxIdx(colors[1], &min, &max);
			mMinCr.push_back(min);
			mMaxCr.push_back(max);

			minMaxIdx(colors[2], &min, &max);
			mMinCb.push_back(min);
			mMaxCb.push_back(max);

			break;
			
			
			//HSV Model
	case 2:	minMaxIdx(colors[0], &min, &max);// function for finding Min and Max value of the colour 
			mMinH.push_back(min);
			mMaxH.push_back(max);

			minMaxIdx(colors[1], &min, &max);
			mMinS.push_back(min);
			mMaxS.push_back(max);

			minMaxIdx(colors[2], &min, &max);
			mMinV.push_back(min);
			mMaxV.push_back(max);
			break;
						
	}
	
		
}
//for skin thresholding on the basis of Skin Sample Range
Mat SkinDetector::SkinThresholding(Mat LiveFrame)
{	
	bool emptyThreshold = false;
	Scalar lowerBound;
	Scalar higherBound;
	switch(mModel)
	{
	case 1: if(mMinY.size() > 0 && mMinCr.size() > 0 && mMinCb.size() > 0) 
			{
				cvtColor(LiveFrame,mThreshold,COLOR_BGR2YCrCb);
				lowerBound = Scalar( *min_element(mMinY.begin(),mMinY.end()), *min_element(mMinCr.begin(),mMinCr.end()), *min_element(mMinCb.begin(),mMinCb.end()));
				higherBound = Scalar(*max_element(mMaxY.begin(),mMaxY.end()), *max_element(mMaxCr.begin(),mMaxCr.end()), *max_element(mMaxCb.begin(),mMaxCb.end()));
				//cout<<lowerBound<<" " <<higherBound;
				inRange(mThreshold,lowerBound,higherBound,mThreshold);
			}
			else
				emptyThreshold = true;
				break;

	case 2: if(mMinH.size() > 0 && mMinS.size() > 0 && mMinV.size() > 0) 
			{
				cvtColor(LiveFrame,mThreshold,COLOR_BGR2HSV);
				lowerBound = Scalar( *min_element(mMinH.begin(),mMinH.end()), *min_element(mMinS.begin(),mMinS.end()), *min_element(mMinV.begin(),mMinV.end()));
				higherBound = Scalar(*max_element(mMaxH.begin(),mMaxH.end()), *max_element(mMaxS.begin(),mMaxS.end()), *max_element(mMaxV.begin(),mMaxV.end()));
				inRange(mThreshold,lowerBound,higherBound,mThreshold);
			}
			else
				emptyThreshold = true;
			
			break;

	case 3: if(mMinY.size() > 0 && mMinCr.size() > 0 && mMinCb.size() > 0 
				&& mMinH.size() > 0 && mMinS.size() > 0 && mMinV.size() > 0
				&& mMinB.size() > 0 && mMinG.size() > 0 && mMinR.size() > 0)
					NewModel_Thresholding(LiveFrame);
			else
				emptyThreshold = true;
			
			break;
	}
	if(emptyThreshold)
		mThreshold = Mat::zeros(LiveFrame.size(),CV_8UC1);
	return mThreshold;
}

void SkinDetector::ShowTrackbars()
{
	namedWindow("Trackbars");
	switch(mModel)
	{
	case 1: //For YCrCb Model
			createTrackbar("Min Y ","Trackbars",&mTrackMinY,255);
			createTrackbar("Max Y ","Trackbars",&mTrackMaxY,255);
			
			createTrackbar("Min Cr ","Trackbars",&mTrackMinCr,255);
			createTrackbar("Max Cr ","Trackbars",&mTrackMaxCr,255);
			
			createTrackbar("Min Cb ","Trackbars",&mTrackMinCb,255);
			createTrackbar("Max Cb ","Trackbars",&mTrackMaxCb,255);
			
			break;
	case 2:	//For HSV Model
			createTrackbar("Min H ","Trackbars",&mTrackMinH,179);
			createTrackbar("Max H ","Trackbars",&mTrackMaxH,179);
			
			createTrackbar("Min S ","Trackbars",&mTrackMinS,255);
			createTrackbar("Max S ","Trackbars",&mTrackMaxS,255);
			
			createTrackbar("Min V ","Trackbars",&mTrackMinV,255);
			createTrackbar("Max V ","Trackbars",&mTrackMaxV,255);
			break;
	case 3: //for new Model
		NewModel_ShowTrackBars();
	
	}
}

Mat SkinDetector::TrackBarThreshold(Mat LiveFrame)
{
	
	Scalar lowerBound;
	Scalar higherBound;

	switch(mModel)
	{
	case 1: cvtColor(LiveFrame,mTrackbarThreshold,COLOR_BGR2YCrCb);
			lowerBound = Scalar( mTrackMinY , mTrackMinCr, mTrackMinCb);
			higherBound = Scalar( mTrackMaxY, mTrackMaxCr, mTrackMaxCb);
			inRange(mTrackbarThreshold,lowerBound,higherBound,mTrackbarThreshold);
			break;
	case 2: cvtColor(LiveFrame,mTrackbarThreshold,COLOR_BGR2HSV);
			lowerBound = Scalar( mTrackMinH , mTrackMinS, mTrackMinV);
			higherBound = Scalar( mTrackMaxH, mTrackMaxS, mTrackMaxV);
			inRange(mTrackbarThreshold,lowerBound,higherBound,mTrackbarThreshold);
			break;
	case 3: NewModel_TrackBarThreshold(LiveFrame);
	}
	return mTrackbarThreshold;
	
}

void SkinDetector::NewModel_Thresholding(Mat LiveFrame)
{
	Scalar lowerBound;
	Scalar higherBound;
	Mat HSV, YCrCb,threshold;
	
	
	if(mMixChannels[B] || mMixChannels[G] || mMixChannels[R])//Consider RGB Range only if any colour of RGB is included in the new model
	{
		lowerBound = Scalar( *min_element(mMinB.begin(),mMinB.end()), *min_element(mMinG.begin(),mMinG.end()), *min_element(mMinR.begin(),mMinR.end()));
		higherBound = Scalar(*max_element(mMaxB.begin(),mMaxB.end()), *max_element(mMaxG.begin(),mMaxG.end()), *max_element(mMaxR.begin(),mMaxR.end()));
		inRange(LiveFrame,lowerBound,higherBound,threshold);
		
		//ErodeAndDilate(threshold,5,5);
	//	imshow("BGR threshold",threshold);

		mMultipleThreshold.push_back(threshold);
	}
	
	
	
	if(mMixChannels[H] || mMixChannels[S] || mMixChannels[V])//Consider HSV Range only if any component of HSV is included in the new model
	{
		vector<Mat> HSVComponent;
		cvtColor(LiveFrame,HSV,COLOR_BGR2HSV);
		lowerBound = Scalar( *min_element(mMinH.begin(),mMinH.end()), *min_element(mMinS.begin(),mMinS.end()), *min_element(mMinV.begin(),mMinV.end()));
		higherBound = Scalar(*max_element(mMaxH.begin(),mMaxH.end()), *max_element(mMaxS.begin(),mMaxS.end()), *max_element(mMaxV.begin(),mMaxV.end()));
		
		split(HSV,HSVComponent);
		if(mMixChannels[H])
		{
			NewModel_InRange(HSVComponent[0], lowerBound.operator[](0), higherBound.operator[](0), HSVComponent[0]);
			//ErodeAndDilate(HSVComponent[0],5,7);
			mMultipleThreshold.push_back(HSVComponent[0]);
			//imshow("H",HSVComponent[0]);
		}
		if(mMixChannels[S])
		{
			NewModel_InRange(HSVComponent[1], lowerBound.operator[](1), higherBound.operator[](1), HSVComponent[1]);
			//ErodeAndDilate(HSVComponent[1],5,7);
			mMultipleThreshold.push_back(HSVComponent[1]);
			//imshow("S",HSVComponent[1]);
		}
		if(mMixChannels[V])
		{
			NewModel_InRange(HSVComponent[2], lowerBound.operator[](2), higherBound.operator[](2), HSVComponent[2]);
			//ErodeAndDilate(HSVComponent[2],5,7);
			mMultipleThreshold.push_back(HSVComponent[2]);
			//imshow("V",HSVComponent[2]);
		}

	}
	
	
	if(mMixChannels[Y] || mMixChannels[Cr] || mMixChannels[Cb])//Consider YCrCb Range only if any component of YCrCb is included in the new model
	{
		vector<Mat> YCrCbComponent;
		cvtColor(LiveFrame,YCrCb,COLOR_BGR2YCrCb);
		
		lowerBound = Scalar( *min_element(mMinY.begin(),mMinY.end()), *min_element(mMinCr.begin(),mMinCr.end()), *min_element(mMinCb.begin(),mMinCb.end()));
		higherBound = Scalar(*max_element(mMaxY.begin(),mMaxY.end()), *max_element(mMaxCr.begin(),mMaxCr.end()), *max_element(mMaxCb.begin(),mMaxCb.end()));
		
		split(YCrCb,YCrCbComponent);
		if(mMixChannels[Y])
		{
			NewModel_InRange(YCrCbComponent[0], lowerBound.operator[](0), higherBound.operator[](0), YCrCbComponent[0]);
			//ErodeAndDilate(YCrCbComponent[0],5);
			mMultipleThreshold.push_back(YCrCbComponent[0]);
			//imshow("Y",YCrCbComponent[0]);
		}
		if(mMixChannels[Cr])
		{
			NewModel_InRange(YCrCbComponent[1], lowerBound.operator[](1), higherBound.operator[](1), YCrCbComponent[1]);
			//ErodeAndDilate(YCrCbComponent[1],5);
			mMultipleThreshold.push_back(YCrCbComponent[1]);
			//imshow("Cr",YCrCbComponent[1]);
		}
		if(mMixChannels[Cb])
		{
			NewModel_InRange(YCrCbComponent[2], lowerBound.operator[](2), higherBound.operator[](2), YCrCbComponent[2]);
			//ErodeAndDilate(YCrCbComponent[2],5);
			mMultipleThreshold.push_back(YCrCbComponent[2]);
			//imshow("Cb",YCrCbComponent[2]);
		}

	}

	
	//cout<<"\nsize : "<<mMultipleThreshold.size();

	for(int i=1;i<mMultipleThreshold.size();++i)
		bitwise_and(mMultipleThreshold[0],mMultipleThreshold[i],mMultipleThreshold[0]);
	
	mThreshold = mMultipleThreshold[0];
	mMultipleThreshold.clear();
}

void SkinDetector::NewModel_CalculateMinMax(Mat ColorROI, vector<double>& Min, vector<double>& Max)
{
	double min,max;
	minMaxIdx(ColorROI, &min, &max);// function for finding Min and Max value of the colour 
	Min.push_back(min);
	Max.push_back(max);	
}

void SkinDetector::NewModel_Range(Mat ROI)
{
	Mat Color;
	vector<Mat> BGR;
	vector<Mat> HSV;
	vector<Mat> YCrCb;
	

	split(ROI,BGR);
	if(mMixChannels[B])//Blue Colour Range
	{
		NewModel_CalculateMinMax(BGR[0],mMinB,mMaxB);
	}
	if(mMixChannels[G])//Green Colour Range
	{
		NewModel_CalculateMinMax(BGR[1],mMinG,mMaxG);
	}
	if(mMixChannels[R])//Red Colour Range
	{
		NewModel_CalculateMinMax(BGR[2],mMinR,mMaxR);
	}

	cvtColor(ROI,Color,COLOR_BGR2HSV);
	split(Color,HSV);
	if(mMixChannels[H])//Turn Off Hue Colour
	{
		NewModel_CalculateMinMax(HSV[0],mMinH,mMaxH);
	}
	if(mMixChannels[S])//Saturation Colour Thresholding
	{
		NewModel_CalculateMinMax(HSV[1],mMinS,mMaxS);
	}
	if(mMixChannels[V])//Value Colour Thresholding
	{
		NewModel_CalculateMinMax(HSV[2],mMinV,mMaxV);
	}

	cvtColor(ROI,Color,COLOR_BGR2YCrCb);
	split(Color,YCrCb);
	if(mMixChannels[Y])//Y component Thresholding
	{
		NewModel_CalculateMinMax(YCrCb[0],mMinY,mMaxY);
	}
	if(mMixChannels[Cr])//Cr component Thresholding
	{
		NewModel_CalculateMinMax(YCrCb[1],mMinCr,mMaxCr);
	}
	if(mMixChannels[Cb])//Cb component Thresholding
	{
		NewModel_CalculateMinMax(YCrCb[2],mMinCb,mMaxCb);
	}
}

void SkinDetector::InitMinMaxValues()
{
	if(!mMixChannels[B])
	{
		mMinB.push_back(0); mMaxB.push_back(0);
	}
	if(!mMixChannels[G])
	{
		mMinG.push_back(0); mMaxG.push_back(0);
	}
	if(!mMixChannels[R])
	{
		mMinR.push_back(0); mMaxR.push_back(0);
	}

	
	if(!mMixChannels[H])
	{
		mMinH.push_back(0); mMaxH.push_back(0);
	}
	if(!mMixChannels[S])
	{
		mMinS.push_back(0); mMaxS.push_back(0);
	}
	if(!mMixChannels[V])
	{
		mMinV.push_back(0); mMaxV.push_back(0);
	}

	if(!mMixChannels[Y])
	{
		mMinY.push_back(0); mMaxY.push_back(0);
	}
	if(!mMixChannels[Cr])
	{
		mMinCr.push_back(0); mMaxCr.push_back(0);
	}
	if(!mMixChannels[Cb])
	{
		mMinCb.push_back(0); mMaxCb.push_back(0);
	}
	
}

void SkinDetector::ErodeAndDilate(Mat& ThresholdFrame, Mat& OutputFrame, int ErodeSize, int DilateSize, int Shape)
{
	
	cv::erode(ThresholdFrame,OutputFrame,getStructuringElement(Shape,Size(ErodeSize,ErodeSize)));
	cv::dilate(OutputFrame,OutputFrame,getStructuringElement(Shape,Size(DilateSize,DilateSize)));

}

void SkinDetector::DilateAndErode(Mat& ThresholdFrame, Mat& OutputFrame, int ErodeSize, int DilateSize, int Shape)
{
	
	cv::dilate(OutputFrame,OutputFrame,getStructuringElement(Shape,Size(DilateSize,DilateSize)));
	cv::erode(ThresholdFrame,OutputFrame,getStructuringElement(Shape,Size(ErodeSize,ErodeSize)));
	
}


void SkinDetector::NewModel_InRange(cv::Mat& Color, double Min, double Max, cv::Mat& ColorBinary)
{
	Mat ColorInverse = Color.clone();
	NewModel_SingleColorThreshold(Color,(((Min-1)<0)?0:Min-1),255,Color,THRESH_BINARY); // setting lower Limits for the thresholds
	NewModel_SingleColorThreshold(ColorInverse,Max,255,ColorInverse,THRESH_BINARY_INV);
	bitwise_and(ColorInverse,Color,Color);
}

void SkinDetector::NewModel_SingleColorThreshold(cv::Mat& ColorThreshold, double Min, double Max, cv::Mat& ThresholdOutput, int Code)
{
	threshold(ColorThreshold,ThresholdOutput,Min,Max,Code);
}

void SkinDetector::Normalize(Mat& LiveFrame, char LightComponent,int MethodCode)
{
	Mat equalizerFrame;
	vector<Mat> channels(3);
	if(MethodCode == 1)
	{
		if(LightComponent == 'Y')
		{
			cv::cvtColor(LiveFrame,equalizerFrame,COLOR_BGR2YCrCb);
			split(equalizerFrame,channels);
			equalizeHist(channels[0], channels[0]);
			merge(channels,equalizerFrame);
			cv::cvtColor(equalizerFrame,LiveFrame,CV_YCrCb2BGR);
		}
		else if(LightComponent = 'V')
		{
			cv::cvtColor(LiveFrame,equalizerFrame,COLOR_BGR2HSV);
			split(equalizerFrame,channels);
			equalizeHist(channels[2], channels[2]);
			merge(channels,equalizerFrame);
			cv::cvtColor(equalizerFrame,LiveFrame,CV_HSV2BGR);
		}
	}
	else if(MethodCode == 2)
	{
		cv::cvtColor(LiveFrame, equalizerFrame, CV_BGR2Lab);
		cv::split(equalizerFrame, channels);  
		
		cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
		clahe->setClipLimit(4);
		cv::Mat dst;
		clahe->apply(channels[0], dst);
				
		dst.copyTo(channels[0]);
		merge(channels,equalizerFrame);
		cv::cvtColor(equalizerFrame, LiveFrame, CV_Lab2BGR);
	}
}

void SkinDetector::ResetLastSample()
{
	if(mModel == 3)
	{
		for(int i=0; i<mRoiRect.size(); i++)
		{
			if(mMixChannels[B] && mMinB.size() > 0)
			{
				mMinB.pop_back(); mMaxB.pop_back();
			}
			if(mMixChannels[G] && mMinG.size() > 0)
			{
				mMinG.pop_back(); mMaxG.pop_back();
			}
			if(mMixChannels[R] && mMinR.size() > 0)
			{
				mMinR.pop_back(); mMaxR.pop_back();
			}


			if(mMixChannels[H] && mMinH.size() > 0)
			{
				mMinH.pop_back(); mMaxH.pop_back();
			}
			if(mMixChannels[S] && mMinS.size() > 0)
			{
				mMinS.pop_back(); mMaxS.pop_back();
			}
			if(mMixChannels[V] && mMinV.size() > 0)
			{
				mMinV.pop_back(); mMaxV.pop_back();
			}


			if(mMixChannels[Y] && mMinY.size() > 0)
			{
				mMinY.pop_back(); mMaxY.pop_back();
			}
			if(mMixChannels[Cr] && mMinCr.size() > 0)
			{
				mMinCr.pop_back(); mMaxCr.pop_back();
			}
			if(mMixChannels[Cb] && mMinCb.size() > 0)
			{
				mMinCb.pop_back(); mMaxCb.pop_back();
			}
		}
	}
	else if(mModel == 2)//for reseting last HSV values
	{
		if(mMinH.size() <= 0)
			return;
		for(int i=0; i<mRoiRect.size(); i++)
		{
			mMinH.pop_back();
			mMaxH.pop_back();

			mMinS.pop_back();
			mMaxS.pop_back();

			mMinV.pop_back();
			mMaxV.pop_back();
		}
	}
	else if(mModel == 1)//for reseting last YCrCb values
	{
		if(mMinY.size() <= 0)
			return;
		for(int i=0; i<mRoiRect.size(); i++)
		{
		
			mMinY.pop_back();
			mMaxY.pop_back();

			mMinCr.pop_back();
			mMaxCr.pop_back();

			mMinCb.pop_back();
			mMaxCb.pop_back();
		}
	}
	
}

void SkinDetector::NewModel_ShowTrackBars()
{
	namedWindow("Trackbars");
	cv::resizeWindow("Trackbars",320,480);

	
	if(mMixChannels[B])
	{
		createTrackbar("Min R ","Trackbars",&mTrackMinR,255);
		createTrackbar("Max R ","Trackbars",&mTrackMaxR,255);
	}
	if(mMixChannels[G])
	{
		createTrackbar("Min G ","Trackbars",&mTrackMinG,255);
		createTrackbar("Max G ","Trackbars",&mTrackMaxG,255);
	}
	if(mMixChannels[R])
	{
		createTrackbar("Min B ","Trackbars",&mTrackMinB,255);
		createTrackbar("Max B ","Trackbars",&mTrackMaxB,255);
	}

	
	if(mMixChannels[H])
	{
		createTrackbar("Min H ","Trackbars",&mTrackMinH,179);
		createTrackbar("Max H ","Trackbars",&mTrackMaxH,179);			
	}
	if(mMixChannels[S])
	{
		createTrackbar("Min S ","Trackbars",&mTrackMinS,255);
		createTrackbar("Max S ","Trackbars",&mTrackMaxS,255);
	}
	if(mMixChannels[V])
	{
		createTrackbar("Min V ","Trackbars",&mTrackMinV,255);
		createTrackbar("Max V ","Trackbars",&mTrackMaxV,255);
	}


	if(mMixChannels[Y])
	{
		createTrackbar("Min Y ","Trackbars",&mTrackMinY,255);
		createTrackbar("Max Y ","Trackbars",&mTrackMaxY,255);
	}
	if(mMixChannels[Cr])
	{
		createTrackbar("Min Cr ","Trackbars",&mTrackMinCr,255);
		createTrackbar("Max Cr ","Trackbars",&mTrackMaxCr,255);
	}
	if(mMixChannels[Cb])
	{
		createTrackbar("Min Cb ","Trackbars",&mTrackMinCb,255);
		createTrackbar("Max Cb ","Trackbars",&mTrackMaxCb,255);
	}
			
}

void SkinDetector::NewModel_TrackBarThreshold(cv::Mat LiveFrame)
{
	Scalar lowerBound;
	Scalar higherBound;
	Mat HSV, YCrCb,threshold;
	
	
	if(mMixChannels[B] || mMixChannels[G] || mMixChannels[R])//Consider RGB Range only if any colour of RGB is included in the new model
	{
		lowerBound = Scalar(mTrackMinB, mTrackMinG, mTrackMinR);
		higherBound = Scalar(mTrackMaxB,mTrackMaxG,mTrackMaxR);
		inRange(LiveFrame,lowerBound,higherBound,threshold);
		
		//ErodeAndDilate(threshold,5,5);
	//	imshow("BGR threshold",threshold);

		mMultipleTrackThreshold.push_back(threshold);
	}
	
	
	
	if(mMixChannels[H] || mMixChannels[S] || mMixChannels[V])//Consider HSV Range only if any component of HSV is included in the new model
	{
		vector<Mat> HSVComponent;
		cvtColor(LiveFrame,HSV,COLOR_BGR2HSV);
		split(HSV,HSVComponent);
		if(mMixChannels[H])
		{
			NewModel_InRange(HSVComponent[0], mTrackMinH, mTrackMaxH, HSVComponent[0]);
			//ErodeAndDilate(HSVComponent[0],5,7);
			mMultipleTrackThreshold.push_back(HSVComponent[0]);
			//imshow("H",HSVComponent[0]);
		}
		if(mMixChannels[S])
		{
			NewModel_InRange(HSVComponent[1], mTrackMinS, mTrackMaxS, HSVComponent[1]);
			//ErodeAndDilate(HSVComponent[1],5,7);
			mMultipleTrackThreshold.push_back(HSVComponent[1]);
			//imshow("S",HSVComponent[1]);
		}
		if(mMixChannels[V])
		{
			NewModel_InRange(HSVComponent[2], mTrackMinV, mTrackMaxV, HSVComponent[2]);
			//ErodeAndDilate(HSVComponent[2],5,7);
			mMultipleTrackThreshold.push_back(HSVComponent[2]);
			//imshow("V",HSVComponent[2]);
		}

	}
	
	
	if(mMixChannels[Y] || mMixChannels[Cr] || mMixChannels[Cb])//Consider YCrCb Range only if any component of YCrCb is included in the new model
	{
		vector<Mat> YCrCbComponent;
		cvtColor(LiveFrame,YCrCb,COLOR_BGR2YCrCb);
		split(YCrCb,YCrCbComponent);
		if(mMixChannels[Y])
		{
			NewModel_InRange(YCrCbComponent[0], mTrackMinY, mTrackMaxY, YCrCbComponent[0]);
			//ErodeAndDilate(YCrCbComponent[0],5);
			mMultipleTrackThreshold.push_back(YCrCbComponent[0]);
			//imshow("Y",YCrCbComponent[0]);
		}
		if(mMixChannels[Cr])
		{
			NewModel_InRange(YCrCbComponent[1], mTrackMinCr, mTrackMaxCr, YCrCbComponent[1]);
			//ErodeAndDilate(YCrCbComponent[1],5);
			mMultipleTrackThreshold.push_back(YCrCbComponent[1]);
			//imshow("Cr",YCrCbComponent[1]);
		}
		if(mMixChannels[Cb])
		{
			NewModel_InRange(YCrCbComponent[2], mTrackMinCb, mTrackMaxCb, YCrCbComponent[2]);
			//ErodeAndDilate(YCrCbComponent[2],5);
			mMultipleTrackThreshold.push_back(YCrCbComponent[2]);
			//imshow("Cb",YCrCbComponent[2]);
		}

	}

	
	//cout<<"\nsize : "<<mMultipleThreshold.size();

	for(int i=1;i<mMultipleTrackThreshold.size();++i)
		bitwise_and(mMultipleTrackThreshold[0],mMultipleTrackThreshold[i],mMultipleTrackThreshold[0]);
	
	mTrackbarThreshold = mMultipleTrackThreshold[0];
	mMultipleTrackThreshold.clear();
}