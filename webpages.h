#ifndef WEBPAGES_H
#define WEBPAGES_H

#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>RF Spectrum Scanner</title>
    <script src="/plotly-rf-scanner.min.js"></script>
    <!-- Tone.js library loaded from LittleFS -->
    <script src="/tone.js"></script>
    <style>
        :root {
            --bg-color: #121212;
            --surface-color: #1e1e1e;
            --primary-color: #ffffff;
            --secondary-color: #bbbbbb;
            --border-color: #333333;
            --error-color: #cf6679;
            --success-color: #66bb6a;
            --font-sans: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
            --font-mono: 'Fira Code', 'Consolas', 'Monaco', monospace;
        }
        html { height: 100%; }
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { font-family: var(--font-sans); background-color: var(--bg-color); color: var(--secondary-color); min-height: 100%; overflow-x: hidden; }
        .container { display: flex; flex-direction: column; padding: 1rem; gap: 1rem; min-height: 100vh; }
        .header { text-align: left; padding-bottom: 1rem; border-bottom: 1px solid var(--border-color); }
        .header h1 { color: var(--primary-color); font-size: 1.5rem; font-weight: 600; margin-bottom: 0.25rem; }
        .header .credits { font-size: 0.875rem; }
        .header .credits a { color: var(--secondary-color); text-decoration: none; }
        .header .credits a:hover { color: var(--primary-color); }
        .info-note { font-size: 0.8rem; color: var(--secondary-color); margin-top: 0.5rem; padding-left: 0.2rem; }
        .main-grid { display: grid; grid-template-columns: 1fr; gap: 1rem; flex-grow: 1; }
        @media (min-width: 1024px) {
            .main-grid { grid-template-columns: 320px 1fr; grid-template-rows: auto 1fr; }
            .status-grid { grid-column: 1 / -1; }
        }
        .status-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(150px, 1fr)); gap: 1rem; }
        .status-item { background-color: var(--surface-color); border: 1px solid var(--border-color); border-radius: 8px; padding: 1rem; }
        .status-item .label { font-size: 0.875rem; margin-bottom: 0.25rem; color: var(--secondary-color); }
        .status-item .value { font-family: var(--font-mono); font-size: 1.25rem; font-weight: 500; color: var(--primary-color); }
        .control-panel, .visualization-panel { background-color: var(--surface-color); border: 1px solid var(--border-color); border-radius: 8px; padding: 1rem; }
        .control-group { margin-bottom: 1.5rem; }
        .control-group:last-child { margin-bottom: 0; }
        .control-group h3 { color: var(--primary-color); margin-bottom: 1rem; font-size: 1rem; font-weight: 600; border-bottom: 1px solid var(--border-color); padding-bottom: 0.5rem; }
        .control-item { margin-bottom: 1rem; }
        .control-item label { display: block; margin-bottom: 0.5rem; font-size: 0.875rem; }
        .control-item input, .control-item select { width: 100%; padding: 0.75rem; background-color: var(--bg-color); border: 1px solid var(--border-color); border-radius: 6px; color: var(--primary-color); font-size: 1rem; }
        .control-item input:focus, .control-item select:focus { outline: none; border-color: var(--primary-color); }
        .range-container { position: relative; }
        .range-value { position: absolute; right: 0; top: 0; font-family: var(--font-mono); color: var(--primary-color); }
        .button-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 0.5rem; }
        .btn { padding: 0.75rem 1rem; border: 1px solid var(--border-color); border-radius: 6px; cursor: pointer; font-weight: 600; transition: all 0.2s ease; background-color: var(--surface-color); color: var(--secondary-color); }
        .btn:hover { background-color: var(--primary-color); color: var(--bg-color); border-color: var(--primary-color); }
        .btn:disabled { opacity: 0.4; cursor: not-allowed; background-color: var(--surface-color); color: var(--secondary-color); border-color: var(--border-color); }
        .tab-container { display: flex; margin-bottom: 1rem; border-bottom: 1px solid var(--border-color); }
        .tab { flex: 0 1 auto; padding: 0.5rem 1rem; background: transparent; border: none; border-bottom: 2px solid transparent; color: var(--secondary-color); cursor: pointer; font-size: 1rem; transition: all 0.2s ease; }
        .tab.active { color: var(--primary-color); border-bottom-color: var(--primary-color); }
        .chart-container { min-height: 400px; height: 100%; }
        .pulse { animation: pulse 2s infinite; }
        @keyframes pulse { 0%, 100% { opacity: 1; } 50% { opacity: 0.6; } }
        
        .modal-overlay { position: fixed; top: 0; left: 0; width: 100%; height: 100%; background: rgba(0,0,0,0.7); display: none; justify-content: center; align-items: center; z-index: 1000; }
        .modal-content { background: var(--surface-color); padding: 2rem; border-radius: 8px; text-align: center; border: 1px solid var(--border-color); }
        .modal-content h2 { margin-bottom: 1rem; color: var(--primary-color); }
        .modal-content p { margin-bottom: 1.5rem; }
        .modal-overlay.visible { display: flex; }

        .toast { position: fixed; bottom: -100px; left: 50%; transform: translateX(-50%); background-color: var(--surface-color); color: var(--primary-color); padding: 1rem 2rem; border-radius: 8px; border: 1px solid var(--border-color); transition: bottom 0.5s ease-in-out; z-index: 1001; }
        .toast.show { bottom: 20px; }
        .toast.success { background-color: var(--success-color); color: #000; }
        .toast.fail { background-color: var(--error-color); color: #000; }
    </style>
</head>
<body>
    <div class="container">
        <header class="header">
            <h1>RF Spectrum Scanner</h1>
            <div class="credits">by <a href="https://dkydivyansh.com" target="_blank">dkydivyansh.com</a> | GitHub: <a href="https://github.com/dkydivyansh" target="_blank">@dkydivyansh</a></div>
            <p class="info-note">Note: Diagonal patterns in the waterfall view are normal and indicate frequency-hopping signals like Bluetooth.</p>
        </header>
        <main class="main-grid">
            <div class="status-grid">
                <div class="status-item"><div class="label">Connection</div><div class="value" id="connectionStatus">...</div></div>
                <div class="status-item"><div class="label">Radio Status</div><div class="value" id="scanningStatus">...</div></div>
                <div class="status-item"><div class="label">Peak Activity</div><div class="value" id="peakActivity">--</div></div>
                <div class="status-item"><div class="label">Total Signals</div><div class="value" id="totalSignals">0</div></div>
            </div>
            <aside class="control-panel">
                <div class="control-group">
                    <h3>Presets</h3>
                    <div class="button-grid">
                        <button class="btn" onclick="setPreset(event, 2412, 2484)">WiFi</button>
                        <button class="btn" onclick="setPreset(event, 2402, 2480)">Bluetooth</button>
                        <button class="btn" onclick="setPreset(event, 2405, 2480)">ZigBee</button>
                        <button class="btn" onclick="setPreset(event, 2400, 2525)">Full Band</button>
                    </div>
                </div>
                <div class="control-group">
                    <h3>RF Configuration</h3>
                    <div class="control-item"><label for="startFreq">Start Freq (MHz)</label><input type="number" id="startFreq" min="2400" max="2524"></div>
                    <div class="control-item"><label for="endFreq">End Freq (MHz)</label><input type="number" id="endFreq" min="2401" max="2525"></div>
                    <div class="control-item"><label for="powerLevel">Power Level</label><select id="powerLevel"><option value="0">MIN</option><option value="1">LOW</option><option value="2">HIGH</option><option value="3">MAX</option></select></div>
                    <div class="control-item"><label for="dataRate">Data Rate</label><select id="dataRate"><option value="0">250kbps</option><option value="1">1Mbps</option><option value="2">2Mbps</option></select></div>
                </div>
                <div class="control-group">
                    <h3>Scan Parameters</h3>
                    <div class="control-item"><label for="scanSpeed">Sweep Delay (ms)</label><div class="range-container"><span class="range-value" id="scanSpeedValue"></span><input type="range" id="scanSpeed" min="0" max="50"></div></div>
                    <div class="control-item"><label for="sensitivity">Sensitivity</label><div class="range-container"><span class="range-value" id="sensitivityValue"></span><input type="range" id="sensitivity" min="10" max="100"></div></div>
                    <div class="control-item"><label for="averaging">Averaging</label><div class="range-container"><span class="range-value" id="averagingValue"></span><input type="range" id="averaging" min="0" max="10"></div></div>
                </div>
                <div class="control-group">
                    <h3>Display</h3>
                    <div class="control-item">
                        <label for="colorscale">Waterfall Colors</label>
                        <select id="colorscale" onchange="updateColorscale(event)">
                            <option value="Viridis">Viridis</option>
                            <option value="Jet">Jet</option>
                            <option value="Hot">Hot</option>
                            <option value="Greys">Greys</option>
                            <option value="YlGnBu">YlGnBu</option>
                            <option value="Earth">Earth</option>
                        </select>
                    </div>
                </div>
                <!-- Audio Sonification Controls -->
                <div class="control-group">
                    <h3>Audio Sonification</h3>
                    <div class="button-grid">
                        <button id="startAudioBtn" class="btn">Start Audio</button>
                        <button id="stopAudioBtn" class="btn" disabled>Stop Audio</button>
                    </div>
                    <div class="control-item" style="margin-top: 1rem;">
                        <label for="volumeSlider">Volume</label>
                        <div class="range-container">
                            <span class="range-value" id="volumeValue">-10 dB</span>
                            <input type="range" id="volumeSlider" min="-40" max="0" value="-10">
                        </div>
                    </div>
                    <div class="control-item">
                        <label for="audioThreshold">Signal Threshold</label>
                        <div class="range-container">
                            <span class="range-value" id="audioThresholdValue">3</span>
                            <input type="range" id="audioThreshold" min="1" max="30" value="3">
                        </div>
                    </div>
                    <div class="control-item">
                        <label for="maxTones">Max Tones</label>
                        <div class="range-container">
                            <span class="range-value" id="maxTonesValue">4</span>
                            <input type="range" id="maxTones" min="1" max="10" value="4">
                        </div>
                    </div>
                </div>
                <div class="control-group">
                    <h3>Controls</h3>
                    <div class="button-grid">
                        <button id="pauseBtn" class="btn">Pause</button>
                        <button id="resumeBtn" class="btn" disabled>Resume</button>
                        <button id="clearBtn" class="btn">Clear</button>
                        <button id="resetBtn" class="btn">Reset</button>
                        <button id="reinitBtn" class="btn" style="grid-column: 1 / -1;">Re-init Radio</button>
                    </div>
                </div>
            </aside>
            <section class="visualization-panel">
                <div class="tab-container">
                    <button class="tab active" onclick="showVisualization(event, 'spectrum')">Spectrum</button>
                    <button class="tab" onclick="showVisualization(event, 'waterfall')">Waterfall</button>
                </div>
                <div class="chart-container">
                    <div id="spectrumChart" style="width:100%;height:100%;"></div>
                    <div id="waterfallChart" style="width:100%;height:100%;display:none;"></div>
                </div>
            </section>
        </main>
    </div>

    <div id="noDataModal" class="modal-overlay">
        <div class="modal-content">
            <h2>Connection Lost</h2>
            <p>No data received from the device. The NRF24L01 may have failed or disconnected.</p>
            <button id="modalReinitBtn" class="btn">Re-initialize</button>
        </div>
    </div>

    <div id="toast" class="toast"></div>

    <script>
        class ProfessionalRFScanner {
            constructor() {
                this.websocket = null; this.isConnected = false; this.isPaused = false;
                this.currentView = 'spectrum'; this.spectrumData = new Array(125).fill(0);
                this.maxWaterfallRows = 300;
                this.waterfallData = new Array(this.maxWaterfallRows).fill(new Array(125).fill(null));
                this.settings = {};
                this.dataTimeout = null; this.reconnectAttempts = 0;
                
                // Audio Sonification properties
                this.audioInitialized = false;
                this.synth = null;
                this.audioLoop = null;
                this.isAudioPlaying = false;

                this.initializeCharts();
                this.initializeWebSocket();
                this.attachEventListeners();
            }
            initializeCharts() {
                const frequencies = Array.from({length: 125}, (_, i) => 2400 + i);
                const commonLayout = {
                    plot_bgcolor: 'transparent', paper_bgcolor: 'transparent',
                    font: { color: 'var(--secondary-color)', family: 'var(--font-sans)' },
                    margin: { t: 40, r: 20, b: 40, l: 50 },
                    xaxis: { title: 'Frequency (MHz)', gridcolor: 'var(--border-color)' }
                };
                Plotly.newPlot('spectrumChart', 
                    [{ x: frequencies, y: this.spectrumData, type: 'scatter', mode: 'lines', line: { color: 'var(--primary-color)', width: 2 }, fill: 'tonexty', fillcolor: 'rgba(255, 255, 255, 0.1)' }], 
                    { ...commonLayout, title: { text: 'Real-time Spectrum', font: { color: 'var(--primary-color)' } }, yaxis: { title: 'Signal Count', gridcolor: 'var(--border-color)' } }, 
                    { responsive: true, displayModeBar: false }
                );
                
                Plotly.newPlot('waterfallChart', 
                    [{ z: this.waterfallData, x: frequencies, type: 'heatmap', colorscale: 'Viridis', showscale: true, zsmooth: 'best', colorbar: { title: 'Count', titleside: 'right', tickfont: { color: 'var(--secondary-color)' } } }], 
                    { ...commonLayout, title: { text: 'Waterfall Spectrogram', font: { color: 'var(--primary-color)' } }, yaxis: { title: 'Time', showticklabels: false } }, 
                    { responsive: true, displayModeBar: false }
                );
            }
            initializeWebSocket() {
                const wsUrl = `ws://${window.location.host}/ws`;
                this.websocket = new WebSocket(wsUrl);

                this.websocket.onopen = () => {
                    this.isConnected = true;
                    this.reconnectAttempts = 0;
                    this.updateConnectionStatus();
                    this.startDataTimeout();
                };

                this.websocket.onclose = () => {
                    this.isConnected = false;
                    clearTimeout(this.dataTimeout);
                    this.handleReconnect();
                };

                this.websocket.onmessage = (event) => {
                    try { const data = JSON.parse(event.data); this.handleMessage(data); } catch (error) { console.error('Error parsing message:', error, event.data); }
                };
            }
            handleReconnect() {
                this.reconnectAttempts++;
                const delay = Math.min(1000 * Math.pow(2, this.reconnectAttempts), 30000);
                this.updateConnectionStatus(delay);
                setTimeout(() => this.initializeWebSocket(), delay);
            }
            handleMessage(data) {
                this.resetDataTimeout(); 
                if (data.type === 'spectrum' && !this.isPaused) { 
                    this.spectrumData = data.data; // Update spectrum data for sonification
                    this.updateSpectrum(data.data); 
                    this.updateStatistics(data.statistics); 
                } 
                else if (data.type === 'settings') { this.receiveSettings(data.settings); }
                else if (data.type === 'reinitStatus') {
                    this.showToast(`Re-init ${data.status}`, data.status);
                }
            }
            receiveSettings(newSettings) {
                this.settings = newSettings;
                Object.keys(this.settings).forEach(key => {
                    const el = document.getElementById(key);
                    if (el) { el.value = this.settings[key]; if (el.type === 'range') el.dispatchEvent(new Event('input')); }
                });
            }
            updateSpectrum(newData) {
                document.getElementById('noDataModal').classList.remove('visible');
                document.getElementById('scanningStatus').textContent = this.isPaused ? 'Paused' : 'Active';
                
                this.waterfallData.pop();
                this.waterfallData.unshift(newData);
                
                if (this.currentView === 'spectrum') {
                    Plotly.restyle('spectrumChart', { y: [newData] }, [0]);
                } else if (this.currentView === 'waterfall') {
                    Plotly.restyle('waterfallChart', { z: [this.waterfallData] }, [0]);
                }
            }
            updateStatistics(stats) {
                if (!stats) return;
                document.getElementById('totalSignals').textContent = stats.total || 0;
                document.getElementById('peakActivity').textContent = stats.peak_value > 0 ? `${stats.peak_freq} MHz` : '--';
            }
            updateConnectionStatus(retryDelay = 0) {
                const statusEl = document.getElementById('connectionStatus');
                if (this.isConnected) {
                    statusEl.textContent = 'Connected';
                    statusEl.classList.remove('pulse');
                } else {
                    statusEl.textContent = `Retrying in ${Math.round(retryDelay / 1000)}s`;
                    statusEl.classList.add('pulse');
                }
            }
            attachEventListeners() {
                // General settings listeners
                ['scanSpeed', 'sensitivity', 'averaging', 'startFreq', 'endFreq', 'powerLevel', 'dataRate'].forEach(id => {
                    const el = document.getElementById(id);
                    if (el.type === 'range') el.addEventListener('input', e => this.updateRangeValue(e.target));
                    el.addEventListener('change', () => this.sendSettings());
                });

                // Audio settings listeners
                ['volumeSlider', 'audioThreshold', 'maxTones'].forEach(id => {
                     document.getElementById(id).addEventListener('input', e => this.updateRangeValue(e.target));
                });
                
                // Specific listener for volume to apply change immediately
                document.getElementById('volumeSlider').addEventListener('change', e => this.setVolume(e.target.value));


                // Button listeners
                document.getElementById('pauseBtn').addEventListener('click', () => this.setPaused(true));
                document.getElementById('resumeBtn').addEventListener('click', () => this.setPaused(false));
                document.getElementById('clearBtn').addEventListener('click', () => this.sendCommand('clear'));
                document.getElementById('resetBtn').addEventListener('click', () => this.sendCommand('reset'));
                
                // Audio button listeners
                document.getElementById('startAudioBtn').addEventListener('click', () => this.startAudio());
                document.getElementById('stopAudioBtn').addEventListener('click', () => this.stopAudio());


                const reinitAction = () => {
                    this.sendCommand('reinit');
                    document.getElementById('noDataModal').classList.remove('visible');
                };
                document.getElementById('reinitBtn').addEventListener('click', reinitAction);
                document.getElementById('modalReinitBtn').addEventListener('click', reinitAction);

                const startFreqEl = document.getElementById('startFreq'), endFreqEl = document.getElementById('endFreq');
                startFreqEl.addEventListener('change', () => { if (parseInt(startFreqEl.value) >= parseInt(endFreqEl.value)) endFreqEl.value = parseInt(startFreqEl.value) + 1; });
                endFreqEl.addEventListener('change', () => { if (parseInt(endFreqEl.value) <= parseInt(startFreqEl.value)) startFreqEl.value = parseInt(endFreqEl.value) - 1; });
            }
            setPaused(paused) {
                this.isPaused = paused;
                this.sendCommand(paused ? 'pause' : 'resume');
                document.getElementById('pauseBtn').disabled = paused;
                document.getElementById('resumeBtn').disabled = !paused;
                document.getElementById('scanningStatus').textContent = paused ? 'Paused' : 'Active';
            }
            updateRangeValue(element) {
                const id = element.id, value = element.value, valueEl = document.getElementById(id.replace('Slider', '') + 'Value');
                if (!valueEl) return;
                
                if (id === 'averaging') valueEl.textContent = value == 0 ? 'Off' : `${value}x`;
                else if (id === 'scanSpeed') valueEl.textContent = `${value} ms`;
                else if (id === 'sensitivity') valueEl.textContent = `${value}%`;
                else if (id === 'volumeSlider') valueEl.textContent = `${value} dB`;
                else if (id === 'audioThreshold' || id === 'maxTones') valueEl.textContent = value;
            }
            sendSettings() {
                ['startFreq', 'endFreq', 'powerLevel', 'dataRate', 'scanSpeed', 'sensitivity', 'averaging'].forEach(id => {
                    const el = document.getElementById(id);
                    this.settings[id] = el.type.match(/number|range/) ? parseInt(el.value) : el.value;
                });
                if (this.websocket?.readyState === WebSocket.OPEN) this.websocket.send(JSON.stringify({ type: 'settings', settings: this.settings }));
            }
            sendCommand(command) {
                if (this.websocket?.readyState === WebSocket.OPEN) this.websocket.send(JSON.stringify({ type: 'command', command: command }));
            }
            startDataTimeout() {
                this.dataTimeout = setTimeout(() => {
                    document.getElementById('noDataModal').classList.add('visible');
                    document.getElementById('scanningStatus').textContent = 'Failed';
                }, 3000);
            }
            resetDataTimeout() {
                clearTimeout(this.dataTimeout);
                this.startDataTimeout();
            }
            showToast(message, type = '') {
                const toast = document.getElementById('toast');
                toast.textContent = message;
                toast.className = 'toast show';
                if (type) toast.classList.add(type);
                setTimeout(() => { toast.className = 'toast'; }, 3000);
            }

            // --- REVISED & ROBUST Core Audio Methods ---
            async startAudio() {
                // Explicitly start the AudioContext on user gesture if it's not already running.
                if (Tone.context.state !== 'running') {
                    try {
                        await Tone.start();
                        console.log('AudioContext started successfully!');
                    } catch (e) {
                        console.error("Could not start AudioContext: ", e);
                        return; // Exit if we can't start audio
                    }
                }

                // Initialize the synth only once.
                if (!this.audioInitialized) {
                    this.synth = new Tone.PolySynth(Tone.Synth, {
                        oscillator: { type: 'sine' },
                        envelope: { attack: 0.01, decay: 0.1, sustain: 0.2, release: 0.2 }
                    }).toDestination();
                    this.audioInitialized = true;
                    this.setVolume(document.getElementById('volumeSlider').value);
                }

                if (this.isAudioPlaying) return;

                // A single, simple loop for processing and playing.
                this.audioLoop = new Tone.Loop(time => {
                    try {
                        const threshold = parseInt(document.getElementById('audioThreshold').value);
                        const maxPeaks = parseInt(document.getElementById('maxTones').value);
                        const peaks = this.findPeaks(this.spectrumData, threshold, maxPeaks);
                        
                        if (peaks.length > 0) {
                            const notes = peaks.map(p => ({
                                note: this.frequencyToNote(2400 + p.index),
                                velocity: this.strengthToVelocity(p.value)
                            }));
                            
                            // Play the notes immediately within the scheduled loop time.
                            notes.forEach(n => {
                                this.synth.triggerAttackRelease(n.note, "16n", time, n.velocity);
                            });
                        }
                    } catch (e) {
                        console.error("Error in audio loop:", e);
                    }
                }, "8n").start(0);

                Tone.Transport.start();
                this.isAudioPlaying = true;
                document.getElementById('startAudioBtn').disabled = true;
                document.getElementById('stopAudioBtn').disabled = false;
            }

            stopAudio() {
                if (!this.isAudioPlaying) return;
                Tone.Transport.stop();
                if (this.audioLoop) {
                    this.audioLoop.stop(0).dispose();
                    this.audioLoop = null;
                }
                if (this.synth) {
                    this.synth.releaseAll();
                }
                this.isAudioPlaying = false;
                document.getElementById('startAudioBtn').disabled = false;
                document.getElementById('stopAudioBtn').disabled = true;
            }

            setVolume(db) {
                if (this.synth) {
                    this.synth.volume.value = db;
                }
            }

            findPeaks(data, threshold, maxPeaks) {
                if (!data) return [];
                const peaks = [];
                for (let i = 0; i < data.length; i++) {
                    if (data[i] >= threshold) {
                        peaks.push({ index: i, value: data[i] });
                    }
                }
                return peaks.sort((a, b) => b.value - a.value).slice(0, maxPeaks);
            }

            frequencyToNote(freq) {
                const minFreq = 2400, maxFreq = 2525;
                const minNote = 48, maxNote = 108; // MIDI notes: C3 to C8
                const noteNum = ((freq - minFreq) / (maxFreq - minFreq)) * (maxNote - minNote) + minNote;
                return Tone.Frequency(noteNum, "midi").toNote();
            }

            strengthToVelocity(strength) {
                const minStrength = 1, maxStrength = 30; 
                const velocity = (strength - minStrength) / (maxStrength - minStrength);
                return Math.max(0, Math.min(1, velocity));
            }
        }
        let scanner;
        function setPreset(e, start, end) {
            document.getElementById('startFreq').value = start;
            document.getElementById('endFreq').value = end;
            scanner.sendSettings();
        }
        function showVisualization(e, type) {
            scanner.currentView = type;
            document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));
            e.target.classList.add('active');
            document.getElementById('spectrumChart').style.display = type === 'spectrum' ? 'block' : 'none';
            document.getElementById('waterfallChart').style.display = type === 'waterfall' ? 'block' : 'none';
            window.dispatchEvent(new Event('resize'));
        }
        function updateColorscale(e) {
            Plotly.restyle('waterfallChart', { colorscale: e.target.value });
        }
        document.addEventListener('DOMContentLoaded', () => { scanner = new ProfessionalRFScanner(); });
    </script>
</body>
</html>
)rawliteral";

#endif
