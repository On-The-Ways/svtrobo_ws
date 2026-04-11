/**
 * SVTROBO Web Control - Camera Feed Module
 */

const Camera = {
    serverUrl: '',
    cameras: ['d405_1', 'd405_2', 'zed'],

    init(serverUrl) {
        this.serverUrl = serverUrl;
        this.setupButtons();
        this.refreshStatus();
        // Auto-refresh status every 5s
        setInterval(() => this.refreshStatus(), 5000);
    },

    setupButtons() {
        for (const name of this.cameras) {
            const safeId = name.replace('_', '-');
            const startBtn = document.getElementById(`cam-${safeId}-start`);
            const stopBtn = document.getElementById(`cam-${safeId}-stop`);
            if (startBtn) startBtn.addEventListener('click', () => this.startCamera(name));
            if (stopBtn) stopBtn.addEventListener('click', () => this.stopCamera(name));
        }
    },

    async startCamera(name) {
        try {
            const resp = await fetch(`${this.serverUrl}/camera/start`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ camera: name }),
            });
            const data = await resp.json();
            const safeId = name.replace('_', '-');
            const fallback = document.getElementById(`cam-${safeId}-fallback`);
            const img = document.getElementById(`cam-${safeId}-feed`);

            if (data.ok) {
                // Set the img src to start streaming
                if (img) img.src = `${this.serverUrl}/camera/${name}`;
                if (fallback) fallback.style.display = 'none';
            } else {
                if (img) img.src = '';
                if (fallback) {
                    fallback.textContent = '无法启动相机';
                    fallback.style.display = 'block';
                }
            }
            this.refreshStatus();
        } catch (e) {
            console.error('Start camera error:', e);
            const safeId = name.replace('_', '-');
            const fallback = document.getElementById(`cam-${safeId}-fallback`);
            if (fallback) {
                fallback.textContent = '请求失败';
                fallback.style.display = 'block';
            }
        }
    },

    async stopCamera(name) {
        try {
            const safeId = name.replace('_', '-');
            const img = document.getElementById(`cam-${safeId}-feed`);
            const fallback = document.getElementById(`cam-${safeId}-fallback`);
            if (img) img.src = '';
            if (fallback) {
                fallback.textContent = '相机已关闭';
                fallback.style.display = 'block';
            }

            await fetch(`${this.serverUrl}/camera/stop`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ camera: name }),
            });
            this.refreshStatus();
        } catch (e) {
            console.error('Stop camera error:', e);
        }
    },

    async refreshStatus() {
        try {
            const resp = await fetch(`${this.serverUrl}/camera/status`);
            const status = await resp.json();
            for (const name of this.cameras) {
                const safeId = name.replace('_', '-');
                const indicator = document.getElementById(`cam-${safeId}-status`);
                const fallback = document.getElementById(`cam-${safeId}-fallback`);
                const img = document.getElementById(`cam-${safeId}-feed`);

                if (indicator) {
                    const running = status[name]?.running;
                    indicator.textContent = running ? '运行中' : '已停止';
                    indicator.className = 'cam-status ' + (running ? 'cam-running' : 'cam-stopped');

                    if (!running && fallback && img) {
                        img.src = '';
                        fallback.textContent = '相机未启动';
                        fallback.style.display = 'block';
                    } else if (running && fallback) {
                        fallback.style.display = 'none';
                    }
                }
            }
        } catch (e) {
            // Server might not be reachable
        }
    },
};
