import cv2
import urllib.request
import numpy as np

# Stream URL from ESP32-CAM Access Point
URL = "http://192.168.4.1/"

print("Connecting to ESP32-CAM stream...")
stream = urllib.request.urlopen(URL)
bytes_buffer = b""

# Create resizable window
cv2.namedWindow("ESP32 Gimbal View", cv2.WINDOW_NORMAL)
cv2.resizeWindow("ESP32 Gimbal View", 960, 720)

while True:
    # Read network chunks
    bytes_buffer += stream.read(1024)
    
    # Look for JPEG frame delimiters
    a = bytes_buffer.find(b"\xff\xd8")  # Start of image
    b = bytes_buffer.find(b"\xff\xd9")  # End of image
    
    if a != -1 and b != -1:
        jpg = bytes_buffer[a : b + 2]
        bytes_buffer = bytes_buffer[b + 2 :]
        
        # Decode and display frame
        frame = cv2.imdecode(np.frombuffer(jpg, dtype=np.uint8), cv2.IMREAD_COLOR)
        if frame is not None:
            cv2.imshow("ESP32 Gimbal View", frame)
            
    # Press 'q' to exit, 's' to save a snapshot
    key = cv2.waitKey(1) & 0xFF
    if key == ord("q"):
        break
    elif key == ord("s"):
        cv2.imwrite("snapshot.jpg", frame)
        print("Snapshot saved!")

cv2.destroyAllWindows()