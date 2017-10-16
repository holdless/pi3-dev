import json
import requests
import urllib
import subprocess
import argparse
import pycurl
import StringIO
import os.path

#import sys
#sys.path.append("/home/pi/Desktop/python_test/")

def callSubprocess(app, a, b, c, d, url):
    subprocess.call([app, a, b, c, d, url], shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE )
def call(speech_string):
    url = 'http://translate.google.com/translate_tts?tl=en&client=tw-ob&q='
    subprocess.call(["mplayer", "-ao", "alsa", "-really-quiet", "-noconsolecontrols", url+speech_string], shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE )
    return 'i am great'
#callSubprocess("mplayer", "-ao", "alsa", "-really-quiet", "-noconsolecontrols", "http://translate.google.com/translate_tts?tl=en&client=tw-ob&q=i am tango")

#tts_url = "http://translate.google.com/translate_tts?tl=en&client=tw-ob&q=i am tango"
#subprocess.call(["mplayer", "-ao", "alsa", "-really-quiet", "-noconsolecontrols", tts_url], shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE )
