# dlib
cd
cd Downloads
git clone https://github.com/davisking/dlib.git

cd /home/pi/Downloads/dlib/examples
mkdir build
cd build
sudo cmake -DCMAKE_BUILD_TYPE=Release -G "CodeBlocks - Unix Makefiles" ..
#sudo cmake --build . --config Release 
sudo make -j4

# cmt
cd
cd Downloads
git clone https://github.com/gnebehay/CppMT.git
cd /home/pi/Downloads/CppMT
mkdir build
cd build
cmake ..
make -j4