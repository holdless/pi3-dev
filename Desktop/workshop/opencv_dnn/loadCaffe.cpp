#include "IhsorihGoogLeNetObjectRecognizer.h"
#include <iostream>
#include <time.h>

using namespace std;

#define _MILLIONTH 1E-9

int main(int argc, char **argv) {
	// load image
	cv::Mat frame = imread("space_shuttle.jpg");
/*	cv::Mat frame;
	cv::VideoCapture cap("D:/Users/changyht/Videos/Google - Deep Learning/L2 Deep Neural Networks Videos/01 - Intro to Lesson 2.mp4");

	if (!cap.isOpened()) {
		std::cout << "Cannot open the video file on C++ API" << std::endl;
		return -1;
	}

	double fps = cap.get(CV_CAP_PROP_FPS); // get the frame per second of the video
	cout << "Frame per second: " << fps << endl;
*/

	// create obj-rec object
	IhsorihGoogLeNetObjectRecognizer objRec;
	objRec.init("bvlc_googlenet.prototxt",
		    "bvlc_googlenet.caffemodel",
		    "synset_words.txt");

	struct timespec t1, t2;
	clock_gettime(CLOCK_REALTIME, &t1);
	objRec.setImage(frame);
	objRec.predict();
	clock_gettime(CLOCK_REALTIME, &t2);
	objRec.putProbBar(frame);
	double lapseTime = (t2.tv_sec - t1.tv_sec) + (t2.tv_nsec - t1.tv_nsec)*_MILLIONTH;
	cout << endl << "predicting time: " << lapseTime << endl;

	cv::imshow("etst", frame);
	if (waitKey(0) == 27) //wait for 'esc' key press for 30 ms. If 'esc' key is pressed, break loop
	{
	    cout << "esc key is pressed by user" << endl;
	    //break;
	}

/*
	int k = 0;
	for (;;) {
		if (!cap.read(frame)) {
			cout << "Cannot read the frame from video file" << endl;
			break;
		}
		// do object recognition
		if (k % 20 == 19) {
			objRec.setImage(frame);
			objRec.predict();
			objRec.putProbBar(frame);
			k = 0;
		}
		else
			k++;

		cv::imshow("etst", frame);
		if (waitKey(30) == 27) //wait for 'esc' key press for 30 ms. If 'esc' key is pressed, break loop
		{
			cout << "esc key is pressed by user" << endl;
			break;
		}
	}
*/
	return 0;
}
