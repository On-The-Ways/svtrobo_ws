/**
 * SVTROBO Web Control - Control Mode Toggle
 * Switch between web control and gamepad control (mutual exclusion).
 * Automatically starts/stops the f710_teleop ROS node via backend API.
 */

const ControlMode = {
    enableTopic: null,
    statusTopic: null,
    // 'web' = web keyboard+buttons active, gamepad disabled
    // 'gamepad' = gamepad active, web controls disabled
    mode: 'gamepad',
    _switching: false,

    init(ros) {
        this.enableTopic = new ROSLIB.Topic({
            ros: ros,
            name: '/f710/enable',
            messageType: 'std_msgs/Bool',
        });

        this.statusTopic = new ROSLIB.Topic({
            ros: ros,
            name: '/f710/status',
            messageType: 'std_msgs/Bool',
        });
        this.statusTopic.subscribe((msg) => {
            // Sync UI if F710 node reports its actual state
            const gamepadActive = msg.data;
            if (gamepadActive && this.mode !== 'gamepad') {
                this.mode = 'gamepad';
                this.applyMode();
            } else if (!gamepadActive && this.mode !== 'web') {
                this.mode = 'web';
                this.applyMode();
            }
        });

        this.applyMode();

        // 默认手柄模式：启动 f710 节点并使能
        fetch(`${App.serverUrl}/f710/start`, { method: 'POST' })
            .then(r => r.json())
            .then(() => {
                setTimeout(() => {
                    this.enableTopic.publish(new ROSLIB.Message({ data: true }));
                }, 500);
            })
            .catch(e => console.warn('F710 auto-start failed:', e));
    },

    async toggle() {
        if (this._switching) return;
        this._switching = true;

        const newMode = this.mode === 'web' ? 'gamepad' : 'web';

        try {
            if (newMode === 'gamepad') {
                // Start f710 node first, then enable
                const resp = await fetch(`${App.serverUrl}/f710/start`, { method: 'POST' });
                const data = await resp.json();
                if (!data.ok && !data.message.includes('already running')) {
                    console.error('Failed to start F710 node:', data.message);
                    this._switching = false;
                    return;
                }
                // Give the node a moment to initialize
                await new Promise(r => setTimeout(r, 500));
            }

            this.mode = newMode;
            this.applyMode();

            if (this.enableTopic) {
                this.enableTopic.publish(new ROSLIB.Message({ data: this.mode === 'gamepad' }));
            }

            if (newMode === 'web') {
                // Stop f710 node after disabling
                await fetch(`${App.serverUrl}/f710/stop`, { method: 'POST' });
            }
        } catch (e) {
            console.error('F710 toggle error:', e);
        } finally {
            this._switching = false;
        }
    },

    applyMode() {
        this.updateUI();
        // Always send stop commands on mode switch (both directions)
        this._emergencyStop();
        if (this.mode === 'gamepad') {
            Chassis.disable();
            Lift.disable();
        } else {
            Chassis.enable();
            Lift.enable();
        }
    },

    _emergencyStop() {
        // Stop chassis
        if (Chassis.cmdTopic) {
            Chassis.cmdTopic.publish(new ROSLIB.Message({
                linear: { x: 0, y: 0, z: 0 },
                angular: { x: 0, y: 0, z: 0 },
            }));
        }
        // Stop lift
        if (Lift.liftTopic) {
            Lift.liftTopic.publish(new ROSLIB.Message({ data: [0, 0] }));
        }
    },

    updateUI() {
        const optWeb = document.getElementById('mode-opt-web');
        const optGamepad = document.getElementById('mode-opt-gamepad');
        const mask = document.getElementById('web-control-mask');

        if (this.mode === 'gamepad') {
            optWeb.className = 'toggle-option web';
            optGamepad.className = 'toggle-option gamepad active';
            if (mask) mask.style.display = 'flex';
        } else {
            optWeb.className = 'toggle-option web active';
            optGamepad.className = 'toggle-option gamepad';
            if (mask) mask.style.display = 'none';
        }
    },
};

document.addEventListener('DOMContentLoaded', () => {
    document.getElementById('control-mode-toggle').addEventListener('click', () => {
        ControlMode.toggle();
    });
    ControlMode.updateUI();
});

// Alias for backward compatibility with app.js
const F710Toggle = ControlMode;
