#+++++++++++++++++++++++
#eigenFacerec/opencv:
#+++++++++++++++++++++++
cd
cd /home/pi/Desktop/tango/facerec
g++ `pkg-config --cflags opencv` -o eigenFacerec eigenFacerec.cpp `pkg-config --libs opencv` -L/opt/vc/lib/ -lraspicam -lraspicam_cv -lmmal -lmmal_core -lmmal_util -lopencv_core -lopencv_highgui
