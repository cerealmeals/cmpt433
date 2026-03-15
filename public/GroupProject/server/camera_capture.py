import cv2
import base64
import os
import eventlet
import time
from datetime import datetime
from detector import detect_pets, pets_detected_callback, pets_not_detected_callback
from logger import update_log
from socket_client import sio

def stream_camera_capture_frames():

        
    # GStreamer pipeline to receive JPEG-encoded RTP on port 5000, then decode it
    # pipeline = (
    #     "udpsrc port=5000 ! "               # Pull from UDP port 5000
    #     "application/x-rtp,encoding-name=JPEG ! " 
    #     "rtpjpegdepay ! "                   # Undo RTP payload
    #     "jpegdec ! "                        # Decode JPEG frames into raw video
    #     "videoconvert ! "                   # Convert to a format OpenCV can read
    #     "appsink"                           # Hand off frames to OpenCV
    # )

    # # Tell OpenCV to open this pipeline (requires OpenCV built with GStreamer support)
    # cap = cv2.VideoCapture(pipeline, cv2.CAP_GSTREAMER)

    # if not cap.isOpened():
    #     print("Failed to open capture via GStreamer pipeline.")
    #     exit(1)

    # print("Receiving UDP stream on port 5000...")

    # Open the default webcam.
    cap = cv2.VideoCapture(0)
    fps = cap.get(cv2.CAP_PROP_FPS)
    if not fps or fps <= 0:
        fps = 24

    # Recording parameters.
    RECORD_SEGMENT_DURATION = 5             # seconds per segment check.
    MAX_CLIP_DURATION = 5 * 60              # maximum clip duration: 5 minutes.

    # Clip state variables.
    clip_start_time = None
    segment_start_time = None

    while cap.isOpened():
        ret, frame = cap.read()
        if not ret:
            break

        # Run YOLO detection on the frame.
        result = detect_pets(frame)
        pet_detected = result.boxes is not None and len(result.boxes) > 0

        # Update detection status.
        if pet_detected:
            pets_detected_callback()
            if clip_start_time is None:
                # Start a new clip when a pet is first detected.
                clip_start_time = time.time()
                segment_start_time = time.time()
                print("Started clip recording at time:", clip_start_time)
        else:
            pets_not_detected_callback()

        # Check if a clip is active and if it's time to finalize.
        if clip_start_time is not None:
            current_segment_duration = time.time() - segment_start_time
            total_clip_duration = time.time() - clip_start_time

            if current_segment_duration >= RECORD_SEGMENT_DURATION:
                if not pet_detected:
                    # Finalize clip as pet is no longer detected.
                    clip_end_time = time.time()
                    print("Finalizing clip from", clip_start_time, "to", clip_end_time)
                    timestamp = datetime.fromtimestamp(clip_start_time).strftime("%Y%m%d_%H%M%S")
                    clip_name = f"clip_{timestamp}"
                    log_entry = {
                        "clip_name": clip_name,
                        "start_time": clip_start_time,
                        "end_time": clip_end_time
                    }
                    update_log(log_entry)
                    #Aki's original code: 
                    os.system(f"./send_clip_and_email {clip_start_time} {clip_end_time}")
                    # os.system(f"/mnt/remote/myApps/clip_and_send.sh {clip_start_time} {clip_end_time}")
                    clip_start_time = None
                    segment_start_time = None
                else:
                    # Check if maximum clip duration is reached.
                    if total_clip_duration >= MAX_CLIP_DURATION:
                        clip_end_time = time.time()
                        print("Maximum clip duration reached. Finalizing clip from", clip_start_time, "to", clip_end_time)
                        timestamp = datetime.fromtimestamp(clip_start_time).strftime("%Y%m%d_%H%M%S")
                        clip_name = f"clip_{timestamp}"
                        log_entry = {
                            "clip_name": clip_name,
                            "start_time": clip_start_time,
                            "end_time": clip_end_time
                        }
                        update_log(log_entry)
                        os.system(f"./send_clip_and_email {clip_start_time} {clip_end_time}")
                        clip_start_time = None
                        segment_start_time = None
                    else:
                        # Reset segment timer for the next check.
                        segment_start_time = time.time()

        # Annotate the frame.
        annotated_frame = result.plot()
        ret2, buffer = cv2.imencode('.jpg', annotated_frame)
        if ret2:
            jpg_as_text = base64.b64encode(buffer).decode('utf-8')
            # Emit the frame via Socket.IO.
            sio.emit('new_webcam_frame', {'image': jpg_as_text})

        # Sleep briefly to match frame rate.
        eventlet.sleep(0.03)

    cap.release()