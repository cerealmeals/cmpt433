//server 
const path = require('path');
require('dotenv');

const express = require('express');
const http = require('http'); // Require Node's HTTP module
const cors = require('cors');
const fs = require('fs');

const socketIO = require('socket.io');

const app = express();
const SERVER_PORT = process.env.SERVER_PORT || 8080;
const CLIENT_URL = process.env.CLIENT_URL || 'http://localhost:3000';

// Serve static files from the React app (build folder)
app.use(express.static(path.join(__dirname, '..', 'client', 'build')));

// Enable CORS for the client .
app.use(cors({ origin: CLIENT_URL }));

// Get list of recordings
app.get('/api/recordings', (req, res) => {
    const recordingsDir = path.join(__dirname, 'recordings');

    // Create directory if it doesn't exist
    if (!fs.existsSync(recordingsDir)) {
        fs.mkdirSync(recordingsDir, { recursive: true });
        return res.json([]);
    }
    
    // Read directory and get file stats
    const files = fs.readdirSync(recordingsDir)
        .filter(file => file.endsWith('.mp4'))
        .map(file => {
            const filePath = path.join(recordingsDir, file);
            const stats = fs.statSync(filePath);
            return {
                name: file,
                path: `/api/recordings/${file}/view`,
                size: stats.size,
                date: stats.mtime
            };
        })
        // Sort by date (newest first)
        .sort((a, b) => b.date - a.date);
    
    res.json(files);
});

// Get log entries.
app.get('/api/log', (req, res) => {
    // The log file is assumed to be in the log directory relative to __dirname.
    const logFilePath = path.join(__dirname, 'log', 'log.json');
    
    // Return an empty array if the file doesn't exist.
    if (!fs.existsSync(logFilePath))
        return res.json([]);

    fs.readFile(logFilePath, 'utf8', (err, data) => {
        if (err) {
            console.error("Error reading log file:", err);
            return res.status(500).json({ error: 'Error reading log file' });
        }
        try {
            const logs = JSON.parse(data);
            const sortedLogs = logs
                .sort((a, b) => new Date(b.timestamp) - new Date(a.timestamp)) // Sort by timestamp (newest first)
                .slice(0, 10); // Take only the first 10 entries

            res.json(sortedLogs);
        } catch (e) {
            console.error("Error parsing log file:", e);
            res.status(500).json({ error: 'Error parsing log file' });
        }
    });
});

app.get('/api/recordings/:filename/view', (req, res) => {
    // Sanitize the filename to prevent path traversal.
    const filename = path.basename(req.params.filename);
    const videoPath = path.join(__dirname, 'recordings/', filename);

    if (!fs.existsSync(videoPath)) {
        return res.status(404).send('Video not found');
    }
    
    const stat = fs.statSync(videoPath);
    const fileSize = stat.size;
    const range = req.headers.range;
    
    if (range) {
        const parts = range.replace(/bytes=/, "").split("-");
        const start = parseInt(parts[0], 10);
        const end = parts[1] ? parseInt(parts[1], 10) : fileSize - 1;
        const chunksize = (end - start) + 1;
        const file = fs.createReadStream(videoPath, { start, end });
        const head = {
            'Content-Range': `bytes ${start}-${end}/${fileSize}`,
            'Accept-Ranges': 'bytes',
            'Content-Length': chunksize,
            'Content-Type': 'video/mp4',
        };
        res.writeHead(206, head);
        file.pipe(res);
    } else {
        const head = {
            'Content-Length': fileSize,
            'Content-Type': 'video/mp4',
        };
        res.writeHead(200, head);
        fs.createReadStream(videoPath).pipe(res);
    }
});

// Download video file endpoint
app.get('/api/recordings/:filename/download', (req, res) => {
    const safeFilename = path.basename(req.params.filename);
    const videoPath = path.join(__dirname, 'recordings', safeFilename);
  
    // Use Express's download helper to set Content-Disposition header
    res.download(videoPath, safeFilename, (err) => {
        if (err) {
            res.status(404).send('Video not found');
        }
    });
});
  
// For all other GET requests not handled by /api, send back React's index.html
app.get('*', (req, res) => {
    res.sendFile(path.join(__dirname, '..', 'client', 'build', 'index.html'));
});

// Create an HTTP server and wrap the Express app
const server = http.createServer(app);

// Initialize Socket.IO using the HTTP server
const io = socketIO(server, {
    cors: {
        origin: "*"
    }
});

// Socket.IO connection events
io.on('connection', (socket) => {
    console.log('Client connected');

    // Listen for the "new_webcam_frame" event from any client
    socket.on('new_webcam_frame', (data) => {
        // Broadcast the new_webcam_frame event to all connected clients
        io.emit('new_webcam_frame', data);
    });
    
    socket.on('disconnect', () => {
        console.log('Client disconnected');
    });
});

// Start the server
server.listen(SERVER_PORT, () => {
    console.log(`Server is listening on port ${SERVER_PORT}`);
});
