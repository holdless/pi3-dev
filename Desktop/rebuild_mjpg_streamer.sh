cd /home/pi/Desktop/tango/mjpg-streamer-experimental
sudo rm -rf _build
sudo mkdir _build
cd _build

# release mode
#sudo cmake -DCMAKE_BUILD_TYPE=Release ..

# debug mode + CodeBlocks IDE
#sudo cmake .. -DCMAKE_BUILD_TYPE=Debug -G "CodeBlocks - Unix Makefiles"

# release mode + CodeBlocks IDE
sudo cmake .. -DCMAKE_BUILD_TYPE=Release -G "CodeBlocks - Unix Makefiles"

sudo make
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

cd
cd Desktop