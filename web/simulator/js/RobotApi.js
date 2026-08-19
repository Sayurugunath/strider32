/**
 * Strider32 Real Robot REST API Client (v0.3)
 * Communicates with physical ESP32 hardware via HTTP REST API.
 * Author: Sayuru Gunathilaka
 */

const RobotApi = {
    baseUrl: "http://192.168.4.1", // Default AP IP or strider32.local
    isConnected: false,
    isEstopped: false,

    setBaseUrl(url) {
        if (!url.startsWith('http://') && !url.startsWith('https://')) {
            url = 'http://' + url;
        }
        this.baseUrl = url.replace(/\/+$/, "");
    },

    async fetchStatus() {
        try {
            const resp = await fetch(`${this.baseUrl}/api/v1/status`, { signal: AbortSignal.timeout(2000) });
            if (resp.ok) {
                const data = await resp.json();
                this.isConnected = true;
                this.isEstopped = (data.state === "EMERGENCY_STOP");
                return data;
            }
        } catch (e) {
            this.isConnected = false;
        }
        return null;
    },

    async sendControl(command, vx = 0, vy = 0, vz = 0, gait = 0, speed = 50) {
        if (!this.isConnected || this.isEstopped) return false;
        try {
            const resp = await fetch(`${this.baseUrl}/api/v1/control`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ command, vx, vy, vz, gait, speed }),
                signal: AbortSignal.timeout(2000)
            });
            return resp.ok;
        } catch (e) {
            return false;
        }
    },

    async triggerEstop() {
        try {
            const resp = await fetch(`${this.baseUrl}/api/v1/estop`, { method: 'POST' });
            if (resp.ok) this.isEstopped = true;
            return resp.ok;
        } catch (e) {
            return false;
        }
    },

    async resetEstop() {
        try {
            const resp = await fetch(`${this.baseUrl}/api/v1/estop/reset`, { method: 'POST' });
            if (resp.ok) this.isEstopped = false;
            return resp.ok;
        } catch (e) {
            return false;
        }
    }
};

if (typeof module !== 'undefined' && module.exports) {
    module.exports = RobotApi;
}
