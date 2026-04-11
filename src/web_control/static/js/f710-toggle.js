/**
 * SVTROBO Web Control - Control Mode Toggle
 * Switch between web control and gamepad control (mutual exclusion).
 */

const ControlMode = {
    enableTopic: null,
    statusTopic: null,
    // 'web' = web keyboard+buttons active, gamepad disabled
    // 'gamepad' = gamepad active, web controls disabled
    mode: 'web',

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
    },

    toggle() {
        this.mode = this.mode === 'web' ? 'gamepad' : 'web';
        this.applyMode();
        if (this.enableTopic) {
            this.enableTopic.publish(new ROSLIB.Message({ data: this.mode === 'gamepad' }));
        }
    },

    applyMode() {
        this.updateUI();
        if (this.mode === 'gamepad') {
            Chassis.disable();
            Lift.disable();
        } else {
            Chassis.enable();
            Lift.enable();
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
