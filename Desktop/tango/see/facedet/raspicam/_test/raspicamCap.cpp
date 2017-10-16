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

    if (!Camera.open())
    	return 0;
    	
    while (1)
    {    
        Camera.grab();
        Camera.retrieve ( image);
        imshow("test", image);
	key = waitKey(1);
    	if (char(key) == 27)    		
    	break;
    }

    Camera.release();
    destroyAllWindows();
    
    return 0;
}
