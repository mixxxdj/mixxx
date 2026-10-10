/**
 * Endpoint URL for Mixxx RControl commands.
 * The backend serves everything under /chameleon/ via QHttpServer,
 * and the /rcontrol endpoint is at the root.
 */
export const RCONTROL_URL = "/rcontrol";

/**
 * Names of RControl commands
 */
export const CMD = {
    // Auth
    LOGIN: "login",

    // Crossfader
    GET_CROSSFADER: "getcrossfader",
    SET_CROSSFADER: "setcrossfader",

    // AutoDJ
    GET_AUTODJ_ENABLED: "getautodjenabled",
    SET_AUTODJ_ENABLED: "setautodjenabled",
    GET_AUTO_TRACKLIST: "getautotracklist",
    ADD_AUTODJ: "addautodj",
    DEL_AUTODJ: "delautodj",
    MOVE_AUTO_TRACKLIST: "moveautotracklist",

    // Decks
    GET_NUM_DECKS: "getnumdecks",
    GET_DECK_STATE: "getdeckstate",
    GET_DECKS_STATUSES: "getDecksStatuses",
    SET_DECK_PLAY: "setdeckplay",
    DECK_STOP: "deckstop",
    DECK_CUE: "deckcue",
    SET_DECK_POSITION: "setdeckposition",
    LOAD_DECK: "loaddeck",

    // Library
    SEARCH_TRACK: "searchtrack",

    // Generic parameter API (mirrors engine.setParameter / getParameter)
    SET_PARAMETER: "setParameter",
    GET_PARAMETER: "getParameter",

    // Batch parameter API (multiple getParameter in one call)
    GET_PARAMETERS: "getParameters",
};
