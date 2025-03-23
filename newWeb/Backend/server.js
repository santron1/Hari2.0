import express from 'express';
import axios from 'axios';
import cors from 'cors';

const app = express();

// Enable CORS
app.use(cors());
app.use(express.json());

// ESP32 IP Addresses
const movementESP32IP = 'http://192.168.43.240'; // Replace with your movement ESP32 IP
const armESP32IP = 'http://192.168.43.241'; // Ensure this is the correct IP

// Retry function for making multiple connection attempts
const retryAxios = async (url, retries = 3) => {
  try {
    const response = await axios.get(url);
    return response;
  } catch (error) {
    if (retries > 0) {
      console.log(`Retrying connection to ${url}, attempts left: ${retries}`);
      return retryAxios(url, retries - 1);
    }
    throw error;
  }
};

// Proxy GET request to check ESP32 movement status
app.get('/api/status/movement', async (req, res) => {
  if (!movementESP32IP) {
    return res.status(500).json({ status: 'error', message: 'Movement ESP32 IP not configured' });
  }
  
  try {
    const response = await retryAxios(`${movementESP32IP}/status`);
    res.json(response.data);
  } catch (error) {
    console.error('Movement ESP32 not reachable:', error.message);
    res.status(500).json({ status: 'error', message: 'Movement ESP32 not reachable' });
  }
});

// Proxy GET request to check ESP32 arm status
app.get('/api/status/arm', async (req, res) => {
  console.log("Received request for /api/status/arm");
  try {
    const response = await retryAxios(`${armESP32IP}/status`);
    res.json(response.data);
  } catch (error) {
    console.error('Arm ESP32 not reachable:', error.message);
    res.status(500).json({ status: 'error', message: 'Arm ESP32 not reachable' });
  }
});

// Proxy POST request for movement commands
app.post('/api/move', async (req, res) => {
  const { dir, speed } = req.body;

  // Validate input
  if (!dir || typeof speed !== 'number') {
    return res.status(400).json({ status: 'error', message: 'Invalid input' });
  }

  try {
    const response = await axios.post(`${movementESP32IP}/move`, { dir, speed });
    res.json(response.data);
  } catch (error) {
    console.error('Movement command failed:', error.response ? error.response.data : error.message);
    res.status(500).json({ status: 'error', message: 'Movement command failed' });
  }
});

// Proxy POST request for arm commands
app.post('/api/arm', async (req, res) => {
  const { joint, value } = req.body;

  // Validate input
  if (!joint || typeof value !== 'number' || value < 0 || value > 180) {
    return res.status(400).json({ status: 'error', message: 'Invalid joint or value' });
  }

  try {
    const response = await axios.post(`${armESP32IP}/api/arm`, { joint, value });
    res.json(response.data);
  } catch (error) {
    console.error('Arm command failed:', error.response ? error.response.data : error.message);
    res.status(500).json({ status: 'error', message: 'Arm command failed' });
  }
});

// Start server
const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log(`Server is running on http://localhost:${PORT}`);
});

/*
import express from 'express';
import axios from 'axios';
import cors from 'cors';  // Import the CORS package

const app = express();

// Enable CORS globally (should come before other middleware)
app.use(cors());
app.use(express.json());

// Proxy GET request to ESP32
app.get('/api/status', (req, res) => {
  axios.get('http://192.168.43.204/api/status')
    .then(response => res.send(response.data))
    .catch(error => res.status(500).send('ESP32 not reachable'));
});

// Proxy POST request to ESP32
app.post('/api/move', (req, res) => {
    axios.post('http://192.168.43.204/api/move', req.body)
      .then(response => res.send(response.data))
      .catch(error => {
        console.error('Error details:', error.response ? error.response.data : error.message);
        res.status(500).send('Failed to send command to ESP32');
      });
  });
  

const port = process.env.PORT || 3000;
app.listen(port, () => {
  console.log(`Server running at http://localhost:${port}`);
});
*/