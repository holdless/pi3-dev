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

#stt
# obtain audio from the microphone
r = sr.Recognizer()
with sr.Microphone() as source:
    print("Say something!")
    audio = r.listen(source)

# recognize speech using Google Speech Recognition
try:
    # for testing purposes, we're just using the default API key
    # to use another API key, use `r.recognize_google(audio, key="GOOGLE_SPEECH_RECOGNITION_API_KEY")`
    # instead of `r.recognize_google(audio)`
    stt_string = r.recognize_google(audio);
    print("Google Speech Recognition thinks you said: " + stt_string)
except sr.UnknownValueError:
    print("Google Speech Recognition could not understand audio")
except sr.RequestError as e:
    print("Could not request results from Google Speech Recognition service; {0}".format(e))

# tts, espeak
engine = pyttsx.init()
engine.setProperty('rate', 175)
engine.say(stt_string)
engine.runAndWait()

# tts, google translate tts
def callSubprocess(app, a, b, c, d, url):
    subprocess.call([app, a, b, c, d, tts_url], shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE )
    
tts_url = "http://translate.google.com/translate_tts?tl=en&client=tw-ob&q="+stt_string
subprocess.call(["mplayer", "-ao", "alsa", "-really-quiet", "-noconsolecontrols", tts_url], shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE )
