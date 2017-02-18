#!/usr/bin/python

from time import sleep

try:
  while True:
    print "hello world"
    sleep(60)
except KeyboardInterrupt, e:
  logging.info("stopping..")