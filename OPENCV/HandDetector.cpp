#include "HandDetector.h"

#define MINAREA	10000
HandDetector::HandDetector()
{
	mObjectTrackFlag = false;
}
void HandDetector::FindConvexHull(Mat& LiveFrame, Mat SkinThreshold)
{
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	Point2f COM; //Center of Mass
	int TiltAngle = 0;
			
	int LargestArea = 0;
	int LargestAreaIndex = -1;

	cv::findContours(SkinThreshold,contours,hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	vector<vector<Point>> hull(contours.size());

	vector<vector<int> > inthull(contours.size());
	vector<vector<Vec4i> > defects(contours.size()); 

	
	for(int i=0;i<contours.size();++i)
	{
		convexHull(Mat(contours[i]),hull[i],false);
		convexHull(Mat(contours[i]), inthull[i], false);
		if (inthull[i].size()>3)
			convexityDefects(contours[i], inthull[i], defects[i]);

	}

	
	for(int i=0;i<contours.size();++i)
	{
		double area = cv::contourArea(contours[i],false);
		if(area > LargestArea)
			
			{
				LargestArea = area;
				LargestAreaIndex = i;
			}
	}
	if(contours.size() > 0 && LargestArea > 10000 && LargestArea < 90000 && !mObjectTrackFlag)
	{
		//cv::drawContours(frame,contours,id,CV_RGB(0,255,0),2,8,hierarchy);
		drawContours(LiveFrame, hull ,LargestAreaIndex, CV_RGB(0, 255, 0), 2, 8, hierarchy);
		drawContours(LiveFrame, contours, LargestAreaIndex, CV_RGB(0, 0, 255), 2, 8, hierarchy);
		//kCurvature(contours,hull,inthull,LiveFrame);
		//cout<<"\nlargest Area : "<<largest_area;
		Moments moment;
		moment = moments( contours[LargestAreaIndex], false ); 
		
		///  Get the mass centers:
		COM = Point2f( moment.m10/moment.m00 , moment.m01/moment.m00 ); 
		circle(LiveFrame,COM,4,Scalar(255,0,0),2);
		//TiltAngle = CalculateHandTilt(moment.m11,moment.m20,moment.m02);
		//cout<<"\n Tilt Angle : "<<TiltAngle;
		//cout<<GetOrientation(contours[LargestAreaIndex],LiveFrame)<<"\n";
		//cout<<"\n"<<COM.x<<" "<<COM.y;
		FindConvexityDefects(defects[LargestAreaIndex], contours[LargestAreaIndex], LiveFrame,COM, TiltAngle);

		
	}
	else if(contours.size() > 0 && LargestArea > 10 && LargestArea < 30000 && mObjectTrackFlag)
	{
		drawContours(LiveFrame, hull ,LargestAreaIndex, CV_RGB(0, 255, 0), 2, 8, hierarchy);
		drawContours(LiveFrame, contours, LargestAreaIndex, CV_RGB(0, 0, 255), 2, 8, hierarchy);
		Moments moment;
		moment = moments( contours[LargestAreaIndex], false ); 
		
		///  Get the mass centers:
		COM = Point2f( moment.m10/moment.m00 , moment.m01/moment.m00 ); 
		circle(LiveFrame,COM,4,Scalar(255,0,0),2);
		Draw(COM.x,COM.y);
		
	}
	else if(mSwipeON)
	{
		mSwipeGesture.Reset();
	}
}

void HandDetector::FindConvexityDefects(vector<Vec4i> ConvexityDefectSet, vector<Point> HandContour, Mat& LiveFrame, Point2f COM, int TiltAngle)
{
	vector<Point> FingerTips;
	
	Point2f HandCenter;
	float radius;
	int fingers=0;

	//minEnclosingCircle(HandContour,HandCenter,radius);
	//Rect contourRect = boundingRect(HandContour);
	//circle(LiveFrame,HandCenter,10,CV_RGB(0,0,255),2,8);
	//rectangle(LiveFrame, contourRect, Scalar(0,0,255), 3);
	//vector<Point> palmCircle;
	for (int i = 0; i < ConvexityDefectSet.size(); i++) 
	{

		int startId = ConvexityDefectSet[i].val[0];
		Point ptStart(HandContour[startId]);

		int endId = ConvexityDefectSet[i].val[1]; 
		Point ptEnd(HandContour[endId]);

		int farId = ConvexityDefectSet[i].val[2]; 
		Point ptFar(HandContour[farId]);

		double depth = (double)((ConvexityDefectSet[i].val[3]) / 256); //for getting real pixel values

		//cout << "depth" << depth << endl;
		//display start points
		//circle(original,ptStart,5,CV_RGB(255,0,0),2,8);
		//display all end points
		//circle(original, ptEnd, 5, CV_RGB(255, 255, 0), 2, 8);
		//display all far points
		//circle(LiveFrame,ptFar,5,CV_RGB(255,255,0),2,8);

		//circle(LiveFrame, HandCenter, radius, cv::Scalar(0,255,255),2);
		if (depth > 10 && ptStart.y < COM.y)//
		{
			circle(LiveFrame,ptStart,5,CV_RGB(255,0,0),2,8);
			//circle(LiveFrame, ptStart, 4, CV_RGB(255, 0,0), 4);
			FingerTips.push_back(ptStart);
			fingers++;
			//palmCircle.push_back(ptStart);
						
		}
	}
		
	if(mMouseControl)
	{
		InitMouseControl(LiveFrame,FingerTips,COM,TiltAngle);
	}
	if(mSwipeON)
	{
		Point tempCOM;
		tempCOM.x = COM.x;
		tempCOM.y = COM.y;
		mSwipeGesture.SwipeInit(tempCOM, LiveFrame);
	}

	//cout<<"Fingers = "<<fingers<<endl;
}

int HandDetector::CalculateHandTilt(double m11, double m20, double m02)
{
	double diff = m20 - m02;
	cout<<"\nm20 : "<<m20<<" m02 : "<<m02;
	 if (diff == 0) 
	 {
		if (m11 == 0)
			return 0;
		else if (m11 > 0)
			return -45;
		else   // m11 < 0
			return 45;
	 }

	  double theta = 0.5 * atan2(2*m11, diff);
	  int tilt = (int) ((theta)*180/CV_PI);
	  //return tilt;
	//  cout<<"\n"<<tilt;
	  if ((diff > 0) && (m11 == 0))
		return 0;
	  else if ((diff < 0) && (m11 == 0))
		return -90;
	  else if ((diff > 0) && (m11 > 0))  // 0 to 45 degrees
		return tilt;
	  else if ((diff > 0) && (m11 < 0))  // -45 to 0
		return (180 + tilt);   // change to counter-clockwise angle
	  else if ((diff < 0) && (m11 > 0))   // 45 to 90
		return tilt;
	  else if ((diff < 0) && (m11 < 0))   // -90 to -45
		return (180 + tilt);  // change to counter-clockwise angle
    
	return 0;

}

void HandDetector::DrawAxis(Mat& img, Point p, Point q, Scalar colour, const float scale )
{
    double angle;
    double hypotenuse;
    angle = atan2( (double) p.y - q.y, (double) p.x - q.x ); // angle in radians
    hypotenuse = sqrt( (double) (p.y - q.y) * (p.y - q.y) + (p.x - q.x) * (p.x - q.x));
//    double degrees = angle * 180 / CV_PI; // convert radians to degrees (0-180 range)
//    cout << "Degrees: " << abs(degrees - 180) << endl; // angle in 0-360 degrees range
    // Here we lengthen the arrow by a factor of scale
    q.x = (int) (p.x - scale * hypotenuse * cos(angle));
    q.y = (int) (p.y - scale * hypotenuse * sin(angle));
    line(img, p, q, colour, 1, CV_AA);
    // create the arrow hooks
    p.x = (int) (q.x + 9 * cos(angle + CV_PI / 4));
    p.y = (int) (q.y + 9 * sin(angle + CV_PI / 4));
    line(img, p, q, colour, 1, CV_AA);
    p.x = (int) (q.x + 9 * cos(angle - CV_PI / 4));
    p.y = (int) (q.y + 9 * sin(angle - CV_PI / 4));
    line(img, p, q, colour, 1, CV_AA);
}
double HandDetector::GetOrientation(const vector<Point> &pts, Mat &img)
{
    if (pts.size() == 0) return false;

    //Construct a buffer used by the pca analysis
    Mat data_pts = Mat(pts.size(), 2, CV_64FC1);
    for (int i = 0; i < data_pts.rows; ++i)
    {
        data_pts.at<double>(i, 0) = pts[i].x;
        data_pts.at<double>(i, 1) = pts[i].y;
    }


    //Perform PCA analysis
    PCA pca_analysis(data_pts, Mat(), CV_PCA_DATA_AS_ROW);

    //Store the position of the object
    Point pos = Point(pca_analysis.mean.at<double>(0, 0),
                      pca_analysis.mean.at<double>(0, 1));

    //Store the eigenvalues and eigenvectors
    vector<Point2d> eigen_vecs(2);
    vector<double> eigen_val(2);
    for (int i = 0; i < 2; ++i)
    {
        eigen_vecs[i] = Point2d(pca_analysis.eigenvectors.at<double>(i, 0),
                                pca_analysis.eigenvectors.at<double>(i, 1));

        eigen_val[i] = pca_analysis.eigenvalues.at<double>(i);
    }

    // Draw the principal components
    circle(img, pos, 3, CV_RGB(255, 0, 255), 2);
    //line(img, pos, pos + 0.02 * Point(eigen_vecs[0].x * eigen_val[0], eigen_vecs[0].y * eigen_val[0]) , CV_RGB(255, 255, 0));
    //line(img, pos, pos + 0.02 * Point(eigen_vecs[1].x * eigen_val[1], eigen_vecs[1].y * eigen_val[1]) , CV_RGB(0, 255, 255));

	Point p1 = pos + 0.02 * Point(eigen_vecs[0].x * eigen_val[0], eigen_vecs[0].y * eigen_val[0]);
    Point p2 = pos + 0.02 * Point(eigen_vecs[1].x * eigen_val[1], eigen_vecs[1].y * eigen_val[1]);
    

	DrawAxis(img, pos, p1, Scalar(0, 255, 0), 1);
    DrawAxis(img, pos, p2, Scalar(255, 255, 0), 5);


    return atan2(eigen_vecs[0].y, eigen_vecs[0].x)*(180/CV_PI);

}


/*
void HandDetector::kCurvature(vector<vector<Point>> contours, vector<vector<Point>> hull,vector< vector<int> >inthull, Mat& LiveFrame)
{
	//Canny(img_thresh, img_canny, 60, 110);

	/////////////////////net getting hull points///////////////////////////////////////
		Vector< vector<Point> > fhull, finhull;
		int fhullIndex=-1;
		Point p1, p2, hullP;
		double th1, th2, th;
		int i, j, k,Kfactor = 30;

		for(i=0; i < contours.size(); i++)
		{
			if (contours[i].size() < 50)
			{
				continue;
			}
			fhull.push_back(vector<Point>());
			finhull.push_back(vector<Point>());
			fhullIndex++;
			cout<<"\n c "<<contours[i].size();
			for(j=0; j< inthull[i].size(); j++)
			{
				hullP = contours[i][inthull[i][j]];
				p1 = contours[i][(inthull[i][j] - Kfactor)%inthull[i].size()];
				p2 = contours[i][(inthull[i][j] + Kfactor)%inthull[i].size()];

				th1 = abs(atan2((float)(hullP.y - p1.y),(float)(hullP.x - p1.x))*180/CV_PI);
				th2 = abs(atan2((float)(hullP.y - p2.y),(float)(hullP.x - p2.x))*180/CV_PI);
				
				th = abs(th1 - th2);
				if (th < 60 && th > 20)
				{
					cout<<th<<"\n";
					finhull[fhullIndex].push_back(hullP);
				}
			}
		}

		


		///finding exact position of fingers

		vector< vector<Point> >nfhull;
		int chullx, chully, add_flag=1, nhullindex=-1;

		for (i=0; i<finhull.size(); i++)
		{
			nfhull.push_back(vector<Point>());
			for(j=0; j<finhull[i].size(); j++)
			{
				chullx = finhull[i][j].x;
				chully = finhull[i][j].y;
				add_flag = 1;
				for (k=0; k<nfhull[i].size(); k++)
				{
					if (nfhull[i][k].x > chullx - Kfactor - 1 && nfhull[i][k].x < chullx + Kfactor - 1
						&& nfhull[i][k].y > chully - Kfactor - 1 && nfhull[i][k].y < chully + Kfactor - 1)
					{
						add_flag = 0;
						break;
					}
				}
				if (add_flag)
				{
					nfhull[i].push_back(Point(chullx, chully));
				}
			}
		}

		
		for (i=0; i<nfhull.size(); i++)
		{
			for(j=0; j<nfhull[i].size(); j++)
			{
				circle(LiveFrame, nfhull[i][j], 15, (255, 0, 255), 1, 8);
			}
		}

		

		for (i=0; i<finhull.size(); i++)
		{
			for(j=0; j<finhull[i].size(); j++)
			{
				circle(LiveFrame, finhull[i][j], 5, (0, 0, 255), 1, 8);
			}
		}

	
}
*/
