#include <ctime>
#include <iostream>
#include <raspicam/raspicam_cv.h>
#include <signal.h>
using namespace std; 
using namespace cv;

int main ( int argc,char **argv ) 
{   
    raspicam::RaspiCam_Cv Camera;
    Mat image;
    char key;

    //set camera params
    Camera.set( CV_CAP_PROP_FORMAT, CV_8UC3 );
    
    // get video size. 512.6.0319 hiroshi
    Size videoSize = Size((int)Camera.get(CV_CAP_PROP_FRAME_WIDTH),
			  (int)Camera.get(CV_CAP_PROP_FRAME_HEIGHT));
    // declare video writer
    VideoWriter writer("out.avi", CV_FOURCC('M','J','P','G'), 25, videoSize);

    if (!Camera.open())
    	return 0;
    	
    while (1)
    {    
        Camera.grab();
        Camera.retrieve ( image);
	// 512.6.0319 hiroshi: write video file to "out.mjpg"
	writer<<image;
	// show video on screen
        imshow("test for raspicam-video-writing", image);
	key = waitKey(1);
    	if (char(key) == 27)    		
    	break;
    }

    Camera.release();
    destroyAllWindows();
    
    return 0;
}
