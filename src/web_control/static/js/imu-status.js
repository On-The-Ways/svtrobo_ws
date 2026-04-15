/**
 * SVTROBO Web Control - IMU Status Module
 * Subscribes to /zed/imu/data for accelerometer, gyroscope, magnetometer.
 */

const IMUStatus = {
    topic: null,
    magTopic: null,
    tempTopic: null,
    count: 0,
    lastTime: 0,
    freqEl: null,

    init(ros) {
        this.freqEl = document.getElementById('imu-freq');

        this.topic = new ROSLIB.Topic({
            ros: ros,
            name: '/zed/imu/data',
            messageType: 'sensor_msgs/msg/Imu',
        });

        this.topic.subscribe((msg) => {
            this.updateDisplay(msg);
        });

        // Magnetometer (lower rate)
        this.magTopic = new ROSLIB.Topic({
            ros: ros,
            name: '/zed/imu/mag',
            messageType: 'sensor_msgs/msg/MagneticField',
        });

        this.magTopic.subscribe((msg) => {
            const f = msg.magnetic_field;
            this.setVal('imu-mag-x', f.x, 2);
            this.setVal('imu-mag-y', f.y, 2);
            this.setVal('imu-mag-z', f.z, 2);
        });

        // Temperature (low rate)
        this.tempTopic = new ROSLIB.Topic({
            ros: ros,
            name: '/zed/imu/temperature',
            messageType: 'sensor_msgs/msg/Temperature',
        });

        this.tempTopic.subscribe((msg) => {
            const el = document.getElementById('imu-temp');
            if (el) el.textContent = msg.temperature.toFixed(1) + ' °C';
        });
    },

    disable() {
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

    updateDisplay(msg) {
        const a = msg.linear_acceleration;
        this.setVal('imu-acc-x', a.x, 3);
        this.setVal('imu-acc-y', a.y, 3);
        this.setVal('imu-acc-z', a.z, 3);

        const g = msg.angular_velocity;
        // Convert rad/s → deg/s for display
        const r2d = 180.0 / Math.PI;
        this.setVal('imu-gyro-x', g.x * r2d, 2);
        this.setVal('imu-gyro-y', g.y * r2d, 2);
        this.setVal('imu-gyro-z', g.z * r2d, 2);

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

    setVal(id, value, decimals) {
        const el = document.getElementById(id);
        if (el) {
            const v = typeof value === 'number' ? value : 0;
            el.textContent = (v >= 0 ? '+' : '') + v.toFixed(decimals);
        }
    },
};
