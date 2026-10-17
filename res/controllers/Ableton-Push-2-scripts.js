// Ableton Push 2 MIDI mapping for Mixxx 2.6.
//
// Use the Push 2 User MIDI port. The separate "Ableton Push 2 Display"
// USB-bulk mapping drives the screen.

// eslint-disable-next-line no-var
var Push2 = {
    shiftPressed: false,
    connections: [],

    transportControls: {
        20: {group: "[Channel1]", key: "play", kind: "toggle"},
        21: {
            group: "[Channel1]",
            key: "cue_default",
            ledKey: "cue_indicator",
            kind: "momentary"
        },
        22: {group: "[Channel1]", key: "sync_enabled", kind: "toggle"},
        23: {group: "[Channel1]", key: "pfl", kind: "toggle"},
        24: {group: "[Channel2]", key: "play", kind: "toggle"},
        25: {
            group: "[Channel2]",
            key: "cue_default",
            ledKey: "cue_indicator",
            kind: "momentary"
        },
        26: {group: "[Channel2]", key: "sync_enabled", kind: "toggle"},
        27: {group: "[Channel2]", key: "pfl", kind: "toggle"}
    },

    // Default Push 2 palette: 126 is green, 127 is red, 125 is blue.
    colors: {
        off: 0,
        deck1: 127,
        deck2: 125,
        active: 126
    }
};

Push2.init = function() {
    // User mode makes Push send control events to the User MIDI port.
    midi.sendSysexMsg(
        [0xF0, 0x00, 0x21, 0x1D, 0x01, 0x01, 0x0A, 0x01, 0xF7],
        9);

    Push2.connectTransportLEDs();
    Push2.connectHotcueLEDs();
    Push2.refreshLEDs();
};

Push2.shutdown = function() {
    for (let control = 20; control <= 27; ++control) {
        midi.sendShortMsg(0xB0, control, 0);
    }
    for (let note = 36; note <= 51; ++note) {
        midi.sendShortMsg(0x90, note, 0);
    }
    for (const connection of Push2.connections) {
        connection.disconnect();
    }
    Push2.connections = [];
};

Push2.relativeDelta = function(value) {
    return value < 64 ? value : value - 128;
};

Push2.transport = function(channel, control, value) {
    const mapping = Push2.transportControls[control];
    if (!mapping) {
        return;
    }

    if (mapping.kind === "momentary") {
        engine.setValue(mapping.group, mapping.key, value > 0 ? 1 : 0);
    } else if (value > 0) {
        script.toggleControl(mapping.group, mapping.key);
    }
};

Push2.shift = function(channel, control, value) {
    Push2.shiftPressed = value > 0;
};

Push2.browse = function(channel, control, value) {
    if (value === 0) {
        return;
    }
    if (control === 44) {
        script.triggerControl("[Channel1]", "LoadSelectedTrack");
    } else if (control === 45) {
        script.triggerControl("[Channel2]", "LoadSelectedTrack");
    } else if (control === 46) {
        script.triggerControl("[Library]", "MoveUp");
    } else if (control === 47) {
        script.triggerControl("[Library]", "MoveDown");
    }
};

Push2.browseEncoder = function(channel, control, value) {
    const delta = Push2.relativeDelta(value);
    const key = delta > 0 ? "MoveDown" : "MoveUp";
    for (let step = 0; step < Math.abs(delta); ++step) {
        script.triggerControl("[Library]", key);
    }
};

Push2.tempoEncoderTouch = function(channel, control, value) {
    // Reuse Mixxx's momentary touch control to communicate with the separate
    // QML display mapping. Touch sends a nonzero velocity; release sends zero.
    engine.setValue("[Controls]", "touch_shift", value > 0 ? 1 : 0);
};

Push2.waveformZoomEncoder = function(channel, control, value) {
    const delta = Push2.relativeDelta(value);
    if (delta === 0) {
        return;
    }

    // Keep the two Push waveforms at the same zoom. A higher Mixxx waveform
    // zoom value shows a longer section of the track.
    const currentZoom = engine.getValue("[Channel1]", "waveform_zoom");
    const nextZoom = Math.max(1, Math.min(10, currentZoom + delta * 0.25));
    for (let deck = 1; deck <= 2; ++deck) {
        engine.setValue("[Channel" + deck + "]", "waveform_zoom", nextZoom);
    }
};

Push2.loadTrack = function(channel, control, value, status, group) {
    if (value > 0) {
        script.triggerControl(group, "LoadSelectedTrack");
    }
};

Push2.deckEncoder = function(channel, control, value, status, group) {
    const encoderIndex = (control - 71) % 4;
    const delta = Push2.relativeDelta(value) * 0.01;
    let targetGroup;
    let targetKey;

    if (encoderIndex < 3) {
        const deck = script.deckFromGroup(group);
        targetGroup = "[EqualizerRack1_[Channel" + deck + "]_Effect1]";
        // Encoders are physically ordered high, mid, low.
        targetKey = "parameter" + (3 - encoderIndex);
    } else {
        targetGroup = "[QuickEffectRack1_" + group + "]";
        targetKey = "super1";
    }

    const nextValue = Math.max(
        0,
        Math.min(1, engine.getParameter(targetGroup, targetKey) + delta));
    engine.setParameter(targetGroup, targetKey, nextValue);
};

Push2.hotcuePad = function(channel, control, value, status, group) {
    if (value === 0) {
        return;
    }

    const hotcue = group === "[Channel1]" ? control - 35 : control - 43;
    const action = Push2.shiftPressed ? "clear" : "activate";
    script.triggerControl(group, "hotcue_" + hotcue + "_" + action);
};

Push2.connectTransportLEDs = function() {
    Object.keys(Push2.transportControls).forEach(function(controlString) {
        const control = Number(controlString);
        const mapping = Push2.transportControls[control];
        const connection = engine.makeConnection(
            mapping.group,
            mapping.ledKey || mapping.key,
            function(value) {
                const color = mapping.group === "[Channel1]"
                    ? Push2.colors.deck1
                    : Push2.colors.deck2;
                midi.sendShortMsg(0xB0, control, value > 0 ? color : 0);
            });
        Push2.connections.push(connection);
    });
};

Push2.connectHotcueLEDs = function() {
    for (let deck = 1; deck <= 2; ++deck) {
        const group = "[Channel" + deck + "]";
        const firstNote = deck === 1 ? 36 : 44;
        const color = deck === 1 ? Push2.colors.deck1 : Push2.colors.deck2;

        for (let hotcue = 1; hotcue <= 8; ++hotcue) {
            const note = firstNote + hotcue - 1;
            const connection = engine.makeConnection(
                group,
                "hotcue_" + hotcue + "_enabled",
                function(value) {
                    midi.sendShortMsg(0x90, note, value > 0 ? color : 0);
                });
            Push2.connections.push(connection);
        }
    }
};

Push2.refreshLEDs = function() {
    Object.keys(Push2.transportControls).forEach(function(controlString) {
        const control = Number(controlString);
        const mapping = Push2.transportControls[control];
        const value = engine.getValue(mapping.group, mapping.ledKey || mapping.key);
        const color = mapping.group === "[Channel1]"
            ? Push2.colors.deck1
            : Push2.colors.deck2;
        midi.sendShortMsg(0xB0, control, value > 0 ? color : 0);
    });

    for (let deck = 1; deck <= 2; ++deck) {
        const group = "[Channel" + deck + "]";
        const firstNote = deck === 1 ? 36 : 44;
        const color = deck === 1 ? Push2.colors.deck1 : Push2.colors.deck2;
        for (let hotcue = 1; hotcue <= 8; ++hotcue) {
            const enabled = engine.getValue(group, "hotcue_" + hotcue + "_enabled");
            midi.sendShortMsg(0x90, firstNote + hotcue - 1, enabled > 0 ? color : 0);
        }
    }
};
