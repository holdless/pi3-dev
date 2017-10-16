import pyttsx
engine = pyttsx.init()
engine.setProperty('rate', 175)
engine.say("hi! my name is Tango! it's a pleasure to meet you! Can you tell me your name?")
engine.runAndWait()
