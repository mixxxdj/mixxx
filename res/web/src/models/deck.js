/**
 * HotCue model — represents a hot cue point on a deck.
 */
export class HotCue {
    constructor(index) {
        this.index = index; // 0 to 31
        this.name = "";
        this.pos = null; // position
    }
}

/**
 * Deck model — represents a deck state.
 */
export class Deck {
    constructor(id) {
    // id from 1 to 4
        this.id = id;
        this.play = false;
        this.pos = 0;
        this.duration = 0;
        this.elapsed = 0;
        this.title = "–";
        this.artist = "";
        this.key = "–";
        this.bpm = null;
        this.cue_pos = null;
        this.hotCue = new Array(32); // sparse array of HotCue instances
        /** Cycles: 'elapsed' → 'remaining' → 'both' → 'elapsed'. Persisted via settings.decks. */
        this.timeDisplayMode = "elapsed";
    }
}
