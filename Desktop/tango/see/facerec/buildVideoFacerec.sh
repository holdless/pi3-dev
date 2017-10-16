# build "videoFacerec.cpp"
# 547.8.1123 time-zone-id: 3/3

# build the .cpp file
g++ `pkg-config --cflags opencv` -o videoFacerec videoFacerec.cpp `pkg-config --libs opencv` -L/opt/vc/lib/ -lraspicam -lraspicam_cv -lmmal -lmmal_core -lmmal_util -lopencv_core -lopencv_highgui
