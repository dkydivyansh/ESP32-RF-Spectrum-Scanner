<div align="center">

# ESP32 RF Spectrum Scanner & Sonifier

</div>

## Why This Project?

In our increasingly wireless world, the 2.4 GHz band is crowded with signals from Wi-Fi, Bluetooth, Zigbee, and countless other devices. Understanding this invisible environment is crucial for hobbyists, network administrators, and security researchers. This project provides a powerful, accessible, and low-cost tool to not only visualize the RF spectrum but also to *hear* it, offering a unique and intuitive way to identify and analyze wireless activity.

## What It Does

This project transforms an ESP32 and a common NRF24L01+ module into a sophisticated, web-based RF spectrum scanner. It continuously scans the 2.4 GHz band (2400-2525 MHz), measures signal strength, and serves a feature-rich web interface to display the data in real-time.

### Key Features

* **Real-time Spectrum Visualization:** A line chart displays live signal strength across the frequency band.
* **Waterfall Spectrogram:** A heatmap shows a history of signal activity over time, making it easy to spot frequency-hopping signals like Bluetooth.
* **Audio Sonification (The "Sonifier"):** The most unique feature. The scanner converts RF signal data into audible musical notes.
    * **Frequency → Pitch:** The RF frequency of a signal maps to a musical note's pitch.
    * **Signal Strength → Volume:** The strength of a signal maps to the note's volume.
    * **Musical Scales:** You can quantize the output to Major, Minor Pentatonic, or Chromatic scales for a more musical and analytical listening experience.
* **Interactive Web Interface:** A clean, modern UI allows for full control over the scanner.
* **Configurable Parameters:** Adjust RF settings (power, data rate), scan parameters (speed, sensitivity, averaging), and display options on the fly.
* **Self-Contained & Offline-Ready:** The ESP32 hosts its own Wi-Fi AP and serves all necessary files (HTML, JS libraries), requiring no internet connection to operate.

## Technologies Used

### Hardware
* **ESP32:** The core microcontroller, providing Wi-Fi, processing power, and GPIO.
* **NRF24L01+PA+LNA:** A low-cost 2.4 GHz transceiver module used to detect RF energy. The PA (Power Amplifier) and LNA (Low-Noise Amplifier) variants are recommended for better sensitivity.

### Software & Libraries
* **Arduino Framework:** For programming the ESP32.
* **ESPAsyncWebServer & AsyncTCP:** For creating a high-performance, non-blocking web server and WebSocket connection.
* **ArduinoJson:** For efficient serialization and deserialization of data between the ESP32 and the web client.
* **RF24 Library:** The driver for the NRF24L01+ module.
* **LittleFS:** A lightweight filesystem for storing web assets (`plotly.js`, `tone.js`) on the ESP32's flash memory.
* **Plotly.js:** A powerful JavaScript charting library used for the spectrum and waterfall visualizations.
* **Tone.js:** A Web Audio framework for creating interactive music and sound in the browser, powering the sonification feature.
* **HTML, CSS, JavaScript:** For the modern, responsive web interface.

## Circuit Diagram & Build Notes
![pins Diagram](https://raw.githubusercontent.com/dkydivyansh/ESP32-RF-Spectrum-Scanner/main/images/pins.png)

**Wiring:**
*(Detail your ESP32 to NRF24L01+ pin connections here, e.g., VCC -> 3.3V, GND -> GND, CSN -> D27, etc.)*

**Important Build Note:** The NRF24L01+ module is sensitive to power fluctuations. To ensure a stable power supply and reduce noise, **it is highly recommended to add a decoupling capacitor (e.g., 10µF) between the VCC and GND pins** of the NRF24L01+ module, as close to the module as possible.

## How It Works

1.  **RF Scanning:** The ESP32 continuously sweeps through the 2.4 GHz channels. On each channel, it uses the NRF24L01's `testRPD()` (Received Power Detector) function to measure the signal strength.
2.  **Data Aggregation:** The results of each sweep are collected. The firmware supports averaging multiple sweeps to create a cleaner signal reading.
3.  **WebSocket Communication:** The ESP32 runs a WebSocket server. After each sweep (or averaging cycle), it sends a JSON payload containing the 125 signal strength values to all connected web clients.
4.  **Web-Based Visualization:** The client-side JavaScript receives the JSON data. It uses Plotly.js to update the spectrum chart and scroll the waterfall display with the new data.
5.  **Audio Sonification Pipeline:**
    * **Peak Finding:** To avoid a wall of noise, the JavaScript first identifies the most significant signal peaks that are above a user-defined threshold.
    * **Data-to-Audio Mapping:** Each identified peak is mapped to the properties of a musical note:
        * The RF frequency (2400-2525 MHz) is mapped to a musical pitch across several octaves.
        * The signal strength is mapped to the note's velocity (volume).
    * **Scale Quantization:** The calculated pitch is then "snapped" to the nearest note in the user-selected musical scale (e.g., Minor Pentatonic).
    * **Audio Synthesis:** Tone.js uses an FM Synthesizer to generate the notes, which are scheduled and played in a timed loop, creating a continuous audio stream representing the live RF environment.

## How to Use

1.  **Clone the Repository:**
    ```bash
    git clone https://github.com/dkydivyansh/ESP32-RF-Spectrum-Scanner.git
    ```
2.  **Upload Files to LittleFS:**
    * Using the [Arduino LittleFS Uploader Tool - Arduino IDE 2.2.1 or higher](https://github.com/earlephilhower/arduino-littlefs-upload), upload the entire `data` directory to your ESP32. This directory must contain:
        * `plotly-rf-scanner.min.js`
        * `tone.js`
3.  **Flash the Firmware:**
    * Open `rg-ana.ino` in the Arduino IDE.
    * Install the required libraries (ESPAsyncWebServer, AsyncTCP, ArduinoJson, RF24).
    * Select your ESP32 board and COM port.
    * Compile and upload the sketch.
4.  **Connect and Scan:**
    * On your computer or phone, search for a Wi-Fi network named **"RF-Analyser-Pro"** and connect to it using the password **"password"**.
    * Open a web browser and navigate to **`192.168.4.1`**.
    * The RF Spectrum Scanner interface should load, and you can begin visualizing and sonifying the airwaves!

## Gallery (Screenshots & Device)

*(Add screenshots of your web interface and final device here)*

![Spectrum View](https://raw.githubusercontent.com/dkydivyansh/ESP32-RF-Spectrum-Scanner/66d538593cd72311e492d34a8d5b6a9f5a227f4d/images/web-gui1.png)
*Caption: Real-time spectrum view showing active signals.*

![Waterfall View](https://raw.githubusercontent.com/dkydivyansh/ESP32-RF-Spectrum-Scanner/main/images/web-gui-waterfall.png)
*Caption: Waterfall spectrogram identifying a frequency-hopping signal.*

![Final Device](https://raw.githubusercontent.com/dkydivyansh/ESP32-RF-Spectrum-Scanner/main/images/pcb-diy.jpg)
*Caption: The completed DIY device.*
![DIY](https://raw.githubusercontent.com/dkydivyansh/ESP32-RF-Spectrum-Scanner/main/images/diy.jpg)
