// eslint-disable-next-line no-var
var VestaxVCI380 = {};

// Variables
VestaxVCI380.beatPos = [0, 0];

// Color table. Colors are binary RGB, each in 2 variants : normal or dimmed
VestaxVCI380.padColor = {"OFF": 0x00, "BLUE": 0x18, "GREEN": 0x28, "CYAN": 0x38, "RED": 0x48, "MAGENTA": 0x58, "YELLOW": 0x68, "WHITE": 0x78, "dimBLUE": 0x10, "dimGREEN": 0x20, "dimCYAN": 0x30, "dimRED": 0x40, "dimMAGENTA": 0x50, "dimYELLOW": 0x60, "dimWHITE": 0x70};

// NI Stems format: up to 4 stems; one pad row per stem
VestaxVCI380.MAX_STEMS = 4;
// Whether our Mixxx version supports stems
VestaxVCI380.hasStems = false;

// Color mapper
VestaxVCI380.ColorMapper = new ColorMapper({
    0xFF0000: 0x48, // red
    0x00FF00: 0x28, // green
    0x0000FF: 0x18, // blue
    0x00FFFF: 0x38, // cyan
    0xFF00FF: 0x58, // magenta
    0xFFFF00: 0x68, // yellow
    0xFFFFFF: 0x78, // white
});

//// COLOR preferences
//// you can change them to your preference
VestaxVCI380.colorHotCueActive=VestaxVCI380.padColor.GREEN;
VestaxVCI380.colorHotcueUnset=VestaxVCI380.padColor.dimBLUE;
VestaxVCI380.colorHotcueActivate=VestaxVCI380.padColor.CYAN;
VestaxVCI380.colorHotcueDelete=VestaxVCI380.padColor.RED;
VestaxVCI380.colorBeatActive=VestaxVCI380.padColor.WHITE;
VestaxVCI380.colorCurrentBeat=VestaxVCI380.padColor.YELLOW;
VestaxVCI380.colorBeatInactive=VestaxVCI380.padColor.dimCYAN;
VestaxVCI380.colorBeatPushedBpm=VestaxVCI380.padColor.RED;
VestaxVCI380.colorBeatPushedAlignGrid=VestaxVCI380.padColor.YELLOW;
VestaxVCI380.colorSamplerEmpty=VestaxVCI380.padColor.dimWHITE;
VestaxVCI380.colorSamplerLoaded=VestaxVCI380.padColor.GREEN;
VestaxVCI380.colorSamplerPlaying=VestaxVCI380.padColor.YELLOW;

// 'Splash screen' : display a customizable color pattern instead of hot cues when no track is loaded
VestaxVCI380.splashScreen = [["RED", "BLUE", "RED", "BLUE", "BLUE", "RED", "BLUE", "RED"], ["GREEN", "WHITE", "GREEN", "WHITE", "WHITE", "GREEN", "WHITE", "GREEN"]];

// We need to map sampler and button numbers so the controller and GUI layout match
VestaxVCI380.buttonToSampler = {1: 1, 2: 2, 3: 5, 4: 6, 5: 3, 6: 4, 7: 7, 8: 8};
VestaxVCI380.samplerToButton = {1: 1, 2: 2, 5: 3, 6: 4, 3: 5, 4: 6, 7: 7, 8: 8};

VestaxVCI380.connections=[];        // permanent control connections
VestaxVCI380.modeConnections=[];    // temporary control connections related to current mode ..
VestaxVCI380.modeConnections[0]=[]; // .. for deck 1
VestaxVCI380.modeConnections[1]=[]; // .. for deck 2



VestaxVCI380.init = function(_id, _debugging) {

    // all pads will flash white as a visual cue that the mapping started or restarted correctly
    // we avoid flashing the other LEDs as that would cause a MIDI buffer overflow on the controller
    VestaxVCI380.setPadColorAll(VestaxVCI380.padColor.WHITE);

    // faders and knobs: we get their position with identity reply, and only activate soft takeover after that
    VestaxVCI380.createPots();

    // headphone mix: the controller mixes cue and master itself with its CUE/MASTER knob,
    // so the headphone output of Mixxx must be cue only (option, enabled by default)
    if (engine.getSetting("headMixInController")) {
        engine.setValue("[Master]", "headMix", -1);
    }

    // check whether our Mixxx version supports stems
    // on 2.5, we detect the absence of softTakeoverWillIgnore which is a 2.6 function. This avoids probing the non-existent stem_count which would log a "non-existent control" warning
    if ("softTakeoverWillIgnore" in engine) {
        const stemConnection = engine.makeConnection("[Channel1]", "stem_count", function() {});
        VestaxVCI380.hasStems = stemConnection !== undefined;
        if (VestaxVCI380.hasStems) {
            stemConnection.disconnect();
        }
    }

    // events connection
    VestaxVCI380.connections.push(engine.makeConnection("[App]", "indicator_250ms", VestaxVCI380.onIndicator250));
    VestaxVCI380.connections.push(engine.makeConnection("[Channel1]", "playposition", VestaxVCI380.updatePlayposition));
    VestaxVCI380.connections.push(engine.makeConnection("[Channel2]", "playposition", VestaxVCI380.updatePlayposition));
    VestaxVCI380.connections.push(engine.makeConnection("[Channel1]", "end_of_track", VestaxVCI380.onEndOfTrack));
    VestaxVCI380.connections.push(engine.makeConnection("[Channel2]", "end_of_track", VestaxVCI380.onEndOfTrack));
    VestaxVCI380.connections.push(engine.makeConnection("[Channel1]", "track_loaded", VestaxVCI380.onTrackLoaded));
    VestaxVCI380.connections.push(engine.makeConnection("[Channel2]", "track_loaded", VestaxVCI380.onTrackLoaded));
    VestaxVCI380.connections.push(engine.makeConnection("[Channel1]", "quantize", VestaxVCI380.refreshLEDs));
    VestaxVCI380.connections.push(engine.makeConnection("[Channel2]", "quantize", VestaxVCI380.refreshLEDs));
    VestaxVCI380.connections.push(engine.makeConnection("[Channel1]", "keylock", VestaxVCI380.refreshLEDs));
    VestaxVCI380.connections.push(engine.makeConnection("[Channel2]", "keylock", VestaxVCI380.refreshLEDs));
    VestaxVCI380.connections.push(engine.makeConnection("[Channel1]", "rate", VestaxVCI380.onRateChange));
    VestaxVCI380.connections.push(engine.makeConnection("[Channel2]", "rate", VestaxVCI380.onRateChange));
    VestaxVCI380.connections.push(engine.makeConnection("[Channel1]", "slip_enabled", VestaxVCI380.refreshLEDs));
    VestaxVCI380.connections.push(engine.makeConnection("[Channel2]", "slip_enabled", VestaxVCI380.refreshLEDs));

    // turning off all LEDs
    VestaxVCI380.setAllLEDs(false);

    // init LEDs status according to settings
    VestaxVCI380.refreshLEDs();

    // pad mode, shown after the pads stayed white for a moment; this also spreads the MIDI
    // messages in time (the controller stops handling incoming MIDI beyond about 400 at once)
    engine.beginTimer(300, function() {
        VestaxVCI380.setMode(1, VestaxVCI380.padMode[0]);
        VestaxVCI380.setMode(2, VestaxVCI380.padMode[1]);
    }, true);

    // align Mixxx with the current position of the faders, knobs and switches
    VestaxVCI380.syncWithController();

    // fallback in case we never receive the identity reply
    engine.beginTimer(2000, VestaxVCI380.enableSoftTakeover, true);

    // splash screen, once the controller has sent its state (pad mode included)
    engine.beginTimer(500, function() {
        for (let deck = 1; deck <= 2; deck++) {
            if (VestaxVCI380.padMode[deck - 1] === 1 && engine.getValue(`[Channel${deck}]`, "track_loaded") === 0) {
                VestaxVCI380.setPadColorSplash(deck);
            }
        }
    }, true);
};

////
// Initial state
// The VCI-380 answers the MIDI Universal Identity Request by sending the current
// state of all its controls (faders, knobs, pitch, crossfader, pad mode, headphone
// cue and FX ON switches), through the usual MIDI messages and so the usual handlers.
//
// Identity reply: F0 7E 00 06 02  00 40 02 (maker)  40 02 (family = USB product
// ID 0x0240)  00 00 (model)  02 01 01 00 (version)  F7
//
// Hardware facts found by testing the controller:
// - note 0x0B on 0x97 / 0x98: MIXER / PC switch of the channel, which mutes its
//   external input (7F = MIXER, input heard; 00 = PC, input muted). The sound
//   card audio goes through in both positions. With the pcMixerSwitch option, the
//   switch also enables or disables the Mixxx input of the channel (onPCMixer)
// - CC 0x56, 0x57, 0x58 on 0xB7: knobs of the centre section.
//   0x56 CF CURVE (crossfader curve) and 0x57 IF CURVE (input fader = channel
//   fader curve): they don't change the MIDI values sent by the crossfader and
//   the faders; they act on the external inputs mixed by the hardware (channel
//   switch on MIXER). 0x56 drives the Mixxx crossfader curve (onCrossfaderCurve)
//   if the option cfCurveKnob is enabled.
//   0x57 is not mapped: Mixxx has no setting for the curve of its channel faders
//   (only the crossfader curve can be changed).
//   0x58 headphone CUE / MASTER mix: the hardware itself mixes the main output
//   and the headphone (PFL / preview) output; it sets the headphone mix of Mixxx
//   if the headMixInController option is disabled (onHeadMix).
// - CC 0x5B on 0xB7 / 0xB8: pad pressure (one value per deck); the pad notes
//   also carry the velocity.
// - CC 0x03 on 0xB7 / 0xB8 (output): position of the jog wheel LED (0x00-0x7F);
//   no message found to switch it off.
// - CC 0x04 on 0xB7 / 0xB8 (output): lights the left / right column of the level
//   meter for a moment only: the controller overwrites it with its own measurement
//   a few ms later (also with CUE on), so the level meter can't be driven by the
//   mapping; sent repeatedly, the column flickers.
// - The controller stops handling incoming MIDI (LEDs frozen, no identity reply,
//   "rawmidi drain error" in the kernel log) when it gets more than about 400
//   messages at once, or more than about 1000 per second for a while, until it is
//   switched off and on again (a USB reset doesn't help).
// - MODE buttons are radio buttons handled by the hardware: changing mode sends
//   the old mode with 00, then the new one with 7F.
// - DEPTH (CC 0x0A) and FX ON (note 0x1C) also drive a hardware high-pass filter
//   on the external input of the channel, besides the MIDI used by this mapping.
// - CUE A/B and FX ON are toggled by the hardware: their LEDs can't
//   be driven by MIDI, but their state is sent (0x99 / 0x9A 0x0A, 0x97 / 0x98 0x1C).
////

VestaxVCI380.IDENTITY_REQUEST = [0xF0, 0x7E, 0x7F, 0x06, 0x01, 0xF7];

VestaxVCI380.syncWithController = function() {
    midi.sendSysexMsg(VestaxVCI380.IDENTITY_REQUEST, VestaxVCI380.IDENTITY_REQUEST.length);
};

// Detecting reception of the identity request reply
// Which means the controller will send all control positions
// After which we can activate the soft takeovers
VestaxVCI380.incomingData = function(data, length) {
    // identity reply: F0 7E <id> 06 02 ...
    if (length > 4 && data[0] === 0xF0 && data[1] === 0x7E && data[3] === 0x06 && data[4] === 0x02) {
        engine.beginTimer(200, VestaxVCI380.enableSoftTakeover, true);
    }
};

// Create some pots to handle soft takeover
// note: softTakeover is initially disabled because at startup
//       - the controller can flush a queue of old control movements
//       - we need to load the events triggered by identity request
//       after this loading, we will enable soft takeover.
VestaxVCI380.createPots = function() {
    for (let deck = 1; deck <= 2; deck++) {
        const group = `[Channel${deck}]`;
        VestaxVCI380[`deck${deck}`] = {
            volume: new components.Pot({group: group, inKey: "volume", softTakeover: false}),
            pregain: new components.Pot({group: group, inKey: "pregain", softTakeover: false}),
            rate: new components.Pot({group: group, inKey: "rate", softTakeover: false}), // 14-bit: inputMSB / inputLSB
            eq: [1, 2, 3].map(parameter => new components.Pot({
                group: `[EqualizerRack1_${group}_Effect1]`, inKey: `parameter${parameter}`, softTakeover: false,
            })),
            depth: new components.Pot({group: `[QuickEffectRack1_${group}]`, inKey: "super1", softTakeover: false}),
        };
    }
    VestaxVCI380.crossfader = new components.Pot({group: "[Master]", inKey: "crossfader", softTakeover: false});
    VestaxVCI380.balance = new components.Pot({group: "[Master]", inKey: "balance", softTakeover: false});
    // balance is mapped to SHIFT + crossfader, which is not standard: it is not in the controller
    // state sent after the identity request, and onCrossfader makes soft takeover ignore the
    // first SHIFT + crossfader value after the crossfader moved (the fader position is the
    // crossfader one), so that value is only used to learn the position

    // CUE/MASTER knob, only used when the headMixInController option is disabled
    VestaxVCI380.headMix = new components.Pot({group: "[Master]", inKey: "headMix", softTakeover: false});
};

// whether the softTakeover has been enabled, from identity reply or fallback timer
VestaxVCI380.softTakeoverEnabled = false;

// soft takeover on every fader and knob, once Mixxx has their real position
VestaxVCI380.enableSoftTakeover = function() {
    if (VestaxVCI380.softTakeoverEnabled) {
        return;
    }
    VestaxVCI380.softTakeoverEnabled = true;
    const pots = [VestaxVCI380.crossfader, VestaxVCI380.balance];
    if (!engine.getSetting("headMixInController")) {
        pots.push(VestaxVCI380.headMix); // CUE/MASTER only drives Mixxx without this option
    }
    for (let deck = 1; deck <= 2; deck++) {
        const d = VestaxVCI380[`deck${deck}`];
        pots.push(d.volume, d.pregain, d.rate, d.depth, ...d.eq);
    }
    for (const pot of pots) {
        pot.softTakeover = true;
        pot.firstValueReceived = true; // also for the balance, which isn't in the controller state
        pot.connect();                 // engine.softTakeover(group, inKey, true)
        // the next value is ignored by soft takeover (it only records the position):
        // send the current value ourselves, so that Mixxx records the knob as being
        // exactly where Mixxx is, and the first real move is taken directly
        if (pot !== VestaxVCI380.balance) {
            engine.setParameter(pot.group, pot.inKey, engine.getParameter(pot.group, pot.inKey));
        }
    }
};

VestaxVCI380.shutdown = function(_id) {

    // disconnect connections
    while (VestaxVCI380.connections.length > 0) {
        VestaxVCI380.connections.pop().disconnect();
    }
    while (VestaxVCI380.modeConnections[0].length > 0) {
        VestaxVCI380.modeConnections[0].pop().disconnect();
    }
    while (VestaxVCI380.modeConnections[1].length > 0) {
        VestaxVCI380.modeConnections[1].pop().disconnect();
    }


    // turning off all LEDs
    VestaxVCI380.setPadColorAll(VestaxVCI380.padColor.OFF);
    VestaxVCI380.setAllLEDs(false);
};

VestaxVCI380.getDeckFromGroup = function(group) {
    if (group.startsWith("[Channel1")) {
        return (1);
    } else if (group.startsWith("[Channel2")) {
        return (2);
    }
};

////
// Play button
// with soft start and brake
////
VestaxVCI380.onPlay = function(channel, _control, value, _status, group) {
    if (value === 0x7F) {
        const deck = VestaxVCI380.getDeck(channel);
        if (VestaxVCI380.shiftStatus) {
            if (engine.getValue(group, "play")) {
                engine.brake(deck, true, 10);
            } else {
                engine.softStart(deck, true, 10);
            }
        } else {
            script.toggleControl(group, "play");
        }
    }
};

////
// WHEELS
////

VestaxVCI380.isScratching=[false, false];
// The button that enables/disables scratching
VestaxVCI380.wheelTouch = function(channel, _control, value, _status) {
    if (value === 0x7F) {

        // select range according to shift status (shift=fast moving)
        let tpr;
        if (VestaxVCI380.shiftStatus) {
            tpr=353;  // 10x speed
        } else {
            tpr=3533; // 1X speed, measured ticks for one wheel turn
        }
        const alpha = 1.0/8;
        const beta = alpha/32;
        engine.scratchEnable(VestaxVCI380.getDeck(channel), tpr, 33+1/3, alpha, beta);
        VestaxVCI380.isScratching[VestaxVCI380.getDeck(channel)-1]=true;
    } else {    // If button up
        engine.scratchDisable(VestaxVCI380.getDeck(channel));
        VestaxVCI380.isScratching[VestaxVCI380.getDeck(channel)-1]=false;
    }
};
// The wheel that actually controls the scratching
VestaxVCI380.tickCounter = [0, 0];
VestaxVCI380.wheelTurn = function(channel, _control, value, _status) {
    const deck=VestaxVCI380.getDeck(channel);
    if (!VestaxVCI380.jogScrollStatus) {
        if (VestaxVCI380.isScratching[deck-1]) { // scratching
            const newValue=(value-64);
            engine.scratchTick(deck, newValue);
        } else { // not scratching = jog mode, or beatjump if shift is pressed
            if (VestaxVCI380.shiftStatus) {
                // beatjump
                if (++VestaxVCI380.tickCounter[deck-1] >100)  {
                    engine.setValue(`[Channel${deck}]`, `beatjump_${(value < 64) ? "backward" : "forward"}`, 1);
                    VestaxVCI380.tickCounter[deck-1]=0;
                }
            } else {
                // jog
                engine.setValue(`[Channel${deck}]`, "jog", script.absoluteLin(value, -3, 3));
            }
        }
    } else {
        // JOG scroll in playlist
        if (++VestaxVCI380.tickCounter[deck-1] >15) {
            VestaxVCI380.tickCounter[deck-1]=0;
            engine.setValue("[Library]", "MoveVertical", value<64 ? -1 : 1);
        }
    }

};

////
// Modifiers
////
// SHIFT buttons
VestaxVCI380.shiftStatus=false;
VestaxVCI380.onShift = function(_channel, _control, value, _status) {
    // left and right shift have different channels, but I consider them to be equivalent
    VestaxVCI380.shiftStatus=(value===0x7F);
    VestaxVCI380.setLED(1, VestaxVCI380.LED.SHIFT, VestaxVCI380.shiftStatus);
    VestaxVCI380.setLED(2, VestaxVCI380.LED.SHIFT, VestaxVCI380.shiftStatus);
};

// Jog Scroll button
VestaxVCI380.jogScrollStatus=false;
VestaxVCI380.onJogScroll = function(_channel, _control, value, _status) {
    VestaxVCI380.jogScrollStatus=(value===0x7F);
    VestaxVCI380.setLED(1, VestaxVCI380.LED.JOGSCROLL, VestaxVCI380.jogScrollStatus);
};


////
// KNOBS
////
VestaxVCI380.onSelectTrackKnob = function(_channel, _control, value, _status) {
    switch (value) {
    case 0x7F:
        engine.setValue("[Library]", "MoveVertical", -1);
        break;
    case 0x01:
        engine.setValue("[Library]", "MoveVertical", 1);
        break;
    case 0x7B:
        engine.setValue("[Library]", "ScrollVertical", -1);
        break;
    case 0x05:
        engine.setValue("[Library]", "ScrollVertical", 1);
    }
};

//EQs - managed by script in order to enable KILL function with shift
VestaxVCI380.onEQ = function(channel, control, value, status) {
    const deck=VestaxVCI380.getDeck(channel);

    if (!VestaxVCI380.shiftStatus) {
        // regular mode with Pot and soft takeover
        const pot = VestaxVCI380[`deck${deck}`].eq[control - 0x46];
        pot.input(channel, control, value, status);
    } else {
        // shift : KILL mode
        engine.softTakeoverIgnoreNextValue(`[EqualizerRack1_[Channel${deck}]_Effect1]`, `parameter${control-0x45}`);
        engine.setValue(`[EqualizerRack1_[Channel${deck}]_Effect1]`, `button_parameter${control-0x45}`, (value<0x40 ? 1 : 0));
    }

};

////
// SLIDERS
////
// CF CURVE knob: crossfader curve of Mixxx, from smooth (left) to sharp cut (right).
// The knob only acts on the external inputs in the hardware; Mixxx does the mixing
// of its own decks, so when the cfCurveKnob option is enabled,
// the knob drives the curve set in Preferences > Mixer.
// Same formula as the preferences slider (dlgprefmixer: linearToOneByX).
VestaxVCI380.onCrossfaderCurve = function(_channel, _control, value, _status) {
    if (!engine.getSetting("cfCurveKnob"))  {
        return; // default behaviour: keep the curve of the preferences
    }
    // same as the crossfader curve slider of the preferences (DlgPrefMixer, linearToOneByX):
    // from 0.6 (smooth) to 999.6 (sharp cut)
    const curve = 1000 / ((127 - value) / 127 * 999 + 1) - 0.4;
    engine.setValue("[Mixer Profile]", "xFaderCurve", curve);
    engine.setValue("[Mixer Profile]", "xFaderCalibration", Math.pow(0.5, 1 / curve));
};

VestaxVCI380.onCrossfader = function(channel, control, value, status) {
    if (VestaxVCI380.shiftStatus) { // SHIFT + crossfader sets Balance
        engine.softTakeoverIgnoreNextValue("[Master]", "crossfader");
        VestaxVCI380.balance.input(channel, control, value, status);
    } else { // crossfader
        engine.softTakeoverIgnoreNextValue("[Master]", "balance");
        VestaxVCI380.crossfader.input(channel, control, value, status);
    }
};

// CUE/MASTER knob: mixed by the controller itself in the headphones (headMixInController
// option, recommended). Without the option, it sets the headphone mix of Mixxx,
// for headphones plugged into another sound card.
VestaxVCI380.onHeadMix = function(channel, control, value, status) {
    if (engine.getSetting("headMixInController")) {
        return;
    }
    VestaxVCI380.headMix.input(channel, control, value, status);
};

// PC / MIXER switch of a channel (note 0x0B: Note on 7F = MIXER, Note off = PC).
// The controller mutes the RCA input in its own mixer in PC position, but always sends it
// to the computer. With the pcMixerSwitch option, the switch also enables the Mixxx input
// of the channel in PC position and disables it in MIXER position.
VestaxVCI380.onPCMixer = function(channel, _control, value, status) {
    const option = engine.getSetting("pcMixerSwitch");
    if (option !== "auxiliary" && option !== "vinyl") {
        return;
    }
    const deck = VestaxVCI380.getDeck(channel);
    const toPC = (status & 0xF0) === 0x80 || value === 0x00;
    if (option === "auxiliary") {
        engine.setValue(`[Auxiliary${deck}]`, "main_mix", toPC ? 1 : 0);
    } else {
        engine.setValue(`[Channel${deck}]`, "vinylcontrol_enabled", toPC ? 1 : 0);
    }
};

////
// BUTTONS
////

// VINYL button used as slip mode
VestaxVCI380.onVinyl = function(channel, _control, value, _status) {
    if  (value===0x7F) {
        script.toggleControl(`[Channel${VestaxVCI380.getDeck(channel)}]`, "slip_enabled");
    }
};

// CUE A/B buttons. Must be used with jog scroll, otherwise they act as headphone cue toggle
// NOTE: the headphone cue function is handled by the controller in hardware
VestaxVCI380.onLoad = function(channel, _control, value, _status) {
    if (VestaxVCI380.jogScrollStatus && value===0x7F) { // value 00 when button released would trigger deck clone (double click)
        engine.setValue(`[Channel${VestaxVCI380.getDeck(channel)}]`, "LoadSelectedTrack", 1);
    }
};

// Headphone cue buttons
// NOTE: the headphone cue function is handled by the controller in hardware
VestaxVCI380.onHeadCue = function(channel, _control, value, _status) {
    engine.setValue(`[Channel${VestaxVCI380.getDeck(channel)}]`, "pfl", value === 0x7F ? 1 : 0);
};

////
// STRIPS
// In mode 4 (Stems), while some stems are held (upper pads), the strip will control their volume
// In other modes, both strips control the needle drop on the preview deck.
//                 or with SHIFT, they do needle drop on the corresponding deck
////

VestaxVCI380.onStrip = function(channel, _control, value, _status) {
    const deck=VestaxVCI380.getDeck(channel);
    if (VestaxVCI380.isStemSelected(deck)) {
        for (const stem of VestaxVCI380.heldStems[deck-1]) {
            engine.setValue(`[Channel${deck}_Stem${stem}]`, "volume", script.absoluteLin(value, 0, 1));
        }
    } else if (VestaxVCI380.shiftStatus) { // SHIFT + strip = needle drop on main decks
        engine.setValue(`[Channel${deck}]`, "playposition", script.absoluteLin(value, 0, 0.99)); // 0.99 instead of 1, to avoid unintentional stop while stripping to the end
    } else { // without shift : needle drop in the preview deck
        engine.setValue("[PreviewDeck1]", "playposition", script.absoluteLin(value, 0, 0.99));
    }
};

////
// COLOR PADS
////

// currently pushed button
VestaxVCI380.pushedButton=[0, 0];

// Keep a list of held stems per deck
VestaxVCI380.heldStems=[[], []];

// when tapped (used as buttons)
VestaxVCI380.onPadTap = function(channel, control, value, _status) {
    const deck = VestaxVCI380.getDeck(channel);
    const padNumber= control - 0x3B;
    let samplerNumber;
    let group;
    if (value>0x00) { // PAD pressed
        VestaxVCI380.pushedButton[deck-1]=padNumber;
        switch (VestaxVCI380.padMode[deck-1]) {
        case 1: // in mode 1 = HOT CUE
            if (VestaxVCI380.shiftStatus) { // press shift-pad to clear hotcue
                engine.setValue(`[Channel${deck}]`, `hotcue_${padNumber}_clear`, 1);
                VestaxVCI380.setPadColor(deck, padNumber, VestaxVCI380.colorHotcueDelete);
            } else { // press pad to set or play hotcue
                engine.setValue(`[Channel${deck}]`, `hotcue_${padNumber}_activate`, 1);
                VestaxVCI380.setPadColor(deck, padNumber, VestaxVCI380.colorHotcueActivate);
            }
            break;

        case 2: // in mode 2 = Beat grid tools
            switch (padNumber) {
            case 1:
            case 2:
            case 3:
            case 4: // BPM tap
                engine.setValue(`[Channel${deck}]`, "bpm_tap", 1);
                VestaxVCI380.setPadColor(deck, padNumber, VestaxVCI380.colorBeatPushedBpm);
                break;
            case 5:
            case 6:
            case 7:
            case 8: // Align beatgrid
                engine.setValue(`[Channel${deck}]`, "beats_translate_curpos", 1);
                VestaxVCI380.setPadColor(deck, padNumber, VestaxVCI380.colorBeatPushedAlignGrid);
                break;

            }
            break;

        case 3: // in mode 3 = LOOP
            // press pad to control loops
            switch (padNumber) {
            case 1: // activate loop
                engine.setValue(`[Channel${deck}]`, "beatloop_activate", 1);
                break;
            case 2: // unused
                break;
            case 3: // lower beatloop size
                engine.setValue(`[Channel${deck}]`, "loop_halve", 1);
                break;
            case 4: // raise beatloop size
                engine.setValue(`[Channel${deck}]`, "loop_double", 1);
                break;
            case 5: // activate reloop
                engine.setValue(`[Channel${deck}]`, "reloop_toggle", 1);
                break;
            case 6: // unused
                break;
            case 7: // move loop left
                engine.setValue(`[Channel${deck}]`, "loop_move", -0.125);
                break;
            case 8: // move loop right
                engine.setValue(`[Channel${deck}]`, "loop_move", 0.125);
                break;
            }
            break;

        case 4: // Stems mode
            if (!VestaxVCI380.hasStems) { break; }
            switch (padNumber) {
            case 1:
            case 2:
            case 3:
            case 4: // select stem for FX and volume: several pads can be held at once
                if (padNumber <= engine.getValue(`[Channel${deck}]`, "stem_count") &&
                    VestaxVCI380.heldStems[deck-1].indexOf(padNumber) === -1) {
                    VestaxVCI380.heldStems[deck-1].push(padNumber);
                }
                midi.sendShortMsg(0x96+deck, 0x3B+padNumber, VestaxVCI380.padColor.WHITE);
                break;
            case 5:
            case 6:
            case 7:
            case 8:
                if (VestaxVCI380.shiftStatus) { // SHIFT: solo stem
                    VestaxVCI380.toggleStemSolo(deck, padNumber-4);
                } else { // mute/unmute stem
                    group=`[Channel${deck}_Stem${padNumber-4}]`;
                    engine.setValue(group, "mute", (engine.getValue(group, "mute")===0 ? 1 : 0));
                }
                break;
            }
            break;

        case 5: // Samplers mode
            samplerNumber=VestaxVCI380.buttonToSampler[padNumber];
            if (samplerNumber <= engine.getValue("[App]", "num_samplers")) {
                if (VestaxVCI380.shiftStatus) {
                    if (engine.getValue(`[Sampler${samplerNumber}]`, "play")) {
                        engine.setValue(`[Sampler${samplerNumber}]`, "play", 0);
                    } else {
                        engine.setValue(`[Sampler${samplerNumber}]`, "eject", 1);
                    }
                } else {
                    if (engine.getValue(`[Sampler${samplerNumber}]`, "track_loaded")) {
                        engine.setValue(`[Sampler${samplerNumber}]`, "cue_gotoandplay", 1);
                    } else {
                        engine.setValue(`[Sampler${samplerNumber}]`, "LoadSelectedTrack", 1);
                    }
                }
            }
            break;
        }


    } else { // PAD released
        VestaxVCI380.pushedButton[deck-1]=0;
        // whatever the mode (it may have changed while the pad was held)
        VestaxVCI380.heldStems[deck-1] = VestaxVCI380.heldStems[deck-1].filter(stem => stem !== padNumber);
        switch (VestaxVCI380.padMode[deck-1]) {
        case 1: // in mode 1, releasing pad de-activates and resets pad color for hotcue
            engine.setValue(`[Channel${deck}]`, `hotcue_${padNumber}_activate`, 0);
            VestaxVCI380.setPadColorHotcuesOne(deck, padNumber);
            break;

        case 2: // in mode 2 = Beat grid tools
            VestaxVCI380.setPadColor(deck, padNumber, VestaxVCI380.trackPadColorDim(deck, VestaxVCI380.colorBeatInactive));
            break;

        case 3:
            break;

        case 4:
            if (VestaxVCI380.hasStems) { VestaxVCI380.refreshPadsStem(deck); }
            break;
        }

    }
};



// PADFX button, select and push
VestaxVCI380.onPadFXSelect = function(channel, _control, value, _status) {
    const deck = VestaxVCI380.getDeck(channel);
    switch (deck) {
    case 1:
        if (VestaxVCI380.shiftStatus) {
            engine.setValue("[Library]", "ScrollVertical", value===0x7f ? -1 : 1);
        } else {
            engine.setValue("[Library]", "MoveVertical", value===0x7f ? -1 : 1);
        }
        break;
    case 2:
        if (VestaxVCI380.shiftStatus) {
            // we set the same zoom value for both decks
            const currentZoom=engine.getValue("[Channel1]", "waveform_zoom");
            engine.setValue("[Channel1]", "waveform_zoom", currentZoom + (value===0x7f ? -0.1 : 0.1));
            engine.setValue("[Channel2]", "waveform_zoom", currentZoom + (value===0x7f ? -0.1 : 0.1));
        } else {
            engine.setValue("[Library]", "MoveHorizontal", value===0x7f ? -1 : 1);
        }
        break;
    }
};

VestaxVCI380.onPadFXPush = function(channel, _control, value, _status) {
    if (value===127) {
        const deck = VestaxVCI380.getDeck(channel);
        if (VestaxVCI380.shiftStatus) {
            engine.setValue(`[Channel${deck}]`, "CloneFromDeck", (deck===1 ? 2 : 1));
        } else {
            engine.setValue("[Library]", "GoToItem", 1);
        }
    }
};

////
// Track end alert management
// We make some library LEDs blinking as a warning
//

VestaxVCI380.trackEndAlert=[false, false];

VestaxVCI380.onEndOfTrack = function(value, group, _control) {
    const deck=VestaxVCI380.getDeckFromGroup(group);
    VestaxVCI380.trackEndAlert[deck-1] = (value === 1);
    if (!VestaxVCI380.trackEndAlert[deck-1]) {
        VestaxVCI380.trackEndAlertSetLED(deck, true); // restore LEDs ON while no alert
    }
};

VestaxVCI380.onIndicator250 = function(value, _group, _control) {
    for (let deck = 1; deck <= 2; deck++) {
        if (VestaxVCI380.trackEndAlert[deck-1]) {
            VestaxVCI380.trackEndAlertSetLED(deck, value === 1);
        }
    }
};

VestaxVCI380.trackEndAlertSetLED = function(deck, state) {
    if (deck===1) {
        VestaxVCI380.setLED(1, VestaxVCI380.LED.AREA, state);
        VestaxVCI380.setLED(1, VestaxVCI380.LED.BACK, state);
    } else {
        VestaxVCI380.setLED(1, VestaxVCI380.LED.SORT, state);
        VestaxVCI380.setLED(1, VestaxVCI380.LED.FWD, state);
    }
};

// Managing the 5 different modes for the colorpads
// Modes are :
// 1 - HOT CUE
// 2 - BEAT GRID
// 3 - LOOP
// 4 - STEMS
// 5 - SAMPLER
////

VestaxVCI380.padMode=[1, 1];
VestaxVCI380.onSelectPadMode = function(channel, control, value, _status) {
    let newMode;
    if (value === 127) {
        const deck = VestaxVCI380.getDeck(channel);
        if (channel>=9) {
            newMode=5; // special case of shift + HOT CUE MODE, 5th mode
        } else {
            newMode=control-0x37;
        }
        if (newMode !== VestaxVCI380.padMode[deck-1]) { // the initial state may repeat the current mode
            VestaxVCI380.setMode(deck, newMode);
        }
    }
};

VestaxVCI380.setMode = function(deck, newMode) {
    const previousMode=VestaxVCI380.padMode[deck-1];
    VestaxVCI380.padMode[deck-1]=newMode;
    VestaxVCI380.clearConnectionsForMode(deck, previousMode);
    VestaxVCI380.setPadMode(deck, newMode);
    VestaxVCI380.makeConnectionsForMode(deck, newMode);
};

VestaxVCI380.setPadMode = function(deck, mode) {
    switch (mode) { // light up relevant LEDs for a mode
    case 1 :
        VestaxVCI380.setPadColorHotcuesDeck(deck);
        break;
    case 2 :
        VestaxVCI380.beatPos[deck-1]=8;
        VestaxVCI380.setPadColorBeatGridMode(deck);
        break;
    case 3 :
        VestaxVCI380.setPadColorLoopMode(deck);
        break;
    case 5 :
        VestaxVCI380.setPadColorSamplerMode(deck);
        engine.setValue("[Skin]", "show_samplers", 1);

        break;

    }
};

VestaxVCI380.makeConnectionsForMode = function(deck, mode) {
    let connector;
    let numSamplers;
    switch (mode) {
    case 1 : // hot cues
        for (let hotcue = 1; hotcue <= 8; hotcue++) {
            VestaxVCI380.modeConnections[deck-1].push(engine.makeConnection(`[Channel${deck}]`, `hotcue_${hotcue}_color`, VestaxVCI380.onHotcueUpdated));
            VestaxVCI380.modeConnections[deck-1].push(engine.makeConnection(`[Channel${deck}]`, `hotcue_${hotcue}_type`, VestaxVCI380.onHotcueUpdated));
        }
        VestaxVCI380.modeConnections[deck-1].push(engine.makeConnection(`[Channel${deck}]`, "track_color", VestaxVCI380.onTrackColor));
        break;

    case 2 : // beat grid
        VestaxVCI380.modeConnections[deck-1].push(engine.makeConnection(`[Channel${deck}]`, "beat_active", VestaxVCI380.onBeatActive));
        VestaxVCI380.modeConnections[deck-1].push(engine.makeConnection(`[Channel${deck}]`, "track_color", VestaxVCI380.onTrackColor));
        break;

    case 3 : // loop
        connector=engine.makeConnection(`[Channel${deck}]`, "loop_enabled", VestaxVCI380.onLoopEnabled);
        connector.trigger();
        VestaxVCI380.modeConnections[deck-1].push(connector);
        break;

    case 4 : // stems
        if (VestaxVCI380.hasStems) { // To keep compatibility with Mixxx versions without stem
            connector=engine.makeConnection(`[Channel${deck}]`, "stem_count", VestaxVCI380.onStemChange);
            connector.trigger();
            VestaxVCI380.modeConnections[deck-1].push(connector);
            for (let stem = 1; stem <= VestaxVCI380.MAX_STEMS; stem++) {
                VestaxVCI380.modeConnections[deck-1].push(engine.makeConnection(`[Channel${deck}_Stem${stem}]`, "color", VestaxVCI380.onStemChange));
                VestaxVCI380.modeConnections[deck-1].push(engine.makeConnection(`[Channel${deck}_Stem${stem}]`, "mute", VestaxVCI380.onStemChange));
            }
        } else {
            VestaxVCI380.setPadColorDeck(deck, VestaxVCI380.padColor.OFF);
        }
        break;

    case 5 : // samplers
        numSamplers=Math.min(engine.getValue("[App]", "num_samplers"), 8);
        for (let sampler = 1; sampler <= numSamplers; sampler++) {
            connector=engine.makeConnection(`[Sampler${sampler}]`, "play", VestaxVCI380.onSampler);
            connector.trigger();
            VestaxVCI380.modeConnections[deck-1].push(connector);
            connector=engine.makeConnection(`[Sampler${sampler}]`, "track_loaded", VestaxVCI380.onSampler);
            connector.trigger();
            VestaxVCI380.modeConnections[deck-1].push(connector);
            connector=engine.makeConnection(`[Sampler${sampler}]`, "track_color", VestaxVCI380.onSampler);
            VestaxVCI380.modeConnections[deck-1].push(connector);
        }
    }
};

VestaxVCI380.clearConnectionsForMode = function(deck, _mode) {
    while (VestaxVCI380.modeConnections[deck-1].length > 0) {
        VestaxVCI380.modeConnections[deck-1].pop().disconnect();
    }
};

VestaxVCI380.onStemChange = function(_value, group, _control) {
    const deck=VestaxVCI380.getDeckFromGroup(group);
    VestaxVCI380.refreshPadsStem(deck);
};


VestaxVCI380.refreshPadsStem = function(deck) {
    const numStems=engine.getValue(`[Channel${deck}]`, "stem_count");
    for (let stem = 1; stem <= numStems; stem++) {
        const stemColor=VestaxVCI380.ColorMapper.getValueForNearestColor(engine.getValue(`[Channel${deck}_Stem${stem}]`, "color"));
        const stemColorDim=VestaxVCI380.dimColor(stemColor);
        midi.sendShortMsg(0x96+deck, 0x3B+stem, VestaxVCI380.heldStems[deck-1].indexOf(stem) !== -1 ? VestaxVCI380.padColor.WHITE : stemColor);
        midi.sendShortMsg(0x96+deck, 0x3B+stem+4, engine.getValue(`[Channel${deck}_Stem${stem}]`, "mute")===0 ? stemColor : stemColorDim);
    }
    for (let emptyStem=numStems+1; emptyStem <= VestaxVCI380.MAX_STEMS; emptyStem++) {
        midi.sendShortMsg(0x96+deck, 0x3B+emptyStem, VestaxVCI380.padColor.dimWHITE);
        midi.sendShortMsg(0x96+deck, 0x3B+emptyStem+4, VestaxVCI380.padColor.dimWHITE);
    }
};

// SHIFT + lower pad in stems mode: only this stem is heard (the others are muted);
// again on the stem already alone: all stems are heard again
VestaxVCI380.toggleStemSolo = function(deck, soloStem) {
    const numStems=engine.getValue(`[Channel${deck}]`, "stem_count");
    if (soloStem > numStems) {
        return;
    }
    let alone=true;
    for (let stem = 1; stem <= numStems; stem++) {
        const muted=engine.getValue(`[Channel${deck}_Stem${stem}]`, "mute")===1;
        if (muted !== (stem !== soloStem)) {
            alone=false;
        }
    }
    for (let stem = 1; stem <= numStems; stem++) {
        engine.setValue(`[Channel${deck}_Stem${stem}]`, "mute", (alone || stem === soloStem) ? 0 : 1);
    }
};

VestaxVCI380.onHotcueUpdated = function(_value, group, _control) {
    const deck=VestaxVCI380.getDeckFromGroup(group);
    if (VestaxVCI380.padMode[deck-1]===1) {
        VestaxVCI380.setPadColorHotcuesDeck(deck);
    }
};

// sampler pads: empty = dim white; loaded = color of the sampler track (green if it has none);
// playing = yellow, or white if the track is already yellow
VestaxVCI380.onSampler = function(_value, group, _control) {
    if (group.startsWith("[Sampler")) {
        const button=VestaxVCI380.samplerToButton[group.match(/\d+/)[0]];
        let color=VestaxVCI380.colorSamplerEmpty;
        if (engine.getValue(group, "track_loaded")===1) {
            const trackColor=engine.getValue(group, "track_color");
            const loadedColor=(trackColor < 0) ? VestaxVCI380.colorSamplerLoaded : VestaxVCI380.ColorMapper.getValueForNearestColor(trackColor);
            if (engine.getValue(group, "play")===1) {
                color=(loadedColor===VestaxVCI380.colorSamplerPlaying) ? VestaxVCI380.padColor.WHITE : VestaxVCI380.colorSamplerPlaying;
            } else {
                color=loadedColor;
            }
        }
        for (let deck = 1; deck <= 2; deck++) {
            if (VestaxVCI380.padMode[deck-1]===5) {
                midi.sendShortMsg(0x96+deck, 0x3B+button, color);
            }
        }
    }
};

VestaxVCI380.onBeatActive = function(value, group, _control) {
    const deck=VestaxVCI380.getDeckFromGroup(group);
    if (VestaxVCI380.padMode[deck-1]===2) {
        let prevBeatPos;
        switch (value) {
        case 1: // Approaching bar - forward
            prevBeatPos=VestaxVCI380.beatPos[deck-1];
            VestaxVCI380.beatPos[deck-1]++;
            if (VestaxVCI380.beatPos[deck-1]>8) {
                VestaxVCI380.beatPos[deck-1]=1;
            }
            midi.sendShortMsg(0x96+deck, 0x3B+prevBeatPos, VestaxVCI380.trackPadColorDim(deck, VestaxVCI380.colorBeatInactive));
            midi.sendShortMsg(0x96+deck, 0x3B+VestaxVCI380.beatPos[deck-1], VestaxVCI380.colorBeatActive);
            break;
        case 2: // Approaching bar - reverse
            prevBeatPos=VestaxVCI380.beatPos[deck-1];
            VestaxVCI380.beatPos[deck-1]--;
            if (VestaxVCI380.beatPos[deck-1]<1) {
                VestaxVCI380.beatPos[deck-1]=8;
            }
            midi.sendShortMsg(0x96+deck, 0x3B+prevBeatPos, VestaxVCI380.trackPadColorDim(deck, VestaxVCI380.colorBeatInactive));
            midi.sendShortMsg(0x96+deck, 0x3B+VestaxVCI380.beatPos[deck-1], VestaxVCI380.colorBeatActive);
            break;
        case 0: // Getting away from bar
            midi.sendShortMsg(0x96+deck, 0x3B+VestaxVCI380.beatPos[deck-1], VestaxVCI380.colorCurrentBeat);
            break;
        }
    }

};

VestaxVCI380.onRateChange = function(value, group, _control) {
    const deck=VestaxVCI380.getDeckFromGroup(group);
    VestaxVCI380.setLED(deck, VestaxVCI380.LED.PADFX, value!==0);
};

// for mode 1, refresh display while a new track is loaded/unloaded
VestaxVCI380.onTrackLoaded = function(_value, group, _control) {
    const deck=VestaxVCI380.getDeckFromGroup(group);
    if (engine.getValue(group, "track_loaded")===0) { // track ejected -> reset display
        VestaxVCI380.setWheelLED(deck, 0);
    } else {

        // new track loaded --> setup display
        switch (VestaxVCI380.padMode[deck-1]) {
        case 1:
            VestaxVCI380.setPadColorHotcuesDeck(deck);
            break;
        case 2:
            VestaxVCI380.setPadColorBeatGridMode(deck);
            break;
        case 3:
            VestaxVCI380.setPadColorLoopMode(deck);
            break;
        }
    }
};

// color of the track changed (in the library, or another track loaded): repaint the pads
VestaxVCI380.onTrackColor = function(_value, group, _control) {
    const deck=VestaxVCI380.getDeckFromGroup(group);
    if (engine.getValue(group, "track_loaded")===0) {
        return; // no track: keep the splash screen
    }
    switch (VestaxVCI380.padMode[deck-1]) {
    case 1:
        VestaxVCI380.setPadColorHotcuesDeck(deck);
        break;
    case 2:
        VestaxVCI380.setPadColorBeatGridMode(deck);
        break;
    }
};

// for mode 3, refresh colors when a loop is enabled or disabled
VestaxVCI380.onLoopEnabled = function(_value, group, _control) {
    const deck=VestaxVCI380.getDeckFromGroup(group);
    if (VestaxVCI380.padMode[deck-1]===3) {
        VestaxVCI380.setPadColorLoopMode(deck);
    }
};

////
// Managing effects
////

// true while at least one stem is selected (upper pad held in stems mode)
VestaxVCI380.isStemSelected = function(deck) {
    return VestaxVCI380.hasStems && VestaxVCI380.padMode[deck-1]===4 &&
        VestaxVCI380.heldStems[deck-1].length > 0;
};

VestaxVCI380.getFXGroups = function(channel) {
    const deck=VestaxVCI380.getDeck(channel);
    if (VestaxVCI380.isStemSelected(deck)) {
        return VestaxVCI380.heldStems[deck-1].map(stem => `[QuickEffectRack1_[Channel${deck}_Stem${stem}]]`);
    } else {
        return [`[QuickEffectRack1_[Channel${deck}]]`];
    }
};

VestaxVCI380.onFXDepth = function(channel, control, value, status) {
    const groups=VestaxVCI380.getFXGroups(channel);
    if (groups.length > 1) { // several stems: no soft takeover, they all take the knob value
        for (const group of groups) {
            engine.setParameter(group, "super1", value / 127);
        }
        return;
    }
    const group=groups[0];
    const depth=VestaxVCI380[`deck${VestaxVCI380.getDeck(channel)}`].depth;
    if (depth.group !== group) {
        depth.disconnect();
        depth.group = group;
        depth.connect();
    }
    depth.input(channel, control, value, status);
};

VestaxVCI380.onFXSelect = function(channel, _control, value, _status) {
    const groups=VestaxVCI380.getFXGroups(channel);
    const direction = (value === 0x7F) ? 1 : -1;
    if (groups.length === 1) {
        engine.setValue(groups[0], "chain_preset_selector", direction);
        return;
    }
    // several stems: they all get the next / previous preset of the first held stem,
    // so that they stay on the same preset
    const num = engine.getValue(groups[0], "num_chain_presets");
    const index = (engine.getValue(groups[0], "loaded_chain_preset") + direction + num) % num;
    for (const group of groups) {
        engine.setValue(group, "loaded_chain_preset", index);
    }
};

VestaxVCI380.onFXSelectPush = function(channel, _control, value, _status) {
    if (value === 0x7F) {
        for (const group of VestaxVCI380.getFXGroups(channel)) {
            engine.setValue(group, "loaded_chain_preset", 1);
        }
    }
};

VestaxVCI380.onFXOnOff = function(channel, _control, value, _status) {
    for (const group of VestaxVCI380.getFXGroups(channel)) {
        engine.setValue(group, "enabled", (value === 0x7F) ? 1 : 0);
    }
};


////
// MIDI tools
////

// tells which deck (1=left deck, 2= right) a command is coming from
// this is indicated by the channel used
VestaxVCI380.getDeck = function(channel) {
    switch (channel) {
    case 0x07 :
        return (1);
    case 0x08 :
        return (2);
    case 0x09 :
        return (1);
    case 0x0A :
        return (2);
    default :
        return (0);
    }
};


////
// Controlling regular LEDs (monochrome)
////

// List of LED MIDI note numbers
// Values taken from Vestax manual, except for JOGSCROLL and PADFX which are undocumented
// Most LEDs have 2 instances, one for left-deck and the other for the right. The MIDI channel selects the desired deck.
// Though browse area leds (area, view, sort, back, fwd, jogscroll) are unique and assigned to deck 1 only
// FX ON/OFF LED is not addressable, it maintains its own status
VestaxVCI380.LED = {"SHIFT": 12, "SYNC": 19, "CUE": 22, "PLAY": 23, "VINYL": 26, "RANGE": 27, "AREA": 80, "VIEW": 81, "SORT": 82, "BACK": 83, "FWD": 79, "JOGSCROLL": 9, "PADFX": 29};

// set a LED to ON or OFF, for given deck# (1 for left, 2 for right)
// led IDs to be taken from LED table
VestaxVCI380.setLED = function(deck, ledID, state) {
    midi.sendShortMsg(0x96+deck, ledID, state ? 0x7F : 0x00);
};

VestaxVCI380.setAllLEDs = function(state) {
    for (const ledID in VestaxVCI380.LED) {
        VestaxVCI380.setLED(1, VestaxVCI380.LED[ledID], state);
        VestaxVCI380.setLED(2, VestaxVCI380.LED[ledID], state);
        VestaxVCI380.setLED(3, VestaxVCI380.LED[ledID], state);
        VestaxVCI380.setLED(4, VestaxVCI380.LED[ledID], state);
    }
};

VestaxVCI380.refreshLEDs = function() {
    VestaxVCI380.setLED(1, VestaxVCI380.LED.RANGE, engine.getValue("[Channel1]", "quantize")===1);
    VestaxVCI380.setLED(2, VestaxVCI380.LED.RANGE, engine.getValue("[Channel2]", "quantize")===1);
    VestaxVCI380.setLED(3, VestaxVCI380.LED.RANGE, engine.getValue("[Channel1]", "keylock")===1);
    VestaxVCI380.setLED(4, VestaxVCI380.LED.RANGE, engine.getValue("[Channel2]", "keylock")===1);
    VestaxVCI380.setLED(1, VestaxVCI380.LED.VINYL, engine.getValue("[Channel1]", "slip_enabled")===1);
    VestaxVCI380.setLED(2, VestaxVCI380.LED.VINYL, engine.getValue("[Channel2]", "slip_enabled")===1);
    VestaxVCI380.setLED(1, VestaxVCI380.LED.PADFX, engine.getValue("[Channel1]", "rate")!==0);
    VestaxVCI380.setLED(2, VestaxVCI380.LED.PADFX, engine.getValue("[Channel2]", "rate")!==0);
    // library LEDs: always on, they only blink for the track end alert
    for (let deck = 1; deck <= 2; deck++) {
        if (!VestaxVCI380.trackEndAlert[deck - 1]) {
            VestaxVCI380.trackEndAlertSetLED(deck, true);
        }
    }
};

/////
// Controlling pad color LEDs
////

// Sets the color of a pad identified by deck# (1-2) and pad# (1-8)
// color from padCOLOR table
VestaxVCI380.setPadColor = function(deck, pad, color) {
    midi.sendShortMsg(0x96+deck, 0x3B + pad, color);
};

// Sets the color of all pads of a deck (1-2)
// color from padCOLOR table
VestaxVCI380.setPadColorDeck = function(deck, color) {
    for (let padID=0x3C; padID<=0x43; padID++) {
        midi.sendShortMsg(0x96+deck, padID, color);
    }
};

// Sets the color of all pads of the device
// color from padCOLOR table
VestaxVCI380.setPadColorAll = function(color) {
    for (let padID=0x3C; padID<=0x43; padID++) {
        midi.sendShortMsg(0x97, padID, color);
        midi.sendShortMsg(0x98, padID, color);
    }
};

// dimmed version of a pad color: the dim colors are the bright ones without bit 0x08
// (a color already dimmed, or OFF, is left unchanged)
VestaxVCI380.dimColor = function(color) {
    return color & ~0x08;
};

// background color of the pads of a deck: the color of the loaded track, dimmed,
// or the given color if the track has none (track_color = -1)
VestaxVCI380.trackPadColorDim = function(deck, fallback) {
    const trackColor = engine.getValue(`[Channel${deck}]`, "track_color");
    if (trackColor < 0) {
        return fallback;
    }
    return VestaxVCI380.dimColor(VestaxVCI380.ColorMapper.getValueForNearestColor(trackColor));
};

// Light up the pads of a deck with colors of the hotcues
VestaxVCI380.setPadColorHotcuesDeck = function(deck) {
    for (let hotcueNumber=1; hotcueNumber<=8; hotcueNumber++) {
        if (engine.getValue(`[Channel${  deck  }]`, `hotcue_${  hotcueNumber  }_status`) !== 1) {
            midi.sendShortMsg(0x96+deck, hotcueNumber+0x3B, VestaxVCI380.trackPadColorDim(deck, VestaxVCI380.colorHotcueUnset));
        } else {
            const hotcueColor=engine.getValue(`[Channel${  deck  }]`, `hotcue_${  hotcueNumber  }_color`);
            midi.sendShortMsg(0x96+deck, hotcueNumber+0x3B, VestaxVCI380.ColorMapper.getValueForNearestColor(hotcueColor));
        }
    }

};

// Light up the pads of a deck according to which hotcues are set or not
VestaxVCI380.setPadColorHotcuesOne = function(deck, hotcueNumber) {
    const hotcueStatus=engine.getValue(`[Channel${  deck  }]`, `hotcue_${  hotcueNumber  }_status`);
    if (hotcueStatus === 0) { // unset
        midi.sendShortMsg(0x96+deck, hotcueNumber+0x3B, VestaxVCI380.trackPadColorDim(deck, VestaxVCI380.colorHotcueUnset));
    } else if (hotcueStatus === 1) { // set
        const hotcueColor=engine.getValue(`[Channel${  deck  }]`, `hotcue_${  hotcueNumber  }_color`);
        midi.sendShortMsg(0x96+deck, hotcueNumber+0x3B, VestaxVCI380.ColorMapper.getValueForNearestColor(hotcueColor));
    } else if (hotcueStatus === 2) { // active
        midi.sendShortMsg(0x96+deck, hotcueNumber+0x3B, VestaxVCI380.colorHotCueActive);
    }
};

// Light up the pads of a deck for looping mode
VestaxVCI380.setPadColorLoopMode = function(deck) {

    if (engine.getValue(`[Channel${deck}]`, "loop_enabled")) { // loop enabled

        midi.sendShortMsg(0x96+deck, 0x3C, VestaxVCI380.padColor.GREEN);
        midi.sendShortMsg(0x96+deck, 0x3D, VestaxVCI380.padColor.OFF);
        midi.sendShortMsg(0x96+deck, 0x3E, VestaxVCI380.padColor.YELLOW);
        midi.sendShortMsg(0x96+deck, 0x3F, VestaxVCI380.padColor.YELLOW);

        midi.sendShortMsg(0x96+deck, 0x40, VestaxVCI380.padColor.GREEN);
        midi.sendShortMsg(0x96+deck, 0x41, VestaxVCI380.padColor.OFF);
        midi.sendShortMsg(0x96+deck, 0x42, VestaxVCI380.padColor.WHITE);
        midi.sendShortMsg(0x96+deck, 0x43, VestaxVCI380.padColor.WHITE);

    } else { // no loop enabled

        midi.sendShortMsg(0x96+deck, 0x3C, VestaxVCI380.padColor.RED);
        midi.sendShortMsg(0x96+deck, 0x3D, VestaxVCI380.padColor.OFF);
        midi.sendShortMsg(0x96+deck, 0x3E, VestaxVCI380.padColor.YELLOW);
        midi.sendShortMsg(0x96+deck, 0x3F, VestaxVCI380.padColor.YELLOW);

        midi.sendShortMsg(0x96+deck, 0x40, VestaxVCI380.padColor.RED);
        midi.sendShortMsg(0x96+deck, 0x41, VestaxVCI380.padColor.OFF);
        midi.sendShortMsg(0x96+deck, 0x42, VestaxVCI380.padColor.WHITE);
        midi.sendShortMsg(0x96+deck, 0x43, VestaxVCI380.padColor.WHITE);

    }
};

// Light up the pads of a deck for Beat Grid mode
VestaxVCI380.setPadColorBeatGridMode = function(deck) {
    VestaxVCI380.setPadColorDeck(deck, VestaxVCI380.trackPadColorDim(deck, VestaxVCI380.colorBeatInactive));
};

// Light up the pads of a deck for Sampler mode
VestaxVCI380.setPadColorSamplerMode = function(deck) {
    VestaxVCI380.setPadColorDeck(deck, VestaxVCI380.padColor.OFF);
};

VestaxVCI380.setPadColorSplash = function(deck) {
    for (let pixel=1; pixel<=8; pixel++) {
        midi.sendShortMsg(0x96+deck, pixel+0x3B, VestaxVCI380.padColor[VestaxVCI380.splashScreen[deck-1][pixel-1]]);
    }
};

////
// Controlling wheels LED indicator
////

// Spinning disc indicator
VestaxVCI380.updatePlayposition = function(value, group, _control) {
    const deck=VestaxVCI380.getDeckFromGroup(group);
    const duration=engine.getValue(group, "duration");
    const tickPerSecond=71.11111;
    const elapsedticks=duration*value*tickPerSecond;
    VestaxVCI380.setWheelLED(deck, Math.floor(elapsedticks%128));
};


// LED position memory
VestaxVCI380.wheelLEDPosition=[0xFF, 0xFF];

// Light up the LED according to the provided value in MIDI range (00-7F)
// NOTE : I couldn't find how to set the LED OFF. It will stay forever on the last set position. Any help appreciated.
VestaxVCI380.setWheelLED = function(deck, value) {
    if (VestaxVCI380.wheelLEDPosition[deck-1]!==value) { // save up unneeded MIDI outgoing messages
        midi.sendShortMsg(0xB6+deck, 0x03, value);
        VestaxVCI380.wheelLEDPosition[deck-1]=value;
    }
};
