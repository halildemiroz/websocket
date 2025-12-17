#!/bin/bash

# --- CONFIGURATION ---
PORT=8082
# The server binary is inside the 'build' folder
SERVER_BIN="./build/server"  
# ---------------------

# Function to clean up background processes on exit
cleanup() {
    echo ""
    echo "Shutting down..."
    if [ ! -z "$SERVER_PID" ]; then
        kill $SERVER_PID 2>/dev/null
        echo "Server (PID $SERVER_PID) stopped."
    fi
    exit
}

# Trap Ctrl+C (SIGINT) to run the cleanup function
trap cleanup SIGINT

echo "========================================"
echo "   Chat Server Launcher with Bore.pub   "
echo "========================================"

# 1. Check for 'bore' installation
if ! command -v bore &> /dev/null; then
    echo "Error: 'bore' is not installed."
    echo "Please install it by running: cargo install bore-cli"
    exit 1
fi

# 2. Check if the server binary exists in the build folder
if [ ! -f "$SERVER_BIN" ]; then
    echo "Error: Could not find the server executable at '$SERVER_BIN'."
    echo "It looks like you haven't compiled the project yet."
    echo ""
    echo "Try running these commands first:"
    echo "  mkdir -p build"
    echo "  cd build"
    echo "  cmake .."
    echo "  make"
    exit 1
fi

# 3. Start the Server in the background
echo "[*] Starting local server on port $PORT..."
$SERVER_BIN &
SERVER_PID=$! # Save the Process ID (PID) so we can kill it later
sleep 1       # Give it a moment to start

# 4. Start the Tunnel
echo "[*] Starting Bore Tunnel..."
echo "----------------------------------------"
echo "SHARE THESE DETAILS WITH YOUR FRIENDS:"
echo "----------------------------------------"
# Run bore and let it print the URL/Port to the screen
bore local $PORT --to bore.pub

# The script waits here until 'bore' is closed.
# Once 'bore' stops, the 'cleanup' function runs automatically.
