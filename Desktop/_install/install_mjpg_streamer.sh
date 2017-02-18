cd /usr/src
# download from git
sudo git clone https://github.com/jacksonliam/mjpg-streamer.git
#sudo apt-get install libv4l-dev imagemagick build-essential cmake subversion

cd mjpg-streamer/mjpg-streamer-experimental

# revise CMakeLists.txt: 
sudo nano CMakeLists.txt
# mark down "add_subdirectory(plugins/input_opencv)"

sudo mkdir _build
cd _build
sudo cmake ..
# for release mode
#sudo make
# for debug mode
sudo make CMAKE_BUILD_TYPE=Debug
sudo make install

# move .exe and input/output.so file to mjpg-streamer-experimental directory
# cd to mjpg-streamer-experimental folder...
cd ..
sudo cp _build/mjpg_streamer mjpg_streamer
#sudo cp _build/plugins/input_file/input_file.so input_file.so
#sudo cp _build/plugins/input_http/input_http.so input_http.so
#sudo cp _build/plugins/input_ptp2/input_ptp2.so input_ptp2.so
sudo cp _build/plugins/input_raspicam/input_raspicam.so input_raspicam.so
sudo cp _build/plugins/input_uvc/input_uvc.so input_uvc.so
#sudo cp _build/plugins/output_file/output_file.so output_file.so
sudo cp _build/plugins/output_http/output_http.so output_http.so
#sudo cp _build/plugins/output_rtsp/output_rtsp.so output_rtsp.so
#sudo cp _build/plugins/output_udp/output_udp.so output_udp.so
#sudo cp _build/plugins/output_viewer/output_viewer.so output_viewer.so