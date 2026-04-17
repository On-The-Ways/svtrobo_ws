/**
 * SVTROBO Web Control - IMU Status Module
 * Fetches IMU data from /api/imu HTTP endpoint (primary)
 * Falls back to rosbridge topics if HTTP not available
 */

const IMUStatus = {
    topic: null,
    magTopic: null,
    tempTopic: null,
    count: 0,
    lastTime: 0,
    freqEl: null,
    pollTimer: null,
    useHttp: true,

    init(ros) {
        this.freqEl = document.getElementById('imu-freq');
        // Try HTTP polling first
        this.startHttpPoll();
    },

    startHttpPoll() {
        this.useHttp = true;
        this._poll();
        this.pollTimer = setInterval(() => this._poll(), 100);  // 10Hz
    },

    stopHttpPoll() {
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
