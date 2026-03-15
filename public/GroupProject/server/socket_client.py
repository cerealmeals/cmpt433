import os
from pathlib import Path
import socketio
from dotenv import load_dotenv

# Load variables from .env file
load_dotenv()

# Create a Socket.IO client.
sio = socketio.Client()

# Use REACT_APP_API_URL value, otherwise, http://localhost:8080
SERVER_URL = os.getenv('SERVER_URL', 'http://localhost:8080')

@sio.event
def connect():
    print("Connected to Express server!")

@sio.event
def disconnect():
    print("Disconnected from Express server.")

def connect_server():
    sio.connect(SERVER_URL)