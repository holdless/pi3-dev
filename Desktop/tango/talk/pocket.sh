# execute pocketsphinx_continuous command
# 551.6.1219

path=/home/pi/
cd $path
pocketsphinx_continuous -hmm /usr/local/share/pocketsphinx/model/en-us/en-us -lm /home/pi/Desktop/7717.lm -dict /home/pi/Desktop/7717.dic -samprate 16000/8000/48000 -inmic yes

