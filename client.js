const ws = new WebSocket('ws://localhost:3000');

ws.onopen = function () {
    console.log('Connected to WebSocket server');
};

ws.onmessage = function (event) {
    const data = event.data;
    // Update the HTML content with the received MQTT data
    document.getElementById('level').innerHTML =  data + "%";
};

