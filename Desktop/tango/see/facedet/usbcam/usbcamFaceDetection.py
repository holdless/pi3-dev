import time
import cv2
#import numpy as np

face_cascade = cv2.CascadeClassifier('/usr/share/opencv/lbpcascades/lbpcascade_frontalface.xml')
eye_cascade = cv2.CascadeClassifier('/usr/share/opencv/haarcascades/haarcascade_eye.xml')

def detectFaces(img):
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

    faces = face_cascade.detectMultiScale(gray, 1.1, 5)
    for (x,y,w,h) in faces:
        cv2.rectangle(img, (x,y), (x+w,y+h), (255,0,0), 2)
        roi_gray = gray[y:y+h, x:x+w]
        roi_color = img[y:y+h, x:x+w]
#        eyes = eye_cascade.detectMultiScale(roi_gray)
#        for (ex,ey,ew,eh) in eyes:
#            cv2.rectangle(roi_color, (ex,ey), (ex+ew,ey+eh), (0,255,0), 2)

    return img
    

    
# allow the camera to warmup
time.sleep(0.1)

# capture frames frome the camera
cap = cv2.VideoCapture(0)
cap.set(3, 320)
cap.set(4, 240)
while True:
    ret, frame = cap.read()
#    frame = cv2.flip(frame, 0)
#    frame = cv2.flip(frame, 1)
    frame = detectFaces(frame)
    cv2.imshow('faceDetection', frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
    
cap.release()
cv2.destroyAllWindows()
for x in range(0, 5):
    cv2.waitKey(1)

