#+++++++++++++++++++++++
#usbcamFaceDetection/opencv:
#+++++++++++++++++++++++
cd
cd /home/pi/Desktop/tango/facerec/facedet/usbcam
g++ `pkg-config --cflags --libs opencv` usbcamFaceDetection.cpp -o usbcamFaceDetection
