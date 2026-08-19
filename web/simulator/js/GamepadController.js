/**
 * Strider32 Browser Gamepad Controller (v0.3)
 * Polls Web Gamepad API for hardware controller inputs and maps axes/buttons.
 * Author: Sayuru Gunathilaka
 */

const GamepadController = {
    connected: false,
    gamepadIndex: null,
    deadzone: 0.15,

    init(onConnectCallback, onDisconnectCallback) {
        window.addEventListener("gamepadconnected", (e) => {
            this.connected = true;
            this.gamepadIndex = e.gamepad.index;
            console.log(`Gamepad connected at index ${e.gamepad.index}: ${e.gamepad.id}`);
            if (onConnectCallback) onConnectCallback(e.gamepad.id);
        });

        window.addEventListener("gamepaddisconnected", (e) => {
            if (this.gamepadIndex === e.gamepad.index) {
                this.connected = false;
                this.gamepadIndex = null;
                console.log("Gamepad disconnected");
                if (onDisconnectCallback) onDisconnectCallback();
            }
        });
    },

    /**
     * Poll active gamepad inputs and return velocity commands (-1.0 to 1.0) and action triggers
     */
    poll() {
        if (!this.connected || this.gamepadIndex === null) {
            return null;
        }

        const gamepads = navigator.getGamepads ? navigator.getGamepads() : [];
        const gp = gamepads[this.gamepadIndex];
        if (!gp) return null;

        // Apply deadzone to analog axes
        const applyDeadzone = (val) => (Math.abs(val) > this.deadzone ? val : 0.0);

        // Standard Gamepad Axis Mapping:
        // Left Stick Y (Axis 1) -> Forward / Backward (-1.0 is up/forward)
        // Left Stick X (Axis 0) -> Strafe Left / Right
        // Right Stick X (Axis 2) -> Yaw Rotate Left / Right
        const forward = -applyDeadzone(gp.axes[1] || 0.0);
        const lateral = applyDeadzone(gp.axes[0] || 0.0);
        const turn    = applyDeadzone(gp.axes[2] || gp.axes[3] || 0.0);

        // Button Triggers:
        // Button 0 (A / Cross): Stand
        // Button 1 (B / Circle): Rest
        // Button 2 (X / Square): Trot Gait
        // Button 3 (Y / Triangle): Crawl Gait
        // Button 7 (RT / R2): E-STOP Trigger
        const btnStand = gp.buttons[0] ? gp.buttons[0].pressed : false;
        const btnRest  = gp.buttons[1] ? gp.buttons[1].pressed : false;
        const btnTrot  = gp.buttons[2] ? gp.buttons[2].pressed : false;
        const btnCrawl = gp.buttons[3] ? gp.buttons[3].pressed : false;
        const btnEstop = gp.buttons[7] ? gp.buttons[7].pressed : false;

        return {
            forward,
            lateral,
            turn,
            btnStand,
            btnRest,
            btnTrot,
            btnCrawl,
            btnEstop
        };
    }
};

if (typeof module !== 'undefined' && module.exports) {
    module.exports = GamepadController;
}
