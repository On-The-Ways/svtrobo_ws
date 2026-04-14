/**
 * SVTROBO Web Control - Main Application
 * Manages rosbridge connection and initializes modules.
 */

const App = {
    ros: null,
    connected: false,
    serverUrl: '',

    init() {
        // Load last used URL
        const savedUrl = localStorage.getItem('rosbridge_url');
        document.getElementById('rosbridge-url').value = savedUrl || 'ws://localhost:9090';

        document.getElementById('connect-btn').addEventListener('click', () => this.toggleConnection());
        document.getElementById('rosbridge-url').addEventListener('keydown', (e) => {
            if (e.key === 'Enter') this.toggleConnection();
        });

        // Derive server URL for camera streams
        const loc = window.location;
        this.serverUrl = `${loc.protocol}//${loc.host}`;

        // Camera module does not depend on ROS, initialize immediately
        Camera.init(this.serverUrl);
    },

    toggleConnection() {
        if (this.connected) {
            this.disconnect();
        } else {
            this.connect();
        }
    },

    connect() {
        const url = document.getElementById('rosbridge-url').value.trim();
        if (!url) return;

        localStorage.setItem('rosbridge_url', url);
        this.setStatus('connecting');

        this.ros = new ROSLIB.Ros({ url: url });

        this.ros.on('connection', () => {
            this.connected = true;
            this.setStatus('connected');
            document.getElementById('connect-btn').textContent = '断开连接';
            // Initialize modules
            Chassis.init(this.ros);
            Lift.init(this.ros);
            StatusMonitor.init(this.ros);
            Diagnostics.init(this.ros);
            ChassisStatus.init(this.ros);
            ControlMode.init(this.ros);
            Camera.setEnabled(true);
            DataRecord.enable();
        });

        this.ros.on('error', (error) => {
            console.error('ROS connection error:', error);
            this.setStatus('error');
        });

        this.ros.on('close', () => {
            this.connected = false;
            this.setStatus('disconnected');
            document.getElementById('connect-btn').textContent = '连接';
            Chassis.disable();
            Lift.disable();
            StatusMonitor.disable();
            Diagnostics.disable();
            ChassisStatus.disable();
            ControlMode.disable();
            Camera.setEnabled(false);
            DataRecord.disable();
        });
    },

    disconnect() {
        if (this.ros) {
            this.ros.close();
            this.ros = null;
        }
        this.connected = false;
        this.setStatus('disconnected');
        document.getElementById('connect-btn').textContent = '连接';
    },

    setStatus(status) {
        const el = document.getElementById('connection-status');
        const map = {
            'disconnected': { text: '已断开', cls: 'status-disconnected' },
            'connecting':   { text: '连接中...', cls: 'status-connecting' },
            'connected':    { text: '已连接', cls: 'status-connected' },
            'error':        { text: '错误', cls: 'status-error' },
        };
        const info = map[status] || map['disconnected'];
        el.textContent = info.text;
        el.className = 'connection-status ' + info.cls;
    },
};

document.addEventListener('DOMContentLoaded', () => {
    App.init();
});
