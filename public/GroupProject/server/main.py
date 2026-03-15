from socket_client import connect_server
from camera_capture import stream_camera_capture_frames

def main():
    # Connect to the Socket.IO server.
    connect_server()

    # Stream webcam.
    stream_camera_capture_frames()

if __name__ == '__main__':
    main()