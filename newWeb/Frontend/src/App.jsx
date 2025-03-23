import { useState, useEffect } from 'react';
import axios from 'axios';
import './App.css';

function App() {
  const [speed, setSpeed] = useState(50);
  const [base, setBase] = useState(90);
  const [shoulder, setShoulder] = useState(90);
  const [elbow, setElbow] = useState(90);
  const [gripper, setGripper] = useState(90);
  const [isConnected, setIsConnected] = useState(false);

  // Check ESP32 connection on load
  useEffect(() => {
    const checkConnection = async () => {
      try {
        await axios.get('/api/status/arm'); // Using relative path
        setIsConnected(true);
      } catch (error) {
        console.error('Connection Error:', error);
        setIsConnected(false);
      }
    };    

    checkConnection();
  }, []);

  const sendMovementCommand = async (direction) => {
    try {
      const response = await axios.post('/api/move', { // Using relative path
        dir: direction,
        speed: parseInt(speed),
      });
      console.log('Movement Command Success:', response.data);
    } catch (error) {
      console.error('Movement Command Error:', error);
    }
  };

  const sendArmCommand = async (joint, value) => {
    try {
      const response = await axios.post('/api/arm', { // Using relative path
        joint,
        value: parseInt(value),
      });
      console.log('Arm Command Success:', response.data);
    } catch (error) {
      console.error('Arm Command Error:', error);
    }
  };

  return (
    <div className="App">
      <h1>Robot Control Panel</h1>
      <div>
        <h2>ESP32 Status: {isConnected ? "Connected" : "Disconnected"}</h2>
        <label>Speed: {speed}</label>
        <input
          type="range"
          min="0"
          max="100"
          value={speed}
          onChange={(e) => setSpeed(e.target.value)}
        />
      </div>
      <div>
        <h3>Movement Controls</h3>
        <button onClick={() => sendMovementCommand('forward')}>Forward</button>
        <button onClick={() => sendMovementCommand('backward')}>Backward</button>
        <button onClick={() => sendMovementCommand('left')}>Left</button>
        <button onClick={() => sendMovementCommand('right')}>Right</button>
        <button onClick={() => sendMovementCommand('stop')}>Stop</button>
      </div>
      <div>
        <h3>Arm Controls</h3>
        <label>Base: {base}°</label>
        <input
          type="range"
          min="0"
          max="180"
          value={base}
          onChange={(e) => {
            setBase(e.target.value);
            sendArmCommand('base', e.target.value);
          }}
        />
        <label>Shoulder: {shoulder}°</label>
        <input
          type="range"
          min="0"
          max="180"
          value={shoulder}
          onChange={(e) => {
            setShoulder(e.target.value);
            sendArmCommand('shoulder', e.target.value);
          }}
        />
        <label>Elbow: {elbow}°</label>
        <input
          type="range"
          min="0"
          max="180"
          value={elbow}
          onChange={(e) => {
            setElbow(e.target.value);
            sendArmCommand('elbow', e.target.value);
          }}
        />
        <label>Gripper: {gripper}°</label>
        <input
          type="range"
          min="0"
          max="180"
          value={gripper}
          onChange={(e) => {
            setGripper(e.target.value);
            sendArmCommand('gripper', e.target.value);
          }}
        />
      </div>
    </div>
  );
}

export default App;








/*
import { useState, useEffect } from 'react';
import axios from 'axios';
import './App.css';

function App() {
  const [speed, setSpeed] = useState(50);
  const [isConnected, setIsConnected] = useState(false);

  // Check ESP32 connection on load
  useEffect(() => {
    axios.get('http://localhost:3000/api/status')
      .then(() => {
        setIsConnected(true);
      })
      .catch((error) => {
        console.error('Connection Error:', error);
        setIsConnected(false);
      });
  }, []);

  const sendCommand = (direction) => {
    axios.post('http://192.168.43.204/move', {
      dir: direction,
      speed: parseInt(speed),
    })
      .then(response => {
        console.log('Command Success:', response.data);
      })
      .catch(error => {
        console.error('Command Error:', error);
      });
  };

  return (
    <div className="App">
      <h1>Robot Control Panel</h1>
      <div>
        <h2>ESP32 Status: {isConnected ? "Connected" : "Disconnected"}</h2>
        <label>Speed: {speed}</label>
        <input
          type="range"
          min="0"
          max="100"
          value={speed}
          onChange={(e) => setSpeed(e.target.value)}
        />
      </div>
      <div>
        <button onClick={() => sendCommand('forward')}>Forward</button>
        <button onClick={() => sendCommand('backward')}>Backward</button>
        <button onClick={() => sendCommand('left')}>Left</button>
        <button onClick={() => sendCommand('right')}>Right</button>
        <button onClick={() => sendCommand('stop')}>Stop</button>
      </div>
    </div>
  );
}

export default App;
*/