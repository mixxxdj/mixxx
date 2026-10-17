import {rcontrol} from "../api/proxy.js";
import {CMD} from "../api/endpoints.js";
import {getKnobParam} from "../utils/knobs.js";

/**
 * Mixer component — crossfader, main output gain, knob changes, generic parameter API.
 *
 * Methods are designed to be mixed into the main Alpine component (they use `this`).
 */
export default {
    // ─── Knob change handler ──────────────────────────────

    /**
     * Called when a knob value changes locally.
     * Uses KNOB_PARAMS mapping to send the new value to the Mixxx backend
     * via setParameter, and also keeps the local Channel model in sync.
     * @param {string} knobName - 'gain', 'high', 'mid', 'bass', 'super_fx', 'volume'
     * @param {number} deckId - deck index (1-based)
     * @param {number} localValue - value in UI range (0..100)
     */
    async knobChange(knobName, deckId, localValue) {
        const localVal = parseFloat(localValue);
        // Keep local model in sync
        if (this.Mixer.Channels[deckId]) {
            this.Mixer.Channels[deckId][knobName] = localVal;
        }
        // Send to backend
        const param = getKnobParam(knobName, deckId);
        await this.setParam(param.group, param.key, param.valueFn(localVal));
    },

    // ─── Generic Parameter API ─────────────────────────────

    /**
     * Set an engine parameter by group and key.
     * Mirrors engine.setParameter(group, key, value) in controller scripts.
     * @param {string} group - e.g. "[Channel1]"
     * @param {string} key - e.g. "rate", "pregain", "filterHigh"
     * @param {number} value - double value
     */
    async setParam(group, key, value) {
        await rcontrol({
            [CMD.SET_PARAMETER]: {group, key, value: parseFloat(value)},
        });
    },

    /**
     * Get an engine parameter value by group and key.
     * @param {string} group - e.g. "[Channel1]"
     * @param {string} key - e.g. "rate", "pregain", "filterHigh"
     * @returns {Promise<number|null>} the parameter value or null on error
     */
    async getParam(group, key) {
        try {
            const res = await rcontrol({
                [CMD.GET_PARAMETER]: {group, key},
            });
            for (const item of res) {
                if (item.group === group && item.key === key) {
                    return item.value;
                }
            }
        } catch (err) {
            console.error("getParam error:", err);
        }
        return null;
    },
};
