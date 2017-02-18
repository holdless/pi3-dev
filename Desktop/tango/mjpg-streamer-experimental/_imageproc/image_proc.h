#include "opencv2/core/core.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <time.h>
#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

void detectFace(Mat);
bool initFaceDetection();
