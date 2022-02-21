/**
 * @param {number} value Value
 * @param {number} min Lower bound
 * @param {number} max Upper bound
 * @returns {number} Value clamped between min and max
 */
export const clamp = function(value, min, max) {
    return Math.max(Math.min(value, max), min);
};

/**
 * @param {number} x Value
 * @param {number} m Modulus
 * @returns {number} Result of y where y = x modulo m and y > 0
 */
export const positiveModulo = function(x, m) {
    console.assert(m > 0);
    let result = x % m;
    while (result < 0) {
        result += m;
    }
    return result;
};
