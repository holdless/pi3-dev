# import the necessary packages
from picamera.array import PiRGBArray
from picamera import PiCamera
import time
import cv2

# init window
#cv2.startWindowThread()
#cv2.namedWindow('myFrame', cv2.CV_WINDOW_AUTOSIZE)

# initialization the camera and grab a reference to the raw camera capture
camera = PiCamera()
camera.resolution = (640, 480)
camera.framerate = 25
stream = PiRGBArray(camera)

# allow the camera to warmup
time.sleep(0.1)

# capture frames frome the camera
while True:
    camera.capture(stream, format ="bgr", use_video_port=True)
    
    cv2.imshow('myFrame', stream.array)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
    
    # reset the stream before the next capture
    stream.seek(0)
    stream.truncate(0)


cv2.destroyAllWindows()
for x in range(0, 5):
    cv2.waitKey(1)
