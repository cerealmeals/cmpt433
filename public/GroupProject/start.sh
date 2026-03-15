#!/bin/bash

# Start the ExpressJS server in the background.
node ./server/index.js &

# Change to the client directory and start the Create React App.
# "yes" forces npm to use an available port if port 3000 is already in use.
(cd ./client && yes | npm install | npm start) &

# Activate the virtual environment.
source .venv/bin/activate

# Run the Python program in the background.
python ./server/main.py &

# Wait for all background processes to finish.
wait
