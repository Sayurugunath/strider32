/**
 * Strider32 Animation & Timeline Controller (v0.3)
 * Handles keyframe sequence recording, playback, keyframe interpolation,
 * JSON export/import, and LittleFS payload serialization.
 * Author: Sayuru Gunathilaka
 */

const AnimationController = {
    keyframes: [],
    isPlaying: false,
    currentFrameIndex: 0,
    playbackSpeedMs: 300,
    timerId: null,

    /**
     * Clear current timeline keyframes
     */
    clearTimeline() {
        this.keyframes = [];
        this.currentFrameIndex = 0;
        this.stopPlayback();
    },

    /**
     * Add keyframe to sequence [FL_C, FL_F, FR_C, FR_F, BL_C, BL_F, BR_C, BR_F, durationMs]
     */
    addKeyframe(angles, durationMs = 300) {
        if (!angles || angles.length < 8) return;
        this.keyframes.push({
            angles: [...angles],
            duration: durationMs
        });
    },

    /**
     * Start timeline sequence playback
     */
    startPlayback(onFrameCallback) {
        if (this.keyframes.length === 0) return;
        this.isPlaying = true;
        this.currentFrameIndex = 0;

        const nextFrame = () => {
            if (!this.isPlaying) return;
            if (this.currentFrameIndex >= this.keyframes.length) {
                this.currentFrameIndex = 0; // Loop playback
            }

            const frame = this.keyframes[this.currentFrameIndex];
            if (onFrameCallback) onFrameCallback(frame.angles);

            this.currentFrameIndex++;
            this.timerId = setTimeout(nextFrame, frame.duration || 300);
        };

        nextFrame();
    },

    /**
     * Stop timeline sequence playback
     */
    stopPlayback() {
        this.isPlaying = false;
        if (this.timerId) {
            clearTimeout(this.timerId);
            this.timerId = null;
        }
    },

    /**
     * Export keyframe sequence as JSON string
     */
    exportJSON(animName = "custom_anim") {
        const payload = {
            name: animName,
            version: "0.3.0",
            frames: this.keyframes
        };
        return JSON.stringify(payload, null, 2);
    },

    /**
     * Import keyframe sequence from JSON string
     */
    importJSON(jsonString) {
        try {
            const data = JSON.parse(jsonString);
            if (data && Array.isArray(data.frames)) {
                this.keyframes = data.frames;
                this.currentFrameIndex = 0;
                return true;
            }
        } catch (e) {
            console.error("Failed to parse animation JSON:", e);
        }
        return false;
    }
};

if (typeof module !== 'undefined' && module.exports) {
    module.exports = AnimationController;
}
