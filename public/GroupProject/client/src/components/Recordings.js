import React, { useEffect, useState } from 'react';

function Recordings() {
    const [recordings, setRecordings] = useState([]);
    const [selectedRecording, setSelectedRecording] = useState(null);
    const [videoReloadKey, setVideoReloadKey] = useState(0);
    
    useEffect(() => {
        const fetchRecordings = () => {
            fetch(`${process.env.REACT_APP_API_URL}/api/recordings`)
                .then((res) => res.json())
                .then((data) => {
                    console.log(data);
                    setRecordings(data);
                })
                .catch((err) => console.error("Error fetching recordings:", err));
        };

        // Initial fetch and then set up polling every process.env.REACT_APP_POLLING_RATE.
        fetchRecordings();

        const pollingRate = process.env.REACT_APP_POLLING_RATE | 2000;
        const recordingsInterval = setInterval(fetchRecordings, pollingRate);

        return () => clearInterval(recordingsInterval);
    }, []);
    
    const handleWatchClick = (rec) => {
        if (selectedRecording && selectedRecording.name === rec.name) {
            // If clicking the same recording, increment the key to force reload
            setVideoReloadKey((prevKey) => prevKey + 1);
        } else {
            // Load a new recording
            setSelectedRecording(rec);
            setVideoReloadKey(0);
        }
    };
    
    return (
        <div className="p-6 bg-gray-100 rounded mb-12">
            <div className="p-4 bg-white border border-gray-300 rounded mb-2">
                <div className="w-124 h-82 mx-auto bg-neutral-100 rounded flex justify-center items-center">
                    {selectedRecording ? (
                        <video
                            key={`${selectedRecording.name}-${videoReloadKey}`}
                            controls
                            autoPlay
                            className="w-full h-full"
                        >
                        <source
                            src={`${process.env.REACT_APP_API_URL}/api/recordings/${selectedRecording.name}/view`}
                            type="video/mp4"
                        />
                        Your browser does not support the video tag.
                        </video>
                    ) : (
                        <span className="min-h-full block py-32">
                        <svg
                            className="mx-auto"
                            width="86px"
                            height="86px"
                            viewBox="0 0 24 24"
                            fill="none"
                            xmlns="http://www.w3.org/2000/svg"
                            >
                            <path
                            d="M16 10L18.5768 8.45392C19.3699 7.97803 19.7665 7.74009 20.0928 7.77051C20.3773 7.79703 20.6369 
                                7.944 20.806 8.17433C21 8.43848 21 8.90095 21 9.8259V14.1741C21 15.099 21 15.5615 20.806 15.8257C20.6369 
                                16.056 20.3773 16.203 20.0928 16.2295C19.7665 16.2599 19.3699 16.022 18.5768 15.5461L16 14M6.2 
                                18H12.8C13.9201 18 14.4802 18 14.908 17.782C15.2843 17.5903 15.5903 17.2843 15.782 16.908C16 16.4802 
                                16 15.9201 16 14.8V9.2C16 8.0799 16 7.51984 15.782 7.09202C15.5903 6.71569 15.2843 6.40973 14.908 
                                6.21799C14.4802 6 13.9201 6 12.8 6H6.2C5.0799 6 4.51984 6 4.09202 6.21799C3.71569 6.40973 3.40973 
                                6.71569 3.21799 7.09202C3 7.51984 3 8.07989 3 9.2V14.8C3 15.9201 3 16.4802 3.21799 16.908C3.40973 
                                17.2843 3.71569 17.5903 4.09202 17.782C4.51984 18 5.07989 18 6.2 18Z"
                            stroke="#4d4d4d"
                            strokeWidth="2"
                            strokeLinecap="round"
                            strokeLinejoin="round"
                            />
                        </svg>
                        </span>
                    )}
                </div>
        
                <div className="w-80 rounded text-center mx-auto mt-4">
                    <p className="text-neutral-800">
                        {selectedRecording
                            ? `Now Playing: ${selectedRecording.name}`
                            : 'Select A Recording To View The Video'}
                    </p>
                </div>
            </div>

            <div className="p-4 bg-white border border-gray-300 rounded">
                <h2 className="text-xl font-bold mb-4">
                    Recordings
                </h2>
                {recordings.length === 0 ? (
                    <p>No recordings found.</p>
                ) : (
                    <ul className="space-y-2">
                        {recordings.map((rec, index) => (
                            <li
                                key={index}
                                className="p-4 bg-white rounded border border-gray-200 transition-all duration-200"
                            >
                                <div className="flex justify-between items-center">
                                    <div>
                                        <p className="text-gray-800 m-0">
                                            <strong className="font-bold">File:</strong> {rec.name} <br />
                                            <strong className="font-bold">Size:</strong> {rec.size} bytes <br />
                                            <strong className="font-bold">Date:</strong>{' '}
                                        {new Date(rec.date).toLocaleString()}
                                        </p>
                                    </div>
                                    <div className="flex space-x-4">
                                        <button
                                            onClick={() => handleWatchClick(rec)}
                                            className="cursor-pointer px-4 py-2 bg-blue-500 
                                            text-white rounded hover:bg-blue-600 focus:outline-none focus:ring-2 focus:ring-blue-300"
                                        >
                                            Watch
                                        </button>
                                        <a
                                            href={`${process.env.REACT_APP_API_URL}/api/recordings/${rec.name}/download`}
                                            className="cursor-pointer px-4 py-2 bg-green-500 text-white rounded 
                                            hover:bg-green-600 focus:outline-none focus:ring-2 focus:ring-green-300"
                                        >
                                            Download
                                        </a>
                                    </div>
                                </div>
                            </li>
                        ))}
                    </ul>
                )}
            </div>
        </div>
    );
}

export default Recordings;