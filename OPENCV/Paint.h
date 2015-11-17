#include "opencv2\opencv.hpp"

class Paint
{
	private:
		cv::Mat mDrawArea;


	protected : 
		void Draw(int x, int y);

	public	:
		void InitPaint();
		void ClosePaint();
		void Erase();

};