/**
 * @param {number} value Value
 * @param {number} min Lower bound
 * @param {number} max Upper bound
 * @returns {number} Value clamped between min and max
 */
export const clamp = function(value, min, max) {
    return Math.max(Math.min(value, max), min);
};
