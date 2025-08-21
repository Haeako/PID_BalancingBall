#!/usr/bin/python
import cv2
import os
import depthai as dai
import pyvirtualcam
from pyvirtualcam import PixelFormat
import numpy as np
import time
import errno
from threading import Thread
from multiprocessing import shared_memory
import struct
import time

FIFO_PATH = '/tmp/stepper_fifo'
TARGET_HSV_MIN = np.array([30, 120, 50], dtype=np.uint8)
TARGET_HSV_MAX = np.array([87, 255, 255], dtype=np.uint8)
MAX_CONTOUR_AREA = 140*140
pipeline = dai.Pipeline()

DEBUG = True

# Define source and output
camRgb = pipeline.createCamera()
pipeline.setXLinkChunkSize(0)
# cam.initialControl.setManualFocus(130)
xoutVideo = pipeline.createXLinkOut()
xoutVideo.setStreamName("video")

# settings
width, height = (256, 209)
fps = 120

# preview output using opencv imshow
preview = False

# print fps on virtual camera
print_fps = False

# Properties
camRgb.setPreviewSize(width, height)
# camRgb.setColorOrder(dai.ColorCameraProperties.ColorOrder.RG)
camRgb.setFps(fps)

xoutVideo.input.setBlocking(False)
xoutVideo.input.setQueueSize(1)

# Linking to preview stream
camRgb.preview.link(xoutVideo.input)
flags = False
    
# Connect to device and start pipeline
try:
    shm = shared_memory.SharedMemory(name="coor_mem", create=True, size=12)
except FileExistsError:
    shm = shared_memory.SharedMemory(name="coor_mem", create=True, size=12)
    

def send_coor(x, y):
    shm.buf[:12] = struct.pack("<iii", x, y, 1)
    print(f"send {x}, {y}")
def main():
    if DEBUG:
        while True:
            send_coor(0,0)
    else:
        try:
            with dai.Device(pipeline) as device:

                video = device.getOutputQueue(name="video", maxSize=1, blocking=False)
            
                with pyvirtualcam.Camera(width, height, fps, print_fps=True) as cam:
                    print(f'Virtual cam started: {cam.device} ({cam.width}x{cam.height} @ {cam.fps}fps)')

                    while True:
                        videoIn = video.get()
                        
                        # Get BGR frame from NV12 encoded video frame to show with opencv
                        frame = videoIn.getCvFrame()
                        frame_hsv = cv2.cvtColor(frame,cv2.COLOR_BGR2HSV); 
                        frame_threshold = cv2.inRange(frame,TARGET_HSV_MIN,TARGET_HSV_MAX)
                        contours, _ = cv2.findContours(
                            frame_threshold, 
                            cv2.RETR_EXTERNAL,
                            cv2.CHAIN_APPROX_SIMPLE)
                        mu=[None]*len(contours)
                        for i in range(len(contours)):
                           mu[i]=cv2.moments(contours[i])
                        mc=[None]*len(contours)
                        index=0
                        for i in range(len(contours)):
                            # add 1e-5 to avoid division by zero
                            if mu[i]['m00'] >40 and mu[i]['m00']< MAX_CONTOUR_AREA:
                                index=i
                                mc[i] = (mu[i]['m10'] / (mu[i]['m00'] + 1e-5), mu[i]['m01'] / (mu[i]['m00'] + 1e-5))
                                break
                        if index <= len(mc) and index!= 0:
                            center = int(mc[index][0]*4),int(mc[index][1]*4)
                            send_coor(x=center[0], y=center[1])
                        else:
                            send_coor(0,0)
        finally:
            print("Clean up")
            shm.close()
if __name__ == "__main__":
    main()
                
