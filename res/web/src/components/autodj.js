import Sortable from "sortablejs";
import * as autodjApi from "../api/autodj.js";
import {findInResponse} from "../utils/format.js";

/**
 * AutoDJ component — AutoDJ state management, queue operations.
 *
 * Methods are designed to be mixed into the main Alpine component (they use `this`).
 */
export default {
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

    /** Remove the selected AutoDJ track from queue */
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

    /** Handle Sortable.js reorder for AutoDJ queue */
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
};
