import cv2
import os
import depthai as dai
import pyvirtualcam
import numpy as np
import time
import errno
from threading import Thread

# Constants - moved out of loop for better performance
FIFO_PATH = '/tmp/stepper_fifo'
TARGET_HSV_MIN = np.array([30, 120, 50], dtype=np.uint8)
TARGET_HSV_MAX = np.array([87, 255, 255], dtype=np.uint8)
MIN_CONTOUR_AREA = 50*50
MAX_CONTOUR_AREA = 240*240
CENTER_RECT = ((int(498/2), int(404/2)), (int(526/2), int(430/2)))

# FIFO setup
try:
    if not os.path.exists(FIFO_PATH):
        os.mkfifo(FIFO_PATH, 0o666)
except OSError as e:
    if e.errno != errno.EEXIST:
        print(f"Failed to create FIFO: {e}")

# Open FIFO in non-blocking mode for writing - only open once
try:
    fd = os.open(FIFO_PATH, os.O_WRONLY | os.O_NONBLOCK)
except OSError as e:
    print(f"Error opening FIFO: {e}")
    fd = None

# Function to write to FIFO in separate thread
def write_to_fifo_async(message):
    def _write():
        try:
            if fd is not None:
                os.write(fd, message.encode())
        except OSError as e:
            if e.errno != errno.EAGAIN:  # Ignore "resource temporarily unavailable"
                print(f"Error writing to FIFO: {e}")
    
    # Start in thread to avoid blocking
    Thread(target=_write, daemon=True).start()

# Set up pipeline
pipeline = dai.Pipeline()
camRgb = pipeline.createCamera()
xoutVideo = pipeline.createXLinkOut()
xoutVideo.setStreamName("video")

# Camera settings
width = 512
height = 418
fps = 60

# Configure camera
camRgb.setPreviewSize(width, height)
camRgb.setFps(fps)
camRgb.setInterleaved(False)  # Planar format is faster for CPU processing

# Optimize queue
xoutVideo.input.setBlocking(False)
xoutVideo.input.setQueueSize(1)

# Link camera preview to output
camRgb.preview.link(xoutVideo.input)

# Processing function for better organization
def process_frame(frame):
    # Convert directly to HSV and threshold in one step
    frame_threshold = cv2.inRange(
        cv2.cvtColor(frame, cv2.COLOR_BGR2HSV),
        TARGET_HSV_MIN, 
        TARGET_HSV_MAX
    )
    
    # Find contours - using EXTERNAL is faster than CCOMP for simple detection
    contours, _ = cv2.findContours(
        frame_threshold, 
        cv2.RETR_EXTERNAL,
        cv2.CHAIN_APPROX_SIMPLE
    )
    
    # Find largest contour in our target area range
    largest_contour_idx = -1
    largest_area = MIN_CONTOUR_AREA
    
    for i, contour in enumerate(contours):
        area = cv2.contourArea(contour)
        if MIN_CONTOUR_AREA < area < MAX_CONTOUR_AREA and area > largest_area:
            largest_contour_idx = i
            largest_area = area
    
    # If we found a suitable contour
    if largest_contour_idx >= 0:
        # Calculate moments for center
        M = cv2.moments(contours[largest_contour_idx])
        center_x = int(M['m10'] / (M['m00'] + 1e-5) * 2)
        center_y = int(M['m01'] / (M['m00'] + 1e-5) * 2)
        return (center_x, center_y), True
    
    return (0, 0), False

# FPS calculation
prev_time = time.time()
frame_count = 0
fps_value = 0

# Main loop
with dai.Device(pipeline) as device:
    video = device.getOutputQueue(name="video", maxSize=1, blocking=False)
    
    with pyvirtualcam.Camera(width, height, fps, fmt=pyvirtualcam.PixelFormat.BGR) as cam:
        print(f'Virtual cam started: {cam.device} ({cam.width}x{cam.height} @ {cam.fps}fps)')
        
        while True:
            # Get frame with timeout to avoid blocking
            videoIn = video.get()
            if videoIn is None:
                continue
                
            # Get frame
            frame = videoIn.getCvFrame()
            
            # Process frame
            center, found_object = process_frame(frame)
            
            # Send data through FIFO
            message = ' '.join(map(str, center))
            write_to_fifo_async(message)
            
            # Only draw visuals if previewing
            if cv2.getWindowProperty("output", cv2.WND_PROP_VISIBLE) >= 0:
                # Draw rectangle
                cv2.rectangle(frame, 
                             (CENTER_RECT[0][0], CENTER_RECT[0][1]), 
                             (CENTER_RECT[1][0], CENTER_RECT[1][1]), 
                             (0, 0, 255), 2)
                
                # Draw center point if found
                if found_object:
                    cv2.circle(frame, (center[0]//2, center[1]//2), 4, (255, 0, 0), -1)
                
                # Calculate and display FPS
                frame_count += 1
                current_time = time.time()
                elapsed = current_time - prev_time
                
                if elapsed > 1.0:
                    fps_value = frame_count / elapsed
                    frame_count = 0
                    prev_time = current_time
                
                cv2.putText(frame, f"FPS: {fps_value:.1f}", (10, 30), 
                           cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
                
                # Show preview
                cv2.imshow("output", frame)
                
            # Send to virtual camera
            cam.send(frame)
            
            # Exit on ESC
            if cv2.waitKey(1) == 27:
                break

# Clean up
if fd is not None:
    os.close(fd)
cv2.destroyAllWindows()
