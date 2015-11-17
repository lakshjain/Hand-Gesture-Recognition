#pragma once

#include "opencv2\opencv.hpp"
#include <iostream>
#include <map>


class SkinDetector
{
	
	public:
			SkinDetector();
			void SelectModel();
			void CreateROI(int x, int y, int width, int height);
			void DeleteLastROI();
			void ShowROI(cv::Mat& LiveFrame);
			void TakeSample(cv::Mat LiveFrame);
			cv::Mat SkinThresholding(cv::Mat LiveFrame);
			cv::Mat TrackBarThreshold(cv::Mat LiveFrame);
			void ErodeAndDilate(cv::Mat& ThresholdFrame, cv::Mat& OutputFrame, int ErodeSize = 5, int DilateSize = 5, int Shape = cv::MORPH_RECT);
			void DilateAndErode(cv::Mat& ThresholdFrame, cv::Mat& OutputFrame, int ErodeSize = 5, int DilateSize = 5, int Shape = cv::MORPH_RECT);
			void ShowTrackbars();
			void Normalize(cv::Mat& LiveFrame, char LightComponent = 'Y', int MethodCode = 1);//1 for simple histogram normalization and 2 for Clahe adpative normalization method
			void ResetLastSample();

			

	
	private:

	void ShowChannels();

	void CalculateRange(cv::Mat ROI, int ModelOption);

	void NewModel_Range(cv::Mat ROI);

	void InitMinMaxValues();

	void NewModel_SingleColorThreshold(cv::Mat& ColorThreshold, double Min, double Max, cv::Mat& ThresholdOutput,int Code = cv::THRESH_BINARY);
	
	void NewModel_Thresholding(cv::Mat LiveFrame);

	void NewModel_CalculateMinMax(cv::Mat ColorROI, std::vector<double>& Min, std::vector<double>& Max);

	void NewModel_InRange(cv::Mat& Color, double Min, double Max, cv::Mat& ColorBinary);

	void NewModel_ShowTrackBars();

	void NewModel_TrackBarThreshold(cv::Mat LiveFrame);

	std::map<int,bool> mMixChannels;

	std::vector<double> 	mMinY, mMaxY,	mMinCr, mMaxCr,	mMinCb, mMaxCb; //for YCrCb Model
	
	std::vector<double> 	mMinH, mMaxH,	mMinS, mMaxS,	mMinV, mMaxV; //for HSV Model

	std::vector<double> 	mMinR, mMaxR,	mMinG, mMaxG,	mMinB, mMaxB; //for YCrCb Model

	cv::Mat mROI;

	std::vector<cv::Rect> mRoiRect;

	cv::Mat mThreshold, mTrackbarThreshold; //for various Thresholds
	std::vector<cv::Mat> mMultipleThreshold;
	std::vector<cv::Mat> mMultipleTrackThreshold;
	
	int mModel; //for Selecting a Particular Color model 
	int mMode;	// for Selecting Realtime or Debugging Mode
	
	int mTrackMinH, mTrackMaxH, mTrackMinS, mTrackMaxS, mTrackMinV, mTrackMaxV; //Values for Trackbars for HSV Model
	int mTrackMinY, mTrackMaxY, mTrackMinCr, mTrackMaxCr, mTrackMinCb, mTrackMaxCb;//Value For Trackbars for YCrCb Model
	int mTrackMinB, mTrackMinG, mTrackMinR, mTrackMaxB, mTrackMaxG, mTrackMaxR; //Values for Trackbars for BGR Model
		
};