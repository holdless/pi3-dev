#!/usr/bin/env python3

# NOTE: this example requires PyAudio because it uses the Microphone class

import speech_recognition as sr
#pyttsx (espeak)
import pyttsx
# google translate tts
import json
import requests
import urllib
import subprocess
import argparse
import pycurl
import StringIO
import os.path
# for waitkey
import cv2

# tts, google translate tts
def callSubprocess(app, a, b, c, d, url):
    subprocess.call([app, a, b, c, d, tts_url], shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE )

#stt
# obtain audio from the microphone
r = sr.Recognizer()
engine = pyttsx.init()
engine.setProperty('rate', 150)

while 1:
    with sr.Microphone() as source:
        print("Say something!")
        audio = r.listen(source)
    # recognize speech using Google Speech Recognition
    try:
        # for testing purposes, we're just using the default API key
        # to use another API key, use `r.recognize_google(audio, key="GOOGLE_SPEECH_RECOGNITION_API_KEY")`
        # instead of `r.recognize_google(audio)`
        stt_string = r.recognize_google(audio);
        print("YOU: " + stt_string)

        if 'introduce' in stt_string or 'Introduce' in stt_string:
            response_string = 'hi, my name is Tango, Tango-Ali-Monet. It is nice to meet you in this lovely morning Sir!'
        elif 'happy birthday' in stt_string or 'Happy Birthday' in stt_string:
            response_string = 'happy birthday my dear Frances, wish you all the joys and beauty! and hello little baby, I, Sir Tango-Ali-Monet loves you! kiss you, muah! ju!'
        elif 'good job' in stt_string or 'well done' in stt_string or 'thank you tango' in stt_string:
            response_string = 'My pleasure sir. Have a nice day sir. And remind, you still have, 3, working items on to-do list today'
        else:
            response_string = 'I do not understand what you said'

        print("Tango: " + response_string)
        # tts, espeak
        engine.say(response_string)
        engine.runAndWait()

    #    tts_url = "http://translate.google.com/translate_tts?tl=en&client=tw-ob&q="+response_string
    #    subprocess.call(["mplayer", "-ao", "alsa", "-really-quiet", "-noconsolecontrols", tts_url], shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE )

    except sr.UnknownValueError:
        print("Google Speech Recognition could not understand audio")
    except sr.RequestError as e:
        print("Could not request results from Google Speech Recognition service; {0}".format(e))

    k = cv2.waitKey(1000)
    if k == 27:
        break
