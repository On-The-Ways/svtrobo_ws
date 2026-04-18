/**
 * SVTROBO Web Control - IMU Status Module
 * Primary: WebSocket connection to /ws/imu (20Hz push)
 * Fallback: HTTP polling to /api/imu (10Hz)
 */

const IMUStatus = {
    topic: null,
    magTopic: null,
    tempTopic: null,
    count: 0,
    lastTime: 0,
    freqEl: null,
    pollTimer: null,
    useHttp: false,
    ws: null,
    wsReconnectTimer: null,
    wsConnected: false,

    init(ros) {
        this.freqEl = document.getElementById('imu-freq');
        // Try WebSocket first, fall back to HTTP
        this.connectWebSocket();
    },

    // --- WebSocket (primary) ---

    connectWebSocket() {
        if (this.ws && (this.ws.readyState === WebSocket.CONNECTING || this.ws.readyState === WebSocket.OPEN)) {
            return;
        }

        const protocol = location.protocol === 'https:' ? 'wss:' : 'ws:';
        const wsUrl = protocol + '//' + location.host + '/ws/imu';

        try {
            this.ws = new WebSocket(wsUrl);

            this.ws.onopen = () => {
                this.wsConnected = true;
                this.stopHttpPoll();
                if (this.wsReconnectTimer) {
                    clearTimeout(this.wsReconnectTimer);
                    this.wsReconnectTimer = null;
                }
            };

            this.ws.onmessage = (event) => {
                try {
                    const json = JSON.parse(event.data);
                    if (json.ok && json.data) {
                        this.updateFromHttp(json.data);
                    }
                } catch (e) {
                    // Ignore parse errors
                }
            };

            this.ws.onclose = () => {
                this.wsConnected = false;
                this.ws = null;
                // Fall back to HTTP polling
                this.startHttpPoll();
                // Auto-reconnect WebSocket after 3 seconds
                this.scheduleWsReconnect();
            };

            this.ws.onerror = () => {
                // onclose will fire after this, which handles fallback
            };
        } catch (e) {
            // WebSocket not supported, use HTTP
            this.startHttpPoll();
        }
    },

    scheduleWsReconnect() {
        if (this.wsReconnectTimer) return;
        this.wsReconnectTimer = setTimeout(() => {
            this.wsReconnectTimer = null;
            this.connectWebSocket();
        }, 3000);
    },

    // --- HTTP polling (fallback) ---

    startHttpPoll() {
        if (this.pollTimer) return;  // Already polling
        this.useHttp = true;
        this._poll();
        this.pollTimer = setInterval(() => this._poll(), 100);  // 10Hz
    },

    stopHttpPoll() {
        this.useHttp = false;
        if (this.pollTimer) {
            clearInterval(this.pollTimer);
            this.pollTimer = null;
        }
    },

    async _poll() {
        try {
            const resp = await fetch('/api/imu');
            const json = await resp.json();
            if (json.ok && json.data) {
                this.updateFromHttp(json.data);
            }
        } catch (e) {
            // Silently ignore
        }
    },

    updateFromHttp(data) {
        const a = data.accel;
        this.setVal('imu-acc-x', a[0], 3);
        this.setVal('imu-acc-y', a[1], 3);
        this.setVal('imu-acc-z', a[2], 3);

        const g = data.gyro_dps;
        this.setVal('imu-gyro-x', g[0], 2);
        this.setVal('imu-gyro-y', g[1], 2);
        this.setVal('imu-gyro-z', g[2], 2);

        const m = data.mag;
        this.setVal('imu-mag-x', m[0], 2);
        this.setVal('imu-mag-y', m[1], 2);
        this.setVal('imu-mag-z', m[2], 2);

        const temp = document.getElementById('imu-temp');
        if (temp) temp.textContent = data.imu_temp.toFixed(1) + ' °C';

        // Frequency estimation
        this.count++;
        const now = performance.now();
        if (this.lastTime > 0 && now - this.lastTime >= 1000) {
            const hz = (this.count * 1000 / (now - this.lastTime)).toFixed(0);
            if (this.freqEl) this.freqEl.textContent = hz + ' Hz';
            this.count = 0;
            this.lastTime = now;
        } else if (this.lastTime === 0) {
            this.lastTime = now;
        }
    },

    disable() {
        this.stopHttpPoll();
        if (this.ws) {
            this.ws.onclose = null;  // Prevent reconnect on intentional close
            this.ws.close();
            this.ws = null;
        }
        if (this.wsReconnectTimer) {
            clearTimeout(this.wsReconnectTimer);
            this.wsReconnectTimer = null;
        }
        const ids = ['imu-acc-x','imu-acc-y','imu-acc-z',
                      'imu-gyro-x','imu-gyro-y','imu-gyro-z',
                      'imu-mag-x','imu-mag-y','imu-mag-z'];
        for (const id of ids) {
            const el = document.getElementById(id);
            if (el) el.textContent = '--';
        }
        const temp = document.getElementById('imu-temp');
        if (temp) temp.textContent = '-- °C';
        const freq = document.getElementById('imu-freq');
        if (freq) freq.textContent = '-- Hz';
    },

    setVal(id, value, decimals) {
        const el = document.getElementById(id);
        if (el) {
            const v = typeof value === 'number' ? value : 0;
            el.textContent = (v >= 0 ? '+' : '') + v.toFixed(decimals);
        }
    },
};
