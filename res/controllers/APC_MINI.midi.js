// Stored for documentation purposes
var ROOT_SLOT_ADDRESSES = [0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F];

// Supported colors shorthand
var COLORS = {
    OFF: 0x00,
    GREEN: 0x01,
    GREEN_FLASHING: 0x02,
    RED: 0x03,
    RED_FLASHING: 0x04,
    YELLOW: 0x05,
    YELLOW_FLASHING: 0x06,
};

/**
 * A Deck represents the mapping between a deck in Mixxx and a column of button on the APC mini.
 * @param {number} n Number of the deck (1-4) 
 * @param {numner} s  Number of slots supported by the deck for hotcues
 * @param {number} c  Id/Address of the first slot (first rectangular button in the column)
 * @param {number} b  Id/Address of the master play/pause button (round button in the deck's column)
 */
var Deck = function(n, s, c, b) {
    this.deckNumber = n;
    this.slots = s;
    this.rootSlotAddress = c;
    this.playButton = b;
    this.track = []; // Hotcue pattern for the track currently loaded in the deck 
    this.activeSlot = undefined; // Slot actually playing
    this.reset();
    this.bind();
}

/**
 * Shuts off all the leds for the given deck.
 */
Deck.prototype.reset = function() {
    var self = this;
    this.forEachSlot(function(s, t) {
        self.setSlotColor(s, COLORS.OFF);
    });
    this.setPlaying(false);
}

/**
 * Toggles the play/pause status of the deck.
 * If the deck is playing the round button under the last slot will be flashing red.
 * @param {boolean} p true if the deck is playing.
 */
Deck.prototype.setPlaying = function(p) {
        if (p) {
            midi.sendShortMsg(0x90, this.playButton, COLORS.GREEN_FLASHING);
        } else {
            midi.sendShortMsg(0x90, this.playButton, COLORS.OFF);
        }
    }
    /**
     * Sets the slot's led color.
     * Note that for ease of use, slots start at 1, the APC has max 8 slots available per deck.
     * @param {number} slot number of the slot which color should be changed
     * @param {number} color the color to set.
     */
Deck.prototype.setSlotColor = function(slot, color) {
    midi.sendShortMsg(0x90, this.slotToAddress(slot), color);
}

/**
 * Converts a slot number for a given deck to its physical midi control Id.
 * @param {number} slot a slot number starting at 1.
 */
Deck.prototype.slotToAddress = function(slot) {
    return this.rootSlotAddress - ((slot - 1) * this.slots);
}

/**
 * Converts a physical midi control Id to the slot number.
 * @param {number} address a midi control Id.
 */
Deck.prototype.addressToSlot = function(address) {
    return this.slots - Math.floor(address / this.slots);
}

/**
 * Helper function that will trigger a callback function for each slot of a deck.
 * @param {function(slot,address)} callback the callback function to be called for each slot.
 */
Deck.prototype.forEachSlot = function(callback) {
    for (var slot = 1; slot <= this.slots; slot++) {
        callback(slot, this.rootSlotAddress - ((slot - 1) * this.slots));
    }
}

/**
 * Binds the deck to its adequate deck in Mixxx.
 */
Deck.prototype.bind = function() {
    var self = this;
    engine.connectControl("[Channel" + this.deckNumber + "]", "duration", function(value, group, control) {
        self.onTrackLoaded(value, group, control);
    });
    engine.connectControl("[Channel" + this.deckNumber + "]", "play", function(value, group, control) {
        self.togglePlay(control);
    });
}

/**
 * Called when the duration of the deck's loaded track changed (new track loaded).
 * this function parses the cue points and sets the slots color to green for each detected cue points.
 * Unused slots will be set to COLOR.YELLOW.
 */
Deck.prototype.onTrackLoaded = function(value, group, control) {
    var self = this;
    this.track = [];
    this.forEachSlot(function(s, t) {
        if (engine.getParameter("[Channel" + self.deckNumber + "]", "hotcue_" + s + "_position") >= 0) {
            self.track.push(COLORS.GREEN);
        } else {
            self.track.push(COLORS.YELLOW);
        }
    });
    this.applyTrack();
}

/**
 * Reapplied the slots lightning for the currently loaded track.
 */
Deck.prototype.applyTrack = function() {
    for (var i = 0; i < this.slots; i++) {
        this.setSlotColor(i + 1, this.track[i]);
    }
}

/**
 * Syncs the play status of a track between Mixxx and the APC.
 */
Deck.prototype.togglePlay = function(control) {
    if (control === this.playButton) {
        if (engine.getParameter("[Channel" + this.deckNumber + "]", "play") === 0) {
            engine.setParameter("[Channel" + this.deckNumber + "]", "play", 1);
            this.setPlaying(true);
        } else {
            engine.setParameter("[Channel" + this.deckNumber + "]", "play", 0);
            this.setPlaying(false);
        }
    }
    if (engine.getParameter("[Channel" + this.deckNumber + "]", "play") === 0) {
        if (this.activeSlot) {
            this.applyTrack();
            this.activeSlot = undefined;
        }
        this.setPlaying(false);
    } else {
        this.setPlaying(true);
    }
}

/**
 * Syncs the hotcue play status of a track between Mixxx and the APC.
 */
Deck.prototype.toggleHotClue = function(control) {
    if (!this.track || this.track.length === 0) return;

    var s = this.addressToSlot(control);
    if (this.track[s - 1] === COLORS.YELLOW) return;

    this.applyTrack();

    engine.setParameter("[Channel" + this.deckNumber + "]", "hotcue_" + s + "_gotoandplay", true);
    this.setSlotColor(s, COLORS.RED_FLASHING);
    this.activeSlot = s;
}

// Registry of all active decks
var Decks = [];

var Apc = {};

/**
 * Inits the controller and configures the decks.
 */
Apc.init = function(id, debugging) {
    Decks = [
        new Deck(1, 8, 0x38, 0x40),
        new Deck(2, 8, 0x39, 0x41),
        new Deck(3, 8, 0x3A, 0x42),
        new Deck(4, 8, 0x3B, 0x43),
    ];
    engine.softTakeover("[Master]", "crossfader", true);
    engine.softTakeover("[Channel1]", "pregain", true);
    engine.softTakeover("[Channel2]", "pregain", true);
    engine.softTakeover("[Channel2]", "volume", true);
    engine.softTakeover("[Channel3]", "pregain", true);
    engine.softTakeover("[Channel3]", "volume", true);
    engine.softTakeover("[Channel4]", "pregain", true);
    engine.softTakeover("[Channel4]", "volume", true);
}

/**
 * When we leave all the leds should be turned off.
 */
Apc.shutdown = function() {
    Decks.forEach(function(d) {
        d.reset();
    }, this);
}

/**
 * Triggered when a slot button is pressed.
 */
Apc.hotcue = function(channel, control, value, status, group) {
    var c = group.substr(8, 1); // Ugly AF to be fixed
    var d = Decks[c - 1];
    d.toggleHotClue(control);
}

/**
 * Triggered when a play/pause button is pressed
 */
Apc.toggle = function(channel, control, value, status, group) {
    var c = group.substr(8, 1); // Ugly AF to be fixed
    var d = Decks[c - 1];
    d.togglePlay(control);
}