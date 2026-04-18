/**
 * SVTROBO Web Control - Main Application
 * Manages rosbridge connection and initializes modules.
 * Auto-connects on page load; manual disconnect only.
 */

const App = {
    ros: null,
    connected: false,
    serverUrl: '',
    _manualDisconnect: false,

    init() {
        // Derive rosbridge URL from current page host
        const url = `ws://${window.location.host}/ws`;
        document.getElementById('rosbridge-url').value = url;

        document.getElementById('connect-btn').addEventListener('click', () => this.toggleConnection());

        // Derive server URL for camera streams
        const loc = window.location;
        this.serverUrl = `${loc.protocol}//${loc.host}`;

        // Camera module does not depend on ROS, initialize immediately
        Camera.init(this.serverUrl);
        // Hardware status: show all devices immediately (HTTP-only polling)
        HardwareStatus.initStandalone();
        // IMU: initialized on connect, disabled on disconnect

        // Auto-connect on page load
        this.connect();
    },

    toggleConnection() {
        if (this.connected) {
            this.disconnect();
        } else {
            this._manualDisconnect = false;
            this.connect();
        }
    },

    connect() {
        const url = document.getElementById('rosbridge-url').value.trim();
        if (!url) return;

        this._connStartTime = Date.now();
        this._connError = null;
        this.setStatus('connecting', `正在连接 ${url} ...`);

        this.ros = new ROSLIB.Ros({ url: url });

        // Timeout: if no connection within 5s, report failure
        this._connTimer = setTimeout(() => {
            if (!this.connected) {
                this._connError = '连接超时，请检查 rosbridge 是否在目标地址运行';
                this.setStatus('error', this._connError);
                if (this.ros) this.ros.close();
            }
        }, 5000);

        this.ros.on('connection', () => {
            clearTimeout(this._connTimer);
            this.connected = true;
            this.setStatus('connected', '已连接');
            document.getElementById('connect-btn').textContent = '断开连接';
            // Initialize modules
            Chassis.init(this.ros);
            Lift.init(this.ros);
            StatusMonitor.init(this.ros);
            Diagnostics.init(this.ros);
            ChassisStatus.init(this.ros);
            HardwareStatus.init(this.ros);
            IMUStatus.init(this.ros);
            ControlMode.init(this.ros);
            Camera.setEnabled(true);
            DataRecord.enable();
        });

        this.ros.on('error', (error) => {
            clearTimeout(this._connTimer);
            // Capture error reason for the close handler to display
            const elapsed = ((Date.now() - this._connStartTime) / 1000).toFixed(1);
            if (error.target && error.target.readyState === WebSocket.CONNECTING) {
                this._connError = `连接被拒绝 — ${url} 无法到达 (耗时 ${elapsed}s)`;
            } else {
                this._connError = `连接错误: ${error.type || '未知'} (耗时 ${elapsed}s)`;
            }
            console.error('ROS connection error:', error);
        });

        this.ros.on('close', () => {
            clearTimeout(this._connTimer);
            const manual = this._manualDisconnect;
            const wasConnected = this.connected;
            this.connected = false;
            this._manualDisconnect = false;
            document.getElementById('connect-btn').textContent = '连接';

            if (manual || wasConnected) {
                this.setStatus('disconnected', '已断开连接');
            } else if (this._connError) {
                this.setStatus('error', this._connError);
            } else {
                const elapsed = ((Date.now() - this._connStartTime) / 1000).toFixed(1);
                this.setStatus('error', `连接失败，请检查地址是否正确以及 rosbridge 是否在运行 (耗时 ${elapsed}s)`);
            }

            Chassis.disable();
            Lift.disable();
            StatusMonitor.disable();
            Diagnostics.disable();
            ChassisStatus.disable();
            HardwareStatus.disable();
            IMUStatus.disable();
            ControlMode.disable();
            Camera.setEnabled(false);
            DataRecord.disable();
        });
    },

    disconnect() {
        this._manualDisconnect = true;
        if (this.ros) {
            this.ros.close();
            this.ros = null;
        }
        this.connected = false;
        this.setStatus('disconnected', '已断开连接');
        document.getElementById('connect-btn').textContent = '连接';
    },

    setStatus(status, detail) {
        const el = document.getElementById('connection-status');
        const map = {
            'disconnected': { text: '已断开', cls: 'status-disconnected' },
            'connecting':   { text: '连接中...', cls: 'status-connecting' },
            'connected':    { text: '已连接', cls: 'status-connected' },
            'error':        { text: '连接失败', cls: 'status-error' },
        };
        const info = map[status] || map['disconnected'];
        el.textContent = detail || info.text;
        el.title = detail || '';
        el.className = 'connection-status ' + info.cls;
    },
};

document.addEventListener('DOMContentLoaded', () => {
    App.init();
});
