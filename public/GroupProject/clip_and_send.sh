#!/bin/bash

# Usage: ./clip_and_send.sh <start_time> <end_time> [source_video_path]
if [ $# -lt 2 ]; then
    echo "Usage: $0 <start_time> <end_time> [source_video_path]"
    exit 1
fi

START_TIME=$1
END_TIME=$2
OUTPUT_DIR="./server/recordings"  # Changed from "/mnt/remote/recordings"
# Use provided path or auto-detect the first file in OUTPUT_DIR
SOURCE_VIDEO=${3:-$(find "$OUTPUT_DIR" -maxdepth 1 -type f -printf '%T@ %p\n' | sort -n | tail -n 1 | cut -d' ' -f2-)}


# Check if source video exists
if [ ! -f "$SOURCE_VIDEO" ]; then
    echo "Error: Source video file not found: $SOURCE_VIDEO"
    exit 1
fi

echo "Source video: $SOURCE_VIDEO"
echo "Clipping from $START_TIME to $END_TIME..."

# Step 1: Clip the video and capture the output path
if [ $# -eq 3 ]; then
    CLIP_OUTPUT=$(./clip_video "$START_TIME" "$END_TIME" "$SOURCE_VIDEO" | grep "OUTPUT_CLIP=" | cut -d= -f2)
else
    CLIP_OUTPUT=$(./clip_video "$START_TIME" "$END_TIME" | grep "OUTPUT_CLIP=" | cut -d= -f2)
fi

if [ -z "$CLIP_OUTPUT" ]; then
    echo "Error: Failed to get output clip path"
    exit 1
fi

# Verify the clip exists and has reasonable size
if [ ! -f "$CLIP_OUTPUT" ]; then
    echo "Error: Clip file not found: $CLIP_OUTPUT"
    exit 1
fi

FILE_SIZE=$(stat -c%s "$CLIP_OUTPUT")
if [ "$FILE_SIZE" -lt 1024 ]; then
    echo "Warning: Clip file is very small ($FILE_SIZE bytes), may be corrupted"
    exit 1
fi

echo "Clip created: $CLIP_OUTPUT ($FILE_SIZE bytes)"

# Step 2: Send the email with the clip
echo "Sending email with the clip..."
./send_email "$CLIP_OUTPUT"

echo "Uploading clip to server..."
RESPONSE=$(curl -s -w "\nHTTP_STATUS:%{http_code}" -F "clip=@${CLIP_OUTPUT}" http://192.168.0.102:8080/api/recordings)
HTTP_STATUS=$(echo "$RESPONSE" | grep HTTP_STATUS | cut -d: -f2)
BODY=$(echo "$RESPONSE" | sed '/HTTP_STATUS/d')

if [ "$HTTP_STATUS" -ne 201 ]; then
    echo "Error: Failed to upload clip. Server response: $BODY"
    exit 1
fi

echo "Server response: $BODY"

# Step 4: Remove the clip
echo "Removing temporary clip file..."
rm -f "$CLIP_OUTPUT"

echo "Process completed!"
