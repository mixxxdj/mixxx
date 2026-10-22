import Alpine from "alpinejs";
import Sortable from "sortablejs";
import {rcontrol} from "./api/proxy.js";
import {CMD} from "./api/endpoints.js";
import {checkAuth, login as apiLogin} from "./api/login.js";

import {Channel} from "./models/channel.js";
import {Deck} from "./models/deck.js";
import {formatTime, findInResponse} from "./utils/format.js";
import {getKnobParam} from "./utils/knobs.js";

import * as decksApi from "./api/decks.js";
import * as autodjApi from "./api/autodj.js";
import * as libraryApi from "./api/library.js";

// ---- Alpine component ----

export default (config = {}) => ({
    // ─── Login (for login.html) ────────────────────────────

    /** Password input (x-model in login.html) */
    passwordInput: "",

    async doLogin() {
        await apiLogin(this.passwordInput);
    },

    // ─── Settings ──────────────────────────────────────────

    settings: {
        decks: Alpine.$persist([
            {id: 1, visible: true, timeDisplayMode: "elapsed"},
            {id: 2, visible: true, timeDisplayMode: "elapsed"},
            {id: 3, visible: false, timeDisplayMode: "elapsed"},
            {id: 4, visible: false, timeDisplayMode: "elapsed"},
        ]).as("settings_decks"),

        get visibleDecks() {
            return this.decks.filter(d => d.visible);
        },

        selectedColor: Alpine.$persist("accent").as("settings_selected_color"),

        /** AutoDJ polling interval in seconds (default 10) */
        autodjPollInterval: Alpine.$persist(10).as("settings_autodj_poll_interval"),

        toggleDeckVisible(id) {
            this.decks = this.decks.map((deck) =>
                deck.id === id ? {...deck, visible: !deck.visible} : deck,
            );
        },

        setDeckVisible(id, visible) {
            this.decks = this.decks.map((deck) =>
                deck.id === id ? {...deck, visible} : deck,
            );
        },
    },

    init_settings_decks_sortable() {
        Sortable.create(this.$refs.decksOrder, {
            animation: 150,
            handle: "input",
            onEnd: (evt) => {
                const movedItem = this.settings.decks.splice(evt.oldIndex, 1)[0];
                this.settings.decks.splice(evt.newIndex, 0, movedItem);
            },
        });
    },

    // ─── State ─────────────────────────────────────────────

    /** Currently selected deck in the UI (for polling) */
    selectedDeck: 1,

    /** How many decks exist on the backend */
    numDecks: 0,

    /** Flag to prevent duplicate polling */
    _deckPollStarted: false,

    /** Duration of the currently selected deck (for seek slider) */
    deckDuration: 0,

    /** Flag to prevent updates during manual seek */
    deckSeeking: false,

    /** Current active tab: 'mixer', 'autodj', 'library' */
    activeTab: Alpine.$persist("mixer").as("dock_activeTab"),

    Decks: {},

    Mixer: {
        main: 1.0,
        crossfader: 0,
        Channels: {},
    },

    Library: {
        searchResultList: [],
        searchQuery: "",
        selectedTrackId: null,

        cleanResultList() {
            this.searchResultList = [];
            this.selectedTrackId = null;
        },
    },

    AutoDJSection: {
        autoDJ_active: false,
        queueList: [],
        selectedTrackId: null,
    },

    // ─── Helpers exposed to template ──────────────────────

    formatTime,

    // ─── playingDecks (getter) ─────────────────────────────

    /**
     * Returns an array of decks that are currently playing.
     * Used in the AutoDJ tab to show now‑playing info.
     */
    get playingDecks() {
        return this.settings.decks
            .map(sd => this.Decks[sd.id])
            .filter(d => d && d.play);
    },

    // ─── Init (called automatically by Alpine) ────────────

    async init() {
    // Initialize models
        this.Decks = {
            1: new Deck(1),
            2: new Deck(2),
            3: new Deck(3),
            4: new Deck(4),
        };

        // Restore persisted timeDisplayMode for each deck
        for (const sDeck of this.settings.decks) {
            if (this.Decks[sDeck.id] && sDeck.timeDisplayMode) {
                this.Decks[sDeck.id].timeDisplayMode = sDeck.timeDisplayMode;
            }
        }

        this.Mixer.Channels = {
            1: new Channel(1),
            2: new Channel(2),
            3: new Channel(3),
            4: new Channel(4),
        };

        // Skip auth check on login page
        if (window.location.pathname.includes("login.html")) { return; }

        // Verify authentication
        const authed = await checkAuth();
        if (!authed) { return; } // will be redirected

        // Load initial state from backend
        await this.loadInitialState();

        // Watcher: crossfader → backend
        this.$watch("Mixer.crossfader", async(val) => {
            await rcontrol({[CMD.SET_CROSSFADER]: {value: parseFloat(val)}});
        });

        // Watcher: Main Output Gain → backend via generic setParameter
        this.$watch("Mixer.main", async(val) => {
            await this.setParam("[Master]", "gain", parseFloat(val));
        });
    },

    /**
     * Load initial state from the Mixxx backend.
     */
    async loadInitialState() {
        try {
            // Crossfader
            const crossRes = await rcontrol({[CMD.GET_CROSSFADER]: "true"});
            const crossVal = findInResponse(crossRes, "crossfader");
            if (crossVal !== undefined) { this.Mixer.crossfader = crossVal; }

            // Main Output Gain
            const gainVal = await this.getParam("[Master]", "gain");
            if (gainVal != null) { this.Mixer.main = gainVal; }

            // AutoDJ enabled
            const adjRes = await rcontrol({[CMD.GET_AUTODJ_ENABLED]: "true"});
            const adjVal = findInResponse(adjRes, "autodjenabled");
            if (adjVal !== undefined) { this.AutoDJSection.autoDJ_active = adjVal; }

            // Number of decks
            const decksRes = await rcontrol({[CMD.GET_NUM_DECKS]: "true"});
            const numDecks = findInResponse(decksRes, "numdecks");
            if (numDecks !== undefined) {
                this.numDecks = numDecks;
                if (numDecks === 1) { this.selectedDeck = 1; }
                this.startDeckPolling();
            }

            await this.loadAutoDJTracklist();
        } catch (err) {
            console.error("loadInitialState error:", err);
        }
    },

    // ─── Deck polling ──────────────────────────────────────

    startDeckPolling() {
        if (this._deckPollStarted) { return; }
        this._deckPollStarted = true;

        // Pause all polling when the browser tab is hidden
        document.addEventListener("visibilitychange", () => {
            if (document.hidden) {
                // Tab is hidden — intervals will skip their work
            }
        });

        this._deckPollTimer = setInterval(() => {
            this.getDecksStatuses();
        }, 1000);

        const adjInterval = (this.settings.autodjPollInterval || 10) * 1000;
        this._adjPollTimer = setInterval(() => {
            if (document.hidden) { return; }
            if (this.activeTab === "autodj") {
                this.loadAutoDJTracklist();
            }
        }, adjInterval);

        // Start mixer knob polling (batch call every 2s, only when mixer tab is visible)
        this._mixerPollTimer = setInterval(() => {
            this.getMixerParameters();
        }, 2000);
    },

    /**
     * Fetch status for all visible decks in a single batch call.
     * Only runs when the page is focused and the active tab needs deck info.
     */
    async getDecksStatuses() {
        if (document.hidden) { return; }
        if (this.deckSeeking) { return; }
        if (this.activeTab !== "mixer" && this.activeTab !== "autodj") { return; }

        const visibleDeckIds = this.settings.visibleDecks.map(d => d.id);
        if (visibleDeckIds.length === 0) { return; }

        try {
            const res = await rcontrol({[CMD.GET_DECKS_STATUSES]: visibleDeckIds});
            for (const item of res) {
                if (item.deck === undefined) { continue; }
                const deck = item.deck;
                const d = this.Decks[deck];
                if (!d) { continue; }
                if (item.playing !== undefined) { d.play = item.playing; }
                if (item.duration !== undefined) {
                    d.duration = item.duration;
                    if (deck === this.selectedDeck) { this.deckDuration = item.duration; }
                }
                if (item.position !== undefined && !this.deckSeeking) {
                    d.pos = item.position;
                    d.elapsed = item.elapsed || 0;
                }
                if (item.title !== undefined) { d.title = item.title || "–"; }
                if (item.artist !== undefined) { d.artist = item.artist || ""; }
                if (item.key !== undefined) { d.key = item.key || "–"; }
                if (item.bpm !== undefined) { d.bpm = item.bpm; }
            }
        } catch (err) {
            console.error("getDecksStatuses error:", err);
        }
    },

    /**
     * Fetch all mixer knob values (gain, EQ, volume, super_fx) for visible decks
     * in a single batch call. Only runs when the mixer tab is visible and the page is focused.
     */
    async getMixerParameters() {
        if (document.hidden) { return; }
        if (this.activeTab !== "mixer") { return; }

        const visibleDeckIds = this.settings.visibleDecks.map(d => d.id);
        if (visibleDeckIds.length === 0) { return; }

        // Build batch: [{group, key}] for 6 knobs × N visible decks
        const params = [];
        const lookup = [];
        for (const deckId of visibleDeckIds) {
            for (const knobName of ["gain", "high", "mid", "bass", "super_fx", "volume"]) {
                const p = getKnobParam(knobName, deckId);
                params.push({group: p.group, key: p.key});
                lookup.push({deckId, knobName});
            }
        }

        try {
            const res = await rcontrol({[CMD.GET_PARAMETERS]: params});
            // Flatten: response is [{parameters: [{group, key, value}, ...]}]
            const flat = [];
            for (const item of res) {
                if (item.parameters && Array.isArray(item.parameters)) {
                    flat.push(...item.parameters);
                }
            }
            // Apply results to channel models (order matches the request)
            for (let i = 0; i < flat.length && i < lookup.length; i++) {
                const {deckId, knobName} = lookup[i];
                const channel = this.Mixer.Channels[deckId];
                if (channel) {
                    channel[knobName] = this.engineToUI(knobName, flat[i].value);
                }
            }
        } catch (err) {
            console.error("getMixerParameters error:", err);
        }
    },

    /**
     * Convert an engine parameter value back to the UI 0..100 range.
     * This is the inverse of the valueFn defined in KNOB_PARAMS.
     * @param {string} knobName - one of 'gain', 'high', 'mid', 'bass', 'super_fx', 'volume'
     * @param {number} engineVal - engine value (0..1)
     * @returns {number} UI value (0..100)
     */
    engineToUI(knobName, engineVal) {
        if (knobName === "super_fx" || knobName === "volume") {
            return engineVal * 100;
        }
        // gain, high, mid, bass: engine = (2 * (v/100))²  →  v = sqrt(engine) / 2 * 100
        return (Math.sqrt(engineVal) / 2) * 100;
    },

    onDeckChange(deckId) {
        this.selectedDeck = deckId;
        this.getDeckState(deckId);
    },

    async getDeckState(deck) {
        try {
            const res = await decksApi.getDeckState(deck);
            for (const item of res) {
                if (item.playing !== undefined) { this.Decks[deck].play = item.playing; }
                if (item.duration !== undefined) {
                    this.Decks[deck].duration = item.duration;
                    if (deck === this.selectedDeck) { this.deckDuration = item.duration; }
                }
                if (item.position !== undefined && !this.deckSeeking) {
                    this.Decks[deck].pos = item.position;
                    this.Decks[deck].elapsed = item.elapsed || 0;
                }
                if (item.title !== undefined) { this.Decks[deck].title = item.title || "–"; }
                if (item.artist !== undefined) { this.Decks[deck].artist = item.artist || ""; }
                if (item.key !== undefined) { this.Decks[deck].key = item.key || "–"; }
                if (item.bpm !== undefined) { this.Decks[deck].bpm = item.bpm; }
            }
        } catch (err) {
            console.error("getDeckState error:", err);
        }
    },

    onDeckSeekInput(sliderValue) {
        this.Decks[this.selectedDeck].pos = sliderValue / 1000;
    },

    async onDeckSeekCommit(sliderValue) {
        await decksApi.setDeckPosition(this.selectedDeck, sliderValue / 1000);
    },

    async toggleDeckPlay(deckId) {
        const playing = !this.Decks[deckId].play;
        await decksApi.setDeckPlay(deckId, playing);
        this.Decks[deckId].play = playing;
    },

    async deckStop(deckId) {
        await decksApi.deckStop(deckId);
        this.Decks[deckId].play = false;
    },

    async deckCue(deckId) {
        await decksApi.deckCue(deckId);
    },

    async loadDeck(trackId, deck, play) {
        await decksApi.loadDeck(trackId, deck, play);
    },

    cycleTimeDisplay(deckId) {
        const deck = this.Decks[deckId];
        if (!deck) { return; }
        const modes = ["elapsed", "remaining", "both"];
        const idx = modes.indexOf(deck.timeDisplayMode);
        deck.timeDisplayMode = modes[(idx + 1) % modes.length];
        this.settings.decks = this.settings.decks.map(d =>
            d.id === deckId ? {...d, timeDisplayMode: deck.timeDisplayMode} : d
        );
    },

    formatDeckTime(deckId) {
        const deck = this.Decks[deckId];
        if (!deck || !deck.duration) { return "0:00"; }
        const elapsed = deck.elapsed || 0;
        const remaining = deck.duration - elapsed;
        switch (deck.timeDisplayMode) {
        case "elapsed": return this.formatTime(elapsed);
        case "remaining": return `-${  this.formatTime(remaining)}`;
        case "both": return `${this.formatTime(elapsed)  } / -${  this.formatTime(remaining)}`;
        default: return this.formatTime(elapsed);
        }
    },

    // ─── Knob change handler ──────────────────────────────

    async knobChange(knobName, deckId, localValue) {
        const localVal = parseFloat(localValue);
        if (this.Mixer.Channels[deckId]) {
            this.Mixer.Channels[deckId][knobName] = localVal;
        }
        const param = getKnobParam(knobName, deckId);
        await this.setParam(param.group, param.key, param.valueFn(localVal));
    },

    // ─── Generic Parameter API ─────────────────────────────

    async setParam(group, key, value) {
        await rcontrol({
            [CMD.SET_PARAMETER]: {group, key, value: parseFloat(value)},
        });
    },

    async getParam(group, key) {
        try {
            const res = await rcontrol({[CMD.GET_PARAMETER]: {group, key}});
            for (const item of res) {
                if (item.group === group && item.key === key) { return item.value; }
            }
        } catch (err) {
            console.error("getParam error:", err);
        }
        return null;
    },

    // ─── AutoDJ ────────────────────────────────────────────

    async autoDJ_fade_now() {
        await this.setParam("[AutoDJ]", "fade_now", 1);
    },

    async setAutoDJEnabled(enabled) {
        await autodjApi.setAutoDJEnabled(enabled);
        this.AutoDJSection.autoDJ_active = !!enabled;
    },

    async loadAutoDJTracklist() {
        try {
            const res = await autodjApi.getAutoTracklist();
            this.AutoDJSection.queueList = [];
            for (const item of res) {
                if (item.tracklist && Array.isArray(item.tracklist)) {
                    this.AutoDJSection.queueList = item.tracklist;
                    break;
                }
            }
        } catch (err) {
            console.error("loadAutoDJTracklist error:", err);
        }
    },

    async addToAutoDJ(trackId, position) {
        await autodjApi.addAutoDJ(trackId, position);
        await this.loadAutoDJTracklist();
    },

    async removeFromAutoDJ(position, trackId) {
        await autodjApi.delAutoDJ(position, trackId);
        await this.loadAutoDJTracklist();
    },

    removeSelectedAutoDJTrack() {
        const trackId = this.AutoDJSection.selectedTrackId;
        if (!trackId) { return; }
        const list = this.AutoDJSection.queueList;
        const track = list.find(t => t.id === trackId);
        if (!track) { return; }
        this.removeFromAutoDJ(track.position, trackId);
        this.AutoDJSection.selectedTrackId = null;
    },

    async moveAutoDJTrack(position, newPosition) {
        await autodjApi.moveAutoTracklist(position, newPosition);
        await this.loadAutoDJTracklist();
    },

    initAutodjSortable() {
        this.$nextTick(() => {
            const el = this.$refs.autodjList;
            if (!el) { return; }
            Sortable.create(el, {
                animation: 150,
                handle: ".drag-handle",
                onEnd: (evt) => {
                    const oldIndex = evt.oldIndex;
                    const newIndex = evt.newIndex;
                    if (oldIndex === newIndex) { return; }
                    const list = this.AutoDJSection.queueList;
                    if (!list || list.length === 0) { return; }
                    const track = list[oldIndex];
                    this.moveAutoDJTrack(track.position, newIndex);
                },
            });
        });
    },

    selectAutoDJTrack(trackId) {
        if (this.AutoDJSection.selectedTrackId === trackId) {
            this.AutoDJSection.selectedTrackId = null;
        } else {
            this.AutoDJSection.selectedTrackId = trackId;
        }
    },

    // ─── Library / Search ──────────────────────────────────

    async searchTracks(text) {
        if (!text || text.length < 2) {
            this.Library.cleanResultList();
            return;
        }
        try {
            const res = await libraryApi.searchTracks(text);
            for (const item of res) {
                if (item.tracklist && Array.isArray(item.tracklist)) {
                    this.Library.searchResultList = item.tracklist;
                    return;
                }
            }
            this.Library.cleanResultList();
        } catch (err) {
            console.error("searchTracks error:", err);
        }
    },

    selectTrack(trackId) {
        if (this.Library.selectedTrackId === trackId) {
            this.Library.selectedTrackId = null;
        } else {
            this.Library.selectedTrackId = trackId;
        }
    },

    addSelectedToAutoDJNext() {
        const trackId = this.Library.selectedTrackId;
        if (!trackId) { return; }
        this.addToAutoDJ(trackId, "begin");
        this.Library.selectedTrackId = null;
    },

    addSelectedToAutoDJEnd() {
        const trackId = this.Library.selectedTrackId;
        if (!trackId) { return; }
        this.addToAutoDJ(trackId, "end");
        this.Library.selectedTrackId = null;
    },

    async loadSelectedToDeck(deckId) {
        const trackId = this.Library.selectedTrackId;
        if (!trackId) { return; }
        await this.loadDeck(trackId, deckId, false);
        this.Library.selectedTrackId = null;
        this.activeTab = "mixer";
    },
});
