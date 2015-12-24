"use strict";
////////////////////////////////////////////////////////////////////////
// JSHint configuration                                               //
////////////////////////////////////////////////////////////////////////
/* global engine                                                      */
/* global print                                                       */
/* global midi                                                        */
/* global SCS3M:true                                                  */
/* jshint -W097                                                       */
/* jshint -W084                                                       */
/* jshint laxbreak: true                                              */
////////////////////////////////////////////////////////////////////////

// manually test messages
// amidi -p hw:1 -S F00001601501F7 # flat mode
// amidi -p hw:1 -S 900302 # 90: note on, 03: id of a touch button, 02: red LED

SCS3M = {
    // The device remembers the selected EQ/FX mode per deck
    // and switches to that mode on deck-switch. Set this to
    // false if you prefer the mode to stay the same on
    // deck-switch.
    eqModePerDeck: true,
};

SCS3M.init = function(id) {
    this.device = this.Device();
    this.agent = this.Agent(this.device);
    this.agent.start();
};

SCS3M.shutdown = function() {
    SCS3M.agent.stop();
};

SCS3M.receive = function(channel, control, value, status) {
    SCS3M.agent.receive(status, control, value);
};

/* Midi map of the SCS.3m device
 *
 * Thanks to Sean M. Pappalardo for the original SCS3 mappings.
 */
SCS3M.Device = function() {
    var NoteOn = 0x90;
    var NoteOff = 0x80;
    var CC = 0xB0;
    var CM = 0xBF; /* this is used for slider mode changes (absolute/relative, sending a control change on channel 16!?) */

    var black = 0x00;
    var blue = 0x01;
    var red = 0x02;
    var purple = blue | red;

    function Logo() {
        var id = 0x69;
        return {
            on: [NoteOn, id, 0x01],
            off: [NoteOn, id, 0x00]
        };
    }

    function Meter(id, lights) {
        function plain(value) {
            if (value <= 0.0) return 1;
            if (value >= 1.0) return lights;
            return 1 + Math.round(value * (lights - 1));
        }

        function clamped(value) {
            if (value <= 0.0) return 1;
            if (value >= 1.0) return lights;
            return Math.round(value * (lights - 2) + 1.5);
        }

        function zeroclamped(value) {
            if (value <= 0.0) return 0;
            if (value >= 1.0) return lights;
            return Math.round(value * (lights - 1) + 0.5);
        }
        return {
            needle: function(value) {
                return [CC, id, plain(value)];
            },
            centerbar: function(value) {
                return [CC, id, 0x14 + clamped(value)];
            },
            bar: function(value) {
                return [CC, id, 0x28 + zeroclamped(value)];
            },
            expand: function(value) {
                return [CC, id, 0x3C + zeroclamped(value)];
            },
        };
    }

    function Slider(id, lights, fields) {
        var touchfields = {};
        for (var fieldn in fields) {
            touchfields[fieldn] = {
                touch: [NoteOn, fields[fieldn]],
                release: [NoteOff, fields[fieldn]],
            };
        }
        return {
            meter: Meter(id, lights),
            slide: [CC, id],
            mode: {
                absolute: [[CM, id, 0x70], [CM, id, 0x7F]],
                relative: [[CM, id, 0x71], [CM, id, 0x7F]],
            },
            field: touchfields,
        };
    }

    function Light(id) {
        return {
            black: [NoteOn, id, black],
            blue: [NoteOn, id, blue],
            red: [NoteOn, id, red],
            purple: [NoteOn, id, purple],
        };
    }

    function Touch(id) {
        return {
            light: Light(id),
            touch: [NoteOn, id],
            release: [NoteOff, id]
        };
    }

    function Side(side) {
        function either(left, right) {
            return ('left' === side) ? left : right;
        }

        function Deck() {
            // The left deck button has a higher address than the right button,
            // for all the other controls the left one has a lower address.
            // I wonder why.
            var id = either(0x10, 0x0F);
            return {
                light: function(bits) {
                    return [NoteOn, id, (bits[0] ? 1 : 0) | (bits[1] ? 2 : 0)];
                },
                touch: [NoteOn, id],
                release: [NoteOff, id]
            };
        }

        function Pitch() {
            return Slider(either(0x00, 0x01), 7, {
                left:   either(0x51, 0x54),
                middle: either(0x52, 0x55),
                right:  either(0x53, 0x56),
            });
        }

        function Eq() {
            return {
                low: Slider(either(0x02, 0x03), 7),
                mid: Slider(either(0x04, 0x05), 7),
                high: Slider(either(0x06, 0x07), 7),
            };
        }

        function Modes() {
            return {
                fx: Touch(either(0x0A, 0x0B)),
                eq: Touch(either(0x0C, 0x0D))
            };
        }

        function Gain() {
            return Slider(either(0x08, 0x09), 7);
        }

        function Touches() {
            return [
                Touch(either(0x00, 0x01)),
                Touch(either(0x02, 0x03)),
                Touch(either(0x04, 0x05)),
                Touch(either(0x06, 0x07)),
            ];
        }

        function Phones() {
            return Touch(either(0x08, 0x09));
        }

        return {
            deck: Deck(),
            pitch: Pitch(),
            eq: Eq(),
            modes: Modes(),
            gain: Gain(),
            touches: Touches(),
            phones: Phones(),
            meter: Meter(either(0x0C, 0x0D), 7)
        };
    }

    return {
        factory: [0xF0, 0x00, 0x01, 0x60, 0x40, 0xF7],
        flat: [0xF0, 0x00, 0x01, 0x60, 0x15, 0x01, 0xF7],
        lightsoff: [CC, 0x7B, 0x00],
        logo: Logo(),
        left: Side('left'),
        right: Side('right'),
        master: Touch(0x0E),
        crossfader: Slider(0x0A, 11)
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

SCS3M.Agent = function(device) {
    // Cache last sent bytes to avoid sending duplicates.
    // The second byte of each message (controller id) is used as key to hold
    // the last sent message for each controller.
    var last = {};

    // Queue of messages to send delayed after modeset()
    var loading = false;
    var throttling = false;
    var pipe = [];

    // Handlers for received messages
    var receivers = {};

    // Connected engine controls
    var watched = {};

    function clear() {
        receivers = {};
        pipe = [];

        // I'd like to disconnect everything on clear, but that doesn't work when using closure callbacks, I guess I'd have to pass the callback function as string name
        // I'd have to invent function names for all handlers
        // Instead I'm not gonna bother and just let the callbacks do nothing
        for (var ctrl in watched) {
            if (watched.hasOwnProperty(ctrl)) {
                watched[ctrl] = [];
            }
        }
    }

    // This function receives Midi messages from the SCS.3m
    // See the XML mapping for all the messages caught
    function receive(type, control, value) {
        var address = (type << 8) + control;
        var handler = receivers[address];
        if (handler) {
            handler(value);
        }
    }

    // Register a handler to listen for messages
    // control: an array with at least two message bytes (type and control id)
    // handler: callback function that will be called each time a value is received
    function expect(control, handler) {
        var address = (control[0] << 8) + control[1];
        receivers[address] = handler;
    }

    function watchRegister(channel, control, handler) {
        // Indirection through a registry that keeps all watched controls
        var ctrl = channel + control;

        if (!watched[ctrl]) {
            watched[ctrl] = [];
            engine.connectControl(channel, control, function(value, group, control) {
                var handlers = watched[ctrl];
                for (var i in handlers) {
                    handlers[i]();
                }
            });
        }

        watched[ctrl].push(handler);
    }

    // Register a handler for changes in engine values
    // This is an abstraction over engine.getParameter()
    function watch(channel, control, handler) {
        watchRegister(channel, control, function() {
            handler(engine.getParameter(channel, control));
        });

        if (loading) {
            // ugly UGLY workaround
            // The device does not light meters again if they haven't changed from last value before resetting flat mode
            // so we send each control some bullshit values which causes awful flicker during startup
            // The trigger will then set things straight
            handler(100);
            handler(-100);
        }

        engine.trigger(channel, control);
    }

    // Register a handler for multiple engine values. It will be called
    // everytime one of the values changes.
    // controls: list of channel/control pairs to watch
    // handler: will receive list of control values as parameter in same order
    function watchmulti(controls, handler) {
        var values = [];
        var watchControl = function(controlpos, controlgroup) {
            var channel = controlgroup[0];
            var control = controlgroup[1];
            values[controlpos] = engine.getParameter(channel, control);
            watchRegister(channel, control, function() {
                values[controlpos] = engine.getParameter(channel, control);
                handler(values);
            });
        };

        for (var i in controls) {
            watchControl(i, controls[i]);
        }
        handler(values);
    }


    // Send MIDI message to device
    // Param message: list of three MIDI bytes
    // Param force: send value regardless of last recorded state
    // Param extra: do not record message as last state
    // Returns whether the massage was sent
    // False is returned if the mesage was sent before.
    function send(message, force, extra) {
        if (!message){
            print("SCS3 warning: send function received invalid message");
            return; // :-(
        }

        var address = (message[0] << 8) + message[1];

        if (!force && last[address] === message[2]) {
            return false; // Not repeating same message
        }

        midi.sendShortMsg(message[0], message[1], message[2]);

        // Record message as sent, unless it as was a mode setting termination message
        if (!extra) {
            last[address] = message[2];
        }
        return true;
    }

    // Wrapper function for send() that delays messages after modesetting
    function tell(message) {
        if (throttling) {
            pipe.push(message);
            return;
        }

        send(message);
    }

    // Send modeset messages to the device
    //
    // messages: list of one or two messages to send
    //
    // Either provide a pair of messages to set a slider to a different
    // mode, or send just one long message in the list. Transmission of
    // subsequent messages will be delayed to give the device some time to apply
    // the changes.
    function modeset(messages) {
        var sent = true;

        // Modeset messages are comprised of the actual modeset message and
        // a termination message that must be sent after.
        var message = messages[0];

        if (message) {
            if (message.length > 3) {
                midi.sendSysexMsg(message, message.length);
            } else {
                sent = send(message);
                if (sent && messages[1]) {
                    // Only send termination message when modeset message was sent
                    send(messages[1], true, true);
                }
            }
        }

        if (sent) {
            // after modesetting, we have to wait for the device to settle
            if (!throttling) {
                throttling = engine.beginTimer(20, flushModeset);
            }
        }
    }

    var flushModeset = function() {
        var message;

        // Now we can flush the rest of the messages.
        // On init, some controls are left unlit if the messages are sent
        // without delay. The causes are unclear. Sending only a few messages
        // per tick seems to work ok.
        var limit = 5; // Determined experimentally
        while (pipe.length) {
            message = pipe.shift();
            send(message);
            if (loading && limit-- < 1) return;
        }

        if (throttling) engine.stopTimer(throttling);
        throttling = false;
        loading = false;
    };

    // Map engine values in the range [0..1] to lights
    // translator maps from [0..1] to a midi message (three bytes)
    function patch(translator) {
        return function(value) {
            tell(translator(value));
        };
    }

    // Cut off at 0.01 because it drops off very slowly
    function vupatch(translator) {
        return function(value) {
            value = value * 1.01 - 0.01;
            tell(translator(value));
        };
    }

    // accelerate away from 0.5 so that small changes become visible faster
    function offcenter(translator) {
        return function(value) {
            // If you want to adjust it, fiddle with the exponent (second argument to pow())
            return translator(Math.pow(Math.abs(value - 0.5) * 2, 0.6) / (value < 0.5 ? -2 : 2) + 0.5);
        };
    }

    function binarylight(off, on) {
        return function(value) {
            tell(value ? on : off);
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

    function setconst(channel, control, value) {
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
    function budge(channel, control, factor) {
        if (factor === undefined) factor = 1;
        var mult = factor / 128;

        return function(offset) {
            engine.setValue(channel, control,
                engine.getValue(channel, control) + (offset - 64) * mult
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

        return {
            'change': function(state) {
                state = !!(state);
                var changed = engaged !== state;
                engaged = state;
                return changed;
            },
            'engage': function() {
                engaged = true;
            },
            'cancel': function() {
                engaged = false;
            },
            'toggle': function() {
                engaged = !engaged;
            },
            'engaged': function() {
                return engaged;
            },
            'choose': function(off, on) {
                return engaged ? on : off;
            }
        };
    }

    function HoldLimit(limit) {
        var start = false;

        var early = function() {
            return limit > new Date() - start;
        };

        return {
            'hold': function() {
                start = new Date();
            },

            'releaseTrigger': function(onEarly) {
                return function() {
                    if (!start) return;
                    if (early()) {
                        onEarly();
                    }
                    start = false;
                };
            },

            'early': function() {
                if (!start) return;
                return early();
            },

            'late': function() {
                if (!start) return;
                return early();
            },

            'held': function() {
                return !!start;
            },

            'choose': function(normalVal, heldVal) {
                return start ? heldVal : normalVal;
            }
        };
    }

    // HoldDelayedSwitches can be engaged, and they can be held.
    // A switch that is held for less than 200 ms will toggle.
    // After 200ms it will enter held-mode.
    function HoldDelayedSwitch() {
        var sw = Switch();

        var held = false;
        var heldBegin = false;

        sw.hold = function(onHeld) {
            return function() {
                heldBegin = true;
                var switchExpire = engine.beginTimer(200, function() {
                    engine.stopTimer(switchExpire);
                    if (heldBegin) {
                        heldBegin = false;
                        held = true;
                        if (onHeld) onHeld();
                    }
                });
            };
        };

        sw.release = function() {
            if (heldBegin) sw.toggle();
            held = false;
            heldBegin = false;
        };

        sw.held = function() {
            return held;
        };

        return sw;
    }

    function Multiswitch(preset, initialSubMode) {
        var engaged = preset;
        var subMode = initialSubMode;
        return {
            'engage': function(pos) {
                engaged = pos;
                if (engaged !== preset) {
                    subMode = pos;
                }
            },
            'engageSub': function() {
                engaged = subMode;
            },
            'cancel': function() {
                engaged = preset;
            },
            'engaged': function(pos) {
                return engaged === pos;
            },
            'choose': function(pos, off, on) {
                return (engaged === pos) ? on : off;
            }
        };
    }

    var master = Switch(); // Whether master key is held
    var deck = {
        left: HoldDelayedSwitch(), // off: channel1, on: channel3
        right: HoldDelayedSwitch() // off: channel2, on: channel4
    };

    var overlayA = Multiswitch('eq', 0);
    var overlayB = Multiswitch('eq', SCS3M.eqModePerDeck ? 1 : 0);
    var overlayC;
    var overlayD;
    if (SCS3M.eqModePerDeck) {
        overlayC = Multiswitch('eq', 2);
        overlayD = Multiswitch('eq', 3);
    } else {
        overlayC = overlayA;
        overlayD = overlayB;
    }

    var overlay = {
        left: [overlayA, overlayC],
        right: [overlayB, overlayD],
    };

    var eqheld = {
        left: Switch(),
        right: Switch()
    };

    var fxHeld = {
        left: HoldLimit(200),
        right: HoldLimit(200)
    };

    var touchheld = {
        left: Multiswitch('none'),
        right: Multiswitch('none')
    };

    function remap() {
        clear();
        patchage();
    }

    // Remap for chainig with handlers
    function repatch(handler) {
        return function(value) {
            var ret = handler(value);
            remap();
            return ret;
        };
    }

    function patchage() {

        function Side(side) {
            var part = device[side];
            var deckside = deck[side];

            // Switch deck/channel when button is touched
            expect(part.deck.touch, deckside.hold(remap));
            expect(part.deck.release, repatch(deckside.release));

            function either(left, right) {
                return (side === 'left') ? left : right;
            }

            var channelno = deck[side].choose(either(1, 2), either(3, 4));
            var channel = '[Channel' + channelno + ']';
            var effectchannel = '[QuickEffectRack1_' + channel + ']';
            var eqsideheld = eqheld[side];
            var touchsideheld = touchheld[side];
            var sideoverlay = overlay[side][deckside.choose(0, 1)];

            // Light the corresponding deck (channel 1: A, channel 2: B, channel 3: C, channel 4: D)
            // Make the lights blink on each beat
            function beatlight(translator, activepos, held) {
                return function(bits) {
                    bits = bits.slice(); // clone
                    if (held) {
                        // When the switch his held, light both LED
                        // turn them off when beat is active
                        bits[0] = !bits[0];
                        bits[1] = !bits[1];
                    } else {
                        // Invert the bit for the light that should be on
                        bits[activepos] = !bits[activepos];
                    }
                    return translator(bits);
                };
            }
            watchmulti([
                ['[Channel' + either(1, 2) + ']', 'beat_active'],
                ['[Channel' + either(3, 4) + ']', 'beat_active'],
            ], patch(beatlight(part.deck.light, deckside.choose(0, 1), deckside.held())));

            if (!master.engaged()) {
                if (sideoverlay.engaged('eq')) {
                    modeset(part.pitch.mode.relative);
                    expect(part.pitch.slide, eqsideheld.choose(
                        budge(effectchannel, 'super1', 0.5),
                        reset(effectchannel, 'super1')
                    ));
                    watch(effectchannel, 'super1', offcenter(patch(part.pitch.meter.centerbar)));
                }
            }

            if (sideoverlay.engaged('eq')) {
                var eff = "[EqualizerRack1_" + channel + "_Effect1]";
                var op = eqsideheld.choose(set, reset);
                expect(part.eq.low.slide, op(eff, 'parameter1'));
                expect(part.eq.mid.slide, op(eff, 'parameter2'));
                expect(part.eq.high.slide, op(eff, 'parameter3'));

                watch(eff, 'parameter1', patch(offcenter(part.eq.low.meter.centerbar)));
                watch(eff, 'parameter2', patch(offcenter(part.eq.mid.meter.centerbar)));
                watch(eff, 'parameter3', patch(offcenter(part.eq.high.meter.centerbar)));
            }

            expect(part.modes.eq.touch, repatch(function() {
                eqsideheld.engage();
                sideoverlay.cancel();
            }));
            expect(part.modes.eq.release, repatch(eqsideheld.cancel));
            tell(part.modes.eq.light[eqsideheld.choose(sideoverlay.choose('eq', 'blue', 'red'), 'purple')]);

            var fxHeldSide = fxHeld[side];

            var fxMap = function(tnr) {
                var softbutton = part.touches[tnr];
                var fxchannel = channel;
                if (master.engaged()) {
                    fxchannel = either('[Headphone]', '[Master]');
                }
                var effectunit = '[EffectRack1_EffectUnit' + (tnr + 1) + ']';
                var effectunit_enable = 'group_' + fxchannel + '_enable';
                var effectunit_effect = '[EffectRack1_EffectUnit' + (tnr + 1) + '_Effect1]';

                if (fxHeldSide.held() || master.engaged()) {
                    expect(softbutton.touch, toggle(effectunit, effectunit_enable));
                } else {
                    expect(softbutton.touch, repatch(function() {
                        sideoverlay.engage(tnr);
                        touchsideheld.engage(tnr);
                    }));
                }
                expect(softbutton.release, repatch(touchsideheld.cancel));

                if (sideoverlay.engaged(tnr)) {
                    watch(effectunit, effectunit_enable, binarylight(
                        softbutton.light.blue,
                        softbutton.light.purple)
                    );
                } else {
                    watch(effectunit, effectunit_enable, binarylight(
                        softbutton.light.black,
                        softbutton.light.red)
                    );
                }

                if (sideoverlay.engaged(tnr)) {
                    // Select effect by touching top slider when button is held
                    // Otherwise the top slider controls effect wet/dry
                    if (touchsideheld.engaged(tnr)) {
                        tell(part.pitch.meter.expand(0.3));
                        expect(
                            part.pitch.field.left.touch,
                            setconst(effectunit, 'chain_selector', 1)
                        );
                        expect(
                            part.pitch.field.right.touch,
                            setconst(effectunit, 'chain_selector', -1)
                        );
                    } else {
                        modeset(part.pitch.mode.absolute);
                        expect(part.pitch.slide, eqsideheld.choose(
                            set(effectunit, 'mix'),
                            reset(effectunit, 'mix')
                        ));
                        watch(effectunit, 'mix', patch(part.pitch.meter.bar));
                    }

                    expect(part.eq.high.slide, fxHeldSide.choose(
                        set(effectunit_effect, 'parameter3'),
                        reset(effectunit_effect, 'parameter3')
                    ));
                    expect(part.eq.mid.slide, fxHeldSide.choose(
                        set(effectunit_effect, 'parameter2'),
                        reset(effectunit_effect, 'parameter2')
                    ));
                    expect(part.eq.low.slide, fxHeldSide.choose(
                        set(effectunit_effect, 'parameter1'),
                        reset(effectunit_effect, 'parameter1')
                    ));
                    watch(effectunit_effect, 'parameter3', patch(part.eq.high.meter.needle));
                    watch(effectunit_effect, 'parameter2', patch(part.eq.mid.meter.needle));
                    watch(effectunit_effect, 'parameter1', patch(part.eq.low.meter.needle));
                }
            };

            for (var tnr = 0; tnr < 4; tnr++) {
                fxMap(tnr);
            }

            expect(part.modes.fx.touch,
                repatch(fxHeldSide.hold)
            );
            expect(part.modes.fx.release,
                repatch(fxHeldSide.releaseTrigger(sideoverlay.engageSub))
            );
            tell(part.modes.fx.light[fxHeldSide.choose(
                sideoverlay.choose('eq', 'red', 'blue'),
                'purple'
            )]);


            if (!master.engaged()) {
                if (deckside.held()) {
                    modeset(part.gain.mode.relative);
                    expect(part.gain.slide, eqsideheld.choose(
                        budge(channel, 'pregain'),
                        reset(channel, 'pregain')
                    ));
                    watch(channel, 'pregain', patch(offcenter(part.gain.meter.needle)));
                } else {
                    modeset(part.gain.mode.absolute);
                    expect(part.gain.slide, set(channel, 'volume'));
                    watch(channel, 'volume', patch(part.gain.meter.bar));
                }
            }

            watch(channel, 'pfl', binarylight(part.phones.light.blue, part.phones.light.red));
            expect(part.phones.touch, toggle(channel, 'pfl'));

            if (deckside.held()) {
                expect(device.crossfader.slide, set(channel, "playposition"));
                watch(channel, "playposition", patch(device.crossfader.meter.needle));
            }

            if (!master.engaged()) {
                watch(channel, 'VuMeter', vupatch(part.meter.bar));
            }
        }

        // Light the logo and let it go out to signal an overload
        watch("[Master]", 'audio_latency_overload', binarylight(
            device.logo.on,
            device.logo.off
        ));

        Side('left');
        Side('right');

        tell(device.master.light[master.choose('blue', 'purple')]);
        expect(device.master.touch, repatch(master.engage));
        expect(device.master.release, repatch(master.cancel));
        if (master.engaged()) {
            modeset(device.left.pitch.mode.absolute);
            watch("[Master]", "headMix", patch(device.left.pitch.meter.centerbar));
            expect(device.left.pitch.slide,
                eqheld.left.engaged() ? reset('[Master]', 'headMix') : set('[Master]', 'headMix')
            );

            modeset(device.right.pitch.mode.absolute);
            watch("[Master]", "balance", patch(device.right.pitch.meter.centerbar));
            expect(device.right.pitch.slide,
                eqheld.right.engaged() ? reset('[Master]', 'balance') : set('[Master]', 'balance')
            );

            modeset(device.left.gain.mode.relative);
            watch("[Master]", "headVolume", patch(device.left.gain.meter.centerbar));
            expect(device.left.gain.slide, budge('[Master]', 'headVolume'));

            modeset(device.right.gain.mode.relative);
            watch("[Master]", "volume", patch(device.right.gain.meter.centerbar));
            expect(device.right.gain.slide, budge('[Master]', 'volume'));

            watch("[Master]", "VuMeterL", vupatch(device.left.meter.bar));
            watch("[Master]", "VuMeterR", vupatch(device.right.meter.bar));
        }

        if (deck.left.held() || deck.right.held()) {
            // Needledrop handled in Side()
        } else {
            expect(device.crossfader.slide, set("[Master]", "crossfader"));
            watch("[Master]", "crossfader", patch(device.crossfader.meter.centerbar));
        }

        // Communicate currently selected channel of each deck so SCS3d can read it
        // THIS USES A CONTROL FOR ULTERIOR PURPOSES AND IS VERY NAUGHTY INDEED
        engine.setValue('[PreviewDeck1]', 'quantize',
            0x4 // Setting bit three communicates that we're sending deck state
            | deck.left.engaged() // left side is in bit one
            | deck.right.engaged() << 1 // right side bit two
        );
        watch('[PreviewDeck1]', 'quantize', function(deckState) {
            var changed = deck.left.change(deckState & 1) || deck.right.change(deckState & 2);
            if (changed) repatch(function() {})();
        });
    }

    return {
        start: function() {
            loading = true;
            modeset([device.flat]);
            patchage();
        },
        receive: receive,
        stop: function() {
            clear();
            tell(device.lightsoff);
            send(device.logo.on, true);
        }
    };
};
