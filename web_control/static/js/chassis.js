/**
 * SVTROBO Web Control - Chassis Control Module
 * WASD/Q/E keyboard control, direction indicator, speed slider.
 */

const Chassis = {
    cmdTopic: null,
    feedbackTopic: null,
    speed: 0.3,
    rotSpeed: 0.5,
    activeKeys: new Set(),
    publishInterval: null,
    currentVx: 0,
    currentVy: 0,
    currentWz: 0,

    // Key mapping: key code -> {vx, vy, wz} direction unit vectors
    keyMap: {
        'KeyW': { vx:  1, vy:  0, wz:  0 },   // Forward
        'KeyS': { vx: -1, vy:  0, wz:  0 },   // Backward
        'KeyA': { vx:  0, vy:  1, wz:  0 },   // Left
        'KeyD': { vx:  0, vy: -1, wz:  0 },   // Right
        'KeyQ': { vx:  0, vy:  0, wz:  1 },   // Rotate CCW
        'KeyE': { vx:  0, vy:  0, wz: -1 },   // Rotate CW
    },

    init(ros) {
        // Publisher for chassis commands
        this.cmdTopic = new ROSLIB.Topic({
            ros: ros,
            name: '/svtrobot_cmd',
            messageType: 'geometry_msgs/Twist',
        });

        // Subscriber for command feedback
        this.feedbackTopic = new ROSLIB.Topic({
            ros: ros,
            name: '/chassis/cmd_feedback',
            messageType: 'geometry_msgs/Twist',
        });
        this.feedbackTopic.subscribe((msg) => {
            this.currentVx = msg.linear.x;
            this.currentVy = msg.linear.y;
            this.currentWz = msg.angular.z;
            this.updateFeedbackDisplay();
        });

        this.setupKeyboard();
        this.setupSpeedSlider();
        this.updateFeedbackDisplay();
    },

    setupKeyboard() {
        const panel = document.getElementById('chassis-panel');

        // Focus management
        panel.addEventListener('click', () => {
            panel.focus();
        });
        panel.addEventListener('focus', () => {
            document.getElementById('keyboard-hint').style.display = 'none';
        });
        panel.addEventListener('blur', () => {
            document.getElementById('keyboard-hint').style.display = 'block';
            this.stopAll();
        });

        panel.addEventListener('keydown', (e) => {
            if (e.code === 'Space') {
                e.preventDefault();
                this.stopAll();
                return;
            }
            if (this.keyMap[e.code]) {
                e.preventDefault();
                if (!this.activeKeys.has(e.code)) {
                    this.activeKeys.add(e.code);
                    this.updateDirection();
                    this.startPublishing();
                }
            }
        });

        panel.addEventListener('keyup', (e) => {
            if (this.keyMap[e.code]) {
                e.preventDefault();
                this.activeKeys.delete(e.code);
                this.updateDirection();
                if (this.activeKeys.size === 0) {
                    this.stopAll();
                }
            }
        });

        // Make panel focusable
        panel.setAttribute('tabindex', '0');
    },

    setupSpeedSlider() {
        const slider = document.getElementById('speed-slider');
        const display = document.getElementById('speed-value');
        slider.addEventListener('input', () => {
            this.speed = parseFloat(slider.value);
            display.textContent = this.speed.toFixed(2);
        });
    },

    computeVelocity() {
        let vx = 0, vy = 0, wz = 0;
        for (const key of this.activeKeys) {
            const dir = this.keyMap[key];
            if (dir) {
                vx += dir.vx;
                vy += dir.vy;
                wz += dir.wz;
            }
        }
        // Normalize if combined magnitude > 1
        const mag = Math.sqrt(vx * vx + vy * vy);
        if (mag > 1) {
            vx /= mag;
            vy /= mag;
        }
        return {
            vx: vx * this.speed,
            vy: vy * this.speed,
            wz: wz * this.rotSpeed,
        };
    },

    publish() {
        if (!this.cmdTopic || this.activeKeys.size === 0) return;
        const vel = this.computeVelocity();
        const msg = new ROSLIB.Message({
            linear: { x: vel.vx, y: vel.vy, z: 0 },
            angular: { x: 0, y: 0, z: vel.wz },
        });
        this.cmdTopic.publish(msg);
    },

    startPublishing() {
        if (this.publishInterval) return;
        this.publish();
        this.publishInterval = setInterval(() => this.publish(), 50); // 20Hz
    },

    stopAll() {
        if (this.publishInterval) {
            clearInterval(this.publishInterval);
            this.publishInterval = null;
        }
        this.activeKeys.clear();
        this.updateDirection();
        // Send zero velocity
        if (this.cmdTopic) {
            this.cmdTopic.publish(new ROSLIB.Message({
                linear: { x: 0, y: 0, z: 0 },
                angular: { x: 0, y: 0, z: 0 },
            }));
        }
    },

    updateDirection() {
        // Clear all highlights
        document.querySelectorAll('.dir-segment, .rot-indicator').forEach(el => el.classList.remove('active'));

        if (this.activeKeys.size === 0) return;

        // Determine combined direction for display
        let vx = 0, vy = 0, wz = 0;
        for (const key of this.activeKeys) {
            const dir = this.keyMap[key];
            if (dir) { vx += dir.vx; vy += dir.vy; wz += dir.wz; }
        }

        // Map to direction segments
        if (wz > 0) this.activateDir('ccw');
        if (wz < 0) this.activateDir('cw');

        // Map vx/vy to one of 8 directions
        if (Math.abs(vx) > 0.01 || Math.abs(vy) > 0.01) {
            const angle = Math.atan2(vy, vx); // angle from forward
            const idx = Math.round(angle / (Math.PI / 4));
            const dirNames = ['e', 'se', 's', 'sw', 'w', 'nw', 'n', 'ne'];
            // atan2: 0=forward, pi/2=left, etc.
            const dirMap = {
                0: 'n', 1: 'nw', 2: 'w', 3: 'sw',
                4: 's', '-4': 's', '-3': 'se', '-2': 'e', '-1': 'ne',
            };
            const dir = dirMap[String(idx)] || 'n';
            this.activateDir(dir);
        }
    },

    activateDir(name) {
        const el = document.getElementById('dir-' + name);
        if (el) el.classList.add('active');
    },

    updateFeedbackDisplay() {
        const el = document.getElementById('feedback-display');
        if (el) {
            el.textContent = `vx=${this.currentVx.toFixed(2)} m/s, vy=${this.currentVy.toFixed(2)} m/s, angularZ=${this.currentWz.toFixed(2)} rad/s`;
        }
    },
};
