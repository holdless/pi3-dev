#!/bin/bash
cd /home/pi/Desktop/tango/mjpg-streamer-experimental
#for raspicam:
./mjpg_streamer -o "output_http.so -w ./www" -i "input_raspicam.so -x 320 -y 240 -quality 60"
#for usb-cam:
#./mjpg_streamer -o "output_http.so -w ./www" -i "input_uvc.so -quality 60"
cd
cd Desktop