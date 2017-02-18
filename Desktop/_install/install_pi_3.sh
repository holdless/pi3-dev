#--------------------
# step.3: apt-get/pip 
#--------------------
# <codeblocks>
sudo apt-get install codeblocks

# <tight vnc>
# sudo apt-get install tightvncserver

# <pi-camera: for OpenCV(python) to identify raspicam> 
#sudo apt-get install python-picamera

# <python math-lib> 
sudo apt-get install python-numpy

# <i2c, telnet, kate> 
sudo apt-get install i2c-tools telnet kate

# <audio>
sudo apt-get install libasound2-dev
sudo apt-get install espeak bison python-dev swig               

# <google speech API related>
sudo apt-get install flac python-pycurl

# <SpeechRecognition /python & related>
sudo apt-get install portaudio19-dev
sudo apt-get install python-all-dev
sudo apt-get install python-pyaudio
sudo pip install SpeechRecognition 
# //(To quickly try it out, run python -m speech_recognition after installing)
# //<fix bug: SpeechRecognition can’t work>
# //(this problem is due to in the new version of SpeechRecognition3.4.x, it requires new version of PyAudio (0.2.9), 
# //and if you only use: sudo pip pyaudio, it’ll tell you “Requirement already satisfied…”, so we need to add following “ --upgrade")
sudo pip install pyaudio --upgrade

# <pyttsx (espeak tts API) /python>
sudo pip install pyttsx

# <google translate tts API related>
sudo apt-get install mplayer

# <pyhton Wifi module>
sudo pip install wifi

reboot