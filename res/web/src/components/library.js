import * as libraryApi from "../api/library.js";

/**
 * Library component — search, track selection, FAB actions.
 *
 * Methods are designed to be mixed into the main Alpine component (they use `this`).
 */
export default {
    // ─── Library / Search ──────────────────────────────────

    async searchTracks(text) {
        if (!text || text.length < 2) {
            this.Library.searchResultList = [];
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
            this.Library.searchResultList = [];
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

    /** Add selected library track to AutoDJ queue at the beginning (next up) */
    addSelectedToAutoDJNext() {
        const trackId = this.Library.selectedTrackId;
        if (!trackId) { return; }
        this.addToAutoDJ(trackId, "begin");
        this.Library.selectedTrackId = null;
    },

    /** Add selected library track to AutoDJ queue at the end */
    addSelectedToAutoDJEnd() {
        const trackId = this.Library.selectedTrackId;
        if (!trackId) { return; }
        this.addToAutoDJ(trackId, "end");
        this.Library.selectedTrackId = null;
    },

    /**
     * Load selected library track to a deck, then switch to Mixer tab
     * @param deckId
     */
    async loadSelectedToDeck(deckId) {
        const trackId = this.Library.selectedTrackId;
        if (!trackId) { return; }
        await this.loadDeck(trackId, deckId, false);
        this.Library.selectedTrackId = null;
        this.activeTab = "mixer";
    },
};
