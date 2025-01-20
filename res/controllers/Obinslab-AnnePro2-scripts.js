// TODO: this all relies on unsyncronzied state. Every single thing in this file
// is racey, but I can't be bothered to learn JavaScript because I hate it. 🙈

const AnnePro2Layout = Object.freeze([
    'esc', '1', '2', '3', '4', '5',  '6',  '7',  '8',  '9',  '0',  'minus',  'equals',  'backspace',
    'tab', 'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  'o',  'p',  'lbracket',  'rbracket',  'backslash',
    'caps', 'a',  's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',  'semicolon',  'apostrophe',  'enter',
    'deadkey1',  'leftshift',  'deadkey2',  'z',  'x',  'c',  'v',  'b',  'n',  'm',  'comma',  'dot',  'slash',  'rightshift',
    'deadkey3',  'leftctrl',  'deadkey4',  'leftsuper',  'leftalt',  'deadkey5',
    'deadkey6',  'space',  'deadkey7',
    'deadkey8',  'rightalt',  'fn',  'context',  'rightctrl',  'deadkey9',
]);
const AnnePro2MCUAddress = 65;
const AnnePro2ServiceData = [0, 123, 16, AnnePro2MCUAddress];
const AnnePro2StaticMessage = [0, 0, 125];
const AnnePro2CommandInfo = [32, 3, 255];

// This function was copied from https://github.com/illixion/Annemone and
// modified under the terms of the MIT license:
// https://github.com/illixion/Annemone/blob/e54b8903f6aa7e28f72eeda0339061c8525034dc/LICENSE
const generateMultiColor = function (arrayOfRgbValues) {
    const real_command_info_length = AnnePro2CommandInfo.length + 1;
    const maxMessageLength = 55 - real_command_info_length;
    const arrayOfRgbValuesCopy = arrayOfRgbValues.slice(0);
    const messagesToSendAmount = Math.ceil(arrayOfRgbValuesCopy.length / maxMessageLength);
    const val_1 = arrayOfRgbValuesCopy.length % maxMessageLength;
    const val_2 = 0 === val_1 ? maxMessageLength : val_1;
    const hid_command = [];
    for (let p = 0; p < messagesToSendAmount; p++) {
        const e = (messagesToSendAmount << 4) + p;
        const a = messagesToSendAmount - 1 === p ?
            val_2 + real_command_info_length :
            maxMessageLength + real_command_info_length;
        hid_command.push([
            ...AnnePro2ServiceData,
            e,
            a,
            ...AnnePro2StaticMessage,
            ...AnnePro2CommandInfo,
            2,
            ...arrayOfRgbValuesCopy.splice(0, maxMessageLength),
        ]);
    }
    return hid_command;
};

// eslint-disable-next-line no-var
var AnnePro2 = {
    write_queue: [],
    changed: false,
};

// For some reason the AnnePro2 won't respond to messages sent less than 50ms
// apart, so create a write_queue and pop from the front of it every 50ms
// instead of sending the command directly.
AnnePro2.doWrite = function() {
    let first = AnnePro2.write_queue.shift();
    if (first) {
        controller.send(first, null, 0);
        engine.stopTimer(AnnePro2.write_timer);
        AnnePro2.write_timer = engine.beginTimer(50, AnnePro2.doWrite);
    } else {
        engine.stopTimer(AnnePro2.write_timer);
        AnnePro2.write_timer = 0;
    }
};
AnnePro2.write = function (msg) {
    AnnePro2.write_queue.push(msg);
    if (!AnnePro2.write_timer) {
        AnnePro2.write_timer = engine.beginTimer(50, AnnePro2.doWrite);
    }
};
AnnePro2.writeState = function() {
    AnnePro2.changed = false;
    const messages = generateMultiColor(AnnePro2.state);
    for (let i = 0; i < messages.length; i++) {
        AnnePro2.write(messages[i]);
    }
};

// Sets individual keys.
// @param {Object} keys { esc: {"red": 255, "green": 0, "blue": 0}, 'h': … }
AnnePro2.setKeys = function (keys, write = false) {
    AnnePro2Layout.forEach((key, i) => {
        let color = keys[key];
        if (color === undefined) {
            return;
        }
        let offset = i*3;
        AnnePro2.state[offset] = color["red"];
        AnnePro2.state[offset+1] = color["green"];
        AnnePro2.state[offset+2] = color["blue"];
    })
    AnnePro2.changed = true;
    if (write) {
        AnnePro2.writeState();
    }
};

// Clear individual keys.
// @param {Array} keys ['esc', '4', 'f']
AnnePro2.clearKeys = function (keys) {
    let keyMap = {};
    keys.forEach((v) => keyMap[v] = {
        "red":   0,
        "green": 0,
        "blue":  0,
    });
    AnnePro2.setKeys(keyMap);
};

const playColor = function() {
    return {
        "red": engine.getSetting("play_color_red"),
        "green": engine.getSetting("play_color_green"),
        "blue": engine.getSetting("play_color_blue"),
    }
};

const recordColor = function() {
    return {
        "red": engine.getSetting("record_color_red"),
        "green": engine.getSetting("record_color_green"),
        "blue": engine.getSetting("record_color_blue"),
    }
};

const killColor = function() {
    return {
        "red": engine.getSetting("kill_color_red"),
        "green": engine.getSetting("kill_color_green"),
        "blue": engine.getSetting("kill_color_blue"),
    }
};

const trackEndColor = function() {
    return {
        "red": engine.getSetting("ending_color_red"),
        "green": engine.getSetting("ending_color_green"),
        "blue": engine.getSetting("ending_color_blue"),
    }
};

const trackColor = function(group) {
    return () => {
        let color = engine.getValue(group, 'track_color');
        if (color === undefined || color === -1) {
            // Default to the playback color if the track doesn't have a color
            // defined.
            return playColor();
        }
        return colorCodeToObject(color);
    };
};

const setOrClear = function(key, get_color, write = false) {
    return (value, _group, _control) => {
        if (value) {
            AnnePro2.setKeys({[key]: get_color()}, write);
        } else {
            AnnePro2.clearKeys([key]);
        }
    }
};

const hotCueStatus = function(cue, key) {
    return (value, group, _control) => {
        if (value) {
            value = engine.getValue(group, `hotcue_${cue}_color`);
        }
        const color = colorCodeToObject(value);
        AnnePro2.setKeys({[key]: color});
    };
};

const hotCueColor = function(key) {
    return (value, _group, _control) => {
        if (value === -1) {
            value = 0;
        }
        const color = colorCodeToObject(value);
        AnnePro2.setKeys({[key]: color});
    };
};

// @param intensity real 0..1
const scaleIntensity = function(color, intensity) {
    return {
        "red":   Math.round(color["red"] * intensity),
        "green": Math.round(color["green"] * intensity),
        "blue":  Math.round(color["blue"] * intensity),
    }
};

// Blend color 1 and color 2, scale is from -1..1 where -1 == fully color 1 and
// 1 == fully color 2 and 0 is an equal mix.
const blend = function(col1, col2, scale) {
    // Swap scale to be between 0 and 1 to make the math easier.
    scale = (scale + 1) / 2;
    const invScale = 1-scale;
    return {
        "red": ((col1["red"]*invScale)+(col2["red"]*scale)),
        "green": ((col1["green"]*invScale)+(col2["green"]*scale)),
        "blue": ((col1["blue"]*invScale)+(col2["blue"]*scale)),
    };
};

AnnePro2.init = function (id, debugging) {
    AnnePro2.id = id;
    AnnePro2.clearAll();

    // App
    AnnePro2.deck1Play = engine.makeConnection('[App]', 'indicator_250ms', (value, group, control) => {
        // Rewrite the state periodically instead of every single time any LED
        // blinks.
        if (AnnePro2.changed) {
            AnnePro2.writeState();
        }
    });

    // Main
    AnnePro2.crossfader = engine.makeConnection('[Master]', 'crossfader', (value, _group, _control) => {
        let mixColor = blend(trackColor('[Channel1]')(), trackColor('[Channel2]')(), value)
        AnnePro2.setKeys({
            'g': scaleIntensity(mixColor, (value <= 0) ? 1 : Math.abs(value-1)),
            'h': scaleIntensity(mixColor, (value >= 0) ? 1 : value+1),
        });
    });
    AnnePro2.record = engine.makeConnection('[Recording]', 'status', setOrClear('r', recordColor));
    AnnePro2.crossfader.trigger();
    AnnePro2.record.trigger();

    // Preview
    AnnePro2.preview1Play = engine.makeConnection('[PreviewDeck1]', 'play_indicator', setOrClear('p', trackColor('[PreviewDeck1]')));
    AnnePro2.preview1Play.trigger();

    // Deck 1
    AnnePro2.deck1Sync = engine.makeConnection('[Channel1]', 'sync_enabled', setOrClear('1', trackColor('[Channel1]')));
    AnnePro2.deck1LoopIn = engine.makeConnection('[Channel1]', 'loop_in', setOrClear('2', trackColor('[Channel1]')));
    AnnePro2.deck1LoopOut = engine.makeConnection('[Channel1]', 'loop_out', setOrClear('3', trackColor('[Channel1]')));
    AnnePro2.deck1Reloop = engine.makeConnection('[Channel1]', 'reloop_toggle', setOrClear('4', trackColor('[Channel1]')));
    AnnePro2.deck1QuickEffect = engine.makeConnection('[QuickEffectRack1_[Channel1]]', 'enabled', setOrClear('5', playColor));
    AnnePro2.deck1Sync.trigger();
    AnnePro2.deck1LoopIn.trigger();
    AnnePro2.deck1LoopOut.trigger();
    AnnePro2.deck1Reloop.trigger();
    AnnePro2.deck1QuickEffect.trigger();

    AnnePro2.deck1Loop = engine.makeConnection('[Channel1]', 'loop_enabled', setOrClear('q', playColor));
    AnnePro2.deck1PFL = engine.makeConnection('[Channel1]', 'pfl', setOrClear('t', playColor));
    AnnePro2.deck1Loop.trigger();
    AnnePro2.deck1PFL.trigger();

    AnnePro2.deck1Play = engine.makeConnection('[Channel1]', 'play_indicator', setOrClear('d', trackColor('[Channel1]')));
    AnnePro2.deck1Cue = engine.makeConnection('[Channel1]', 'cue_indicator', setOrClear('f', trackColor('[Channel1]')));
    AnnePro2.deck1Fwd = engine.makeConnection('[Channel1]', 'beatjump_forward', setOrClear('s', playColor, true));
    AnnePro2.deck1Rev = engine.makeConnection('[Channel1]', 'beatjump_backward', setOrClear('a', playColor, true));
    AnnePro2.deck1Play.trigger();
    AnnePro2.deck1Cue.trigger();
    AnnePro2.deck1Fwd.trigger();
    AnnePro2.deck1Rev.trigger();

    AnnePro2.deck1Cue1 = engine.makeConnection('[Channel1]', 'hotcue_1_status', hotCueStatus(1, 'z'));
    AnnePro2.deck1Cue2 = engine.makeConnection('[Channel1]', 'hotcue_2_status', hotCueStatus(2, 'x'));
    AnnePro2.deck1Cue3 = engine.makeConnection('[Channel1]', 'hotcue_3_status', hotCueStatus(3, 'c'));
    AnnePro2.deck1Cue4 = engine.makeConnection('[Channel1]', 'hotcue_4_status', hotCueStatus(4, 'v'));
    AnnePro2.deck1CueColor1 = engine.makeConnection('[Channel1]', 'hotcue_1_color', hotCueColor('z'));
    AnnePro2.deck1CueColor2 = engine.makeConnection('[Channel1]', 'hotcue_2_color', hotCueColor('x'));
    AnnePro2.deck1CueColor3 = engine.makeConnection('[Channel1]', 'hotcue_3_color', hotCueColor('c'));
    AnnePro2.deck1CueColor4 = engine.makeConnection('[Channel1]', 'hotcue_4_color', hotCueColor('v'));
    AnnePro2.deck1LowKill = engine.makeConnection('[Channel1]', 'filterLowKill', setOrClear('b', killColor));
    AnnePro2.deck1Cue1.trigger();
    AnnePro2.deck1Cue2.trigger();
    AnnePro2.deck1Cue3.trigger();
    AnnePro2.deck1Cue4.trigger();
    AnnePro2.deck1CueColor1.trigger();
    AnnePro2.deck1CueColor2.trigger();
    AnnePro2.deck1CueColor3.trigger();
    AnnePro2.deck1CueColor4.trigger();
    AnnePro2.deck1LowKill.trigger();

    // Deck 2
    AnnePro2.deck2Sync = engine.makeConnection('[Channel2]', 'sync_enabled', setOrClear('6', trackColor('[Channel2]')));
    AnnePro2.deck2LoopIn = engine.makeConnection('[Channel2]', 'loop_in', setOrClear('7', trackColor('[Channel2]')));
    AnnePro2.deck2LoopOut = engine.makeConnection('[Channel2]', 'loop_out', setOrClear('8', trackColor('[Channel2]')));
    AnnePro2.deck2Reloop = engine.makeConnection('[Channel2]', 'reloop_toggle', setOrClear('9', trackColor('[Channel2]')));
    AnnePro2.deck2QuickEffect = engine.makeConnection('[QuickEffectRack1_[Channel2]]', 'enabled', setOrClear('0', playColor));
    AnnePro2.deck2Sync.trigger();
    AnnePro2.deck2LoopIn.trigger();
    AnnePro2.deck2LoopOut.trigger();
    AnnePro2.deck2Reloop.trigger();
    AnnePro2.deck2QuickEffect.trigger();

    AnnePro2.deck2Loop = engine.makeConnection('[Channel2]', 'loop_enabled', setOrClear('u', playColor));
    AnnePro2.deck2PFL = engine.makeConnection('[Channel2]', 'pfl', setOrClear('y', playColor));
    AnnePro2.deck2Loop.trigger();
    AnnePro2.deck2PFL.trigger();

    AnnePro2.deck2Play = engine.makeConnection('[Channel2]', 'play_indicator', setOrClear('l', trackColor('[Channel2]')));
    AnnePro2.deck2Cue = engine.makeConnection('[Channel2]', 'cue_indicator', setOrClear('semicolon', trackColor('[Channel2]')));
    AnnePro2.deck2Fwd = engine.makeConnection('[Channel2]', 'beatjump_forward', setOrClear('k', playColor, true));
    AnnePro2.deck2Rev = engine.makeConnection('[Channel2]', 'beatjump_backward', setOrClear('j', playColor, true));
    AnnePro2.deck2Play.trigger();
    AnnePro2.deck2Cue.trigger();
    AnnePro2.deck2Fwd.trigger();
    AnnePro2.deck2Rev.trigger();

    AnnePro2.deck2Cue1 = engine.makeConnection('[Channel2]', 'hotcue_1_status', hotCueStatus(1, 'm'));
    AnnePro2.deck2Cue2 = engine.makeConnection('[Channel2]', 'hotcue_2_status', hotCueStatus(2, 'comma'));
    AnnePro2.deck2Cue3 = engine.makeConnection('[Channel2]', 'hotcue_3_status', hotCueStatus(3, 'dot'));
    AnnePro2.deck2Cue4 = engine.makeConnection('[Channel2]', 'hotcue_4_status', hotCueStatus(4, 'slash'));
    AnnePro2.deck2CueColor1 = engine.makeConnection('[Channel2]', 'hotcue_1_color', hotCueColor('m'));
    AnnePro2.deck2CueColor2 = engine.makeConnection('[Channel2]', 'hotcue_2_color', hotCueColor('comma'));
    AnnePro2.deck2CueColor3 = engine.makeConnection('[Channel2]', 'hotcue_3_color', hotCueColor('dot'));
    AnnePro2.deck2CueColor4 = engine.makeConnection('[Channel2]', 'hotcue_4_color', hotCueColor('slash'));
    AnnePro2.deck2LowKill = engine.makeConnection('[Channel2]', 'filterLowKill', setOrClear('n', killColor));
    AnnePro2.deck2Cue1.trigger();
    AnnePro2.deck2Cue2.trigger();
    AnnePro2.deck2Cue3.trigger();
    AnnePro2.deck2Cue4.trigger();
    AnnePro2.deck2CueColor1.trigger();
    AnnePro2.deck2CueColor2.trigger();
    AnnePro2.deck2CueColor3.trigger();
    AnnePro2.deck2CueColor4.trigger();
    AnnePro2.deck2LowKill.trigger();

    // All decks
    AnnePro2.endOfTrackConn = []
    AnnePro2.endOfTrackMarker = 0;
    AnnePro2.numDecks = engine.makeConnection('[App]', 'num_decks', (value, _group, _control) => {
        AnnePro2.endOfTrackConn.forEach((conn) => {
            conn.disconnect();
        });
        AnnePro2.endOfTrackConn = [];
        for (let i = 0; i < value; i++) {
            const newConn = engine.makeConnection(`[Channel${i}]`, 'end_of_track', (value, _group, _control) => {
                // Toggle the deck-th bit in a bitmask.
                // If any bit is set this will be truthy, so we can check this
                // later before setting the state of the space key to see if
                // *any* deck is within 30 seconds of ending.
                AnnePro2.endOfTrackMarker = AnnePro2.endOfTrackMarker & ~(1<<i) | (value<<i);
                setOrClear('space', trackEndColor)(AnnePro2.endOfTrackMarker, '', '');
            });
            if (newConn) {
                newConn.trigger();
                AnnePro2.endOfTrackConn.push(newConn);
            }
        }
    });
    AnnePro2.numDecks.trigger();

    AnnePro2.writeState();
};

AnnePro2.clearAll = function() {
    AnnePro2.state = new Array(AnnePro2Layout.length*3).fill(0);
    AnnePro2.changed = true;
};

AnnePro2.shutdown = function() {
    // TODO: this won't work because we don't block until the write queue has
    // been flushed (see the timer stuff later on in this file).
    // It's unclear to me how best to handle that in this old syncronous version
    // of JavaScript.
    AnnePro2.clearAll();
    AnnePro2.writeState();
};

AnnePro2.incomingData = function(data, length) {
};

// vim: set expandtab ts=4 sw=4:
