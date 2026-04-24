/**
 * SVTROBO Master Lock
 * Only one browser tab can control the robot. Additional tabs are read-only.
 */
const MasterLock = {
    sessionId: null,
    isMaster: false,
    heartbeatTimer: null,
    serverUrl: '',
    _onMasterChange: null,  // callback(isMaster)

    init(serverUrl, onChange) {
        this.serverUrl = serverUrl;
        this._onMasterChange = onChange;
        // Generate or restore session ID per tab
        if (!sessionStorage.getItem('master_session_id')) {
            sessionStorage.setItem('master_session_id', crypto.randomUUID());
        }
        this.sessionId = sessionStorage.getItem('master_session_id');
    },

    async request() {
        try {
            const resp = await fetch(this.serverUrl + '/api/master/request', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ session_id: this.sessionId }),
            });
            const data = await resp.json();
            if (data.session_id) {
                this.sessionId = data.session_id;
                sessionStorage.setItem('master_session_id', data.session_id);
            }
            const wasMaster = this.isMaster;
            this.isMaster = data.master === true;
            if (wasMaster !== this.isMaster && this._onMasterChange) {
                this._onMasterChange(this.isMaster, data.holder);
            }
            if (this.isMaster && !this.heartbeatTimer) {
                this._startHeartbeat();
            }
            return data;
        } catch (e) {
            return { master: false };
        }
    },

    async release() {
        this._stopHeartbeat();
        try {
            await fetch(this.serverUrl + '/api/master/release', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ session_id: this.sessionId }),
            });
        } catch (e) {}
        this.isMaster = false;
        if (this._onMasterChange) this._onMasterChange(false);
    },

    _startHeartbeat() {
        this._stopHeartbeat();
        this.heartbeatTimer = setInterval(() => this.request(), 8000);
    },

    _stopHeartbeat() {
        if (this.heartbeatTimer) {
            clearInterval(this.heartbeatTimer);
            this.heartbeatTimer = null;
        }
    },
};
