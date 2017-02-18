sudo apt-get update
sudo apt-get upgrade

sudo apt-get install build-essential cmake cmake-gui pkg-config
sudo apt-get install libjpeg-dev libtiff5-dev libjasper-dev libpng12-dev
sudo apt-get install libavcodec-dev libavformat-dev libswscale-dev libv4l-dev
sudo apt-get install libxvidcore-dev libx264-dev
sudo apt-get install libgtk2.0-dev
sudo apt-get install libatlas-base-dev gfortran
sudo apt-get install libblas-dev liblapacke-dev libopenblas-dev
sudo apt-get install python python2.7-dev python3-dev python-numpy

sudo apt-get update
sudo apt-get upgrade

cd ~
cd Downloads
mkdir opencv
cd opencv
git clone https://github.com/opencv/opencv.git
git clone https://github.com/opencv/opencv_contrib.git
#wget -O opencv.zip https://github.com/Itseez/opencv/archive/3.2.0.zip
#unzip opencv.zip
#wget -O opencv_contrib.zip https://github.com/Itseez/opencv_contrib/archive/3.2.0.zip
#unzip opencv_contrib.zip


cd ~
wget https://bootstrap.pypa.io/get-pip.py
sudo python get-pip.py

sudo apt-get update
sudo apt-get upgrade

pip install numpy

cd Downloads/opencv
mkdir build
cd build

# >>> use cmake-gui to build and generate

###sudo make -j4
###make install