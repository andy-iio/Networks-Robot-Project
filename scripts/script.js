const connectForm = document.getElementById('connectForm');
const robotIPInput = document.getElementById('robotIP');
const robotPortInput = document.getElementById('robotPort');
const driveButton = document.getElementById('driveBtn');
const sleepButton = document.getElementById('sleepBtn');
const responseButton = document.getElementById('responseBtn');
const telemetryOutput = document.getElementById('telemetryOutput');
const ackOutput = document.getElementById('ackOutput');

function updateTelemetry(text) {
    console.log("Updating telemetry:", text);
    telemetryOutput.textContent = text;
}

function updateAck(text) {
    console.log("Updating ACK:", text);
    ackOutput.textContent = text;
}

// --- Event Listeners ---

connectForm.addEventListener('submit', async (event) => {
    event.preventDefault();
    const ipAddress = robotIPInput.value.trim();
    const port = robotPortInput.value.trim();
    if (!ipAddress || !port) {
        updateTelemetry("Please enter both IP address and port.");
        return;
    }
    const connectUrl = `/connect/${encodeURIComponent(ipAddress)}/${encodeURIComponent(port)}`;
    updateTelemetry(`Connecting to ${ipAddress}:${port}...`);
    updateAck("Waiting for connection...");

    try {
        const response = await fetch(connectUrl, {
            method: 'POST'
        });
        const result = await response.text();
        if (!response.ok) {
            updateTelemetry(`Error connecting: ${response.status} ${response.statusText}\n${result}`);
            updateAck("Connection failed.");
            console.error('Server error:', response.status, response.statusText, result);
        } else {
            updateTelemetry(`Connection request sent. Server response:\n${result}`);
            updateAck("Connection successful (request sent).");
            console.log("Server response:", result);
        }
    } catch (error) {
        updateTelemetry(`Network error during connection: ${error}`);
        updateAck("Connection failed (network error).");
        console.error('Network error:', error);
    }
});

// Drive Button Click - Updated for PUT with JSON Body
driveButton.addEventListener('click', async () => {
    console.log('Drive button clicked');
    updateTelemetry('Sending DRIVE command...');
    updateAck('Waiting for ACK...');

    const direction = document.getElementById('driveDirection').value;
    const speed = document.getElementById('driveSpeed').value;
    const duration = document.getElementById('driveDuration').value;

    if (!direction || speed === '' || duration === '') {
        updateTelemetry("Please ensure Direction, Speed, and Duration are set.");
        return;
    }

    const commandUrl = '/telecommand/'; // Target the single telecommand route

    // Prepare data object for JSON body
    const commandData = {
        command: 'drive',
        direction: parseInt(direction, 10), // Ensure sending numbers
        speed: parseInt(speed, 10),
        duration: parseInt(duration, 10)
    };

    console.log('Sending PUT request to:', commandUrl, 'with body:', commandData);

    try {
        const response = await fetch(commandUrl, {
            method: 'PUT', //PUT
            headers: {
                'Content-Type': 'application/json', // Specify JSON content
            },
            body: JSON.stringify(commandData) // Send data in the body
        });

        const result = await response.text();

        if (!response.ok) {
            updateTelemetry(`Error sending drive command: ${response.status} ${response.statusText}\n${result}`);
            updateAck('Drive command failed.');
            console.error('Server error:', response.status, response.statusText, result);
        } else {
            updateTelemetry(`Drive command sent. \nResponse: ${result}`);
            updateAck('Drive command acknowledged/processed.');
            console.log("Server response:", result);
        }
    } catch (error) {
        updateTelemetry(`Network error sending drive command: ${error}`);
        updateAck('Drive command failed (network error).');
        console.error('Network error:', error);
    }
});

//PUT
sleepButton.addEventListener('click', async () => {
    console.log('Sleep button clicked');
    updateTelemetry('Sending SLEEP command...');
    updateAck('Waiting for ACK...');
    const commandUrl = '/telecommand/';

    try {
        const response = await fetch(commandUrl, {
            method: 'PUT',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({ command: 'sleep' }) // Send command in the body
        });

        const result = await response.text();

        if (!response.ok) {
            updateTelemetry(`Error sending sleep command: ${response.status} ${response.statusText}\n${result}`);
            updateAck('Sleep command failed.');
            console.error('Server error:', response.status, response.statusText, result);
        } else {
            updateTelemetry(`Sleep command sent. \nResponse: ${result}`);
            updateAck('Sleep command acknowledged.');
            console.log("Server response:", result);
        }
    } catch (error) {
        updateTelemetry(`Network error sending sleep command: ${error}`);
        updateAck('Sleep command failed (network error).');
        console.error('Network error:', error);
    }
});

// Response Button Click (Telemetry Request) - GET
responseButton.addEventListener('click', async () => {
    console.log('Response button clicked');
    updateTelemetry('Requesting telemetry data...');
    updateAck('Telemetry requested...');
    try {
        const response = await fetch('/telementry_request/', {
            method: 'GET'
        });

        if (!response.ok) {
            throw new Error(`HTTP error! Status: ${response.status}`);
        }

        const data = await response.text();
        document.getElementById('telemetryOutput').textContent = data;
    } catch (error) {
        console.error('Error fetching telemetry data:', error);
        document.getElementById('telemetryOutput').textContent = 'Error: ' + error.message;
    }
});

// --- Initial State ---
updateTelemetry("Interface loaded. Enter Robot IP and Port, then click Connect.");
updateAck("Waiting for connection...");