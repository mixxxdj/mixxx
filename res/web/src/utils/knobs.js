/**
 * Maps UI 0..100 → engine using 2x² (x = v/100).
 * Used for gain and EQ parameters.
 * @param {number} v - value in UI range 0..100
 * @returns {number} engine value 0..1
 */
export function mapToRange_0_1_4(v) {
    const x = v / 100;
    return (2 * x) ** 2;
}

/**
 * Knob parameter mapping definitions.
 * Maps knob logical names to Mixxx control groups and keys.
 * The valueFn normalizes the UI range (0..100) to the engine range.
 */
export const KNOB_PARAMS = {
    gain: {groupFn: (deck) => `[Channel${deck}]`,                       key: "pregain",  valueFn: mapToRange_0_1_4},
    high: {groupFn: (deck) => `[EqualizerRack1_[Channel${deck}]_Effect1]`, key: "parameter3", valueFn: mapToRange_0_1_4},
    mid: {groupFn: (deck) => `[EqualizerRack1_[Channel${deck}]_Effect1]`, key: "parameter2", valueFn: mapToRange_0_1_4},
    bass: {groupFn: (deck) => `[EqualizerRack1_[Channel${deck}]_Effect1]`, key: "parameter1", valueFn: mapToRange_0_1_4},
    super_fx: {groupFn: (deck) => `[QuickEffectRack1_[Channel${deck}]]`,          key: "super1",   valueFn: (v) => v / 100},
    volume: {groupFn: (deck) => `[Channel${deck}]`,                       key: "volume",   valueFn: (v) => v / 100},
};

/**
 * Get the Mixxx control mapping for a knob by type and deck index.
 * @param {string} knobName - one of 'gain', 'high', 'mid', 'bass', 'super_fx', 'volume'
 * @param {number} deck - deck index (1-based)
 * @returns {{group: string, key: string, valueFn: Function}}
 */
export function getKnobParam(knobName, deck) {
    const def = KNOB_PARAMS[knobName];
    if (!def) { throw new Error(`Unknown knob: ${knobName}`); }
    return {
        group: def.groupFn(deck),
        key: def.key,
        valueFn: def.valueFn,
    };
}
