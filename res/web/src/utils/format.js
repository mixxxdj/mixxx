/**
 * Format seconds to mm:ss display.
 * @param {number} seconds
 * @returns {string}
 */
export function formatTime(seconds) {
    if (!isFinite(seconds) || seconds < 0) {
        seconds = 0;
    }
    const mins = Math.floor(seconds / 60);
    const secs = Math.floor(seconds % 60);
    return `${mins  }:${  secs < 10 ? "0" : ""  }${secs}`;
}

/**
 * Find a value in the RControl response (array of objects).
 * @param {Array} res - response array
 * @param {string} key - key to find
 * @returns {*|undefined}
 */
export function findInResponse(res, key) {
    for (const item of res) {
        if (item[key] !== undefined) { return item[key]; }
    }
    return undefined;
}
