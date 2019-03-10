////////////////////////////////////////////////////////////////////////
// JSHint configuration                                               //
////////////////////////////////////////////////////////////////////////
/*jshint bitwise: false, futurehostile: true, undef: true, unused: false*/
/*jshint trailingcomma: false */
/* global engine                                                      */
/* global script                                                      */
/* global print                                                       */
/* global components                                                  */
/* global _                                                           */
/* global bpm                                                         */
/* global midi                                                        */
////////////////////////////////////////////////////////////////////////

/*
TODO:
Fix pad_container trigger() not called (waiting for HotcueColorButtons)
Handle knob_cfContour (UNTESTED)
handle pitch slider softTakeover LEDs (DONE)
Include velocity samplerbuttons (DONE)
include colored hotcuebuttons [waiting for #2030]
fix load button loading the same track. (DONE)


Abstract:
Controller:
  1x Mixer
  2x FXUnit (Components)
  (2x FXUnit (Components)) (configurable)
  4x Deck
  btn_touch_mode (enable capazitive top of FX- &&/|| EQ-knobs) [0x9F,0x59] // read only
  btn_filter_knob_behavior (doesnt really make sense IMO repurposeable) [0x9F,0x5A] // read only
  cont_nav_bar
Mixer:
  4x Channel
  knob_cfContour (find  way to combine xFaderCalibration, xFaderMode && xFaderCurve into single knob)
  slider_crossfader (regular crossfader)
  switch_split_cue (configures mixxx to split the cue signal into mono mixdown as well)
  mngr_lib_enc_focus (may be handled by the controller automatically)
  mngr_master_vu_meter (binds to the vumeter)
  mngr_ext_input (controls whenever the 3&4 Channel are accepting input from line in)
Channel:
  btn_load_track_into_deck [0x9F, 0x02](s: eject [same control but it sends the message twice 4sumReason]
  btn_fx_activate_right (s: _activate_4)
  knob_pre_gain [0x]
  knob_high_eq
  cap_high_kill
  knob_mid_eq
  cap_mid_kill
  knob_low_eq
  cap_low_kill
  knob_channel_filter
  (cap_channel_filter)
  btn_pfl
  switch_cf_orientation
  slider_vol
Deck:
  btn_slip (s: quantize)
  btn_bleep (play reverse but continue as if it hadn't (slip)) (configurable behavior) (s: keylock)
  slider_pitch (plain old pitch/bpm slider)
  btn_pitch_bend_plus (s: increase pitch range)
  btn_pitch_bend_plus (s: decrease pitch range)
  btn_scratch (typical scratch enable button) (s: toggles between elapsed/remaining on display)
  btn_shift (acts on current deck and all singleton containers)
  cont_pad_mode_selector
  cont_active_pad_page
  btn_sync (s: deactivate sync) (configurable if acts like serato or components.js sync)
  btn_play
  btn_cue
  enc_jog_wheel_move
  cap_jog_wheel_touch
  cap_strip_search
  btn_toggle_deck
  cont_display
  regular buttons have 3 states: 0x00: off, 0x01: dimm, 0x2: bright,
FXUnit:
  Standard component Effect Unit
cont_nav_bar:
  enc_lib_nav (s: accelerate)
  btn_view (max library) (s: sort lib by bpm (prob imposible))
  btn_back (move back; find equivalent in mixxx) (s: sort by track_name)
  btn_area (toggle through panels; mby mixxx forward button?) (s: sort by key)
  btn_lprep (load into 'prepare area'; mby preview deck is fine?) (s: sort by artist_name)
cont_display:
  mngr_bpm
  mngr_time
  mngr_pitch_percentage
  mngr_pitch_range
  mngr_keylock
  mngr_deck
  mngr_pitch_adjust (also takes care of LEDs besides slider_pitch)
  mngr_platter_pos
cont_pad_mode_selector:
  btn_hotcue_mode (cont_active_pad_page = (when previously unselected ) ? cont_pm_hotcue_regular : cont_pm_hotcue_auto;)
  btn_loop_mode (cont_active_pad_page = (when previously unselected ) ? cont_pm_loop_auto : cont_pm_roll;))
cont_pm_hotcue_regular:
  bind array components.HotcueColorButton
  8x regular hotcue button
  param_adjust (no function)
cont_pm_hotcue_auto:
  same as cont_pm_hotcue_regular but create a loop right after pressing
  8x regular hotcue buttons but loop afterwards
  param_adjust (change auto_loop size)
cont_pm_loop_auto:
  create loops similar to how the launchpad behaves.
  param_adjust (change auto_loop size)
cont_pm_loop_roll:
  same as loop_auto but beatlooproll
cont_pm_loop_ctrl:
  param_adjust = shift_loop left/right
  pads[0] = loop_in_goto,
  pads[1] = loop_in_goto,
  pads[2] = halve loop,
  pads[3] = double loop,
  pads[4] = set loop in,
  pads[5] = set loop out,
  pads[6] = loop on/off,
  pads[7] = reloop,
cont_pm_sampler_normal:
  8x SamplerButton
cont_pm_sampler_velocity:
  like sampler button but override inValueScale
cont_pm_slicer:
  only accesible by pressing shift to avoid accidental manipulation of beatgrid.
  pads[0,1] = beats_adjust_faster,slower
  pads[2,3] = beats_adjust_earlier,later
  pads[4] = beats_translate_curpos
  pads[5] = bpm_tap (CO)
Ideas:
  combine touch fx button with [Master],"show_killswitches"


Reverse Engineering notes:
  Platter: 1000 steps/revolution
  Dual-precision elements: search strips, pitch_slider
  CC: 0x06 setup display controls
  CC: 0x0E set fine pitch of display
  CC: Ox3F set track duration leds
  CC: 0x06 set platter pos led
  CC: 0x75 setup default LEDs
  CC: 0x7F (val: 0 turn all of, val: !0 turn all elements on) (Display)
  on: 0x09 pitch up led;
  on: 0x0A pitch down led;
  on: 0x0D KeyLock display;
  on: 0x0E pitch range value;
  on: 0x51 pitch led 0;
  CC: 0x1F channel VuMeter: 1: off, 1: 1, 21: 2, 41: 3, 61: 4, 81: 5 (clipping)
  Master VUMeters: is set by controller
  cont_pad_mode_selector buttons velocities: 0: off; 1: dimm lit; 2: blinking; 3: blink 3x; 4: lit bright;
  pad_colors: 0:off, very dimm pink: 1, dimm blue: 2, full blue: 3, very dimm green (dirt): 4, very dimm turquoise: 5
    dimm blue2: 6, full blue 2: 7, dimm green: 8, , dimm turquoise: 10, full slightly light blue: 11, full green: 12,
    full mint: 14, full turquoise: 15, very dimm red: 16, very dimm purple/wine red: 17, dimm purple: 18, full blue3: 19
    dirt/dimm yellow: 20, dimm white: 21, dimm light purple: 22, full blue4: 23, dimm green 2, , dimm turquoise2: 26,
    full light blue2: 27, full green (light): 28, , full green (turquoise): 30, full turquoise: 31, dimm red: 32,
    dimm purple: 33, dimm purple2: 34, dimm blue+purple: 35, dimm brown/orange: 36, dimm meat color: 37, dimm purple: 38,
    full blue: 39, dimm yellow: 40, dimm beige: 41, dimm white: 42,  full light blue+purple: 43, full green: 44, ,
    full mint: 46, full turquoise: 47, full red: 48, fill lighter red: 49, full light purple: 50, full pink: 51,
    full red: 52, full lighter red: 53, light ligther pink: 54, full pink: 55, full orange: 56, full lighter orange: 57,
    full lighter meat color: 58, full light pink: 59, full yellow, , full lighter yellow,
    >=63: full white,
  Startup sequence:
    SYX: 0x00,0x20,0x04,0x7F,0x03,0x01,0x05
  BPM Display:
    Absolute BPM Syx: 0x00,0x20,0x7f,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    Least-significant-Nibble of Last 5 bytes are responsible for the display value
    Pitch_percentage_change syx: 0x00,0x20,0x7f,0x01,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    Pitch_change_ratio: 0.1bpm=10 offset 5 => 100*bpm (so it hits right in the middle)
    Pitch_percentage_change 0%: 0x00,0x20,0x7f,,0xf,0xf,0xf,0xf,0xd,0x5
    set pitch percentage by
    get 2's complement of d: (~d + 1 >>> 0)

  pitch range:
    midi: ["note_on", note=0x0E, velocity=|bpm|] (abs(bpm) = 100bpm=100velocity)
  Time Display:
    Set Current Time Syx: 0x00,0x20,0x7f,0x01,0x04,0x08,0x00,0x00,0x00,0x00,0x00,0x00
    Set Track duration syx: 0x00,0x20,0x7f,0x01,0x03,0x08,0x00,0x00,0x00,0x00,0x00,0x00
    Set Track duration syx: 0x00,0x20,0x7f,0x01,0x03,0x08,0x00,0x04,0x0a,0x07,0x03,0x07
    syx[3] = channel (1-based)
    switch time display: ["note_on",control=0x46,velocity] only 0x00 || 0x7F (0x00 display elapsed)
    Least-significant-Nibble of Last 5 bytes are responsible for the display value
    6bit value increase in sysex = 1ms timer increase on display
*/

var NS6II = {};

// UserSettings
NS6II.rate_ranges = [0.04, 0.08, 0.10, 0.16, 0.24, 0.50, 0.90, 1.00,];

NS6II.navigation_encoder_acceleration = 5;

NS6II.default_loop_root_size = -3;

NS6II.use_button_backlight = true;

NS6II.hide_killswitches_when_unused = true;

// Globals

NS6II.scratch_settings = {
    alpha: 1/8,
    beta: 0.125/32,
};


NS6II.colors = {
    off: 0,
    red: 48,
    red_dimm: 32,
    yellow: 60,
    yellow_dimm: 40,
    green: 12,
    green_dimm: 8,
    celeste: 31, // sky_blue
    celeste_dimm: 10,
    blue: 3,
    blue_dimm: 2,
    purple: 59,
    purple_dimm: 34,
    pink: 58,
    pink_dimm: 37,
    white: 63,
    white_dimm: 42,
    grey: 21, // even dimmer white
    // misc colors
    mint: 14,
    orange: 56,
    orange_dimm: 36, //could be brown as well
    red_very_dimm: 16,
}
// using dict to make ids more robust
NS6II.hotcue_colors = {
    0: NS6II.colors.orange, // no_color fallback
    1: NS6II.colors.red,
    4: NS6II.colors.yellow,
    2: NS6II.colors.green,
    5: NS6II.colors.celeste,
    3: NS6II.colors.blue,
    6: NS6II.colors.purple,
    7: NS6II.colors.pink,
    8: NS6II.colors.white,
};

NS6II.serato_syx_prefix = [0x00, 0x20, 0x7f];

components.Button.prototype.off = NS6II.use_button_backlight ? 0x01 : 0x00;

NS6II.Pad = function (options) {
    components.Button.call(this, options);
};
NS6II.Pad.prototype = new components.Button({
    // grey could be an alternative as well as a backlight color.
    off: NS6II.use_button_backlight ? NS6II.hotcue_colors.red_very_dimm : NS6II.hotcue_colors.off,
    sendShifted: true,
    shiftControl: true,
    shiftOffset: 8,
});

components.HotcueButton.prototype.off = NS6II.hotcue_colors.off; // overwrite components.Button.off
components.HotcueButton.prototype.sendShifted = true;
components.HotcueButton.prototype.shiftControl = true;
components.HotcueButton.prototype.shiftOffset = 8;

components.SamplerButton.prototype.sendShifted = true;
components.SamplerButton.prototype.shiftControl = true;
components.SamplerButton.prototype.shiftOffset = 8;


NS6II.Deck = function(channel_offset) {
    var theDeck = this;
    var deckNumber = channel_offset + 1;
    this.group = '[Channel' + deckNumber + ']';
    this.btn_slip = new components.Button({
        midi: [0x90+channel_offset,0x1F],
            // shift: [0x90+channel_offset,0x04],
        type: components.Button.prototype.types.toggle,
        unshift: function() {
            this.inKey = "slip_enabled";
            this.outKey = this.inKey;
        },
        shift: function() {
            // use repeat instead of quantize since that
            // is already handled by the SyncButton
            this.inKey = "repeat";
            this.outKey = this.inKey;
        },
    });
    this.btn_bleep = new components.Button({
        // also known as "censor"
        midi: [0x90 + channel_offset, 0x10],
        // shift: [0x90+channel_offset,0x0D]
        unshift: function() {
            this.inKey = "reverseroll";
            this.outKey = this.inKey;
            this.type = components.Button.prototype.push;
        },
        shift: function() {
            this.inKey = "keylock";
            this.outKey = this.inKey;
            this.type = components.Button.prototype.toggle;
        },
    });
    // features 14-bit precision
    this.slider_pitch = new components.Pot({
        midi: [0xB0 + channel_offset, 0x9],
        // LSB: [0x90+channel_offset,0x29]
        group: theDeck.group,
        inKey: "rate",
        // using inSetParameter to hook into the the high-res value
        inSetParameter: function(value) {
            engine.setParameter(this.group, this.inKey, value);
            switch (channel_offset) {
                case 0:
                case 2:
                    // round values by decreasing the resolution.
                    // still good enough for the LED indicator.
                    NS6II.slider_pitch_physical_left_val = value*50 | 0;
                    break;
                case 1:
                case 3:
                    NS6II.slider_pitch_physical_right_val = value*50 | 0;
                    break;
            }
        },
    });
    this.btn_pitch_bend_plus = new components.Button({
        // Doesnt have LED feedback
        midi: [0x90 + channel_offset, 0x0B],
        // shift: [0x90+channel_offset,0x2B]
        unshift: function() {
            this.inKey = "rate_temp_up";
            this.input = components.Button.prototype.input;
        },
        shift: function() {
            this.inKey = "rateRange";
            this.input = function() {
                NS6II.current_rate_range_index = NS6II.current_rate_range_index + 1 % NS6II.current_rate_range_index.length;
                this.setParameter(NS6II.current_rate_range_index);
            };
        },
    });
    this.btn_pitch_bend_minus = new components.Button({
            // Doesnt have LED feedback
            midi: [0x90 + channel_offset, 0x0C],
            // shift: [0x90+channel_offset,0x2C]
            unshift: function() {
                this.inKey = "rate_temp_down";
                this.input = components.Button.prototype.input;
            },
            shift: function() {
                this.inKey = "rateRange";
                this.input = function() {
                    NS6II.current_rate_range_index = ((NS6II.current_rate_range_index + NS6II.current_rate_range_index.length) + 1 ) % NS6II.current_rate_range_index.length;
                this.setParameter(NS6II.current_rate_range_index);
            };
        },
    });
    this.btn_shift = new components.Button({
        midi: [0x90 + channel_offset, 0x20],
        input: function(channelmidi, control, value, status, group) {
            if (this.isPress(channelmidi, control, value, status)) {
                NS6II.Mixer_instance.shift();
                NS6II.EffectUnits[channel_offset % 2 + 1].shift();
                theDeck.shift();
            } else {
                NS6II.Mixer_instance.unshift();
                NS6II.EffectUnits[channel_offset % 2 + 1].unshift();
                theDeck.unshift();
            }
            // no LED feedback
            // this.output(this.state);
        },
    });
    this.btn_sync = new components.SyncButton({
        midi: [0x90 + channel_offset, 0x02],
        // shift: [0x90+channel_offset,0x03]
    });

    this.btn_play = new components.PlayButton({
        midi: [0x90 + channel_offset, 0x00],
        // shift: [0x90+channel_offset,0x04]
    });
    this.btn_cue = new components.CueButton({
        midi: [0x90 + channel_offset, 0x01],
        // shift: [0x90+channel_offset,0x05]
    });

    this.enc_jog_wheel_move = new components.Pot({
        midi: [0xB0 + channel_offset, 0x06],
        inKey: "jog",
        group: theDeck.group,
        input: function(channelmidi, control, value, status, group) {
            if (engine.isScratching(deckNumber)) {
                engine.scratchTick(deckNumber, this.inValueScale(value));
            } else {
                this.inSetValue(this.inValueScale(value));
            }
        },
        inValueScale: function (value) {
            // centers values around 0
            return (value < 0x40 ? value : value - 0x80);
        }
    });
    this.cap_jog_wheel_touch = new components.Button({
        midi: [0x90 + channel_offset, 0x06],
        scratchEnabled: true,
        input: function(channelmidi, control, value, status, group) {
            if (this.isPress(channelmidi, control, value, status) && this.scratchEnabled) {
                engine.scratchEnable(deckNumber,
                    1140, // measurement (1000) wasn't producing accurate results
                    theDeck.cont_display.vinylcontrol_speed_type*60,
                    NS6II.scratch_settings.alpha,
                    NS6II.scratch_settings.beta);
            } else {
                engine.scratchDisable(deckNumber);
            }
        },
    });
    this.cap_strip_search = new components.Pot({
        midi: [0xB0 + channel_offset, 0x4D], // no feedback
        // input MSB: [0xB0+deck,0x2F] LSB
        group: theDeck.group,
        inKey: "playposition",
        shift: function() {
            this.inSetParameter = components.Pot.prototype.inSetParameter;
        },
        unshift: function() {
            this.inSetParameter = function(value) {
                // only allow searching when deck is not playing.
                if (!engine.getParameter(this.group, "play")) {
                    engine.setParameter(this.group, this.inKey, value);
                }
            };
        },
    });
    this.btn_scratch = new components.Button({
        midi: [0x90 + channel_offset, 0x07],
        // shift: [0x90+channel_offset,0x46]
        timer_mode: false,
        unshift: function() {
            this.input = function (channelmidi, control, value, status, group) {
                if (this.isPress(channelmidi, control, value, status)) {
                    theDeck.cap_jog_wheel_touch.scratchEnabled = !theDeck.cap_jog_wheel_touch.scratchEnabled;
                    this.output(theDeck.cap_jog_wheel_touch.scratchEnabled);
                }
            };
            this.output(theDeck.cap_jog_wheel_touch.scratchEnabled);
        },
        shift: function() {
            this.input = function (channelmidi, control, value, status, group) {
                if (this.isPress(channelmidi, control, value, status)) {
                    // toggle between time_elapsed/_remaining display mode
                    this.timer_mode = !this.timer_mode;
                    midi.sendShortMsg(0x90 + channel_offset,0x46,this.timer_mode ? 0x7F : 0x00);
                }
            };
        },
    });

    this.cont_display = new NS6II.Display(channel_offset);

    this.cont_pad_unit = new NS6II.cont_pad_mode_selector(channel_offset+4,this.group);

    this.reconnectComponents(function (c) {
        if (c.group === undefined) {
            c.group = theDeck.group;
        }
    });
};

NS6II.Deck.prototype = new components.Deck();

// JS implementation of engine/enginexfader.cpp:getPowerCalibration  (8005e8cc81f7da91310bfc9088802bf5228a2d43)
NS6II.getPowerCalibration = function (transform) {
    return Math.pow(0.5,1.0/transform);
}

// JS implementation of util/rescaler.h:linearToOneByX (a939d976b12b4261f8ba14f7ba5e1f2ce9664342)
NS6II.linearToOneByX = function (input, inMin, inMax, outMax) {
    var outRange = outMax - 1;
    var inRange = inMax - inMin;
    return outMax / (((inMax - input) / inRange * outRange) + 1);
}

NS6II.Mixer = function() {
    this.Channels = [];
    for (var i = 0; i <= 3; i++) {
        this.Channels[i+1] = new NS6II.Channel(i);
    }
    this.slider_crossfader = new components.Pot({
        midi: [0xBF, 0x08],
        group: "[Master]",
        inKey: "crossfader",
    });
    this.switch_split_cue = new components.Button({
        midi: [0x9F, 0x1C],
        group: "[Master]",
        inKey: "headSplit",
    });
    this.knob_cfContour = new components.Pot({
        midi: [0xBF,0x09],
        input: function(channelmidi, control, value, status, group) {
            // mimic preferences/dialog/dlgprefcrossfader.cpp:slotUpdateXFader
            var transform = NS6II.linearToOneByX(value,0,0x7F,999.6);
            engine.setValue("[Mixer Profile]","xFaderCurve",transform)
            var calibration = NS6II.getPowerCalibration(transform);
            engine.setValue("[Mixer Profile]","xFaderCalibration",calibration);
        },
    });
    print("print created knob_cfContour");
    this.knob_head_gain = new components.Pot({
        midi: [0xBF,0x0D],
        group: "[Master]",
        inKey: "headGain",
    });
    this.switch_ext_input_left = new components.Button({
        midi: [0x9F, 0x57],
        group: "[Channel3]",
        max: 2,
        inKey: "mute"
    });
    this.switch_ext_input_right = new components.Button({
        midi: [0x9F, 0x60],
        group: "[Channel4]",
        max: 2,
        inKey: "mute"
    });
    this.navBar = new NS6II.navBar();
};

NS6II.Mixer.prototype = new components.ComponentContainer();

NS6II.number_to_syx_payload = function(number,signed) {
    out = Array(6);
    // build 2's complement in case number is negative
    if (number < 0) {
        number = ( (~Math.abs(number|0) + 1) >>> 0);
    }
    // split nibbles of number into array
    for (var i = out.length; i; i--) {
        out[i-1] = number & 0xF;
        number = number >> 4;
    }
    // set signed bit in sysex payload
    if (signed) {
        out[0] = (number < 0) ? 0x07 : 0x08;
    }
    return out;
};
NS6II.send_syx_message = function(channel, location, payload) {
    var msg = [0xF0].concat(NS6II.serato_syx_prefix,channel, location, payload,0xF7);
    midi.sendSysexMsg(msg,msg.length);
};
// Display might be unique per physical Deck which would mean that it would have to interface with the Deck
NS6II.Display = function(channel_offset) {
    var channel = (channel_offset + 1);
    var deck = "[Channel" + channel + "]";
    var theDisplay = this;
    // stored as rts (per seconds) instead of rpm because it is easier to deal with it later that way.
    this.vinylcontrol_speed_type = 0;
    this.mngr_vinylcontrol_speed_time = engine.makeConnection(deck, "vinylcontrol_speed_type", function(value) {
        theDisplay.vinylcontrol_speed_type = value/60;
    });
    this.mngr_vinylcontrol_speed_time.trigger();
    this.data_track_info = {
        duration: 0,
        loaded: false,
    };
    this.conn_duration = engine.makeConnection(deck, "duration", function(value) {

        theDisplay.data_track_info.duration = value;
        // update track duration of for on Display
        // controller requires one more nibble of precision for this control
        var payload = NS6II.number_to_syx_payload(
            value*62.5, // arbitrary controller specific scaling factor
            true // signed int
        );
        payload.splice(1,0,0x0);
        NS6II.send_syx_message(
            channel,
            0x3,
            payload
        );
    });
    this.conn_duration.trigger();

    this.conn_pitch_range = engine.makeConnection(deck, "rateRange", function(value) {
        theDisplay.data_rate_range = value;
        midi.sendShortMsg(0x90 + channel_offset, 0x0E, (value * 100)|0);
    });
    this.conn_pitch_range.trigger();
    // binds to playpos and sets everything related to it (absolute position, platter pos, time display)
    this.mngr_playpos = function(playpos) {
        var elapsed_time = theDisplay.data_track_info.duration * playpos;
        var platter_strip_pos = (elapsed_time) * theDisplay.vinylcontrol_speed_type;
        var payload = NS6II.number_to_syx_payload(
            elapsed_time*62.5, // arbitrary controller specific scaling factor
            true // signed int
        );
        // controller requires one more nibble of precision for this control
        payload.splice(1,0,0x0);
        NS6II.send_syx_message(
            channel,
            0x04,
            payload
        );
        // update absolute playpos element of display (clamped to not go under 0)
        midi.sendShortMsg(
            0xB0 + channel_offset,
            0x3F,
            // check if track is loaded because playpos value is 0.5 when there isn't a track loaded.
            theDisplay.data_track_info.loaded ?
                Math.max((playpos * 0x7F) | 0, 0x00) :
                0x00
        );
        // check if value is positive so we can use a simplified formula.
        var platter_strip_pos_rel = (platter_strip_pos >= 0 ?
            platter_strip_pos%1 :
            (platter_strip_pos%1 + 1)%1)*0x7F
        // update elapsed time on Display
        midi.sendShortMsg(0xB0 + channel_offset, 0x06, platter_strip_pos_rel);
    };
    this.conn_playpos = engine.makeConnection(deck, "playposition", this.mngr_playpos);
    this.conn_playpos.trigger();

    this.deck_loaded = engine.makeConnection(deck, "track_loaded", function (value) {
        theDisplay.data_track_info.loaded = value;
    });
    this.deck_loaded.trigger();
    // manages everything related to the bpm feedback
    this.conn_bpm = engine.makeConnection(deck, "bpm", function(value) {
            // send absolute bpm
            NS6II.send_syx_message(channel,
                0x01,
                NS6II.number_to_syx_payload(value * 1000)
            );
    });
    // manages everything related to the rate/pitch feedback
    this.conn_rate = engine.makeConnection(deck, "rate", function(value) {
        // set pitch = 0% led
        midi.sendShortMsg(0x90 + channel_offset, 0x51, !value ? 0x7F : 0x00);
        // send pitch change.
        NS6II.send_syx_message(channel,
            0x02, // type
            NS6II.number_to_syx_payload(
                value * theDisplay.data_rate_range * 10000,
                true // signed int
            )
        );
    });
    // gets called whenever the user switches the deck on the controller
    this.mngr_deck_watcher = function () {
        engine.softTakeoverIgnoreNextValue(deck,"rate");
        this.conn_rate_diff.trigger();
    }
    this.conn_rate_diff = engine.makeConnection(deck, "rate", function(value) {
        // scale value from [-1;1] value to [0,50] parameter format
        value = ((value + 1)*25) | 0;
        switch (channel_offset) {
            case 0:
            case 2:
                midi.sendShortMsg(0x90 + channel_offset, 0x09, NS6II.slider_pitch_physical_left_val > value ? 0x7F : 0x00);
                midi.sendShortMsg(0x90 + channel_offset, 0x0A, NS6II.slider_pitch_physical_left_val < value ? 0x7F : 0x00);
                break;
            case 1:
            case 3:
                midi.sendShortMsg(0x90 + channel_offset, 0x09, NS6II.slider_pitch_physical_right_val > value ? 0x7F : 0x00);
                midi.sendShortMsg(0x90 + channel_offset, 0x0A, NS6II.slider_pitch_physical_right_val < value ? 0x7F : 0x00);
                break;
        }
    });

    this.conn_keylock = engine.makeConnection(deck, "keylock", function(value) {
        midi.sendShortMsg(0x90 + channel_offset, 0x0D, value);
    });
};

NS6II.Display.prototype = new components.ComponentContainer();

NS6II.cont_pm_hotcue_regular = function(channel_offset) {
    this.pads = [];
    for (var i = 1; i <= 8; i++) {
        this.pads[i] = new components.HotcueButton({
            midi: [0x90 + channel_offset, 0x13 + i],
            // shift: [0x94+channel_offset,0x1b+i],
            number: i,
            colors: NS6II.hotcue_colors,
            off: NS6II.colors.off,
        });
    }
    this.param_left = new components.Button({
        midi: [0x90 + channel_offset, 0x28],
        input: function(channelmidi, control, value, status, group) {
            // do nothing
        },
    });
    this.param_right = new components.Button({
        midi: [0x90 + channel_offset, 0x29],
        input: function(channelmidi, control, value, status, group) {
            // do nothing
        },
    });
};
NS6II.cont_pm_hotcue_regular.prototype = new components.ComponentContainer();

// no functionality yet since there are no loop cues in mixxx
NS6II.cont_pm_hotcue_auto = function(channel_offset) {
    this.pads = [];
    for (var i = 1; i <= 8; i++) {
        this.pads[i] = new components.HotcueButton({
            midi: [0x90 + channel_offset, 0x13 + i],
            number: i,
            colors: NS6II.hotcue_colors,
        });
    }
    this.param_left = new components.Button({
        midi: [0x90 + channel_offset, 0x28],
        input: function(channelmidi, control, value, status, group) {
            // do nothing
        },
    });
    this.param_right = new components.Button({
        midi: [0x90 + channel_offset, 0x29],
        input: function(channelmidi, control, value, status, group) {
            // do nothing
        },
    });
};
NS6II.cont_pm_hotcue_auto.prototype = new components.ComponentContainer();


NS6II.cont_pm_loop_auto = function(channel_offset) {
    var theContainer = this;
    this.pads = [];
    this.current_root_loop_size = -3; // 2**-3 = 1/8
    for (var i = 1; i <= 8; i++) {
        this.pads[i] = new NS6II.Pad({
            midi: [0x90 + channel_offset, 0x13 + i],
            on: NS6II.colors.red,
            off: NS6II.colors.red_dimm,
            // key is set by change_loop_size()
        });
    }
    this.param_left = new components.Button({
        midi: [0x90 + channel_offset, 0x28],
        input: function(channelmidi, control, value, status, group) {
            if (this.isPress(channelmidi, control, value, status)) {
                theContainer.change_loop_size(theContainer.current_root_loop_size - 1);
            }
        },
    });
    this.param_right = new components.Button({
        midi: [0x90 + channel_offset, 0x29],
        input: function(channelmidi, control, value, status, group) {
            if (this.isPress(channelmidi, control, value, status)) {
                theContainer.change_loop_size(theContainer.current_root_loop_size + 1);
            }
        },
    });
    this.change_loop_size = function(loop_size) {
        // clamp loop_size to [-5;7]
        theContainer.current_root_loop_size = Math.min(Math.max(-5, loop_size), 7);
        var i = 0;
        var loop_size = 0;
        _.forEach(theContainer.pads,function (c) {
            if (c instanceof components.Component) {
                c.disconnect();
                loop_size = Math.pow(2,theContainer.current_root_loop_size + (i++));
                c.inKey = "beatloop_" + loop_size + "_toggle";
                c.outKey = "beatloop_" + loop_size + "_enabled";
                c.connect();
                c.trigger();
            }
        });
    };
    this.change_loop_size(NS6II.default_loop_root_size);
};

NS6II.cont_pm_loop_auto.prototype = new components.ComponentContainer();

NS6II.cont_pm_loop_roll = function(channel_offset) {
    var theContainer = this;
    this.pads = [];
    this.current_root_loop_size = -3; // 2**-3 = 1/8
    for (var i = 1; i <= 8; i++) {
        this.pads[i] = new NS6II.Pad({
            midi: [0x90 + channel_offset, 0x13 + i],
            on: NS6II.colors.green,
            off: NS6II.colors.green_dimm,
            type: components.Button.prototype.types.toggle,
            // key is set by change_loop_size()
        });
    }
    this.param_left = new components.Button({
        midi: [0x90 + channel_offset, 0x28],
        input: function(channelmidi, control, value, status, group) {
            if (this.isPress(channel_offset, control, value, status)) {
                theContainer.change_loop_size(theContainer.current_root_loop_size - 1);
            }
        },
    });
    this.param_right = new components.Button({
        midi: [0x90 + channel_offset, 0x29],
        input: function(channelmidi, control, value, status, group) {
            if (this.isPress(channelmidi, control, value, status)) {
                theContainer.change_loop_size(theContainer.current_root_loop_size + 1);
            }
        },
    });
    this.change_loop_size = function(loop_size) {
        // clamp loop_size to [-5;7]
        theContainer.current_root_loop_size = Math.min(Math.max(-5, loop_size), 7);
        var i = 0;
        _.forEach(theContainer.pads,function (c) {
            if (c instanceof components.Component) {
                c.disconnect()
                c.inKey = "beatlooproll_" + Math.pow(2,theContainer.current_root_loop_size + (i++)) + "_activate";
                c.outKey = c.inKey;
                c.connect();
                c.trigger();
            }
        });
    };
    this.change_loop_size(NS6II.default_loop_root_size);
};

NS6II.cont_pm_loop_roll.prototype = new components.ComponentContainer();

NS6II.cont_pm_loop_ctrl = function(channel_offset) {
    this.pads = [];
    this.pads[1] = new NS6II.Pad({
        midi: [0x90 + channel_offset, 0x14],
        key: "loop_in",
        on: NS6II.colors.blue,
        off: NS6II.colors.blue_dimm,
    });
    this.pads[2] = new NS6II.Pad({
        midi: [0x90 + channel_offset, 0x15],
        key: "loop_out",
        on: NS6II.colors.blue,
        off: NS6II.colors.blue_dimm
    });
    this.pads[3] = new NS6II.Pad({
        midi: [0x90 + channel_offset, 0x16],
        key: "beatloop_activate",
        on: NS6II.colors.green,
        off: NS6II.colors.green_dimm,
    });
    this.pads[4] = new components.LoopToggleButton({
        midi: [0x90 + channel_offset, 0x17],
        on: NS6II.colors.mint,
        off: NS6II.colors.green_dimm,
    });
    this.pads[5] = new NS6II.Pad({
        midi: [0x90 + channel_offset, 0x18],
        key: "beatjump_forward",
        on: NS6II.colors.orange,
        off: NS6II.colors.orange_dimm,
    });
    this.pads[6] = new NS6II.Pad({
        midi: [0x90 + channel_offset, 0x19],
        key: "beatjump_backward",
        on: NS6II.colors.orange,
        off: NS6II.colors.orange_dimm,
    });
    this.pads[7] = new NS6II.Pad({
        midi: [0x90 + channel_offset, 0x1A],
        key: "loop_halve",
        on: NS6II.colors.red,
        off: NS6II.colors.red_dimm,
    });
    this.pads[8] = new NS6II.Pad({
        midi: [0x90 + channel_offset, 0x1B],
        key: "loop_double",
        on: NS6II.colors.red,
        off: NS6II.colors.red_dimm,
    });
    this.param_left = new components.Button({
        midi: [0x90 + channel_offset, 0x28],
        input: function(channelmidi, control, value, status, group) {
            // do nothing
        },
    });
    this.param_right = new components.Button({
        midi: [0x90 + channel_offset, 0x29],
        input: function(channelmidi, control, value, status, group) {
            // do nothing
        },
    });
}
NS6II.cont_pm_loop_ctrl.prototype = new components.ComponentContainer();

NS6II.cont_pm_sampler_normal = function(channel_offset) {
    this.pads = [];
    for (var i = 1; i <= 8; i++) {
        this.pads[i] = new components.SamplerButton({
            midi: [0x90 + channel_offset, 0x13 + i],
            number: i,
            empty: NS6II.colors.off,
            playing: NS6II.colors.white,
            loaded: NS6II.colors.white_dimm,
        });

    }
    this.param_left = new components.Button({
        midi: [0x90 + channel_offset, 0x28],
        input: function(channelmidi, control, value, status, group) {
            // do nothing
        },
    });
    this.param_right = new components.Button({
        midi: [0x90 + channel_offset, 0x29],
        input: function(channelmidi, control, value, status, group) {
            // do nothing
        },
    });
};
NS6II.cont_pm_sampler_normal.prototype = new components.ComponentContainer();

NS6II.cont_pm_sampler_velocity = function(channel_offset) {
    this.pads = [];
    for (var i = 1; i <= 8; i++) {
        this.pads[i] = new components.SamplerButton({
            midi: [0x90 + channel_offset, 0x13 + i],
            number: i,
            empty: NS6II.colors.off,
            playing: NS6II.colors.pink,
            loaded: NS6II.colors.pink_dimm,
            volumeByVelocity: true,
        });
    }

    this.param_left = new components.Button({
        midi: [0x90 + channel_offset, 0x28],
        input: function(channelmidi, control, value, status, group) {
            // do nothing
        },
    });
    this.param_right = new components.Button({
        midi: [0x90 + channel_offset, 0x29],
        input: function(channelmidi, control, value, status, group) {
            // do nothing
        },
    });
};
NS6II.cont_pm_sampler_velocity.prototype = new components.ComponentContainer();


NS6II.cont_pm_settings = function(channel_offset) {
    this.pads = [];
    this.pads[1] = new NS6II.Pad({
        midi: [0x90 + channel_offset, 0x14],
        key: "beats_adjust_slower",
    });
    this.pads[2] = new NS6II.Pad({
        midi: [0x90 + channel_offset, 0x15],
        key: "beats_adjust_faster",
    });
    this.pads[3] = new NS6II.Pad({
        midi: [0x90 + channel_offset, 0x16],
        key: "beats_translate_earlier",
    });
    this.pads[4] = new NS6II.Pad({
        midi: [0x90 + channel_offset, 0x17],
        key: "beats_translate_later",
    });
    this.pads[5] = new NS6II.Pad({
        midi: [0x90 + channel_offset, 0x18],
        key: "beats_translate_curpos",
    });
    this.pads[6] = new NS6II.Pad({
        midi: [0x90 + channel_offset, 0x19],
        key: "bpm_tap",
    });
    // pad7 is not mapped
    this.pads[8] = new NS6II.Pad({
        midi: [0x90 + channel_offset, 0x1B],
        input: function(channelmidi, control, value, status, group) {
            if (this.isPress(channelmidi, control, value, status)) {
                bpm.tapButton(channel_offset+1);
                this.output(0x01);
            } else {
                this.output(0x00);
            }
        },
    });
    this.param_left = new components.Button({
        midi: [0x90 + channel_offset, 0x28],
        input: function(channelmidi, control, value, status, group) {
            // do nothing
        },
    });
    this.param_right = new components.Button({
        midi: [0x90 + channel_offset, 0x29],
        input: function(channelmidi, control, value, status, group) {
            // do nothing
        },
    });
};
NS6II.cont_pm_settings.prototype = new components.ComponentContainer();

NS6II.pad_mode_mapper = {
    0x00: [NS6II.cont_pm_hotcue_regular, NS6II.cont_pm_hotcue_auto],
    0x02: 0x00, // shift alias
    0x10: [NS6II.cont_pm_loop_auto, NS6II.cont_pm_loop_roll],
    0x0E: NS6II.cont_pm_loop_ctrl,
    0x0B: [NS6II.cont_pm_sampler_normal, NS6II.cont_pm_sampler_velocity],
    0x0F: 0x0B,
    0x09: NS6II.cont_pm_settings,
};
NS6II.cont_pad_mode_selector = function(channel_offset,group) {
    var theSelector = this;
    var current_pad_index = 0;
    var next_pad_index = 0;
    var page_layer_to_led = {
        0: 0x04, // solid on
        1: 0x02, // blink on/off
        2: 0x03, // blink twice (unused currently)
    };
    // used to keep track which "radiobutton" was pressed previously for turning it off.
    var last_control = null;
    this.input = function(channelmidi, control, value, status, group) {
        if (value === 0x7F) {
            print("current_pad_index 1:"+current_pad_index);
            var preliminary_pad = NS6II.pad_mode_mapper[control];
            var resolved_control = control;
            print(control);
            print(typeof preliminary_pad);
            while (typeof preliminary_pad !== "function") {
                if (Number.isInteger(preliminary_pad)) {
                    // resolve shift control reroute
                    resolved_control = preliminary_pad;
                    preliminary_pad = NS6II.pad_mode_mapper[resolved_control];
                } else if (preliminary_pad instanceof Array){
                    var preliminary_pad_len = preliminary_pad.length;
                    // reset pad index if button changed.
                    if (resolved_control !== last_control) {
                        current_pad_index = 0;
                    }
                    // get actual pad_layer from array.
                    preliminary_pad = preliminary_pad[current_pad_index];
                    // cycle through the current_pad_layers
                    next_pad_index = ((current_pad_index+1)%preliminary_pad_len);
                }
            }
            print("current_pad_index 2:"+current_pad_index);
            // call function do disconnect existing layer and instantiate the new one.
            this.set_pads(preliminary_pad);
            midi.sendShortMsg(0x90 + channel_offset, resolved_control, page_layer_to_led[current_pad_index]);
            current_pad_index = next_pad_index;
            if (last_control !== null && last_control !== resolved_control) {
                midi.sendShortMsg(0x90 + channel_offset, last_control, components.Button.prototype.off)
            }
            last_control = resolved_control;
        }
    };
    this.set_pads = function(pad_container) {
        theSelector.pad_unit.forEachComponent(function(component) {
            component.disconnect();
        });
        theSelector.pad_unit = new pad_container(channel_offset);
        theSelector.pad_unit.reconnectComponents(function (c) {
            if (c.group === undefined) {
                c.group = group;
            }
        });
    };

    this.pad_unit = new components.ComponentContainer();
    this.set_pads(NS6II.cont_pm_hotcue_regular);
};

NS6II.cont_pad_mode_selector.prototype = new components.ComponentContainer();

NS6II.Channel = function(channel_offset) {
    var deck = "[Channel" + (channel_offset+1) + "]";
    var theChannel = this;
    this.btn_load_track_into_deck = new components.Button({
         midi: [0x9F, 0x02 + channel_offset],
        // midi: [0x90 + channel_offset, 0x17],
        group: deck,
        shift: function() {
            this.inKey = "eject";
            this.outKey = this.inKey;
        },
        unshift: function() {
            this.inKey = "LoadSelectedTrack";
            this.outKey = this.inKey;
        },
        // input: function(channelmidi, control, value, status, group) {
        //     // call regular input function when clone mode is not active
        //     if (!NS6II.clone_mode_active) {
        //         components.Button.prototype.input.call(this, channelmidi, control, value, status, group);
        //         return;
        //     }
        //     // clone mode is active; save deck to load from / load from saved deck
        //     if (this.isPress(channelmidi, control, value, status)) {
        //         if (!NS6II.clone_deck_channel_offset) {
        //             NS6II.clone_decks_channel_offset = channel_offset+1;
        //         } else {
        //             this.inKey = "CloneFromDeck";
        //             this.inSetValue(NS6II.clone_decks_channel_offset);
        //             NS6II.clone_deck_channel_offset = 0;
        //         }
        //     } else {
        //         NS6II.clone_deck_channel_offset = 0;
        //     }
        // },
    });
    // used to determine wheter vumeter on the controller would change
    // so messages get only when that is the case.
    this.last_vu_level = 0;
    this.conn_vumeter_level = engine.makeConnection(deck, "VuMeter", function(value) {
        // check if channel is peaking and increase value so that the peaking led gets lit as well
        // (the vumeter and the peak led are driven by the same control) (values > 81 light up the peakLED as well)

        // convert high res value to 5 LED resolution
        value = (value*4) | 0 + engine.getValue(deck, "PeakIndicator");
        if (value === this.last_vu_level) {
            // return early if vumeter has not changed (on the controller)
            return;
        } else {
            this.last_vu_level = value;
        }
        midi.sendShortMsg(0xB0 + channel_offset, 0x1F,value * 20);
    });
    this.knob_pre_gain = new components.Pot({
        midi: [0xB0 + channel_offset, 0x16],
        group: deck,
        inKey: "pregain"
    });
    this.eqKnobs = [];
    for (var i = 1; i <= 3; i++) {
        this.eqKnobs[i] = new components.Pot({
            midi: [0xB0 + channel_offset, 0x16 + i],
            group: '[EqualizerRack1_' + deck + '_Effect1]',
            inKey: 'parameter' + (4-i),
        });
    }
    this.eqCaps = [];
    for (var i = 1; i <= 3; i++) {
        this.eqCaps[i] = new components.Button({
            midi: [0x90 + channel_offset, 0x16 + i],
            group: '[EqualizerRack1_' + deck + '_Effect1]',
            inKey: 'button_parameter' + (4-i),
            isPress: function (channel, control, value, status) {
                return NS6II.btn_knob_cap_behavior.state > 1 && value > 0;
            }
        });
    }
    this.knob_channel_filter = new components.Pot({
        midi: [0xB0 + channel_offset, 0x1A],
        group: "[QuickEffectRack1_" + deck + "]",
        inKey: "super1",
    });
    this.cap_channel_filter = new components.Button({
        midi: [0x90 + channel_offset, 0x1A],
        group: "[QuickEffectRack1_" + deck + "_Effect1]",
        inKey: "enabled",
        isPress: function (channel, control, value, status) {
            // always press cap in state0
            return NS6II.btn_filter_knob_behavior.state === 0 ||
                // respond to value in state1
                ((NS6II.btn_filter_knob_behavior.state === 1) && value > 0);
                // always ignore press in state2
                // state2 is used for moving the knob after it had been left
                // off-center in state1
        }
    });
    this.btn_pfl = new components.Button({
        midi: [0x90 + channel_offset, 0x1B],
        group: deck,
        key: "pfl",
        // override off as pfl buttons are always backlit
        // and only turn off with a value of 0x00
        off: 0x00,
    });
    this.switch_cf_orientation = new components.Button({
        midi: [0x90 + channel_offset, 0x1E],
        group: deck,
        inKey: "orientation",
        input: function(channelmidi, control, value, status, group) {
            // Controller values to represent the orientation and mixxx
            // orientation representation don't match.
            switch (value) {
                case 0:
                    this.inSetValue(1);
                    break;
                case 1:
                    this.inSetValue(0);
                    break;
                case 2:
                    this.inSetValue(2);
                    break;
            }
        },
    });
    this.slider_vol = new components.Pot({
        midi: [0xB0 + channel_offset, 0x1C],
        group: deck,
        inKey: "volume",
    });
};
NS6II.Channel.prototype = new components.ComponentContainer();

NS6II.navBar = function() {
    this.enc_lib_nav = new components.Encoder({
        midi: [0xBF, 0x00], // shift: [0xBF,0x01]
        group: "[Library]",
        inKey: "MoveVertical",
        shift: function() {
            this.stepsize = NS6II.navigation_encoder_acceleration;
        },
        unshift: function() {
            this.stepsize = 1;
        },
        input: function(midiChannel, control, value, status, group) {
            this.inSetValue(value === 0x01 ? this.stepsize : -this.stepsize); // value "rescaling"; possibly inefficent.
        },
    });
    this.btn_lib_nav = new components.Button({
        midi: [0x9F, 0x06],
        group: "[Library]",
        inKey: "GoToItem",
    });
    this.btn_view = new components.Button({
        midi: [0x9F, 0x0E], // shift: [0x9F,0x13],
        group: "[Master]",
        inKey: "maximize_library",
        type: components.Button.prototype.types.toggle,
    });
    this.btn_back = new components.Button({
        midi: [0x9F, 0x11], // shift: [0x9F,0x12]
        group: "[Library]",
        key: "MoveFocusBackward",
    });
    this.btn_area = new components.Button({
        midi: [0x9F, 0xF], // shift: [0x9F, 0x1E]
        group: "[Library]",
        key: "MoveFocusForward",
    });
    this.btn_lprep = new components.Button({
        midi: [0x9F, 0x1B], // shift: [0x9F, 0x14]
        inKey: "LoadSelectedTrack",
        group: "[PreviewDeck1]",
    });
};
NS6II.navBar.prototype = new components.ComponentContainer();


NS6II.btn_knob_cap_behavior = new components.Button({
    midi: [0x9F,0x59],
    state: 2,
    input: function (channel, control, value, status, group) {
        this.state = value/63 | 0;
        if (NS6II.hide_killswitches_when_unused) {
            // only show UI killswitches if they are activated
            engine.setParameter("[Skin]","show_eq_kill_buttons",this.state>1);
        }
    },
});

NS6II.btn_filter_knob_behavior = new components.Button({
    midi: [0x9F,0x5A],
    state: 0,
    input: function (channel, control, value, status, group) {
        this.state = value/63 | 0;
    },
});
NS6II.EffectUnits = [];
for (var i = 1; i <= 2; i++) {
    NS6II.EffectUnits[i] = new components.EffectUnit(i);
    NS6II.EffectUnits[i].cap_temp_enable = [];
    for (var ii = 0; ii < 3; ii++) {
        NS6II.EffectUnits[i].enableButtons[ii + 1].midi = [0x97 + i, ii]; // shift: [0x97+i,0x0B+ii]
        NS6II.EffectUnits[i].cap_temp_enable[ii + 1] = new components.Button({
            midi: [0x97 + i, 0x21 + ii],
            group: '[EffectRack1_EffectUnit' + NS6II.EffectUnits[i].currentUnitNumber +
                '_Effect' + (ii+1) + ']',
            inKey: "enabled",
            shifted: false, // custom static property; used to disable fx input while selecting
            isPress: function (channel, control, value, status) {
                return !this.shifted && NS6II.btn_knob_cap_behavior.state > 0 && value > 0;
            },
            unshift: function () {
                this.shifted = false;
            },
            shift: function () {
                this.shifted = true;
            },
        });
        NS6II.EffectUnits[i].knobs[ii + 1].midi = [0xB7 + i, ii];
    }
    NS6II.EffectUnits[i].effectFocusButton.midi = [0x97 + i, 0x04];
    NS6II.EffectUnits[i].dryWetKnob.midi = [0xB7 + i, 0x03];
    NS6II.EffectUnits[i].dryWetKnob.input = function(midichannel, control, value, status, group) {
        if (value === 1) {
            this.inSetParameter(this.inGetParameter() + 0.04);
        } else if (value === 127) {
            this.inSetParameter(this.inGetParameter() - 0.04);
        }
    };
    NS6II.EffectUnits[i].btn_mix_mode = new components.Button({
        midi: [0xB7 + i, 0x41],
        type: components.Button.prototype.types.toggle,
        inKey: "mix_mode",
        group: NS6II.EffectUnits[i].group,
    })
    for (var ii = 0; ii < 4; ii++) {
        var channel = "Channel"+(ii + 1)
        NS6II.EffectUnits[i].enableOnChannelButtons.addButton(channel);
        NS6II.EffectUnits[i].enableOnChannelButtons[channel].midi = [0x97 + i, 0x05 + ii];
    }
    NS6II.EffectUnits[i].init();
}

NS6II.init = function() {
    // checks whether the currently set rateRange is in the user settings.
    // if it is, current index of the rate_range array is set accordingly
    // if it isn't, it searches for a matching spot in the array where to
    // insert it and then sets the index accordingly
    NS6II.pot_caps_active = true; // handle by touch fx button later

    // force headMix to 1 because it is managed by the controller hardware mixer.
    engine.setParameter("[Master]","headMix",1);

    var default_rate_range = engine.getValue("[Channel1]", "rateRange");
    NS6II.current_rate_range_index = NS6II.rate_ranges.indexOf(default_rate_range);
    if (NS6II.current_rate_range_index === -1) {
        for (var i = 0; i < NS6II.rate_ranges.length; i++) {
            if (default_rate_range > NS6II.rate_ranges[i] && default_rate_range < NS6II.rate_ranges[i + 1]) {
                NS6II.splice(i, 0, default_rate_range);
                NS6II.current_rate_range_index = i;
            }
        }
    }
    NS6II.Decks = [];
    for (var i = 1; i <=4; i++) {
        NS6II.Decks[i] = new NS6II.Deck(i-1);
        print("initialized Deck"+i);
    }
    print("created decks");
    NS6II.Mixer_instance = new NS6II.Mixer();
    print("constructed mixer");
};



// ES3 Polyfills
var Number = {};
Number.isInteger = function(value) {
  return typeof value === 'number' &&
    isFinite(value) &&
    Math.floor(value) === value;
};
