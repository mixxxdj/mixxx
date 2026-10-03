import {rcontrol} from "./proxy.js";
import {CMD} from "./endpoints.js";

/**
 * Get the state of a deck from the backend.
 * @param {number} deck - deck index (1-based)
 * @returns {Promise<object>} response with playing, position, duration, elapsed, artist, title, key, bpm
 */
export async function getDeckState(deck) {
    return await rcontrol({
        [CMD.GET_DECK_STATE]: {deck: parseInt(deck)},
    });
}

/**
 * Set deck play/pause state.
 * @param {number} deckId - deck index (1-based)
 * @param {boolean} playing
 * @returns {Promise<object>}
 */
export async function setDeckPlay(deckId, playing) {
    return await rcontrol({
        [CMD.SET_DECK_PLAY]: {deck: parseInt(deckId), playing},
    });
}

/**
 * Stop a deck.
 * @param {number} deckId - deck index (1-based)
 * @returns {Promise<object>}
 */
export async function deckStop(deckId) {
    return await rcontrol({
        [CMD.DECK_STOP]: {deck: parseInt(deckId)},
    });
}

/**
 * Trigger cue on a deck.
 * @param {number} deckId - deck index (1-based)
 * @returns {Promise<object>}
 */
export async function deckCue(deckId) {
    return await rcontrol({
        [CMD.DECK_CUE]: {deck: parseInt(deckId)},
    });
}

/**
 * Set deck playback position.
 * @param {number} deckId - deck index (1-based)
 * @param {number} position - position 0..1
 * @returns {Promise<object>}
 */
export async function setDeckPosition(deckId, position) {
    return await rcontrol({
        [CMD.SET_DECK_POSITION]: {
            deck: parseInt(deckId),
            position,
        },
    });
}

/**
 * Load a track to a deck.
 * @param {number} trackId
 * @param {number} deck - deck index (1-based)
 * @param {boolean} play - whether to start playing immediately
 * @returns {Promise<object>}
 */
export async function loadDeck(trackId, deck, play) {
    return await rcontrol({
        [CMD.LOAD_DECK]: {
            trackid: trackId,
            deck: parseInt(deck),
            play: !!play,
        },
    });
}

/**
 * Get the number of decks from the backend.
 * @returns {Promise<object>}
 */
export async function getNumDecks() {
    return await rcontrol({[CMD.GET_NUM_DECKS]: "true"});
}
