import time
import cv2
    
# allow the camera to warmup
time.sleep(0.1)

# capture frames frome the camera
cap = cv2.VideoCapture(0)

while True:
    ret, frame = cap.read()
    cv2.imshow('faceDetection', frame)
    if cv2.waitKey(10) & 0xFF == ord('q'):
        break
    
cap.release()
cv2.destroyAllWindows()
for x in range(0, 5):
    cv2.waitKey(1)

