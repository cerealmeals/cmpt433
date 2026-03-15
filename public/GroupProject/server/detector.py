import time
from ultralytics import YOLO

# Load the YOLO model.
model = YOLO("yolov8m-oiv7.pt")

def pets_detected_callback():
    print("Pets detected!")

def pets_not_detected_callback():
    print("No pets detected.")

def detect_pets(frame):
    """
    Run YOLO prediction on a given frame.
    Returns the result for further processing.
    """
    results = model.predict(
        source=frame,
        classes=[411, 160, 96, 8, 412, 488],  # Rabbit, Dog, Cat, Animal, Raccoon, Squirrel.
        show=False,
        save=False,
        imgsz=576,
        max_det=5,
        vid_stride=10,
        device="cpu"
    )
    return results[0]