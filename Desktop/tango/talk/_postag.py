# google translate tts
import json
import requests
import urllib
import subprocess
import argparse
import pycurl
import StringIO
import os.path
import nltk

def mySum(a, b):
    c = a + b
    return c

# pos-taggin init
def initPosTagger():
    _POS_TAGGER =  'taggers/maxent_treebank_pos_tagger/english.pickle'
    tagger = nltk.load(_POS_TAGGER)
    return tagger

# pos-tagging, nltk
def posTag(tagger, string):
    tokenized_string = nltk.word_tokenize(string)
    tagged_string = tagger.tag(tokenized_string)
    print tagged_string
    return tagged_string

# tts, google translate tts
def call(string):
    url = "http://translate.google.com/translate_tts?tl=en&client=tw-ob&q=" + string
    subprocess.call(["mplayer", "-ao", "alsa", "-really-quiet", "-noconsolecontrols", url], shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE )

#print mySum(1,2)
#string = 'I am Jack'    
tagger =  initPosTagger()
#call(string)
#tagged_string = posTag(tagger, string)
