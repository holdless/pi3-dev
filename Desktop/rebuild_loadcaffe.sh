cd /home/pi/Desktop/workshop/opencv_dnn
sudo rm -rf build
sudo mkdir build
cd build

#sudo cmake -DCMAKE_BUILD_TYPE=Release -G "CodeBlocks - Unix Makefiles" ..
sudo cmake -DCMAKE_BUILD_TYPE=Release ..

sudo make

cd ..
sudo chmod 777 build
sudo cp build/loadCaffe loadCaffe

cd /home/pi/Desktop/workshop/opencv_dnn
