#include "opencv2/highgui/highgui.hpp"
//#include <iostream>

using namespace cv;

int main()
{
  Mat image;
  char key;
  
  VideoCapture cap;
  cap.open("http://localhost:8080/?action=stream?dummy=param.mjpg");
  
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