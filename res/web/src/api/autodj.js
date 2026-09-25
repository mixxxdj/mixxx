import {rcontrol} from "./proxy.js";
import {CMD} from "./endpoints.js";

/**
 * Get the current AutoDJ enabled state.
 * @returns {Promise<object>}
 */
export async function getAutoDJEnabled() {
    return await rcontrol({[CMD.GET_AUTODJ_ENABLED]: "true"});
}

/**
 * Set AutoDJ enabled/disabled.
 * @param {boolean} enabled
 * @returns {Promise<object>}
 */
export async function setAutoDJEnabled(enabled) {
    return await rcontrol({[CMD.SET_AUTODJ_ENABLED]: {enabled: !!enabled}});
}

/**
 * Get the AutoDJ tracklist.
 * @returns {Promise<object>}
 */
export async function getAutoTracklist() {
    return await rcontrol({[CMD.GET_AUTO_TRACKLIST]: "true"});
}

/**
 * Add a track to the AutoDJ queue.
 * @param {number|string} trackId
 * @param {string} position - 'begin' or 'end'
 * @returns {Promise<object>}
 */
export async function addAutoDJ(trackId, position) {
    return await rcontrol({
        [CMD.ADD_AUTODJ]: {trackid: String(trackId), position},
    });
}

/**
 * Remove a track from the AutoDJ queue.
 * @param {number|string} position
 * @param {number|string} trackId
 * @returns {Promise<object>}
 */
export async function delAutoDJ(position, trackId) {
    return await rcontrol({
        [CMD.DEL_AUTODJ]: {position: String(position), trackid: String(trackId)},
    });
}

/**
 * Move a track in the AutoDJ queue.
 * @param {number|string} position
 * @param {number|string} newPosition
 * @returns {Promise<object>}
 */
export async function moveAutoTracklist(position, newPosition) {
    return await rcontrol({
        [CMD.MOVE_AUTO_TRACKLIST]: {position: String(position), newposition: String(newPosition)},
    });
}
