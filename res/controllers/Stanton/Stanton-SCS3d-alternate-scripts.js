"use strict";
////////////////////////////////////////////////////////////////////////
// JSHint configuration                                               //
////////////////////////////////////////////////////////////////////////
/* global engine                                                      */
/* global print                                                       */
/* global midi                                                        */
/* global SCS3D:true                                                  */
/* jshint -W097                                                       */
/* jshint laxbreak: true                                              */
////////////////////////////////////////////////////////////////////////

// Issues
// - Each deck rembembers the mode it was in, confusing? Would it be better to
//   keep the current mode on deck switch?

// Useful tinkering commands, channel reset and flat mode
// amidi -p hw:1 -S F00001600200F7
// amidi -p hw:1 -S F00001601000F7

SCS3D = {};

SCS3D.init = function(id) {
    this.device = this.Device();
    this.agent = this.Agent(this.device);
    this.agent.start();
};

SCS3D.shutdown = function() {
    SCS3D.agent.stop();
};

SCS3D.receive = function(channel, control, value, status) {
    SCS3D.agent.receive(status, control, value);
};


/* Midi map of the SCS.3d device
 *
 * Thanks to Sean M. Pappalardo for the original SCS3 mappings.
 */
SCS3D.Device = function() {
    var NoteOn = 0x90;
    var NoteOff = 0x80;
    var CC = 0xB0;

    var black = 0x00;
    var blue = 0x02;
    var red = 0x01;
    var purple = blue | red;

    function Logo() {
        var id = 0x7A;
        return {
            on: [NoteOn, id, 0x01],
            off: [NoteOn, id, 0x00]
        };
    }

    function Decklight(id) {
        return function(value) {
            return [NoteOn, id, +value]; // value might be boolean, coerce to int
        };
    }

    function Meter(id, lights) {
        var ctrl = [];
        var i = 0;
        for (; i < lights; i++) {
            ctrl[i] = [NoteOn, id + lights - i - 1];
        }
        return ctrl;
    }

    function Light(id) {
        return {
            bits: function(bits) {
                return [NoteOn, id, bits];
            },
            black: [NoteOn, id, black],
            blue: [NoteOn, id, blue],
            red: [NoteOn, id, red],
            purple: [NoteOn, id, purple],
        };
    }

    function Slider(id, meterid, lights) {
        return {
            meter: Meter(meterid, lights),
            slide: {
                abs: [CC, id],
                rel: [CC, id + 1],

            },
            touch: [NoteOn, id],
            release: [NoteOff, id]
        };
    }

    function LightSlider(id, meterid, lights) {
        var slider = Slider(id, meterid, lights);
        var redled = meterid - 2;
        var blueled = meterid - 1;

        // Contrary to the other lights, the pitch light as separate addresses for red and blue
        slider.light = {
            red: {
                on: [NoteOn, redled, 1],
                off: [NoteOn, redled, 0]
            },
            blue: {
                on: [NoteOn, blueled, 1],
                off: [NoteOn, blueled, 0]
            }
        };
        return slider;
    }

    function Touch(id, lightid) {
        if (!lightid) lightid = id;
        return {
            light: Light(lightid),
            touch: [NoteOn, id],
            release: [NoteOff, id]
        };
    }


    // Stanton changed button mode in newer devices.
    // Originally, in button mode, the three columns held 4 "buttons"
    // each. Hitting one of those without accidentally touching another requires
    // very accurate motor control beyond the capabilities of a DJ (even when
    // sober). Later versions of the device send only two buttons per column
    // (top/bottom), these are easy to hit.
    //
    // So not only do we need two id to map to the same field, we want to
    // control multiple lights for this control as well. This is why we're using
    // the plural here. The respective functions expect() and tell() know about
    // this, see demux().
    function Field(ids, lightids) {
        return {
            touch: [NoteOn, ids],
            release: [NoteOff, ids],
            light: Light(lightids)
        };
    }

    return {
        modeset: {
            // Byte three is the channel
            version: [0xF0, 0x7E, 0x00, 0x06, 0x01, 0xF7],
            flat: [0xF0, 0x00, 0x01, 0x60, 0x10, 0x00, 0xF7],
            circle: [0xF0, 0x00, 0x01, 0x60, 0x01, 0x00, 0xF7],
            slider: [0xF0, 0x00, 0x01, 0x60, 0x01, 0x03, 0xF7],
            button: [0xF0, 0x00, 0x01, 0x60, 0x01, 0x04, 0xF7],
            // Byte 6 sets the channel
            channel: [0xF0, 0x00, 0x01, 0x60, 0x02, 0x00, 0xF7]
        },
        logo: Logo(),
        decklight: [
            Decklight(0x71), // A
            Decklight(0x72) // B
        ],
        gain: Slider(0x07, 0x34, 9),
        pitch: LightSlider(0x03, 0x3F, 9),
        mode: {
            fx: Touch(0x20),
            loop: Touch(0x22),
            vinyl: Touch(0x24),
            eq: Touch(0x26),
            trig: Touch(0x28),
            deck: Touch(0x2A),
        },
        top: {
            left: Touch(0x2C),
            right: Touch(0x2E)
        },
        slider: {
            circle: Slider(0x62, 0x5d, 16),
            left: Slider(0x0C, 0x48, 7),
            middle: Slider(0x01, 0x56, 7),
            right: Slider(0x0E, 0x4F, 7)
        },
        field: [
            Touch([0x48, 0x4A], [0x61, 0x62, 0x63]),
            Touch([0x4C, 0x4E], [0x5E, 0x5F, 0x60]),
            Touch([0x4F, 0x51], [0x66, 0x67, 0x68]),
            Touch([0x53, 0x55], [0x69, 0x6A, 0x6B]),
            Touch(0x01, [0x64, 0x65, 0x5D, 0x6C])
        ],
        bottom: {
            left: Touch(0x30),
            right: Touch(0x32)
        },
        button: {
            play: Touch(0x6D),
            cue: Touch(0x6E),
            sync: Touch(0x6F),
            tap: Touch(0x70),
        }
    };
};

// debugging helper
var printmess = function(message, text) {
    var i;
    var s = '';

    for (i in message) {
        s = s + ('0' + message[i].toString(16)).slice(-2);
    }
    print("Midi " + s + (text ? ' ' + text : ''));
};



SCS3D.Comm = function() {
    // Build a control identifier (CID) from the first two message bytes.
    function CID(message) {
        return (message[0] << 8) + message[1];
    }

    // Static state of the LED, indexed by CID
    // This keeps the desired state before modifiers, so that adding
    // or removing modifiers is possible without knowing the base state.
    var base = {};

    // Modifier functions over base, indexed by CID
    // The functions receive the last byte of the message and return the
    // modified value.
    var mask = {};

    // List of masks that depend on time
    var ticking = {};

    // CID that may need updating
    var dirty = {};

    // Last sent messages, indexed by CID
    var actual = {};

    // Tick counter
    var ticks = 0;

    // Handler functions indexed by CID
    var receivers = {};

    // List of handlers for control changes from the engine
    var watched = {};

    // Last sent SYSEX message
    var actual_sysex = [];

    // List of functions to call on each tick
    var repeats = [];

    function send() {
        for (var cid in dirty) {
            var message = base[cid];
            if (!message) continue; // As long as no base is set, don't send anything

            var last = actual[cid];

            var value = +message[2];
            if (mask[cid]) {
                value = mask[cid](value, ticks);
            }
            if (last === undefined || last !== value) {
                midi.sendShortMsg(message[0], message[1], value);
                actual[cid] = value;
            }
        }
        dirty = {};
    }

    return {
        base: function(message, force) {
            var cid = CID(message);

            base[cid] = message;
            dirty[cid] = true;

            if (force) {
                delete actual[cid];
            }
        },

        mask: function(message, mod, changes) {
            var cid = CID(message);
            mask[cid] = mod;
            dirty[cid] = true;

            if (changes) ticking[cid] = true;
        },

        unmask: function(message) {
            var cid = CID(message);
            if (mask[cid]) {
                delete mask[cid];
                dirty[cid] = true;
            }
        },

        repeat: function(rep) {
            repeats.push(rep);
        },

        tick: function() {
            for (var i in repeats) {
                repeats[i](ticks);
            }
            for (var cid in ticking) {
                dirty[cid] = true;
            }
            send();
            ticks += 1;
        },

        clear: function() {
            receivers = {};
            ticking = {};
            repeats = [];
            for (var cid in mask) {
                dirty[cid] = true;
            }
            mask = {};
            // base and actual are not cleared

            // I'd like to disconnect all controls on clear, but that doesn't
            // work when using closure callbacks. So we just don't listen to
            // those
            for (var ctrl in watched) {
                if (watched.hasOwnProperty(ctrl)) {
                    watched[ctrl] = [];
                }
            }
        },

        expect: function(message, handler) {
            if (!message || message.length < 2) print("ERROR: invalid message to expect: " + message);
            var cid = CID(message);
            if (receivers[cid]) return; // Don't steal
            receivers[cid] = handler;
        },

        receive: function(type, control, value) {
            var cid = CID([type, control]);
            var handler = receivers[cid];
            if (handler) {
                handler(value);
                send();
            }
        },

        watch: function(channel, control, handler) {
            var ctrl = channel + control;

            if (!watched[ctrl]) {
                watched[ctrl] = [];
                engine.connectControl(channel, control, function(value, group, control) {
                    var handlers = watched[ctrl];
                    if (handlers.length) {
                        // Fetching parameter value is easier than mapping to [0..1] range ourselves
                        value = engine.getParameter(group, control);

                        var i = 0;
                        for (; i < handlers.length; i++) {
                            handlers[i](value);
                        }
                        send();
                    }
                });
            }

            watched[ctrl].push(handler);

            engine.trigger(channel, control);
        },

        sysex: function(message) {
            if (message.length === actual_sysex.length) {
                var same = true;
                for (var i in message) {
                    same = same && message[i] === actual_sysex[i];
                }
                if (same) return;
            }

            midi.sendSysexMsg(message, message.length);
            actual_sysex = message;
        }
    };
};


// Create a function that sets the rate of each channel by the timing between
// calls
SCS3D.Syncopath = function() {
    // Lists of last ten taps, per deck, in epoch milliseconds
    var deckTaps = {};

    return function(channel) {
        var now = new Date().getTime();
        var taps = deckTaps[channel] || [];

        var last = taps[0] || 0;
        var delta = now - last;

        // Reset when taps are stale
        if (delta > 2000) {
            deckTaps[channel] = [now];
            return;
        }

        taps.unshift(now);
        taps = taps.slice(0, 8); // Keep last eight
        deckTaps[channel] = taps;

        //  Don't set rate until we have enough taps
        if (taps.length < 3) return;

        // Calculate average bpm
        var intervals = taps.length - 1;
        var beatLength = (taps[0] - taps[intervals]) / intervals;
        var bpm = 60000 / beatLength; // millis to 1/minutes

        // The desired pitch rate depends on the BPM of the track
        var rate = bpm / engine.getValue(channel, "file_bpm");

        // Balk on outlandish rates
        if (isNaN(rate) || rate < 0.05 || rate > 50) return;

        // Translate rate into pitch slider position
        // This depends on the configured range of the slider
        var pitchPos = (rate - 1) / engine.getValue(channel, "rateRange");

        engine.setValue(channel, "rate", pitchPos);
    };
};


SCS3D.Agent = function(device) {

    // Multiple controller ID may be specified in the MIDI messages used
    // internally. The output functions will demux and run the same action on
    // both messages.
    //
    // demux(function(message) { print message; })(['hello', ['me', 'you']])
    // -> hello,me
    // -> hello,you
    function demux(action) {
        return function(message, nd) {
            if (!message || message.length < 2) {
                print("ERROR: demux over invalid message: " + message);
                return false;
            }
            var changed = false;
            if (message[1].length) {
                var i;
                for (i in message[1]) {
                    var demuxd = [message[0], message[1][i], message[2]];
                    changed = action(demuxd, nd) || changed;
                }
            } else {
                changed = action(message, nd);
            }
            return changed;
        };
    }

    var comm = SCS3D.Comm();
    var taps = SCS3D.Syncopath();

    function expect(control, handler) {
        demux(function(control) {
            comm.expect(control, handler);
        })(control);
    }

    function watch(channel, control, handler) {
        comm.watch(channel, control, handler);
    }

    function watchmulti(controls, handler) {
        var values = {};
        var wait = 0;

        var watchPos = function(valuePos) {
            watch(controls[k][0], controls[k][1], function(value) {
                values[valuePos] = value;

                // Call handler once all values are collected
                // The simplistic wait countdown works because watch()
                // triggers all controls and they answer in series
                if (wait > 1) {
                    wait -= 1;
                } else {
                    handler(values);
                }
            });
        };

        for (var k in controls) {
            wait += 1;
            watchPos(k);
        }
    }

    // Send MIDI message to device
    // Param message: list of three MIDI bytes
    // Param force: send value regardless of last recorded state
    var tell = demux(function(message, force) {
        comm.base(message, force);
    });

    // Map engine values in the range [0..1] to lights
    // translator maps from [0..1] to a midi message (three bytes)
    function patch(translator) {
        return function(value) {
            tell(translator(value));
        };
    }

    // Patch multiple
    function patchleds(translator) {
        return function(value) {
            var msgs = translator(value);
            for (var i in msgs) {
                if (msgs.hasOwnProperty(i)) tell(msgs[i]);
            }
        };
    }

    function binarylight(off, on) {
        return function(value) {
            tell(value ? on : off);
        };
    }

    // Return a handler that lights one LED depending on value
    function Needle(lights) {
        var range = lights.length - 1;
        return function(value) {
            // Where's the needle?
            // On the first light for zero values, on the last for one.
            var pos = null;
            if (value !== null) {
                pos = Math.min(range, Math.round(value * range));
            }

            var i = 0;
            for (; i <= range; i++) {
                var light = lights[i];
                var on = i === pos;
                tell([light[0], light[1], +on]);
            }
        };
    }

    // Return a handler that lights LED from the center of the meter
    function Centerbar(lights) {
        var count = lights.length;
        var range = count - 1;
        var center = Math.round(count / 2) - 1; // Zero-based
        return function(value) {
            var pos = Math.max(0, Math.min(range, Math.round(value * range)));
            var left = Math.min(center, pos);
            var right = Math.max(center, pos);
            var i = 0;
            for (; i < count; i++) {
                var light = lights[i];
                var on = i >= left && i <= right;
                tell([light[0], light[1], +on]);
            }
        };
    }


    function CircleCenterbar(lights) {
        var count = lights.length;
        var range = count - 1;
        var center = range / 2; // Zero-based
        return function(value) {
            var pos = Math.max(0, Math.min(range, (1 - value) * range));
            var left = Math.min(center, pos);
            var right = Math.max(center, pos);

            var i = 0;
            for (; i < count; i++) {
                var light = lights[i];
                var on = i >= left && i <= right;
                tell([light[0], light[1], +on]);
            }
        };
    }

    // Return a handler that lights LED from the bottom of the meter
    // For zero values no light is turned on
    function Bar(lights) {
        var count = lights.length;
        var range = count - 1;
        return function(value) {
            var pos;
            if (value === 0) {
                pos = -1; // no light
            } else {
                pos = Math.max(0, Math.min(range, Math.round(value * range)));
            }
            var i = 0;
            for (; i < lights.length; i++) {
                var light = lights[i];
                var on = i <= pos;
                tell([light[0], light[1], +on]);
            }
        };
    }

    // Create a function that returns a constant value
    var constant = function(val) {
        var constant = val;
        return function() {
            return constant;
        };
    };

    // Light leds according to function
    function lightsmask(lights, maskfunc) {
        var mask = function(nr) {
            return function(value, ticks) {
                return maskfunc(lights.length, nr, value, ticks);
            };
        };

        for (var nr in lights) {
            var light = lights[lights.length - 1 - nr];
            comm.mask(light, mask(nr), true);
        }
    }

    // Light leds in the circle according to pattern
    // Pattern is a two-dimensional array 3 x 7 of bools
    function centerlights(pattern, rate) {
        var slidernames = ['left', 'middle', 'right'];
        for (var y in slidernames) {
            var lights = device.slider[slidernames[y]].meter;
            for (var x in lights) {
                var light = lights[lights.length - 1 - x];
                var pat = pattern[x][y];
                if (pat.length) {
                    // It moves!
                    comm.mask([light[0], light[1]], Blinker(rate, pat), true);
                } else {
                    comm.mask([light[0], light[1]], constant(pat), false);
                }
            }
        }
    }


    // Create a function that returns the value or its boolean inverse
    // First parameter controls the blink rate where bigger is slower
    // (starts at 1; 2 is half the speed)
    // Second parameter provides a blink pattern which is a list of bits
    function Blinker(rate, pattern) {
        return function(value, ticks) {
            return pattern[Math.floor(ticks / rate) % pattern.length] ? !value : value;
        };
    }

    var blinken = {
        fast: new Blinker(1, [1, 0]),
        heartbeat: new Blinker(1, [1, 0, 1, 0, 0, 0, 0, 0, 0]),
    };

    // Show a spinning light in remembrance of analog technology
    function spinLight(channel, warnDuration) {
        watchmulti({
            'position': [channel, 'playposition'],
            'duration': [channel, 'duration'],
            'play': [channel, 'play'],
            'play_indicator': [channel, 'play_indicator'],
            'rate': [channel, 'rate'],
            'range': [channel, 'rateRange']
        }, function(values) {
            // Duration is not rate-corrected
            var duration = values.duration;

            // Which means the seconds we get are not rate-corrected either.
            // They tick faster for higher rates.
            var seconds = duration * values.position;

            // 33⅓rpm = 100 / 3 / 60 rounds/second = 1.8 seconds/round
            var rounds = seconds / 1.8;

            // Fractional part is needle's position in the circle
            // Light addressing starts bottom left, add offset so it starts at top like the spinnies
            var needle = (rounds + 0.5) % 1;

            var lights = device.slider.circle.meter;
            var count = lights.length;
            var pos = false;

            // Don't show position indicator when the end is reached
            if (values.play_indicator) {
                pos = count - Math.floor(needle * count) - 1; // Zero-based index
            }

            // Add a warning indicator for the last seconds of a song
            var left = duration - seconds;

            // Because the seconds are not rate-corrected, we must scale
            // warnDuration according to pitch rate.
            var scaledWarnDuration = warnDuration + warnDuration * ((values.rate - 0.5) * 2 * values.range);
            var warnPos = false;
            if (values.play_indicator && left < scaledWarnDuration) {
                // Add a blinking light that runs a tad slower so the needle
                // will reach it when the track runs out
                var warnOffset = count - Math.floor(count * (left / scaledWarnDuration));
                warnPos = (pos + warnOffset) % count;
            }

            var i = 0;
            for (; i < count; i++) {
                if (i === warnPos) {
                    comm.mask(lights[i], values.play ? blinken.heartbeat : blinken.fast, true);
                } else if (i === pos) {
                    comm.mask(lights[i], function(value) {
                        return !value;
                    }); // Invert
                } else {
                    comm.unmask(lights[i]);
                }
            }
        });
    }


    function both(c1, c2) {
        return function(value) {
            c1(value);
            c2(value);
        };
    }

    function invert(handler) {
        return function(value) {
            return handler(1 - value);
        };
    }


    // absolute control
    function set(channel, control) {
        return function(value) {
            engine.setParameter(channel, control,
                value / 127
            );
        };
    }


    // use circle as a fader with dead zone to avoid accidental cutover
    function circleset(channel, control) {
        return function(value) {
            var turned = (value / 127 + 0.5) % 1;
            var centered = (turned - 0.5) * 20 / 16;
            var normalized = Math.max(0, Math.min(1, centered + 0.5));
            engine.setParameter(channel, control, normalized);
        };
    }

    function setConst(channel, control, value) {
        return function() {
            engine.setParameter(channel, control, value);
        };
    }

    function reset(channel, control) {
        return function() {
            engine.reset(channel, control);
        };
    }

    // relative control
    function budge(channel, control, scale) {
        var length = 128 / (scale || 1);
        return function(offset) {
            engine.setValue(channel, control,
                engine.getValue(channel, control) + (offset - 64) / length
            );
        };
    }

    // switch
    function toggle(channel, control) {
        return function() {
            engine.setValue(channel, control, !engine.getValue(channel, control));
        };
    }

    function Switch() {
        var engaged = false;

        function change(state) {
            var prev = engaged;
            engaged = !!state; // Coerce to bool
            return engaged !== prev;
        }
        return {
            'change': function(state) {
                return change(state);
            },
            'engage': function() {
                return change(true);
            },
            'cancel': function() {
                return change(false);
            },
            'toggle': function() {
                return change(!engaged);
            },
            'engaged': function() {
                return engaged;
            },
            'choose': function(off, on) {
                return engaged ? on : off;
            }
        };
    }

    function Modeswitch(presetMode, patches) {
        var engaged = presetMode;

        return {
            engage: function(newMode) {
                return function() {
                    if (engaged === newMode) return false;
                    engaged = newMode;
                    return true;
                };
            },
            engaged: function() {
                return engaged;
            },
            patch: function() {
                return patches[engaged];
            }
        };
    }

    function MultiModeswitch(presetMode, modePatches) {
        var engagedMode = presetMode;
        var engagedPatch = modePatches[engagedMode][0];

        // For every mode, keep the patch that was engaged last
        var engaged = {};
        engaged[presetMode] = engagedPatch;

        var heldMode = false;
        var heldPatch = false;
        var lastHold = 0;

        return {
            hold: function(newHeldMode) {
                return function() {
                    heldMode = newHeldMode;
                    heldPatch = engaged[heldMode];
                    if (!heldPatch) heldPatch = modePatches[heldMode][0];

                    lastHold = new Date().getTime();
                    return true;
                };
            },
            release: function(releasedMode) {
                return function() {
                    if (releasedMode === heldMode || releasedMode === true) {
                        if (new Date().getTime() - lastHold < 200) {
                            // The button was just touched, not held
                            var patches = modePatches[heldMode];
                            if (engagedMode === heldMode) {
                                // Cycle to the next patch
                                engagedPatch = patches[(patches.indexOf(engagedPatch) + 1) % patches.length];
                                engaged[heldMode] = engagedPatch;
                            } else {
                                // Switch to the mode
                                engagedMode = heldMode;
                                engagedPatch = heldPatch;
                                engaged[heldMode] = heldPatch;
                            }
                        }
                    }
                    heldMode = false;
                    heldPatch = false;
                    return true;
                };
            },
            held: function() {
                return heldMode;
            },
            engaged: function() {
                return engagedMode;
            },
            active: function() {
                return heldPatch || engagedPatch;
            }
        };
    }


    var ExpectHeld = function(ctrl, whenShort, whenHeld) {
        var lastHold = 0;
        expect(ctrl.touch, function() {
            lastHold = new Date().getTime();
        });
        expect(ctrl.release, function(val) {
            if (new Date().getTime() > lastHold + 1000) {
                return whenHeld(val);
            } else {
                return whenShort(val);
            }
        });
    };

    // The current deck
    // Deck 1: 0b00
    // Deck 2: 0b01
    // Deck 3: 0b10
    // Deck 4: 0b11
    var deck = 0; // Deck 1 is preset

    // Glean current channels from control value
    function gleanChannel(value) {
        var readDeck;
        var changed = false;

        // check third bit and proceed if it's set
        // otherwise the control is assumed not to carry deck information
        if (value & 0x4) {
            // I don't often get the pleasure to work with bits
            // Sure a simple if() cluster would be more clear

            // Get side we're on (1 == right)
            var side = deck & 1;

            // Which bit to read (read bit 2 for right)
            var altBit = 1 << side;

            // Whether the main or the alt deck is selected on the SCS3M
            var alt = !!(value & altBit);

            // construct new deck value
            var newDeck = side | alt << 1;

            changed = newDeck !== deck;
            deck = newDeck;
        }

        if (changed) {
            // Prevent stuck mode buttons on deck switch
            mode[0].release(true);
            mode[1].release(true);
            mode[2].release(true);
            mode[3].release(true);
        }
        return changed;
    }

    function repatch(handler) {
        return function(value) {
            var changed = handler(value);
            if (changed) {
                comm.clear();
                patchage();
            }
        };
    }

    var buttons = [device.top.left, device.top.right, device.bottom.left, device.bottom.right];

    var deckLights = function() {
        for (var i in buttons) {
            tell(buttons[i].light[deck === +i ? 'red' : 'black']);
        }
    };

    var FxPatch = function(nr) {
        return function(channel, held) {
            var effectunit = '[EffectRack1_EffectUnit' + (nr + 1) + ']';
            var effectunit_effect = '[EffectRack1_EffectUnit' + (nr + 1) + '_Effect1]';
            comm.sysex(device.modeset.slider);

            // The first three effect parameters are mapped onto the circle sliders
            var params = {
                'parameter1': device.slider.left,
                'parameter2': device.slider.middle,
                'parameter3': device.slider.right,
            };

            var sliderPatch = function(slider, paramName) {
                expect(slider.slide.abs, set(effectunit_effect, paramName));

                // When the control is available for this effect unit, we
                // show a needle indicator on the meter
                var updater = Needle(slider.meter);
                watchmulti({
                    loaded: [effectunit_effect, paramName+'_loaded'],
                    value:  [effectunit_effect, paramName]
                }, function(param) {
                    if (param.loaded) {
                        updater(param.value);
                    } else {
                        // Don't display the needle
                        updater(null);
                    }
                });
            };

            for (var paramName in params) {
                sliderPatch(params[paramName], paramName);
            }

            if (held) {
                // change effect when slider is touched
                // Because the slider release does not tell us where it was released, we read the direction from the slide events
                var direction = 1;
                expect(device.pitch.slide.abs, function(value) {
                    direction = value < 64 ? 1 : -1;
                });
                expect(device.pitch.release, function() {
                    setConst(effectunit, 'chain_selector', direction)();
                });

                Bar(device.pitch.meter)(0); // Turn off pitch bar lights
                lightsmask(device.pitch.meter, function(count, nr, value, ticks) {
                    var range = (count) / 2;
                    var center = Math.round(count / 2) - 1; // Zero-based
                    var lighted = Math.abs(nr - center) === ticks % range;
                    return lighted ? !value : value;
                });
            } else {
                // Pitch slider controls wetness
                tell(device.pitch.light.red.off);
                tell(device.pitch.light.blue.off);
                watch(effectunit, 'mix', Bar(device.pitch.meter));
                expect(device.pitch.slide.abs, set(effectunit, 'mix'));
            }

            // Button light color:
            // When effect is assigned to deck: blue
            // When effect is the currently active: red
            // May be both
            var fxlight = function(light, active) {
                return function(enabled) {
                    var color = enabled ? (active ? 'purple' : 'blue') : (active ? 'red' : 'black');
                    tell(light[color]);
                };
            };

            for (var i in buttons) {
                var button = buttons[i];
                var unit = (+i + 1); // coerce i to num
                var assigned_effectunit = '[EffectRack1_EffectUnit' + unit + ']';
                var effectunit_enable = 'group_' + channel + '_enable';
                if (held) {
                    expect(button.touch, repatch(toggle(assigned_effectunit, effectunit_enable)));
                    watch(assigned_effectunit, effectunit_enable, fxlight(button.light, deck === +i));
                } else {
                    var activate = repatch(effectModes[channel].engage(i));
                    expect(button.touch, activate);
                    watch(assigned_effectunit, effectunit_enable, fxlight(button.light, nr === +i));
                }
            }
        };
    };

    // Active effect mode per channel
    var effectPatches = [FxPatch(0), FxPatch(1), FxPatch(2), FxPatch(3)];
    var effectModes = {
        '[Channel1]': Modeswitch(0, effectPatches),
        '[Channel2]': Modeswitch(1, effectPatches),
        '[Channel3]': Modeswitch(2, effectPatches),
        '[Channel4]': Modeswitch(3, effectPatches)
    };

    var FxSuperPatch = function(nr) {
        return function(channel, held) {
            var effectunit = '[EffectRack1_EffectUnit' + (nr + 1) + ']';
            comm.sysex(device.modeset.circle);
            watch(effectunit, 'super1', CircleCenterbar(device.slider.circle.meter));
            expect(device.slider.circle.slide.abs, circleset(effectunit, 'super1'));
        };
    };

    // Active effect mode per channel
    var effectSuperPatches = [FxSuperPatch(0), FxSuperPatch(1), FxSuperPatch(2), FxSuperPatch(3)];
    var effectSuperModes = {
        '[Channel1]': Modeswitch(0, effectSuperPatches),
        '[Channel2]': Modeswitch(1, effectSuperPatches),
        '[Channel3]': Modeswitch(2, effectSuperPatches),
        '[Channel4]': Modeswitch(3, effectSuperPatches)
    };


    function fxpatch(channel, held) {
        tell(device.mode.fx.light.red);
        effectModes[channel].patch()(channel, held);
    }

    function fxsuperpatch(channel, held) {
        tell(device.mode.fx.light.blue);
        effectSuperModes[channel].patch()(channel, held);
    }

    function eqpatch(channel, held) {
        comm.sysex(device.modeset.slider);
        tell(device.mode.eq.light.red);
        pitchPatch(channel);

        var eff = "[EqualizerRack1_" + channel + "_Effect1]";
        watch(eff, 'parameter1', Centerbar(device.slider.left.meter));
        watch(eff, 'parameter2', Centerbar(device.slider.middle.meter));
        watch(eff, 'parameter3', Centerbar(device.slider.right.meter));

        var op = held ? reset : set;
        expect(device.slider.left.slide.abs, op(eff, 'parameter1'));
        expect(device.slider.middle.slide.abs, op(eff, 'parameter2'));
        expect(device.slider.right.slide.abs, op(eff, 'parameter3'));


        deckLights();
    }

    function LoopPatch(rolling) {
        return function(channel) {
            // Keeps the current loop length
            var currentLen = false;

            var cancel = function() {
                if (currentLen) setConst(channel, 'reloop_exit', 1)();
                currentLen = false;
            };

            var setup = function(engage, cancelIfEngaged) {
                comm.sysex(device.modeset.circle);
                tell(rolling ? device.mode.loop.light.blue : device.mode.loop.light.red);
                pitchPatch(channel);
                deckLights();
                beatJump(channel);

                // Available loop lengths are powers of two in the range [-5..6]
                var lengths = ['0.03125', '0.0625', '0.125', '0.25', '0.5', '1', '2', '4', '8', '16', '32', '64'];
                expect(device.slider.circle.slide.abs, function(value) {
                    // Map to range [-63..64] where 0 is top center
                    var lr = ((value + 64) % 128 - 63);

                    // Map the circle slider position to a loop length
                    var exp = Math.ceil(Math.max(-5, Math.min(6, lr / 8)));
                    var len = lengths[4 + exp]; // == Math.pow(2, exp);

                    if (len === undefined) return;
                    if (len === currentLen) return;
                    currentLen = len;

                    if (rolling) {
                        set(channel, 'beatlooproll_' + len + '_activate')(true);
                        engage();
                    } else {
                        set(channel, 'beatloop_' + len + '_activate')(true);
                    }
                });

                var engineControls = {};
                lengths.forEach(function(len, index) {
                    engineControls[index] = [channel, 'beatloop_' + len + '_enabled'];
                });
                watchmulti(engineControls, function(values) {
                    var activeIndex = false;
                    lengths.forEach(function(len, index) {
                        if (values[index]) {
                            currentLen = len;
                            activeIndex = index;
                        }
                    });
                    if (activeIndex === false) {
                        // Turn off all lights
                        Bar(device.slider.circle.meter)(0);
                        currentLen = false;
                    } else {
                        Centerbar(device.slider.circle.meter)(
                            (12.5 - activeIndex) / 16
                        );
                    }
                });

                // Rolling loops are released as soon as the finger is taken off the circle
                // All loops are released when touching center
                // This covers the case when you are in rolling mode but a nonrolling loop is playing and you want to release that one by touching center
                if (rolling) {
                    expect(device.slider.circle.release, cancelIfEngaged);
                    expect(device.slider.middle.release, cancel);
                } else {
                    // In normal loop mode, touching center when the loop is
                    // not active engages it
                    expect(device.slider.middle.release, setConst(channel, 'reloop_exit', 1));
                }

                watchmulti({
                    enabled: [channel, 'loop_enabled'],
                    position: [channel, 'playposition'],
                    start: [channel, 'loop_start_position'],
                    end: [channel, 'loop_end_position'],
                    samples: [channel, 'track_samples']
                }, function(values) {
                    if (values.enabled) {
                        var samplepos = values.position * values.samples;
                        var pos = Math.floor((samplepos - values.start) / (values.end - values.start) * 8);
                        centerlights([
                            [0, 0, 0],
                            [0, pos === 0 ? 0 : 1, 0],
                            [pos === 7 ? 0 : 1, 0, pos === 1 ? 0 : 1],
                            [pos === 6 ? 0 : 1, 0, pos === 2 ? 0 : 1],
                            [pos === 5 ? 0 : 1, 0, pos === 3 ? 0 : 1],
                            [0, pos === 4 ? 0 : 1, 0],
                            [0, 0, 0]
                        ], 2);
                    } else {
                        centerlights([
                            [0, 0, 0],
                            [0, 0, 0],
                            [1, 0, rolling ? 0 : 1],
                            [1, 1, 1],
                            [1, 0, rolling ? 0 : 1],
                            [0, 0, 0],
                            [0, 0, 0]
                        ], 2);
                    }
                });

            };

            if (rolling) {
                // The rolling loop will be canceled as soon as you release the circle
                // or switch to another mode.
                Autocancel('rolling', setup, cancel);
            } else {
                // Normal loops are canceled only by touching center
                setup(false, false);
            }
        };
    }

    // Keep track of hotcue to reset on layout changes or when another hotcue
    // becomes active.
    var resetHotcue = false;

    /* Patch circle buttons to five hotcues
     *
     * left top = hotcue 1
     * left bottom = hotcue 2
     * right top = hotcue 3
     * right bottom = hotcue 4
     * center strip = hotcue 5
     *
     * The trigset parameter selects the set of hotcues to
     * activate. Passing 0 selects hotcues 1 through 5, passing 2
     * selects hotcues 11 through 15.
     */
    function Trigpatch(trigset) {
        var touchRelease = function(channel, field, control) {
            // We need to send a zero when the control is released again.
            // But this may happen after another control is touched.
            // So the reset must happen whenever
            // 1. this trigger button is released
            // 2. another trigger button is touched
            // 3. repatch() happens
            //
            // To avoid sending spurious resets we only do reset once after a
            // touch. Unfortunately there is a border case we can't cover
            // without intricate logic. For older devices two fields are
            // merged into one but we receive note on then off when sliding
            // between the two.
            var cocked = 0;

            var release = function() {
                if (cocked === 1) engine.setValue(channel, control, 0);
                if (cocked > 0) cocked -= 1;
            };
            expect(field.touch, function() {
                // resetHotcue might be set by another hotcue
                if (cocked === 0) {
                    if (resetHotcue) resetHotcue();
                    engine.setValue(channel, control, 1);
                }
                resetHotcue = function() {
                    release();
                    resetHotcue = false;
                };
                cocked += 1;

            });
            expect(field.release, release);
        };

        return function(channel, held) {
            comm.sysex(device.modeset.button);
            tell(device.mode.trig.light.bits(trigset + 1));
            pitchPatch(channel);
            deckLights();

            var i = 0;
            var offset = trigset * 5;
            for (; i < 5; i++) {
                var hotcue = offset + i + 1;
                var field = device.field[i];
                if (held) {
                    expect(field.touch, setConst(channel, 'hotcue_' + hotcue + '_clear', true));
                } else {
                    touchRelease(channel, field, 'hotcue_' + hotcue + '_activate');
                }
                watch(channel, 'hotcue_' + hotcue + '_enabled', binarylight(field.light.black, field.light.red));
            }

            var t = held ? [1, 0] : 1;
            centerlights([
                [0, 0, 0],
                [t, trigset === 2 ? t : 0, t],
                [0, trigset === 1 ? t : 0, 0],
                [0, t, 0],
                [0, trigset === 1 ? t : 0, 0],
                [t, trigset === 2 ? t : 0, t],
                [0, 0, 0]
            ], 1);
        };
    }

    // On mode switch, temporary loops and rate changes must be canceled
    // This dictionary keeps the canceling callbacks
    var autocancel = {};

    // Registrar for modes that have temporary states to be canceled on mode changes
    // Arguments:
    // name: register autocanceling under this name
    //       Only the last canceling operation registered under this name will be called
    // setup: setup function that configures the mode
    //        this functions gets passed two arguments: engage and cancelIfEngaged.
    //        when the temp mode is activated, setup should call engage()
    //        when the temp mode should be canceled, cancelIfEngaged() which in turn will call the cancel callback passed to the registrar if (and only if) the temp mode was activated.
    // cancel: function to call to cancel the temp mode
    function Autocancel(name, setup, cancel) {
        // Nausea: The feeling you're implementing overcomplicated logic
        var engage = function() {
            autocancel[name] = cancel;
        };
        var cancelIfEngaged = function() {
            if (autocancel[name]) autocancel[name]();
            delete autocancel[name];
        };
        setup(engage, cancelIfEngaged);
    }

    function needleDrop(channel) {
        // Needledrop into track
        expect(device.slider.circle.slide.abs, circleset(channel, "playposition"));
        watch(channel, "playposition", invert(Bar(device.slider.circle.meter)));
    }

    function beatJump(channel) {
        expect(device.top.left.touch, setConst(channel, 'beatjump_1_backward', 1));
        expect(device.top.right.touch, setConst(channel, 'beatjump_1_forward', 1));
    }

    function scratchpatch(channel, held) {
        comm.sysex(device.modeset.circle);
        tell(device.mode.vinyl.light.blue);
        pitchPatch(channel);

        // The four buttons select pitch slider mode when vinyl is held
        if (held) {
            pitchModeSelect();
            needleDrop(channel);
        } else {
            deckLights();
        }

        var lights = function(forward) {
            centerlights([
                [forward ? 1 : 0, 1, forward ? 0 : 1],
                [0, 1, 0],
                [0, 1, 0],
                [0, 1, 0],
                [0, 1, 0],
                [0, 1, 0],
                [forward ? 0 : 1, 1, forward ? 1 : 0]
            ], 1);
        };
        lights(true);

        // HACK ugly logic to avoid restructuring
        if (held) return;

        var channelno = parseInt(channel[8], 10); // Extract channelno to integer
        Autocancel('scratch', function(engage, cancelIfEngaged) {
            expect(device.slider.circle.touch, function() {
                engage();
                engine.scratchEnable(channelno, 128, 33 + 1 / 3, 1 / 8, 1 / 8 / 32);
            });
            expect(device.slider.middle.touch, function() {
                engage();
                engine.scratchEnable(channelno, 128, 33 + 1 / 3, 1 / 16, 1 / 16 / 32);
            });
            expect(device.slider.circle.slide.rel, function(val) {
                engine.scratchTick(channelno, val - 64);
                lights(val > 63);
            });
            expect(device.slider.middle.slide.rel, function(val) {
                engine.scratchTick(channelno, val - 64);
                lights(val > 63);
            });
            expect(device.slider.circle.release, cancelIfEngaged);
            expect(device.slider.middle.release, cancelIfEngaged);
        }, function() {
            engine.scratchDisable(channelno, true);
        });
    }


    /* Patch the circle for beatmatching.
     * Sliding on the center bar will temporarily raise or lower the rate by a
     * fixed amount. The circle slider functions as a slow jog wheel.
     */
    function vinylpatch(channel, held) {
        comm.sysex(device.modeset.circle);
        pitchPatch(channel);
        beatJump(channel);

        // The four buttons select pitch slider mode when vinyl is held
        if (held) {
            pitchModeSelect();
            needleDrop(channel);
        } else {
            deckLights();

            expect(device.slider.circle.slide.rel, function(value) {
                var playing = engine.getValue(channel, 'play');
                var amount = (value - 64) / 64;
                var jogAmount = playing ? amount * 10 : amount;
                engine.setValue(channel, 'jog', jogAmount);
            });
        }

        Autocancel('temprate', function(engage, cancel) {
            expect(device.slider.middle.slide.abs, function(value) {
                engage();
                engine.setParameter(channel, 'rate_temp_down', value < 57);
                engine.setParameter(channel, 'rate_temp_up', value > 69);
            });
            expect(device.slider.middle.release, cancel);
        }, function() {
            engine.setParameter(channel, 'rate_temp_down', false);
            engine.setParameter(channel, 'rate_temp_up', false);
        });

        watchmulti({
            'down': [channel, 'rate_temp_down'],
            'up': [channel, 'rate_temp_up']
        }, function(values) {
            var dir = (values.up - values.down) / 2 + 0.5;
            Centerbar(device.slider.left.meter)(dir - 0.1);
            Centerbar(device.slider.middle.meter)(dir + 0.1);
            Centerbar(device.slider.right.meter)(dir - 0.1);
        });

        Autocancel('fast', function(engage, cancel) {
            expect(device.bottom.left.touch, both(engage, setConst(channel, 'back', 1)));
            expect(device.bottom.left.release, cancel);
            expect(device.bottom.right.touch, both(engage, setConst(channel, 'fwd', 1)));
            expect(device.bottom.right.release, cancel);
        }, function() {
            setConst(channel, 'back', 0)();
            setConst(channel, 'fwd', 0)();
        });
    }


    /* Patch the circle for library browsing
     * Touching the center bar loads the highlighted track into the deck.
     * Sliding om the circle changes the highlighted track up or down.
     *
     * Holding the deck button allows changing the active deck by pressing
     * one of the four buttons around the circle.
     */
    function deckpatch(channel, held) {
        comm.sysex(device.modeset.circle);
        pitchPatch(channel);
        deckLights();

        if (held) {
            tell(device.mode.deck.light.purple);
            var setDeck = function(newDeck) {
                return function() {
                    var changed = deck !== newDeck;
                    if (changed) {
                        deck = newDeck;

                        // see gleanChannel() for what we're doing here
                        var deckState = engine.getValue('[PreviewDeck1]', 'quantize');
                        if (deckState & 0x4) {
                            var side = deck & 1;
                            var altBit = 1 << side;
                            var alt = !!(deck & 2);

                            // shit gives me headaches, so I'm going to leave it like this
                            deckState = (deckState & ~altBit) | (alt << side);
                            engine.setValue('[PreviewDeck1]', 'quantize', deckState);
                        }
                    }
                    return changed;
                };
            };
            expect(device.top.left.touch, repatch(setDeck(0)));
            expect(device.top.right.touch, repatch(setDeck(1)));
            expect(device.bottom.left.touch, repatch(setDeck(2)));
            expect(device.bottom.right.touch, repatch(setDeck(3)));
        } else {
            tell(device.mode.deck.light.red);
            expect(device.top.left.touch, setConst('[Playlist]', 'SelectPrevPlaylist', 1));
            expect(device.bottom.left.touch, setConst('[Playlist]', 'SelectNextPlaylist', 1));
            expect(device.top.right.touch, setConst('[Playlist]', 'SelectPrevTrack', 1));
            expect(device.bottom.right.touch, setConst('[Playlist]', 'SelectNextTrack', 1));
        }

        expect(device.slider.middle.release, function() {
            engine.setValue(channel, 'LoadSelectedTrack', true);
        });

        expect(device.slider.circle.slide.rel, function(value) {
            engine.setValue('[Playlist]', 'SelectTrackKnob', (value - 64));
        });

        watch(channel, 'play', function(play) {
            if (play) {
                centerlights([
                    [1, 1, 1],
                    [0, 1, 0],
                    [0, 1, 0],
                    [0, 1, 0],
                    [0, 1, 0],
                    [0, 1, 0],
                    [0, 1, 0]
                ], 1);
            } else {
                centerlights([
                    [0, [1, 1, 1, 1, 1, 1, 0, 1, 1], 0],
                    [1, [1, 1, 1, 1, 0, 0, 1, 1, 1], 1],
                    [1, [1, 1, 1, 0, 0, 1, 1, 1, 1], 1],
                    [0, [1, 1, 0, 0, 1, 1, 1, 1, 1], 0],
                    [0, [1, 0, 0, 1, 1, 1, 1, 1, 1], 0],
                    [0, [1, 0, 1, 1, 1, 1, 1, 1, 1], 0],
                    [0, [0, 1, 1, 1, 1, 1, 1, 1, 1], 0]
                ], 1);
            }
        });
    }

    var modeMap = {
        'fx': [fxpatch, fxsuperpatch],
        'eq': [eqpatch],
        'loop': [LoopPatch(false), LoopPatch(true)],
        'trig': [Trigpatch(0), Trigpatch(1), Trigpatch(2)],
        'vinyl': [vinylpatch, scratchpatch],
        'deck': [deckpatch],
    };


    // mode for each channel
    var mode = {
        0: MultiModeswitch('deck', modeMap),
        1: MultiModeswitch('deck', modeMap),
        2: MultiModeswitch('deck', modeMap),
        3: MultiModeswitch('deck', modeMap)
    };

    // Setup a process that keeps sliding a control by a rate that can be changed
    var Sliding = function(channel, control) {
        var slidingRate = 0;
        var budge = function() {
            if (slidingRate !== 0) {
                engine.setValue(channel, control,
                    engine.getValue(channel, control) + slidingRate
                );
            }
        };
        comm.repeat(budge);
        return function(newVal) {
            var initial = slidingRate === 0;
            slidingRate = newVal;
            if (initial) budge();
        };
    };

    var pitchModeSelect = function() {
        var activePitchMode = pitchMode[deck];
        var engagedMode = activePitchMode.engaged();
        var pitchButtons = {
            'absrate': device.top.left,
            'pitch': device.top.right,
            'rate': device.bottom.left,
            'relpitch': device.bottom.right,
        };
        for (var modeName in pitchButtons) {
            var pitchButton = pitchButtons[modeName];
            expect(pitchButton.touch, repatch(activePitchMode.engage(modeName)));
            tell(pitchButton.light[engagedMode === modeName ? 'blue' : 'black']);
        }
    };

    var pitchModeMap = {
        rate: function(channel, held) {
            tell(device.pitch.light.red.on);
            tell(device.pitch.light.blue.on);
            watch(channel, 'rate', Centerbar(device.pitch.meter));

            if (held) {
                expect(device.pitch.slide.abs, reset(channel, 'rate'));
            } else {
                var setRate = Sliding(channel, 'rate');
                expect(device.pitch.slide.abs, function(value) {
                    var rate = (value - 63) / 32;
                    var sign = rate > 0 ? 1 : -1;
                    setRate(
                        sign * 0.01 * Math.pow(Math.abs(rate), 3)
                    );
                });
                expect(device.pitch.release, function(value) {
                    setRate(0);
                });
            }
        },
        absrate: function(channel, held) {
            tell(device.pitch.light.red.on);
            tell(device.pitch.light.blue.off);
            watch(channel, 'rate', Centerbar(device.pitch.meter));

            if (held) {
                expect(device.pitch.slide.abs, reset(channel, 'rate'));
            } else {
                expect(device.pitch.slide.abs, set(channel, 'rate'));
            }
        },
        relpitch: function(channel, held) {
            tell(device.pitch.light.red.off);
            tell(device.pitch.light.blue.off);
            watch(channel, 'pitch', Centerbar(device.pitch.meter));

            if (held) {
                expect(device.pitch.slide.rel, reset(channel, 'pitch'));
            } else {
                expect(device.pitch.slide.rel, budge(channel, 'pitch'));
            }
        },
        pitch: function(channel, held) {
            tell(device.pitch.light.red.off);
            tell(device.pitch.light.blue.on);
            watch(channel, 'pitch', Centerbar(device.pitch.meter));
            if (held) {
                expect(device.pitch.slide.rel, reset(channel, 'pitch'));
            } else {
                var direction = 1;
                expect(device.pitch.slide.abs, function(val) {
                    direction = val < 64 ? -1 : 1;
                });
                expect(device.pitch.release, function() {
                    engine.setValue(channel, 'pitch', Math.round(engine.getValue(channel, 'pitch') + direction));
                });
            }
        }
    };

    // pitch slider mode per channel
    var pitchMode = {
        0: Modeswitch('absrate', pitchModeMap),
        1: Modeswitch('absrate', pitchModeMap),
        2: Modeswitch('absrate', pitchModeMap),
        3: Modeswitch('absrate', pitchModeMap)
    };

    var pitchPatch = function(channel) {
        var activePitchMode = pitchMode[deck];
        activePitchMode.patch()(channel, mode[deck].held() === 'vinyl');
    };

    function patchage() {
        tell(device.logo.on);

        var channelno = deck + 1;
        var channel = '[Channel' + channelno + ']';

        tell(device.decklight[0](!(deck & 1)));
        tell(device.decklight[1](deck & 1));

        for (var name in autocancel) {
            autocancel[name]();
        }
        autocancel = {};
        if (resetHotcue) resetHotcue();

        var activeMode = mode[deck];

        // reset gain when EQ is held
        var gainOp = activeMode.held() === 'eq' ? reset : budge;
        expect(device.gain.slide.rel, gainOp(channel, 'pregain'));
        watch(channel, 'pregain', Needle(device.gain.meter));

        tell(device.mode.fx.light.black);
        tell(device.mode.eq.light.black);
        tell(device.mode.loop.light.black);
        tell(device.mode.trig.light.black);
        tell(device.mode.vinyl.light.black);
        tell(device.mode.deck.light.black);
        tell(device.mode[activeMode.engaged()].light.red);
        if (activeMode.held()) tell(device.mode[activeMode.held()].light.purple);
        expect(device.mode.fx.touch, repatch(activeMode.hold('fx')));
        expect(device.mode.fx.release, repatch(activeMode.release('fx')));
        expect(device.mode.eq.touch, repatch(activeMode.hold('eq')));
        expect(device.mode.eq.release, repatch(activeMode.release('eq')));
        expect(device.mode.loop.touch, repatch(activeMode.hold('loop')));
        expect(device.mode.loop.release, repatch(activeMode.release('loop')));
        expect(device.mode.trig.touch, repatch(activeMode.hold('trig')));
        expect(device.mode.trig.release, repatch(activeMode.release('trig')));
        expect(device.mode.vinyl.touch, repatch(activeMode.hold('vinyl')));
        expect(device.mode.vinyl.release, repatch(activeMode.release('vinyl')));
        expect(device.mode.deck.touch, repatch(activeMode.hold('deck')));
        expect(device.mode.deck.release, repatch(activeMode.release('deck')));

        // Reset circle lights
        Bar(device.slider.circle.meter)(0);
        Bar(device.slider.left.meter)(0);
        Bar(device.slider.middle.meter)(0);
        Bar(device.slider.right.meter)(0);

        // Call the patch function for the active mode
        activeMode.active()(channel, activeMode.held());

        expect(device.button.play.touch, toggle(channel, 'play'));
        watch(channel, 'play_indicator', binarylight(
            device.button.play.light.black,
            device.button.play.light.red));


        expect(device.button.cue.touch, setConst(channel, 'cue_default', true));
        expect(device.button.cue.release, setConst(channel, 'cue_default', false));
        watch(channel, 'cue_indicator', binarylight(
            device.button.cue.light.black,
            device.button.cue.light.red));

        // Sync button, red when sync lock is on
        watch(channel, 'sync_enabled', binarylight(
            device.button.sync.light.black,
            device.button.sync.light.red));

        if (activeMode.held() === 'vinyl') {
            // When VINYL is held, SYNC adjusts the beatgrid
            // When the track is playing, the beatgrid is synced to the other
            // track (we assume the tracks were beatmatched and the other
            // track's grid is fine). When the track is not playing, we align
            // the grid with the current cursor position.

            // We don't know which control we have to release until
            // the button is pressed, because the behaviour depends on
            // whether the track is playing or not
            var affectedSyncControl;

            Autocancel('beatsync',
                function(engage, cancelIfEngaged) {
                    expect(device.button.sync.release, cancelIfEngaged);
                    expect(device.button.sync.touch, function() {
                        affectedSyncControl = engine.getValue(channel, 'play') ? 'beats_translate_match_alignment' : 'beats_translate_curpos';
                        engine.setValue(channel, affectedSyncControl, true);
                        engage();
                    });
                },
                function() {
                    engine.setValue(channel, affectedSyncControl, false);
                }
            );
        } else {
            // Hold to toggle sync lock
            ExpectHeld(device.button.sync,
                setConst(channel, 'beatsync', true),
                toggle(channel, 'sync_enabled')
            );
        }

        expect(device.button.tap.touch, function() {
            taps(channel);
        });
        watch(channel, 'beat_active', binarylight(device.button.tap.light.black, device.button.tap.light.red));

        spinLight(channel, 30);

        // Read deck state from unrelated control which may be set by the 3m
        // Among all the things WRONG about this, two stand out:
        // 1. The control is not meant to transmit this information.
        // 2. A value > 1 is expected from a control which is just a toggle (suggesting a binary value)
        // This may fail at any future or past version of Mixxx and you have only me to blame for it.
        watch('[PreviewDeck1]', 'quantize', repatch(gleanChannel));
    }

    var timer = false;

    return {
        start: function() {
            // Tell it to use channel 0 and set it to flat mode
            comm.sysex(device.modeset.channel);
            comm.sysex(device.modeset.flat);

            // Initial setup
            patchage();
            if (!timer) timer = engine.beginTimer(100, comm.tick);
        },
        receive: comm.receive,
        stop: function() {
            // No need for stopTimer() because it's done automatically

            // Forget all mods
            comm.clear();

            // Send off-message to all light addresses
            var i = 0;
            for (; i < 0x80; i++) {
                tell([0x90, i, 0]);
            }
            tell(device.logo.on); // Turn the logo back on
            comm.tick(); // The last tick, causes sending of messages
        }
    };
};
