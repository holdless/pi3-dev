#+++++++++++++++++++++++
#raspicamFaceDetection/opencv:
#+++++++++++++++++++++++
cd 
cd /home/pi/Desktop/tango/facerec/facedet/raspicam
g++ `pkg-config --cflags opencv` -o raspicamFaceDetection raspicamFaceDetection.cpp `pkg-config --libs opencv` -L/opt/vc/lib/ -lraspicam -lraspicam_cv -lmmal -lmmal_core -lmmal_util -lopencv_core -lopencv_highgui
