import subprocess

mpath = '/home/pi/Downloads/test/mjpg-streamer/mjpg-streamer-experimental/'
subprocess.call([	mpath+"mjpg_streamer",
			"-o", mpath+"output_http.so -w ./www", 
			"-i", mpath+"input_raspicam.so -x 320 -y 240 -quality 60"])
#subprocess.call(["node", "/home/pi/Desktop/tango/remote/_webServer_tcpClient"])
#subprocess.call(["ls", "-l"])
