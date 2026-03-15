#!/bin/bash
echo "Stopping ExpressJS server..."
pkill -f "node ./server/index.js"

# Kill process.
echo "Stopping Create React App..."
pkill -f "npm start"

# Kill process.
echo "Stopping Python program..."
pkill -f "python ./server/main.py"

# Remove the virtual environment.
if [ -d ".venv" ]; then
    echo "Removing virtual environment..."
    rm -rf .venv
fi

# Remove the client node_modules.
if [ -d "./client/node_modules" ]; then
    echo "Removing client node_modules..."
    rm -rf ./client/node_modules
fi

echo "Uninstallation complete."