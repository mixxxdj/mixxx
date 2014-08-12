// Script for the richly-featured Denon HS5500 controller.

function DNHS5500() {}

// Eventually we will support 2 of these devices -- in that case
// one device will be decks 1 and 3, and the other will be 2 and 4.
// More work needed.
DNHS5500.firstDeckGroup = "[Channel1]";
DNHS5500.secondDeckGroup = "[Channel2]";
DNHS5500.midiChannelBase = 0xB0;

DNHS5500.init = function init(id,debug) {
    engine.connectControl(DNHS5500.firstDeckGroup,"rate", "DNHS5500.rateDisplay");
    engine.connectControl(DNHS5500.secondDeckGroup,"rate", "DNHS5500.rateDisplay");
    engine.connectControl(DNHS5500.firstDeckGroup, "play", "DNHS5500.playChanged");
    engine.connectControl(DNHS5500.secondDeckGroup, "play", "DNHS5500.playChanged");
//    engine.connectControl(DNHS5500.firstDeckGroup, "spinny_angle", "DNHS5500.spinnyAngleChanged");
//    engine.connectControl(DNHS5500.secondDeckGroup, "spinny_angle", "DNHS5500.spinnyAngleChanged");
    engine.connectControl(DNHS5500.firstDeckGroup, "playposition", "DNHS5500.playPositionChanged");
    engine.connectControl(DNHS5500.secondDeckGroup, "playposition", "DNHS5500.playPositionChanged");
    engine.connectControl(DNHS5500.firstDeckGroup,"eject","DNHS5500.eject");
    engine.connectControl(DNHS5500.secondDeckGroup,"eject","DNHS5500.eject");
    engine.connectControl(DNHS5500.firstDeckGroup,"scratch2_enable","DNHS5500.scratchLight");
    engine.connectControl(DNHS5500.secondDeckGroup,"scratch2_enable","DNHS5500.scratchLight");

    // Lights on this controller are chosen by midi value, not control value, so they all
    // need to be script-controlled.
    engine.connectControl(DNHS5500.firstDeckGroup,"play_indicator","DNHS5500.playLight");
    engine.connectControl(DNHS5500.secondDeckGroup,"play_indicator","DNHS5500.playLight");
    engine.connectControl(DNHS5500.firstDeckGroup,"cue_indicator","DNHS5500.cueLight");
    engine.connectControl(DNHS5500.secondDeckGroup,"cue_indicator","DNHS5500.cueLight");
    engine.connectControl(DNHS5500.firstDeckGroup,"keylock","DNHS5500.keylockLight");
    engine.connectControl(DNHS5500.secondDeckGroup,"keylock","DNHS5500.keylockLight");
    engine.connectControl(DNHS5500.firstDeckGroup,"hotcue_1_enabled","DNHS5500.hotcue1Light");
    engine.connectControl(DNHS5500.secondDeckGroup,"hotcue_1_enabled","DNHS5500.hotcue1Light");
    engine.connectControl(DNHS5500.firstDeckGroup,"hotcue_2_enabled","DNHS5500.hotcue2Light");
    engine.connectControl(DNHS5500.secondDeckGroup,"hotcue_2_enabled","DNHS5500.hotcue2Light");
    engine.connectControl(DNHS5500.firstDeckGroup,"hotcue_3_enabled","DNHS5500.hotcue3Light");
    engine.connectControl(DNHS5500.secondDeckGroup,"hotcue_3_enabled","DNHS5500.hotcue3Light");
    engine.connectControl(DNHS5500.firstDeckGroup,"reverse","DNHS5500.reverseLight");
    engine.connectControl(DNHS5500.secondDeckGroup,"reverse","DNHS5500.reverseLight");
    engine.connectControl(DNHS5500.firstDeckGroup,"loop_enabled","DNHS5500.loopEnabledLight");
    engine.connectControl(DNHS5500.secondDeckGroup,"loop_enabled","DNHS5500.loopEnabledLight");
    engine.connectControl(DNHS5500.firstDeckGroup,"reloop_exit","DNHS5500.exitReloopLight");
    engine.connectControl(DNHS5500.secondDeckGroup,"reloop_exit","DNHS5500.exitReloopLight");
    engine.connectControl(DNHS5500.firstDeckGroup,"loop_start_position","DNHS5500.loopInLight");
    engine.connectControl(DNHS5500.secondDeckGroup,"loop_start_position","DNHS5500.loopInLight");
    engine.connectControl(DNHS5500.firstDeckGroup,"loop_end_position","DNHS5500.loopOutLight");
    engine.connectControl(DNHS5500.secondDeckGroup,"loop_end_position","DNHS5500.loopOutLight");
    engine.connectControl(DNHS5500.firstDeckGroup,"repeat","DNHS5500.repeatLight");
    engine.connectControl(DNHS5500.secondDeckGroup,"repeat","DNHS5500.repeatLight");

    // The jog wheel does not send events when it's not moving, so we have to
    // detect that.  We do so by adding one to this accumulator every time
    // we get a platter rotation message and resetting it to zero every time
    // we get a real jog wheel message.  When the accumulator gets above a magic
    // number, we assume that the platter is stopped and send a zero-velocity message.
    DNHS5500.wheelZeroVelocityAccum = [0, 0];

    DNHS5500.scratchEnable(1, false);
    DNHS5500.scratchEnable(2, false);
    DNHS5500.clearTrackDisplay(DNHS5500.firstDeckGroup);
    DNHS5500.clearTrackDisplay(DNHS5500.secondDeckGroup);

    DNHS5500.braking = [false, false];
}

DNHS5500.shutdown = function shutdown() {
    // Turn off platter movement.
    midi.sendShortMsg(DNHS5500.midiChannelBase, 0x66, 0x00);
    midi.sendShortMsg(DNHS5500.midiChannelBase + 1, 0x66, 0x00);

    // Clear most of the LCD and lights.
    DNHS5500.clearTrackDisplay(DNHS5500.firstDeckGroup);
    DNHS5500.clearTrackDisplay(DNHS5500.secondDeckGroup);

    // Now clear all of the lights that are not track-associated.
    DNHS5500.repeatLight(0, DNHS5500.firstDeckGroup);
    DNHS5500.repeatLight(0, DNHS5500.secondDeckGroup);
    DNHS5500.keylockLight(0, DNHS5500.firstDeckGroup);
    DNHS5500.keylockLight(0, DNHS5500.secondDeckGroup);
}

DNHS5500.clearTrackDisplay = function (group) {
    DNHS5500.hotcue1Light(0, group);
    DNHS5500.hotcue2Light(0, group);
    DNHS5500.hotcue3Light(0, group);
    DNHS5500.loopInLight(0, group);
    DNHS5500.loopOutLight(0, group);
    DNHS5500.rateDisplayClear(group);
//    DNHS5500.spinnyAngleChanged(0, group);
    DNHS5500.playPositionChanged(0, group);
    DNHS5500.loopInLight(-1, group);
    DNHS5500.loopOutLight(-1, group);
    DNHS5500.scratchLight(0, group);
    DNHS5500.playLight(0, group);
    DNHS5500.cueLight(0, group);
}

DNHS5500.channelForGroup = function (group) {
    if (group == DNHS5500.firstDeckGroup) {
        return DNHS5500.midiChannelBase;
    } else {
        return DNHS5500.midiChannelBase + 1;
    }
}

// XXX: how to support decks 3 and 4?
DNHS5500.groupForChannel = function (channel) {
    if (channel == 0) {
        return DNHS5500.firstDeckGroup;
    } else {
        return DNHS5500.secondDeckGroup;
    }
}

// XXX: properly support decks 3 and 4.
DNHS5500.deckForChannel = function (channel) {
    if (channel == 0) {
        return 1;
    } else {
        return 2;
    }
}

DNHS5500.rateDisplay = function (value, group) {
    var channelmidi = DNHS5500.channelForGroup(group);
    var rateDir = engine.getValue(group, "rate_dir");
    var raterange = engine.getValue(group, "rateRange");
    var rate = engine.getValue(group, "rate");
    var slider_rate = ((rate * raterange) * 100) * rateDir;
    var rate_abs = Math.abs(slider_rate);
    var rate_dec = Math.floor(rate_abs);
    var rate_frac = Math.round((rate_abs - rate_dec) * 100);

    // MM.LL
    midi.sendShortMsg(channelmidi, 0x71, rate_dec);
    midi.sendShortMsg(channelmidi, 0x72, rate_frac);

    // +/- symbol
    if (rate >= 0) {
        midi.sendShortMsg(channelmidi, 0x45, 0x01);
    } else {
        midi.sendShortMsg(channelmidi, 0x45, 0x02);
    }

    // BPM
    var bpm = engine.getValue(group, "bpm");
    // bpm display is split like MML.L
    var bpm_dec = Math.floor(bpm / 10);
    var bpm_frac = Math.round((bpm - (bpm_dec * 10)) * 10);
    midi.sendShortMsg(channelmidi, 0x73, bpm_dec);
    midi.sendShortMsg(channelmidi, 0x74, bpm_frac);
}

DNHS5500.rateDisplayClear = function (group) {
    // We can't make this information disappear, so set to zeros.
    var channelmidi = DNHS5500.channelForGroup(group);
    midi.sendShortMsg(channelmidi, 0x71, 0);
    midi.sendShortMsg(channelmidi, 0x72, 0);
    midi.sendShortMsg(channelmidi, 0x45, 0x01);
    midi.sendShortMsg(channelmidi, 0x73, 0);
    midi.sendShortMsg(channelmidi, 0x74, 0);
}

DNHS5500.playPositionChanged = function (value, group) {
    var channelmidi = DNHS5500.channelForGroup(group);

    // Track percentage position.
    var reversed = engine.getValue(group, "reverse");
    if (reversed) {
        midi.sendShortMsg(channelmidi, 0x49, Math.round(value * 100));
    } else {
        midi.sendShortMsg(channelmidi, 0x48, Math.round(value * 100));
    }

    // Time position.
    // TODO: support remaining time.
    // mm:ss.ff
    var duration = engine.getValue(group, "duration");
    var pos_total_secs = value * duration;
    var pos_minutes = Math.floor(pos_total_secs / 60);
    var pos_secs = pos_total_secs % 60;
    var pos_frac = Math.round((pos_total_secs - Math.floor(pos_total_secs)) * 100);

    midi.sendShortMsg(channelmidi, 0x42, pos_minutes);
    midi.sendShortMsg(channelmidi, 0x43, pos_secs);
    midi.sendShortMsg(channelmidi, 0x44, pos_frac);
}

/*DNHS5500.spinnyAngleChanged = function (angle, group) {
    // XXX: This only gets updated if the spinnys are visible in the UI.
    var channelmidi = DNHS5500.channelForGroup(group);
    // The angle is 1-180 to the right, and 0- -180 to the left.
    var midi_value = 0;
    if (angle > 0) {
        // Values above and equal zero are mapped from 0x22 to 0x31, which is a range of 16 (0x10).
        midi_value = Math.round(angle * 16.0 / 180.0 + 0x22);
    } else {
        // Values below zero are mapped from 0x41 to 0x32, also 16 (0x10).
        // But the midi numbers are flopped versus the angle values.
        midi_value = Math.round((180.0 + angle) * 16.0 / 180.0 + 0x31);
    }
    midi.sendShortMsg(channelmidi, 0x4D, midi_value);
}*/

// The button that enables/disables scratching.  The wheel itself is supposed
// to send this message but never does.  Instead we use the "platter source" button.
DNHS5500.wheelTouch = function (channel, control, value, status) {
    var decknum = DNHS5500.deckForChannel(channel);
    if (value == 0x40) {
        // Ramp from current rate to scratch rate.
        DNHS5500.scratchEnable(decknum, true);
    } else {
        // Ramp from scratch rate to current rate.
        engine.scratchDisable(decknum, true);
    }
}

DNHS5500.scratchEnable = function (deck, ramp) {
    // Try lower damp values for more responsive scratching.
    var damp_value = 32;
    var alpha = 1.0 / damp_value;
    var beta = alpha / damp_value;
    engine.scratchEnable(deck, 1480, 33+1/3, alpha, beta, ramp);
}

//DNHS5500.scratchEnableDamped = function (deck) {
//    var alpha = 1.0/256;
//    var beta = alpha/256;
//    engine.scratchEnable(deck, 1480, 33+1/3, alpha, beta, ramp);
//}

// See above.  We never get this signal.
DNHS5500.wheelRelease = function (channel, control, value, status) {
    var decknum = DNHS5500.deckForChannel(channel);
    if (value == 0x00) {
        // Ramp from to saved rate from scratch rate.
        engine.scratchDisable(decknum, true);
    }
}

DNHS5500.wheelTurn = function (channel, control, value, status) {
    // Reports the velocity of the top disc-like wheel, as well as touch events.
    var deckNumber = DNHS5500.deckForChannel(channel);
    var group = DNHS5500.groupForChannel(channel);

    // Velocity values.
    var velocity = value - 0x40;
    // We use control == 0 when making calls internally.
    if (control != 0) {
        // We got a real update, so reset the accumulator.
        DNHS5500.wheelZeroVelocityAccum[channel] = 0;
    }

    var currentlyPlaying = engine.getValue(group, "play");
    if (currentlyPlaying) {
        // If we are playing, a velocity of 1 means no change.
        velocity -= 1;
    }

    if (engine.isScratching(deckNumber)) {
        engine.scratchTick(deckNumber, velocity);
    } else {
        engine.setValue(group, "jog", velocity);
    }
}

DNHS5500.platterTurn = function (channel, control, value, status) {
    // Reports the velocity of the lower platter.
    var velocity = value - 0x40;
    var deckNumber = DNHS5500.deckForChannel(channel);
    var group = DNHS5500.groupForChannel(channel);

    var currentlyPlaying = engine.getValue(group, "play");
    if (currentlyPlaying && velocity == 1) {
        DNHS5500.wheelZeroVelocityAccum[channel] += 1;
        // If the accumulator doesn't get reset, the wheel is stopped.
        // So far, 3 seems to be high enough that we don't get spurious reports.
        if (DNHS5500.wheelZeroVelocityAccum[channel] > 3) {
            // Fake a zero
            DNHS5500.wheelTurn(channel, 0, 0x40, 0);
        }
    }
}

DNHS5500.playButton = function (channel, control, value, status) {
    // Only respond to presses.
    if (value == 0) {
        return;
    }
    var channelname = DNHS5500.groupForChannel(channel);
    var currentlyPlaying = engine.getValue(channelname, "play");
    // Toggle it.
    if (currentlyPlaying) {
        engine.setValue(channelname, "play", 0.0);
    } else {
        engine.setValue(channelname, "play", 1.0);
    }
    DNHS5500.playChanged(value, DNHS5500.groupForChannel(channel));
}

DNHS5500.playChanged = function (value, group) {
    var channelmidi = DNHS5500.channelForGroup(group);
    // XXX: support 4 decks.
    if (group == DNHS5500.firstDeckGroup) {
        var deck = 1;
    } else {
        var deck = 2;
    }

    var currentlyPlaying = engine.getValue(group, "play");
    if (currentlyPlaying == 1) {
        // Disable scratch mode, don't ramp.
        engine.scratchDisable(deck, false);
        // Turn on platter!
        midi.sendShortMsg(channelmidi, 0x66, 0x7F);
        // platter FWD
        midi.sendShortMsg(channelmidi, 0x67, 0x00);
    } else {
        // Platter off
        midi.sendShortMsg(channelmidi, 0x66, 0x00);
        // Scratch on, don't ramp.
        DNHS5500.scratchEnable(deck, false);
        // Reset brake. XXX: HACK;
        DNHS5500.brake(deck - 1, 0, 1, 1);
    }
}

DNHS5500.selectTrack = function (channel, control, value, status) {
    if (value > 0) {
        engine.setValue("[Playlist]", "SelectPrevTrack", 1);
    } else {
        engine.setValue("[Playlist]", "SelectNextTrack", 1);
    }
}

DNHS5500.eject = function (channel, control, value, status) {
    if (value == 0) {
        return;
    }
    var group = DNHS5500.groupForChannel(channel);
    DNHS5500.clearTrackDisplay(group);
}

DNHS5500.brake = function (channel, control, value, status) {
    if (value == 0) {
        return;
    }

    var decknum = DNHS5500.deckForChannel(channel);
    var group = DNHS5500.groupForChannel(channel);
    if (DNHS5500.braking[channel] == true) {
        DNHS5500.braking[channel] = false;
        engine.brake(decknum, false);
        DNHS5500.brakeLight(0, group);
    } else {
        DNHS5500.braking[channel] = true;
        engine.brake(decknum, true);
        DNHS5500.brakeLight(1, group);
    }
}

DNHS5500.toggleLightLayer1 = function (group, light, status) {
    var channel = DNHS5500.channelForGroup(group);
    if (status > 0) {
        midi.sendShortMsg(channel, 0x4A, light);
    } else {
        midi.sendShortMsg(channel, 0x4B, light);
    }
}

DNHS5500.toggleLightLayer2 = function (group, light, status) {
    var channel = DNHS5500.channelForGroup(group);
    if (status > 0) {
        midi.sendShortMsg(channel, 0x4D, light);
    } else {
        midi.sendShortMsg(channel, 0x4E, light);
    }
}

DNHS5500.playLight = function (value, group) {
    DNHS5500.toggleLightLayer1(group, 0x27, value);
}

DNHS5500.cueLight = function (value, group) {
    DNHS5500.toggleLightLayer1(group, 0x26, value);
}

DNHS5500.keylockLight = function (value, group) {
    DNHS5500.toggleLightLayer1(group, 0x08, value);
    DNHS5500.toggleLightLayer2(group, 0x14, value);
}

DNHS5500.hotcue1Light = function (value, group) {
    DNHS5500.toggleLightLayer1(group, 0x11, value);
}

DNHS5500.hotcue2Light = function (value, group) {
    DNHS5500.toggleLightLayer1(group, 0x13, value);
}

DNHS5500.hotcue3Light = function (value, group) {
    DNHS5500.toggleLightLayer1(group, 0x15, value);
}

DNHS5500.reverseLight = function (value, group) {
    DNHS5500.toggleLightLayer1(group, 0x3A, value);
}

DNHS5500.scratchLight = function (value, group) {
    DNHS5500.toggleLightLayer1(group, 0x3B, value);
}

DNHS5500.exitReloopLight = function (value, group) {
    DNHS5500.toggleLightLayer1(group, 0x42, value);
}

DNHS5500.loopEnabledLight = function (value, group) {
    if (value != 0) {
        // Include the () when active.
        DNHS5500.toggleLightLayer2(group, 0x42, 1);
        DNHS5500.toggleLightLayer2(group, 0x44, 1);
    } else {
        // Turn off () when inactive.
        DNHS5500.toggleLightLayer2(group, 0x1A, 1);
        DNHS5500.toggleLightLayer2(group, 0x1C, 1);
    }
}

DNHS5500.loopInLight = function (value, group) {
    var enabled = (value != -1);
    DNHS5500.toggleLightLayer2(group, 0x1A, enabled);
}

DNHS5500.loopOutLight = function (value, group) {
    var enabled = (value != -1);
    DNHS5500.toggleLightLayer2(group, 0x1C, enabled);
}

DNHS5500.repeatLight = function (value, group) {
    if (value > 0) {
        DNHS5500.toggleLightLayer2(group, 0x04, 1);
    } else {
        DNHS5500.toggleLightLayer2(group, 0x05, 1);
    }
}

DNHS5500.brakeLight = function (value, group) {
    DNHS5500.toggleLightLayer1(group, 0x28, value);
}
