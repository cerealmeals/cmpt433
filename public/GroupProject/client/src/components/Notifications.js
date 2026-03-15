import React from 'react';

function Notifications() {
    return (
        <div className="notifications">
            <h2>Notifications</h2>
            <div className="notification">
                <img
                    src="https://via.placeholder.com/150"
                    alt="Notification"
                    style={{ display: 'block', marginBottom: '10px' }}
                />
                <p>
                    A video clip [clip_timestamp].mp4 has been recorded [start_time, end_time] 
                    and notified to [email].
                </p>
            </div>
        </div>
    );
}

export default Notifications;