import {rcontrol} from "./proxy.js";
import {CMD} from "./endpoints.js";

/**
 * Search tracks in the library.
 * @param {string} query - search text (min 2 characters)
 * @returns {Promise<object>}
 */
export async function searchTracks(query) {
    return await rcontrol({[CMD.SEARCH_TRACK]: query});
}
