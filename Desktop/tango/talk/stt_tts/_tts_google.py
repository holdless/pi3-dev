import json
import requests
import urllib
import subprocess
import argparse
import pycurl
import StringIO
import os.path

def callSubprocess(app, a, b, c, d, url):
    subprocess.call([app, a, b, c, d, tts_url], shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE )
    
tts_url = "http://translate.google.com/translate_tts?tl=en&client=tw-ob&q=i am tango"
subprocess.call(["mplayer", "-ao", "alsa", "-really-quiet", "-noconsolecontrols", tts_url], shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE )
