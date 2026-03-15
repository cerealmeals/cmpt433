#!/bin/bash

CLIENT_DIR="./client"
SERVER_DIR="./server"
CLIENT_ENV="${CLIENT_DIR}/.env"
SERVER_ENV="${SERVER_DIR}/.env"

# Create .env for the client, if the directory exists.
if [ -d "$CLIENT_DIR" ]; then
    if [ ! -f "$CLIENT_ENV" ]; then
        cat <<EOF > "$CLIENT_ENV"
REACT_APP_API_URL=http://localhost:8080
REACT_APP_POLLING_RATE=5000
EOF
        echo "Created ${CLIENT_ENV}"
    else
        echo "${CLIENT_ENV} already exists. Skipping creation."
    fi
else
    echo "Client directory (${CLIENT_DIR}) does not exist. Skipping client .env creation."
fi

# Create .env for the server, if the directory exists.
if [ -d "$SERVER_DIR" ]; then
    if [ ! -f "$SERVER_ENV" ]; then
        cat <<EOF > "$SERVER_ENV"
SERVER_URL=http://localhost:8080
CLIENT_URL=http://localhost:3000
SERVER_PORT=8080
EOF
        echo "Created ${SERVER_ENV}"
    else
        echo "${SERVER_ENV} already exists. Skipping creation."
    fi
else
    echo "Server directory (${SERVER_DIR}) does not exist. Skipping server .env creation."
fi

# Check if npm is installed.
if ! command -v npm &> /dev/null; then
    echo "npm not found. Attempting to install npm..."
    # Linux
    if [ -x "$(command -v apt-get)" ]; then
        sudo apt-get update && sudo apt-get install -y npm
    # MacOS
    elif [ -x "$(command -v brew)" ]; then
        brew install npm
    else
        echo "No supported package manager found. Please install Node.js (which includes npm) manually."
        exit 1
    fi
fi

# Install npm packages in the client directory.
if [ -d "./client" ]; then
    if [ -f "./client/package.json" ]; then
        echo "Installing npm packages in ./client..."
        (cd ./client && npm install)
    else
        echo "No package.json found in ./client. Please ensure your React app is set up."
        exit 1
    fi
else
    echo "./client directory does not exist. Please ensure your React app is located in ./client."
    exit 1
fi

# Install npm packages (ExpressJS & NodeJS Server) in the server directory.
if [ -d "./server" ]; then
    (cd ./server && npm install)
else
    echo "./server directory does not exist."
    exit 1
fi

# Create the virtual environment if it doesn't exist.
if [ ! -d ".venv" ]; then
    python3 -m venv .venv
fi

# Activate the virtual environment and install pip packages.
source .venv/bin/activate
pip install -r requirements.txt