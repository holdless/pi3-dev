#include <time.h>
#include <iostream>
#include <stdio.h>
#include <raspicam/raspicam_cv.h>
#include "opencv2/core/core.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;
using namespace cv;

// function declaration
void detectFace(Mat);

// global variables
String face_cascade_name = "/usr/share/opencv/lbpcascades/lbpcascade_frontalface.xml";
String eyes_cascade_name = "/usr/share/opencv/haarcascades/haarcascade_eye_tree_eyeglasses.xml";
CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;
string window_name = "FaceDetection";

int main ( int argc,char **argv ) 
{
	//-- 1. Load the cascades
	if( !face_cascade.load( face_cascade_name ) ){ printf("--(!)Error loading\n"); return -1; };
	if( !eyes_cascade.load( eyes_cascade_name ) ){ printf("--(!)Error loading\n"); return -1; };
     
    raspicam::RaspiCam_Cv Camera;
    //  raspicam setup
    Camera.set(CV_CAP_PROP_FRAME_WIDTH, 320);
    Camera.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
//    Camera.set(CV_CAP_PROP_FORMAT, CV_8UC3);
    
    cv::Mat image;
    char key;

    //Open camera
    if (!Camera.open())
		return 0;
      
      
    while (Camera.isOpened()) 
    {
        Camera.grab();
        Camera.retrieve (image);
        flip(image, image, 0);
        // face detection
        detectFace(image);
        // show window
        imshow(window_name, image);
	
		key = waitKey(1);
		if (char(key) == 27)
			break;
    }

    Camera.release();
    destroyAllWindows();
}


void detectFace(Mat frame)
{
	std::vector<Rect> faces;
	Mat frame_gray;

	cvtColor( frame, frame_gray, CV_BGR2GRAY );
	equalizeHist( frame_gray, frame_gray );

	//-- Detect faces
	face_cascade.detectMultiScale( frame_gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30) );

	for( size_t i = 0; i < faces.size(); i++ )
	{
		Point center( faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5 );
		ellipse( frame, center, Size( faces[i].width*0.5, faces[i].height*0.5), 0, 0, 360, Scalar( 255, 0, 255 ), 4, 8, 0 );

		Mat faceROI = frame_gray( faces[i] );
		std::vector<Rect> eyes;

		//-- In each face, detect eyes
		eyes_cascade.detectMultiScale( faceROI, eyes, 1.1, 3, 0 |CV_HAAR_SCALE_IMAGE, Size(30, 30) );
/*
		for( size_t j = 0; j < eyes.size(); j++ )
		{
			Point center( faces[i].x + eyes[j].x + eyes[j].width*0.5, faces[i].y + eyes[j].y + eyes[j].height*0.5 );
			int radius = cvRound( (eyes[j].width + eyes[j].height)*0.25 );
			circle( frame, center, radius, Scalar( 255, 0, 0 ), 4, 8, 0 );
		}
*/     
	}
}

