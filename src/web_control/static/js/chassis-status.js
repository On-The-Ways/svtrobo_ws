/**
 * SVTROBO Web Control - Chassis Status Module
 * Subscribes to /chassis/joint_states for steer & wheel data.
 */

const ChassisStatus = {
    jointTopic: null,
    diagTopic: null,

    init(ros) {
        this.jointTopic = new ROSLIB.Topic({
            ros: ros,
            name: '/chassis/joint_states',
            messageType: 'sensor_msgs/msg/JointState',
        });

        this.jointTopic.subscribe((msg) => {
            this.updateSteerDisplay(msg);
        });

        // Also subscribe to diagnostics for actual wheel speeds
        this.diagTopic = new ROSLIB.Topic({
            ros: ros,
            name: '/chassis/diagnostics',
            messageType: 'chassis_control/msg/ChassisDiagnostics',
        });

        this.diagTopic.subscribe((msg) => {
            this.updateWheelActual(msg.wheel_speeds_actual);
        });
    },

    disable() {
        const positions = ['fl', 'fr', 'rl', 'rr'];
        for (const pos of positions) {
            const angleEl = document.getElementById('steer-' + pos + '-angle');
            const velEl = document.getElementById('steer-' + pos + '-vel');
            const torqueEl = document.getElementById('steer-' + pos + '-torque');
            const targetEl = document.getElementById('wheel-' + pos + '-target');
            const actualEl = document.getElementById('wheel-' + pos + '-actual');
            if (angleEl) angleEl.textContent = '--°';
            if (velEl) velEl.textContent = '-- rad/s';
            if (torqueEl) torqueEl.textContent = '-- Nm';
            if (targetEl) targetEl.textContent = '-- RPM';
            if (actualEl) { actualEl.textContent = '-- RPM'; actualEl.style.color = ''; }
        }
    },

    updateSteerDisplay(msg) {
        const positions = ['fl', 'fr', 'rl', 'rr'];

        if (!msg.position || !msg.velocity || !msg.effort) return;

        for (let i = 0; i < 4; i++) {
            // Steer angle (rad → deg)
            const angleDeg = (msg.position[i] * 180.0 / Math.PI).toFixed(1);
            const angleEl = document.getElementById('steer-' + positions[i] + '-angle');
            if (angleEl) angleEl.textContent = angleDeg + '\u00B0';

            // Steer velocity (rad/s)
            const velEl = document.getElementById('steer-' + positions[i] + '-vel');
            if (velEl) velEl.textContent = msg.velocity[i].toFixed(2) + ' rad/s';

            // Steer torque (Nm)
            const torqueEl = document.getElementById('steer-' + positions[i] + '-torque');
            if (torqueEl) torqueEl.textContent = msg.effort[i].toFixed(2) + ' Nm';

            // Wheel target speed (RPM) — indices 4-7
            const wheelTargetEl = document.getElementById('wheel-' + positions[i] + '-target');
            if (wheelTargetEl) wheelTargetEl.textContent = Math.abs(msg.velocity[i + 4]).toFixed(0) + ' RPM';
        }
    },

    updateWheelActual(speeds) {
        if (!speeds || speeds.length < 4) return;

        const positions = ['fl', 'fr', 'rl', 'rr'];
        for (let i = 0; i < 4; i++) {
            const el = document.getElementById('wheel-' + positions[i] + '-actual');
            if (el) {
                const rpm = Math.abs(speeds[i]).toFixed(1);
                el.textContent = rpm + ' RPM';
                // Highlight if wheel is spinning
                if (Math.abs(speeds[i]) > 1.0) {
                    el.style.color = '#3b82f6';
                } else {
                    el.style.color = '';
                }
            }
        }
    },
};
