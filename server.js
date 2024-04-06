// Import required modules
const express = require('express'); // Import Express.js web application framework
const http = require('http'); // Import Node.js HTTP module
const WebSocket = require('ws'); // Import WebSocket library
const mqtt = require('mqtt'); // Import MQTT client library
const path = require('path'); // Import Node.js path module

// Create an Express application
const app = express();
// Create an HTTP server using Express
const server = http.createServer(app);
// Create a WebSocket server instance
const wss = new WebSocket.Server({ server });

// Connect to MQTT broker
const mqttClient = mqtt.connect('mqtt://172.20.10.3', {
    username: 'sd',
    password: 'a'
});

// Event handler when MQTT client connects to the broker
mqttClient.on('connect', () => {
    console.log('Connected to MQTT broker');
});

// Define MQTT topic to subscribe to
const topic = 'intravenous_bag/weight';
// Subscribe to the MQTT topic
mqttClient.subscribe(topic, (err) => {
    if (err) {
        console.error('Error subscribing to MQTT topic:', err);
    } else {
        console.log('Subscribed to MQTT topic:', topic);
    }
});

// Variable to store the latest MQTT message
let latestMessage = null;

// Event handler for incoming MQTT messages
mqttClient.on('message', (receivedTopic, message) => {
    // Check if the received topic matches the subscribed topic
    if (receivedTopic === topic) {
        // Store the latest MQTT message
        latestMessage = message.toString();
        // Log the received message
        console.log('Patient saline value:', latestMessage);
        // Broadcast the MQTT data to all connected WebSocket clients
        wss.clients.forEach(client => {
            // Check if the WebSocket connection is open
            if (client.readyState === WebSocket.OPEN) {
                // Send the latest MQTT message to the WebSocket client
                client.send(latestMessage);
            }
        });
    }
});

// Define route to serve signup.html on root URL
app.get("/", (req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'signup.html'));
});

// Define route to redirect to index.html on form submission
app.post("/submit",(req,res)=>{
    res.redirect("/index.html");
});

// Serve static files from the "public" directory
app.use(express.static('public'));

// Start the HTTP server
server.listen(3000, () => {
    console.log('Server listening on port 3000');
});
