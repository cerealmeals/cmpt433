import React, { useEffect, useState } from 'react';
import io from 'socket.io-client';

const socket = io(process.env.REACT_APP_API_URL);

function LiveView() {
    const [imageSrc, setImageSrc] = useState('');
    const [cameraDetected, setCameraDetected] = useState(false);
    const [logs, setLogs] = useState([]);
    
    useEffect(() => {
        // Listen for live webcam frames.
        socket.on('new_webcam_frame', (data) => {
            setImageSrc('data:image/jpeg;base64,' + data.image);
            setCameraDetected(true);
        });

        const fetchLogs = () => {
            fetch(`${process.env.REACT_APP_API_URL}/api/log`)
                .then((res) => res.json())
                .then((data) => setLogs(data))
                .catch((err) => console.error("Error fetching logs:", err));
        };

        // Initial fetch and then set up polling every process.env.REACT_APP_POLLING_RATE.
        fetchLogs();

        const pollingRate = process.env.REACT_APP_POLLING_RATE | 2000;
        const logInterval = setInterval(fetchLogs, pollingRate);

        // Cleanup on component unmount.
        return () => {
            clearInterval(logInterval);
            setCameraDetected(false);
            socket.off('new_webcam_frame');
        };
    }, []);
    
    return (
        <div className="p-6 bg-gray-100 rounded mb-12">
            <div className="max-w-4xl mx-auto">
                {cameraDetected ? (
                    <div className="p-4 bg-white border border-gray-300 rounded mb-6">
                        <img 
                            id="video_feed" 
                            className="w-124 h-82 mx-auto"
                            alt="Video Stream"
                            src={imageSrc}
                        />
                    </div>
                ) : (
                    <div className="p-4 bg-white border border-gray-300 rounded mb-2">
                        <div className="w-124 h-82 mx-auto bg-neutral-100 rounded">
                            <span className="min-h-full block py-32">
                                <svg
                                    className="mx-auto"
                                    width="86px"
                                    height="86px"    
                                    viewBox="0 0 24 24" 
                                    fill="none" 
                                    xmlns="http://www.w3.org/2000/svg">
                                    <path 
                                        d="M11.65 6H12.8C13.9201 6 14.4802 6 14.908 6.21799C15.2843 6.40973 15.5903 
                                        6.71569 15.782 7.09202C16 7.51984 16 8.0799 16 9.2V10L18.5768 8.45392C19.3699 7.97803 
                                        19.7665 7.74009 20.0928 7.77051C20.3773 7.79703 20.6369 7.944 20.806 8.17433C21 8.43848 
                                        21 8.90095 21 9.8259V14.1741C21 14.679 21 15.0462 20.9684 15.3184M3 3L6.00005 6.00005M21 
                                        21L15.9819 15.9819M6.00005 6.00005C5.01167 6.00082 4.49359 6.01337 4.09202 6.21799C3.71569 
                                        6.40973 3.40973 6.71569 3.21799 7.09202C3 7.51984 3 8.07989 3 9.2V14.8C3 15.9201 3 16.4802 
                                        3.21799 16.908C3.40973 17.2843 3.71569 17.5903 4.09202 17.782C4.51984 18 5.07989 18 6.2 18H12.8C13.9201 
                                        18 14.4802 18 14.908 17.782C15.2843 17.5903 15.5903 17.2843 15.782 16.908C15.9049 16.6668 
                                        15.9585 16.3837 15.9819 15.9819M6.00005 6.00005L15.9819 15.9819" 
                                        stroke="#4d4d4d" 
                                        strokeWidth="2" 
                                        strokeLinecap="round" 
                                        strokeLinejoin="round"
                                    />
                                </svg>
                            </span>
                        </div>
                        <div className="w-64 p-2 bg-red-100 border border-red-300 rounded text-center mx-auto mt-4">
                            <p className="text-red-700">Video Streaming Not Found</p>
                        </div>
                    </div>
                )}
                
                <div className="p-4 bg-white border border-gray-300 rounded">
                    <h2 className="text-xl font-bold mb-4">Log</h2>
                    <ul className="space-y-2">
                        {logs.length > 0 ? (
                            logs.map((entry, index) => (
                                <li key={index} className="text-gray-700 text-xs">
                                    A video clip <span className="font-mono font-bold">{entry.clip_name}.mp4</span> has been recorded from{' '}
                                    <span className="font-mono font-bold">
                                        {new Date(entry.start_time * 1000).toLocaleString()}
                                    </span> to{' '}
                                    <span className="font-mono font-bold">
                                        {new Date(entry.end_time * 1000).toLocaleString()}
                                    </span>.
                                </li>
                            ))
                        ) : (
                            <li className="text-gray-700">No logs available.</li>
                        )}
                    </ul>
                </div>
            </div>
        </div>
    );
}

export default LiveView;