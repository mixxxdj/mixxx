/**
 * Denon DJ MC4000 DJ controller script for Mixxx 2.1
 * 
 * Copyright (C) 2016 Tim Rae <perceptualchaos2@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**/

var MC4000 = {}; 

/**
 * User definable controller parameters
 */
 
// Pitch fader ranges to cycle through with the RANGE button
MC4000.rateRanges = [0.08, 0.16, 0.5];

// Scratch algorithm parameters
MC4000.scratchParams = {
    recordSpeed: 33.3333333333,
    alpha: (1.0/8),
    beta: (1.0/8)/32.0
};

// Sensitivity of the jog wheel (NOTE: sensitivity depends on audio latency)
MC4000.jogParams = {
    sensitivity: 25,
    maxJogValue: 3
};

// Hardware constant for resolution of the jog wheel
// This was found by averaging total 'ticks' received over 100 rotations
MC4000.jogWheelTicksPerRevolution = 605;


/**
 * Initialization method for the MC4000 container class
 */
MC4000.init = function () {
    // Decks
    MC4000.leftDeck = new MC4000.Deck(0);
    MC4000.rightDeck = new MC4000.Deck(1);
    // VU meters
    // TODO: implement VU meters for the PFL when cue button is on
    engine.connectControl("[Master]", "VuMeterL", "MC4000.OnVuMeterChangeL");
    engine.connectControl("[Master]", "VuMeterR", "MC4000.OnVuMeterChangeR");
    // Control all sampler levels simultaneously with the single knob on the mixer
    MC4000.samplerLevel = function (channel, control, value, status, group) {
        engine.setValue("[Sampler1]", "pregain", script.absoluteNonLin(value, 0, 1.0, 4.0));
        engine.setValue("[Sampler2]", "pregain", script.absoluteNonLin(value, 0, 1.0, 4.0));
        engine.setValue("[Sampler3]", "pregain", script.absoluteNonLin(value, 0, 1.0, 4.0));
        engine.setValue("[Sampler4]", "pregain", script.absoluteNonLin(value, 0, 1.0, 4.0));
    }
    // Get the controller to send its current status (Sniffed from Serato with Snoize Midi Monitor spy function)
    var byteArray = [ 0xF0, 0x00, 0x02, 0x0B, 0x7F, 0x01, 0x60, 0x00, 0x04, 0x04, 0x01, 0x00, 0x00, 0xF7 ];
    midi.sendSysexMsg(byteArray,byteArray.length);
};

MC4000.shutdown = function () {};


/**
 * Container class to hold the controls which are repeated on both decks
 */
MC4000.Deck = function (channel) {
    // Some state variables
    this.isVinylMode = true;
    this.midiChannel = channel;
    this.engineChannel = channel + 1;
    this.group = '[Channel' + this.engineChannel + ']';
    
    // Initialize vinyl mode LED
    midi.sendShortMsg(0x90 + this.midiChannel, 0x07, this.isVinylMode ? 0x7F: 0x00);
    // Match pitch fader direction with controller
    engine.setValue(this.group, "rate_dir", -1);
    // Callback for touching the jog wheel platter
    this.platterTouch = function (channel, control, value, status, group) {
        if (this.isVinylMode) {
            if (value > 0) {
                engine.scratchEnable(this.engineChannel, MC4000.jogWheelTicksPerRevolution, 
                    MC4000.scratchParams.recordSpeed , MC4000.scratchParams.alpha, MC4000.scratchParams.beta);
            } else { 
                engine.scratchDisable(this.engineChannel);
            }
        }
    }
    // Callback for toggling the vinyl mode button
    this.vinylModeToggle = function (channel, control, value, status, group) {
        if (value == 0) return;     // don't respond to note off messages
        this.isVinylMode = !this.isVinylMode;
        midi.sendShortMsg(0x90 + channel, 0x07, this.isVinylMode ? 0x7F: 0x00);
    }

    // Rate range toggle button callback
    this.rateRange = function(midichan, control, value, status, group) {
        if (value == 0) return;     // don't respond to note off messages
        var currRateRange = engine.getValue(group, "rateRange");
        engine.setValue(this.group, "rateRange", MC4000.getNextRateRange(currRateRange));
    };


    // The jog wheel periodically sends the number of "ticks" it has been rotated by since the last message
    // For scratching, we just tell this directly to the scratch engine, which rotates the record by specified amount.
    // Jogging works differently: the "jog" engine samples the jog value at a rate of 1/(sound_card_latency),
    // adds this jog value to an internal 25 sample buffer, and then sets the jog value to zero.
    // The engine takes the average of the 25 sample buffer, divides by 10, and adds this to the rate at 
    // which the song is playing (e.g. determined by the pitch fader). Since the effect of this depends on many factors 
    // we can only really give an empirical senstivity which makes jog work "how we like it".        
    this.jogWheel = function (channel, control, value, status, group) {
        var numTicks = (value < 0x40) ? value: (value - 0x80);
        if (engine.isScratching(this.engineChannel)) {
            engine.scratchTick(this.engineChannel, numTicks);
        } else {
            var jogDelta = numTicks/MC4000.jogWheelTicksPerRevolution*MC4000.jogParams.sensitivity;
            var jogAbsolute = jogDelta + engine.getValue(group, "jog");
            var limit = MC4000.jogParams.maxJogValue;
            engine.setValue(group, "jog", Math.max(-limit, Math.min(limit, jogAbsolute)));
        }
    }
};

/// Callback to set the FX wet/dry value
MC4000.fxWetDry = function(midichan, control, value, status, group) {
    var numTicks = (value < 0x40) ? value: (value - 0x80);
    var newVal = engine.getValue(group, "mix") + numTicks/64.0*2;
    engine.setValue(group, "mix", Math.max(0, Math.min(1, newVal)));
};

/// Callback to set the headphone split mode
MC4000.headphoneSplit = function (channel, control, value, status, group) {
    engine.setValue(group, "headSplit", Math.min(value, 1.0));
}

/// Callback to set the left Vu Meter
MC4000.OnVuMeterChangeL = function(value, group, control) {
    midi.sendShortMsg(0xBF, 0x44, value*0x7F);
}

/// Callback to set the right Vu Meter
MC4000.OnVuMeterChangeR = function(value, group, control) {
    midi.sendShortMsg(0xBF, 0x45, value*0x7F);
}

/** 
 * Return the next rate range which is greater than the currently set
 * rate range, or cycle back to the first rate range if at the last value 
 */
MC4000.getNextRateRange = function (currRateRange) {
    for (var i = 0; i < MC4000.rateRanges.length; i++) {
        if (MC4000.rateRanges[i] > currRateRange) {
            return MC4000.rateRanges[i];
        }
    }
    return MC4000.rateRanges[0];
};
