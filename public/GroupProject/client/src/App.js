import React from 'react';
import { BrowserRouter as Router, Routes, Route, NavLink } from 'react-router-dom';
import LiveView from './components/LiveView';
import Recordings from './components/Recordings';
import Notifications from './components/Notifications';

function App() {
    // Base classes for the button styling
    const buttonClass = "px-4 py-2 border rounded-md transition-colors duration-200";
    // Classes when the button is not active
    const inactiveClass = "border-blue-500 text-blue-500 hover:bg-blue-100";
    // Classes when the button is active (current page)
    const activeClass = "bg-blue-500 text-white";

    return (
        <Router>
            <div className="flex flex-col items-center justify-center mt-12">
                <header className="mb-8 text-center">
                    <div className="flex items-center mb-6">
                        <img
                            src="/logo.svg" // Make sure this file is in your public folder
                            alt="Pet Logo"
                            className="w-24 h-24"
                        />
                        <h1 className="text-3xl font-bold mr-14">Pet Security Camera</h1>
                    </div>
                    <nav className="space-x-2 mt-6">
                        <NavLink
                            to="/"
                            className={({ isActive }) =>
                                `${buttonClass} ${isActive ? activeClass : inactiveClass}`
                            }
                        >
                            Live
                        </NavLink>
                        <NavLink
                            to="/recordings"
                            className={({ isActive }) =>
                                `${buttonClass} ${isActive ? activeClass : inactiveClass}`
                            }
                        >
                            Recordings
                        </NavLink>
                    </nav>
                </header>
                <main className="w-full max-w-4xl">
                    <Routes>
                        <Route path="/" element={<LiveView />} />
                        <Route path="/recordings" element={<Recordings />} />
                        <Route path="/notifications" element={<Notifications />} />
                    </Routes>
                </main>
            </div>
        </Router>
    );
}

export default App;