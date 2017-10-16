#include "opencv2/highgui/highgui.hpp"

using namespace cv;

int main()
{
  Mat image;
  char key;
  
  VideoCapture cap;
  cap.open(0);
  
  while (cap.isOpened())
  {
    cap>>image;
    
    if (!image.empty())
      imshow("webcam", image);
    
    key = waitKey(33);
    if (char(key) == 27)
    	break;
  }
  
  cap.release();
  destroyAllWindows();
  
  
  return 0;
}