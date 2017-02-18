cd /home/pi/Downloads/dlib/examples
sudo rm -rf build
sudo mkdir build
cd build

# -DCMAKE_BUILD_TYPE=Release is not necessarily needed..
sudo cmake -DCMAKE_BUILD_TYPE=Release -G "CodeBlocks - Unix Makefiles" ..
#sudo cmake ..

# on linux/unix make, --config has no function. it is redundant
sudo cmake --build . --config Release 
