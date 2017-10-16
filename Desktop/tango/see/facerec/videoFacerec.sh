# build and run "videoFacerec.cpp"
# 547.7.1122 time-zone-id: 1/3

#path=/home/pi/tango/facerec/
#cd $path

# build the .cpp file
g++ `pkg-config --cflags opencv` -o videoFacerec videoFacerec.cpp `pkg-config --libs opencv` -L/opt/vc/lib/ -lraspicam -lraspicam_cv -lmmal -lmmal_core -lmmal_util -lopencv_core -lopencv_highgui

# user input the related database file (.csv)
read -p "please input DB.csv file name: " csv_name

# run the .cpp file
./videoFacerec $csv_name
