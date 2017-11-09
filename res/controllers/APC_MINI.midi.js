////////////////////////////////////////////////////////////////////////
// JSHint configuration                                               //
////////////////////////////////////////////////////////////////////////
/* global engine                                                      */
/* global script                                                      */
/* global print                                                       */
/* global midi                                                        */
//////////////////////////////////////////////////////////////////////// 

// Supported colors enumeration
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
 * A Deck represents the mapping between a control (Standard deck or Sampler deck) in Mixxx and a column of button on the APC mini.
 * @param {number} group Control group name (ChannelN, SamplerN with the square brackets) 
 * @param {number} slots  Number of slots supported by the deck for hotcues
 * @param {number} primarySlotAddress  Id/Address of the first slot (first rectangular button in the column)
 * @param {number} playControlAddress  Id/Address of the master play/pause button (round button in the deck's column)
 * @param {bool} isSamplerDeck  true if the deck is a sample deck, false if it is a standard deck
 */
var Deck = function(group, slots, primarySlotAddress, playControlAddress, isSamplerDeck) {
    this.group = group;
    this.slots = slots;
    this.primarySlotAddress = primarySlotAddress;
    this.playControlAddress = playControlAddress;
    this.isSamplerDeck = isSamplerDeck;

    this.track = []; // Hotcue pattern for the track currently loaded in the deck 
    this.activeSlot = undefined; // Slot actually playing

    this.reset();
    this.bind();
};

/**
 * Shuts off all the lights for the given deck.
 */
Deck.prototype.reset = function() {
    var self = this;
    this.forEachSlot(function(s, t) {
        self.setSlotColor(s, COLORS.OFF);
    });
    this.setPlaying(false);
};

/**
 * Toggles the play/pause status of the deck.
 * If the deck is playing the round button under the last slot will be flashing red.
 * @param {boolean} p true if the deck is playing.
 */
Deck.prototype.setPlaying = function(p) {
    if (p) {
        midi.sendShortMsg(0x90, this.playControlAddress, COLORS.GREEN_FLASHING);
    } else {
        midi.sendShortMsg(0x90, this.playControlAddress, COLORS.OFF);
    }
};

/**
 * Sets the slot's led color.
 * Note that for ease of use, slots start at 1, the APC has max 8 slots available per deck.
 * @param {number} slot number of the slot which color should be changed
 * @param {number} color the color to set.
 */
Deck.prototype.setSlotColor = function(slot, color) {
    midi.sendShortMsg(0x90, this.slotToAddress(slot), color);
};

/**
 * Converts a slot number for a given deck to its physical midi control address.
 * @param {number} slot a slot number starting at 1.
 */
Deck.prototype.slotToAddress = function(slot) {
    return this.primarySlotAddress - ((slot - 1) * this.slots);
};

/**
 * Converts a physical midi control address to the slot number.
 * @param {number} address a midi control Id.
 */
Deck.prototype.addressToSlot = function(address) {
    return this.slots - Math.floor(address / this.slots);
};

/**
 * Helper function that will trigger a callback function for each slot of a deck.
 * @param {function(slot,address)} callback the callback function to be called for each slot.
 */
Deck.prototype.forEachSlot = function(callback) {
    for (var slot = 1; slot <= this.slots; slot++) {
        callback(slot, this.primarySlotAddress - ((slot - 1) * this.slots));
    }
};

/**
 * Binds the deck to its adequate control in Mixxx.
 */
Deck.prototype.bind = function() {
    var self = this;
    engine.connectControl(this.group, "duration", function(value, group, control) {
        self.onTrackLoaded(value, group, control);
    });
    engine.connectControl(this.group, "play", function(value, group, control) {
        self.togglePlay(control);
    });
};

/**
 * Called when the duration of the deck's loaded track changed (new track loaded).
 * this function parses the cue points and sets the slots color to COLOR.GREEN for each detected cue points.
 * Unused slots will be set to COLOR.YELLOW.
 */
Deck.prototype.onTrackLoaded = function(value, group, control) {
    var self = this;
    this.track = [];
    this.forEachSlot(function(s, t) {
        if (engine.getParameter(self.group, "hotcue_" + s + "_position") >= 0) {
            self.track.push(COLORS.GREEN);
        } else {
            self.track.push(COLORS.YELLOW);
        }
    });
    this.applyTrack();
};

/**
 * Reapplied the slots lightning for the currently loaded track.
 */
Deck.prototype.applyTrack = function() {
    for (var i = 0; i < this.slots; i++) {
        this.setSlotColor(i + 1, this.track[i]);
    }
};

/**
 * Syncs the play status of a track between Mixxx and the APC.
 */
Deck.prototype.togglePlay = function(control) {
    if (control === this.playControlAddress) {
        if (engine.getParameter(this.group, "play") === 0) {
            engine.setParameter(this.group, "play", 1);
            this.setPlaying(true);
        } else {
            engine.setParameter(this.group, "play", 0);
            this.setPlaying(false);
        }
    }
    if (engine.getParameter(this.group, "play") === 0) {
        if (this.activeSlot) {
            this.applyTrack();
            this.activeSlot = undefined;
        }
        this.setPlaying(false);
    } else {
        this.setPlaying(true);
    }
};

/**
 * Syncs the hotcue play status of a track between Mixxx and the APC.
 */
Deck.prototype.toggleHotClue = function(control) {
    if (!this.track || this.track.length === 0) return;

    var s = this.addressToSlot(control);
    if (this.track[s - 1] === COLORS.YELLOW) return; // Unused button should do no harm

    this.applyTrack(); // We reset the leds in the column

    engine.setParameter(this.group, "hotcue_" + s + "_gotoandplay", true);
    this.setSlotColor(s, COLORS.RED_FLASHING);
    this.activeSlot = s;
};

var Levels = function(group) {
    this.group = group;
    this.pregain = engine.getParameter(group, "pregain");
    this.volume = engine.getParameter(group, "pregain");
    this.prevVolume = undefined;
    this.prevGain = undefined;

    this.bind();
}

Levels.prototype.bind = function() {
    var self = this;
    engine.connectControl(this.group, "volume", function(value, group, control) {
        self.volume = value;
        self.prevVolume = value;
        print(self.volume);
    });
    engine.connectControl(this.group, "pregain", function(value, group, control) {
        self.pregain = value / 127;
    });
}

Levels.prototype.setVolume = function(value) {
    var v = value / 127;
    if (this.prevVolume === this.volume) {
        this.prevVolume = v;
        return;
    }
    print("VOL " + this.volume);
    print("PREV " + this.prevVolume);
    print("V " + v);
    if (this.prevVolume < v && v > this.volume) {
        // going up
        engine.setParameter(this.group, "volume", v);
        this.prevVolume = v;
    }
    if (this.prevVolume > v && v < this.volume) {
        // going down
        engine.setParameter(this.group, "volume", v);
        this.prevVolume = v;
    }

}

Levels.prototype.setPregain = function(value) {
    engine.setParameter(this.group, "pregain", value / 127);
}

var Group = function(group, slots, primarySlotAddress, playControlAddress, isSamplerDeck) {
    this.deck = new Deck(group, slots, primarySlotAddress, playControlAddress, isSamplerDeck);
    this.levels = new Levels(group);
}


// Object exposed through the XML configuration file
var Apc = { "groups": {}, "shift": false };

// Stored for documentation purposes
var ROOT_SLOT_ADDRESSES = [0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F];

/**
 * Inits the controller and configures the decks.
 */
Apc.init = function(id, debugging) {
    Apc.groups = {
        "[Channel1]": new Group("[Channel1]", 8, 0x38, 0x40, false),
        "[Channel2]": new Group("[Channel2]", 8, 0x39, 0x41, false),
        "[Channel3]": new Group("[Channel3]", 8, 0x3A, 0x42, false),
        "[Channel4]": new Group("[Channel4]", 8, 0x3B, 0x43, false),
        "[Sampler1]": new Group("[Sampler1]", 8, 0x3C, 0x44, true),
        "[Sampler2]": new Group("[Sampler2]", 8, 0x3D, 0x45, true),
        "[Sampler3]": new Group("[Sampler3]", 8, 0x3E, 0x46, true),
        "[Sampler4]": new Group("[Sampler4]", 8, 0x3F, 0x47, true)
    };

    engine.softTakeover("[Master]", "crossfader", true);
    engine.softTakeover("[Channel1]", "pregain", true);
    engine.softTakeover("[Channel1]", "volume", true);
    engine.softTakeover("[Channel2]", "pregain", true);
    engine.softTakeover("[Channel2]", "volume", true);
    engine.softTakeover("[Channel3]", "pregain", true);
    engine.softTakeover("[Channel3]", "volume", true);
    engine.softTakeover("[Channel4]", "pregain", true);
    engine.softTakeover("[Channel4]", "volume", true);
    engine.softTakeover("[Sampler1]", "pregain", true);
    engine.softTakeover("[Sampler1]", "volume", true);
    engine.softTakeover("[Sampler2]", "pregain", true);
    engine.softTakeover("[Sampler2]", "volume", true);
    engine.softTakeover("[Sampler3]", "pregain", true);
    engine.softTakeover("[Sampler3]", "volume", true);
    engine.softTakeover("[Sampler4]", "pregain", true);
    engine.softTakeover("[Sampler4]", "volume", true);
};

/**
 * When we leave all the leds should be turned off.
 */
Apc.shutdown = function() {
    Decks.forEach(function(d) {
        d.reset();
    }, this);
};

/**
 * Triggered when a slot button is pressed.
 */
Apc.hotcue = function(channel, control, value, status, group) {
    Apc.groups[group].deck.toggleHotClue(control);
};

/**
 * Triggered when a play/pause button is pressed
 */
Apc.toggle = function(channel, control, value, status, group) {
    Apc.groups[group].deck.togglePlay(control);
};

Apc.shiftOn = function(channel, control, value, status, group) {
    Apc.shift = true;
}

Apc.shiftOff = function(channel, control, value, status, group) {
    Apc.shift = false;
}

Apc.volume = function(channel, control, value, status, group) {
    if (Apc.shift) {
        Apc.groups[group].levels.setPregain(value);
    } else {
        Apc.groups[group].levels.setVolume(value);
    }
}