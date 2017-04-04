'use strict';
/* *******************************************
 * = Configration Options
 ********************************************/
// Read official user guide for details.
// I scripting near behavior on userguide.
// = Filter roll and Filter fx mode config.
// chain filter channel. 1 to 4
var filterfx_channel = 4;
var multiPressTimeout = 50; // Multi press time out. milisecs.


/**************************
 * Constants for scratching :
 **************************/
var intervalsPerRev = 1200, // or 556
  rpm = 33 + 1 / 3, //Like a real vinyl !!! :)
  alpha = 1 / 8, //Adjust to suit.
  beta = alpha / 32; //Adjust to suit.

/********************************
* Numark NV Controller Script.
* Author:: TAKAHASHI Hidetsugu a.k.a manzyun
* Version:: 0.1.0
* License:: GPL v2
*
* = Key features
*
* * Slip mode
* ------------------------------------------------
*
* = User References
* * support forum: http://www.mixxx.org/forums/viewtopic.php?f=7&t=8055
* * e-mail manzyun@gmail.com
*
* = Thanks

* Stéphane Morin, largely based on script from Chloé AVRILLON (DJ Chloé),
 *and Numark-Mixtrack-3-scripts.js authors.
**********************************/

// TODO research LED message

////////////////////////////////////////////////////////////////////////
// JSHint configuration                                               //
////////////////////////////////////////////////////////////////////////
/* global engine                                                      */
/* global script                                                      */
/* global print                                                       */
/* global midi                                                        */
////////////////////////////////////////////////////////////////////////

var loop_number = [16, 8, 4, 2, 1, 0.5, 0.25, 0.125];
var range_width = [0.08, 0.16, 0.50];

var mixer_channel = {
  0x44: '[Channel1]',
  0x45: '[Channel2]',
  0x46: '[Channel3]',
  0x47: '[Channel4]'
}; // Because mixer channel is 0xND

var connectedFunctions = [
  'play', 'cue', 'sync', 'bleep', 'filterRollMode',
  'monitor.range', 'monitor.load',
  'fx.fx1', 'fx.fx2', 'fx.fx3',
  'pad.pad01', 'pad.pad02', 'pad.pad03', 'pad.pad04',
  'pad.pad05', 'pad.pad06', 'pad.pad07', 'pad.pad08',
  'padAction.cues'
];

var controlNames = [

];

var channelsString = [
  '[Channel1]', '[Channel2]', '[Channel3]', '[Channel4]'
];

var numarkNV = {
  'shift': false,
  'pad_mode': [0x1A, 0x1A, 0x1A, 0x1A],
  'rate_range': [0.08, 0.08, 0.08, 0.08],
  'filter_roll': false,
  'filter_fx': false,
  'touch_mode': false
};

numarkNV.activeButtons = {};
numarkNV.activePadChanger = {};

numarkNV.on_off = {
  'ON': 0x7F,
  'OFF': 0x00
};
numarkNV.pad_mode_num = {
  'cues': 0x1A,
  'auto': 0x1C,
  'loop': 0x1B,
  'sampler': 0x1D,
  'slicer': 0x1E
};
numarkNV.groupToDeck = function groupToDeck(group) {
  var channelRegEx = /\[Channel(\d+)\]/
  var re_int = parseInt(channelRegEx.exec(group)[1])
  return re_int
}
numarkNV.padDeckNum = function padDeckNum(group) {
  if (numarkNV.groupToDeck(group) === 1 || numarkNV.groupToDeck(group) === 3) {
    return 1
  } else if (numarkNV.groupToDeck(group) === 2 || numarkNV.groupToDeck(group) === 4) {
    return 2
  } else {
    return 1
  }
};

// = WheelTurn section
/**************************************************
details midi_scripting [Mixxx Wiki] http://mixxx.org/wiki/doku.php/midi_scripting
****************************************************/
numarkNV.Scratch = {
  'scratch_mode': [false, false, false, false],
  'old_value': 0,

  pushed: function pushed(channel, control, value, status, group) {
    numarkNV.Scratch.scratch_mode[numarkNV.groupToDeck(group) - 1] = !numarkNV.Scratch.scratch_mode
  },

  wheelTouch: function wheelTouch(channel, control, value, status, group) {;
    deck = numarkNV.groupToDeck(group)
    if (value === numarkNV.on_off.ON) {
      if (numarkNV.Scratch.scratch_mode[deck - 1]) {
        engine.scratchEnable(deck, intervalsPerRev, rpm, alpha, beta);
      } else {
        engine.scratchDisable(deck);
      }
    }
  },

  wheelTurn: function wheelTurn(channel, control, value, status, group) {
    var _adjustedJog = parseFloat(value);
    var _posNeg = 1;
    var _deck = numarkNV.groupToDeck(group)
    if (_adjustedJog < 64) {
      _adjustedJog = value
    } else if (_adjustedJog > 64) {
      _adjustedJog = value - 63;
    }
    if (numarkNV.Scratch.old_value > value || (numarkNV.Scratch.old_value ===
        0 && value === 0x7F)) { // Counter-clockwise
      _adjustedJog = -_adjustedJog
      _posNeg = -1
    }
    numarkNV.Scratch.old_value = value

    if (engine.getParameter(group, 'play') == false) {
      engine.scratchEnable(deck, intervalsPerRev, rpm, alpha, beta);
      engine.scratchTick(deck, adjustedJog); // Scratch!
      var _gammaInputRange = 13; // Max jog speed
      var _maxOutFraction = 0.8; // Where on the curve it should peak; 0.5 is half-way
      var _sensitivity = 0.5; // Adjustment gamma
      var _gammaOutputRange = 2; // Max rate change
      engine.setParameter(group, 'jog', _posNeg *
        _gammaOutputRange * Math.pow(Math.abs(_adjustedJog) / (
          _gammaInputRange * _maxOutFraction), _sensitivity)); // Pitch bend
    }

    if (engine.getParameter(group, 'play')) {
      var _gammaInputRange = 13; // Max jog speed
      var _maxOutFraction = 0.8; // Where on the curve it should peak; 0.5 is half-way
      var _sensitivity = 0.5; // Adjustment gamma
      var _gammaOutputRange = 2; // Max rate change
      engine.setParameter(group, 'jog', _posNeg *
        _gammaOutputRange * Math.pow(Math.abs(_adjustedJog) / (
          _gammaInputRange * _maxOutFraction), _sensitivity)); // Pitch bend
    }
  }
}

/* = Touch Effect */
/* If the Touch Effect mode enabled, you can touch EQ and quic effects knob touch controll */
numarkNV.touchModePushed = function touchModePushed(channel, control, value, status, group) {
  numarkNV.touchMode.touch_mode = !numarkNV.touchMode.touch_mode
}
/* TODO I don't know EQ killing...
numarkNV.touchMode = {
  ch1: {
    high: function (channel, control, value, status, group) {

    },
    mid: function (channel, control, value, status, group) {

    },
    low: function (channel, control, value, status, group) {

    },
    super: function (channel, control, value, status, group) {

    },
    fx1: function (channel, control, value, status, group) {

    },
    fx2 = function (channel, control, value, status, group) {

    },
    fx3 = function (channel, control, value, status, group) {

    }
  },

  ch2: {
    high: function (channel, control, value, status, group) {

    },
    mid: function (channel, control, value, status, group) {

    },
    low: function (channel, control, value, status, group) {

    },
    super: function (channel, control, value, status, group) {

    },
    fx1: function (channel, control, value, status, group) {

    },
    fx2 = function (channel, control, value, status, group) {

    },
    fx3 = function (channel, control, value, status, group) {

    }
  },

  ch3: {
    high: function (channel, control, value, status, group) {

    },
    mid: function (channel, control, value, status, group) {

    },
    low: function (channel, control, value, status, group) {

    },
    super: function (channel, control, value, status, group) {

    },
    fx1: function (channel, control, value, status, group) {

    },
    fx2 = function (channel, control, value, status, group) {

    },
    fx3 = function (channel, control, value, status, group) {

    }
  },

  ch4: {
    high: function (channel, control, value, status, group) {

    },
    mid: function (channel, control, value, status, group) {

    },
    low: function (channel, control, value, status, group) {

    },
    super: function (channel, control, value, status, group) {

    },
    fx1: function (channel, control, value, status, group) {

    },
    fx2 = function (channel, control, value, status, group) {

    },
    fx3 = function (channel, control, value, status, group) {

    }
  }
} */

/* = filterRoll Section */
numarkNV.filterRoll = function filterRoll(channel, controll, value, status) {
  if (numarkNV.filter_roll) {
    var _ch = mixer_channel[controll - 0x0D - 1] // Because mixer channel is 0xND
    if (vale <= 0x0F) {
      engine.setParameter('[Channel' + _ch + ']', 'beatloop_' +
        loop_number[7] + '_enabled', 1)
    } else if (value <= 0x1E) {
      engine.setParameter('[Channel' + _ch + ']', 'beatloop_' +
        loop_number[6] + '_enabled', 1)
    } else if (value <= 0x2D) {
      engine.setParameter('[Channel' + _ch + ']', 'beatloop_' +
        loop_number[5] + '_enabled', 1)
    } else if (value <= 0x3C) {
      engine.setParameter('[Channel' + _ch + ']', 'beatloop_' +
        loop_number[4] + '_enabled', 1)
    } else if (value <= 0x4B) {
      engine.setParameter('[Channel' + _ch + ']', 'beatloop_' +
        loop_number[3] + '_enabled', 1)
    } else if (value <= 0x5a) {
      engine.setParameter('[Channel' + _ch + ']', 'beatloop_' +
        loop_number[2] + '_enabled', 1)
    } else if (value <= 0x69) {
      engine.setParameter('[Channel' + _ch + ']', 'beatloop_' +
        loop_number[1] + '_enabled', 1)
    } else {
      engine.setParameter('[Channel' + _ch + ']', 'beatloop_' +
        loop_number[0] + '_enabled', 1)
    }
  }
}
numarkNV.filterRollFx = function filterRollFx(channel, controll, value, status, group) {
  if (numarkNV.filter_fx) {
    var _ch = mixer_channel[controll - 0x0D - 1]
    var _wet = value / 127
    engine.setParameter('[EffectRack1_EffectUnit' + filterfx_channel +
      ']', 'enabled', 1)
    engine.setParameter('[EffectRack1_EffectUnit' + filterfx_channel +
      ']', 'mix', wet)
    if (vale <= 0x0F) {
      engine.setParameter('[Channel' + _ch + ']', 'beatloop_' +
        loop_number[7] + '_enabled', 1)
    } else if (value <= 0x1E) {
      engine.setParameter('[Channel' + _ch + ']', 'beatloop_' +
        loop_number[6] + '_enabled', 1)
    } else if (value <= 0x2D) {
      engine.setParameter('[Channel' + _ch + ']', 'beatloop_' +
        loop_number[5] + '_enabled', 1)
    } else if (value <= 0x3C) {
      engine.setParameter('[Channel' + _ch + ']', 'beatloop_' +
        loop_number[4] + '_enabled', 1)
    } else if (value <= 0x4B) {
      engine.setParameter('[Channel' + _ch + ']', 'beatloop_' +
        loop_number[3] + '_enabled', 1)
    } else if (value <= 0x5a) {
      engine.setParameter('[Channel' + _ch + ']', 'beatloop_' +
        loop_number[2] + '_enabled', 1)
    } else if (value <= 0x69) {
      engine.setParameter('[Channel' + _ch + ']', 'beatloop_' +
        loop_number[1] + '_enabled', 1)
    } else {
      engine.setParameter('[Channel' + _ch + ']', 'beatloop_' +
        loop_number[0] + '_enabled', 1)
    }
  }
}
/* ---------------------------------- */

/* = Rate Knob
/* Because, this controller max value 7F. */
numarkNV.rateKnob = function rateKnob(channel, control, value, status, group) {
  engine.setParameter(group, 'rate', value / 0x7F)
}
/*-----------------------------------------*/

/* = Un Shift Button Section */
numarkNV.unShiftedButtons = {
  play: function play(channel, control, value, status, group) {
    if (value) {
      engine.setParameter(group, 'play', !(engine.getParameter(group, 'play')));
    }
  },

  cue: function cue(channel, control, value, status, group) {
    if (engine.getValue(group, "playposition") <= 0.97) {
      engine.setValue(group, "cue_default", value ? 1 : 0);
    } else {
      engine.setValue(group, "cue_preview", value ? 1 : 0);
    }
  },

  sync: function sync(channel, control, value, status, group) {
    if (value) {
      engine.setParameter(group, 'sync_master', 0);
      engine.setParameter(group, 'sync_enabled', 1);
    }
  },

  bleep: function bleep(channel, control, value, status, group) {
    engine.setParameter(group, 'reverseroll', value ? 1 : 0);
  },

  filterRollMode: function felterRollMode(channel, control, value, status, group) {
    numarkNV.filter_fx = false;
    numarkNV.FilterRollMode.filter_roll_mode = !numarkNV.filter_roll;
  },

  monitor: {
    range: function range(channel, control, value, status, group) {
      if (value) {
        switch (numarkNV.rate_range[channel - 1]) {
          case 0.08:
            engine.setParameter(group, 'rateRange',
              range_width[1])
            break;
          case 0.16:
            engine.setParameter(group, 'rateRange',
              range_width[2])
            break;
          case 0.50:
            engine.setParameter(group, 'rateRange',
              range_width[0])
            break;
          default:
            engine.setParameter(group, 'rateRange',
              range_width[0]);
        }
      }
    },

    load: function load(channel, control, value, status, group) {
      if (value) {
        engine.setParameter(group, 'LoadSelectedTrack', 1);
      }
    }
  },

  fx: {
    fx1: function fx1(channel, control, value, status, group) {
      if (control <= 0x11) {
        engine.setParameter('[EffectRack1_EffectUnit1]', 'group_' + group +
          '_enable', !engine.getParameter('[EffectRack1_EffectUnit1]', 'group_' + group + '_enabled'))
      } else {
        engine.setParameter('[EffectRack1_EffectUnit1]', 'group_' + group +
          '_enable', !engine.getParameter('[EffectRack1_EffectUnit1]', 'group_' + group + '_enabled'))
      }
    },

    fx2: function fx2(channel, control, value, status, group) {
      if (control <= 0x11) {
        engine.setParameter('[EffectRack1_EffectUnit2]', 'group_' + group +
          '_enable', !engine.getParameter('[EffectRack1_EffectUnit2]', 'group_' + group + '_enabled'))
      } else {
        engine.setParameter('[EffectRack1_EffectUnit2]', 'group_' + group +
          '_enable', !engine.getParameter('[EffectRack1_EffectUnit2]', 'group_' + group + '_enabled'))
      }
    },

    fx3: function fx3(channel, control, value, status, group) {
      if (control <= 0x11) {
        engine.setParameter('[EffectRack1_EffectUnit3]', 'group_' + numarkNV.nowDeck.deck1 +
          '_enable', !engine.getParameter('[EffectRack1_EffectUnit3]', 'group_' + group + '_enable'))
      } else {
        engine.setParameter('[EffectRack1_EffectUnit3]', 'group_' + numarkNV.nowDeck.deck2 +
          '_enable', !engine.getParameter('[EffectRack1_EffectUnit3]', 'group_' + group + '_enable'))
      }
    }
  },

  pad: {
    pad01: function pad01(channel, control, value, status, group) {
      if (value) {
        var deck = 0;
        switch (numarkNV.padmode) {
          case numarkNV.pad_mode_num.cues:
            numarkNV.unShiftedButtons.pad.padAction.cues(group, 1)
            break;
          case numarkNV.pad_mode_num.auto:
            if (engine.getParameter(group, 'beatloop_' + loop_number[7] + '_enabled')) {
              engine.setParameter(group, 'loop_enabled', 0)
              engine.setParameter(group, 'beatloop_' + loop_number[7] + '_enabled', 0);
            } else {
              engine.setParameter(group, 'beatloop_' + loop_number[7] + '_activate', 1)
            }
            break;
          case numarkNV.pad_mode_num.loop:
            engine.setParameter(group, 'loop_in', 1)
            break;
          case numarkNV.pad_mode_num.sampler:
            numarkNV.unShiftedButtons.pad.padAction.cues('[Sampler' + numarkNV.padDeckNum(group) + ']', 1)
            break;
          case numarkNV.pad_mode_num.slicer:
            engine.setParameter(group, 'beatjump_' + loop_number[7] + '_forward', 1)
            break;
          default:
            numarkNV.unShiftedButtons.pad.padAction.cues(group, 1)
        }
      }
    },

    pad02: function pad02(channel, control, value, status, group) {
      if (value) {
        switch (numarkNV.padmode) {
          case numarkNV.pad_mode_num.cues:
            numarkNV.unShiftedButtons.pad.padAction.cues(group, 2)
            break;
          case numarkNV.pad_mode_num.auto:
            if (engine.getParameter(group, 'beatloop_' + loop_number[6] + '_enabled')) {
              engine.setParameter(group, 'loop_enabled', 0)
              engine.setParameter(group, 'beatloop_' + loop_number[6] + '_enabled', 0);
            } else {
              engine.setParameter(group, 'beatloop_' + loop_number[6] + '_activate', 1)
            }
            break;
          case numarkNV.pad_mode_num.loop:
            engine.setParameter('[Channel' + (channel) + ']', 'loop_out', 1)
            break;
          case numarkNV.pad_mode_num.sampler:
            numarkNV.unShiftedButtons.pad.padAction.cues('[Sampler' + numarkNV.padDeckNum(group) + ']', 2)
            break;
          case numarkNV.pad_mode_num.slicer:
            engine.setParameter(group, 'beatjump_' + loop_number[6] + '_forward', 1)
          default:
            numarkNV.unShiftedButtons.pad.padAction.cues(group, 2)
        }
      }
    },

    pad03: function pad03(channel, control, value, status, group) {
      if (value) {
        switch (numarkNV.padmode) {
          case numarkNV.pad_mode_num.cues:
            numarkNV.unShiftedButtons.pad.padAction.cues(group, 3)
            break;
          case numarkNV.pad_mode_num.auto:
            if (engine.getParameter(group, 'beatloop_' + loop_number[5] + '_enabled')) {
              engine.setParameter(group, 'loop_enabled', 0)
              engine.setParameter(group, 'beatloop_' + loop_number[5] + '_enabled', 0);
            } else {
              engine.setParameter(group, 'beatloop_' + loop_number[5] + '_activate', 1)
            }
            break;
          case numarkNV.pad_mode_num.loop:
            engine.setParameter(group, 'reloop_exit', 1)
            break;
          case numarkNV.pad_mode_num.sampler:
            numarkNV.unShiftedButtons.pad.padAction.cues('[Sampler' + numarkNV.padDeckNum(group) + ']', 3)
            break;
          case numarkNV.pad_mode_num.slicer:
            if (numarkNV.shift) {
              engine.setParameter(group, 'beatjump_' + loop_number[5] +
                '_backword', 1)
            } else {
              engine.setParameter(group, 'beatjump_' + loop_number[5] +
                '_forward', 1)
            }
            break;
          default:
            numarkNV.unShiftedButtons.pad.padAction.cues(group, 3)
        }
      }
    },

    pad04: function pad04(channel, control, value, status, group) {
      if (value) {
        switch (numarkNV.padmode) {
          case numarkNV.pad_mode_num.cues:
            numarkNV.unShiftedButtons.pad.padAction.cues(group, 4)
            break;
          case numarkNV.pad_mode_num.auto:
            if (engine.getParameter(group, 'beatloop_' + loop_number[4] + '_enabled')) {
              engine.setParameter(group, 'loop_enabled', 0)
              engine.setParameter(group, 'beatloop_' + loop_number[4] + '_enabled', 0);
            } else {
              engine.setParameter(group, 'beatloop_' + loop_number[4] + '_activate', 1)
            }
            break;
          case numarkNV.pad_mode_num.loop:
            engine.setParameter(group, 'reloop_exit', 1)
            break;
          case numarkNV.pad_mode_num.sampler:
            numarkNV.unShiftedButtons.pad.padAction.cues('[Sampler' + numarkNV.padDeckNum(group) + ']', 4)
            break;
          case numarkNV.pad_mode_num.slicer:
            engine.setParameter(group, 'beatjump_' + loop_number[4] + '_forward', 1)
            break;
          default:
            numarkNV.unShiftedButtons.pad.padAction.cues(group, 4)
        }
      }
    },

    pad05: function pad05(channel, control, value, status, group) {
      if (value) {
        switch (numarkNV.pad_mode) {
          case numarkNV.pad_mode_num.cues:
            numarkNV.unShiftedButtons.pad.padAction.cues(group, 5)
            break;
          case numarkNV.pad_mode_num.auto:
            if (engine.getParameter(group, 'beatloop_' + loop_number[3] + '_enabled')) {
              engine.setParameter(group, 'loop_enabled', 0)
              engine.setParameter(group, 'beatloop_' + loop_number[3] + '_enabled', 0);
            } else {
              engine.setParameter(group, 'beatloop_' + loop_number[3] + '_activate', 1)
            }
            break;
          case numarkNV.pad_mode_num.loop:
            engine.setParameter('[Channel' + (deck + 2) + ']', 'loop_in', 1)
            break;
          case numarkNV.pad_mode_num.sampler:
            numarkNV.unShiftedButtons.pad.padAction.cues('[Sampler' + (numarkNV.padDeckNum(group) + 2) + ']', 1)
            break;
          case numarkNV.pad_mode_num.slicer:
            if (numarkNV.shift) {
              engine.setParameter(group, 'beatjump_' + loop_number[3] + '_backword', 1)
            } else {
              engine.setParameter(group, 'beatjump_' + loop_number[3] + '_forward', 1)
            }
            break;
          default:
            numarkNV.unShiftedButtons.pad.padAction.cues(group, 5)
        }
      }
    },

    pad06: function pad06(channel, control, value, status, group) {
      if (value) {
        switch (numarkNV.padmode) {
          case numarkNV.pad_mode_num.cues:
            numarkNV.unShiftedButtons.pad.padAction.cues(group, 6)
            break;
          case numarkNV.pad_mode_num.auto:
            if (engine.getParameter(group, 'beatloop_' + loop_number[2] + '_enabled')) {
              engine.setParameter(group, 'loop_enabled', 0)
              engine.setParameter(group, 'beatloop_' + loop_number[2] + '_enabled', 0);
            } else {
              engine.setParameter(group, 'beatloop_' + loop_number[2] + '_activate', 1)
            }
            break;
          case numarkNV.pad_mode_num.loop:
            engine.setParameter('[Channel' + (deck + 2) + ']', 'loop_out', 1)
            break;
          case numarkNV.pad_mode_num.sampler:
            numarkNV.unShiftedButtons.pad.padAction.cues('[Sampler' + (numarkNV.padDeckNum(group) + 2) + ']', 2)
            break;
          case numarkNV.pad_mode_num.slicer:
            if (numarkNV.shift) {
              engine.setParameter(group, 'beatjump_' + loop_number[2] +
                '_backword', 1)
            } else {
              engine.setParameter(group, 'beatjump_' + loop_number[2] +
                '_forward', 1)
            }
            break;
          default:
            numarkNV.unShiftedButtons.pad.padAction.cues(group, 6)
        }
      }
    },

    pad07: function pad07(channel, control, value, status, group) {

      if (value) {
        switch (numarkNV.padmode) {
          case numarkNV.pad_mode_num.cues:
            numarkNV.unShiftedButtons.pad.padAction.cues(group, 7)
            break;
          case numarkNV.pad_mode_num.auto:
            if (engine.getParameter(group, 'beatloop_' + loop_number[1] + '_enabled')) {
              engine.setParameter(group, 'loop_enabled', 0)
              engine.setParameter(group, 'beatloop_' + loop_number[1] + '_enabled', 0);
            } else {
              engine.setParameter(group, 'beatloop_' + loop_number[1] +
                '_activate', 1)
            }
            break;
          case numarkNV.pad_mode_num.loop:
            engine.setParameter('[Channel' + (deck + 2) + ']', 'reloop_exit', 1)
            break;
          case numarkNV.pad_mode_num.sampler:
            numarkNV.unShiftedButtons.pad.padAction.cues('[Sampler' + (numarkNV.padDeckNum(group) + 2) + ']', 3)
            break;
          case numarkNV.pad_mode_num.slicer:
            engine.setParameter(group, 'beatjump_' + loop_number[1] + '_forward', 1)
            break;
          default:
            numarkNV.unShiftedButtons.pad.padAction.cues(group, 7)
        }
      }
    },

    pad08: function pad08(channel, control, value, status, group) {

      if (value) {
        switch (numarkNV.padmode) {
          case numarkNV.pad_mode_num.cues:
            numarkNV.unShiftedButtons.pad.padAction.cues(group, 8)
            break;
          case numarkNV.pad_mode_num.auto:
            if (engine.getParameter(group, 'beatloop_' + loop_number[0] + '_enabled')) {
              engine.setParameter(group, 'loop_enabled', 0)
              engine.setParameter(group, 'beatloop_' + loop_number[0] + '_enabled', 0);
            } else {
              engine.setParameter(group, 'beatloop_' + loop_number[0] +
                '_activate', 1)
            }
            break;
          case numarkNV.pad_mode_num.loop:
            if (numarkNV.shif) {
              engine.setParameter('[Channel' + (channel + 2) + ']',
                'reloop_exit', 0)
            } else {
              engine.setParameter('[Channel' + (deck + 2) + ']',
                'reloop_exit', 1)
            }
            break;
          case numarkNV.pad_mode_num.sampler:
            numarkNV.unShiftedButtons.pad.padAction.cues('[Sampler' + (numarkNV.padDeckNum(group) + 2) + ']', 3)
            break;
          case numarkNV.pad_mode_num.slicer:
            engine.setParameter(group, 'beatjump_' + loop_number[0] + '_forward', 1)
            break;
          default:
            numarkNV.unShiftedButtons.pad.padAction.cues(group, 8)
        }
      }
    },

    padAction: {
      cues: function cues(group, padnumber) {
        if (engine.getParameter(group, 'hotcue_' + padnumber + '_enabled') === 1) { // is setted
          engine.setParameter(group, 'hotcue_' + padnumber + '_gotoandplay', 1)
        } else {
          engine.setParameter(group, 'hotcue_' + padnumber + '_set', 1)
        }
      }
    }
  }
}
/* --------------------------------------- */

/* = Shft Button Section */
numarkNV.shiftedButtons = {
  play: function play(channel, control, value, status, group) {
    if (value) {
      engine.setParameter(group, 'play_stutter', !(engine.getParameter(group, 'play_stutter')));
    }
  },

  cue: function cue(channel, control, value, status, group) {
    if (value) {
      engine.setParameter(group, 'start_stop', !(engine.getParameter(group, 'start_stop')));
    }
  },

  sync: function sync(channel, control, value, status, group) {
    if (value) {
      engine.setParameter(group, 'sync_enabled', 0);
      engine.setParameter(group, 'sync_master', 1);
    }
  },

  bleep: function bleep(channel, control, value, status, group) {
    if (value) {
      engine.setParameter(group, 'reverse', 1);
    } else {
      engine.setParameter(group, 'reverse', 0);
    }
  },

  filterRollMode: function fillterRollMode(channel, control, value, status, group) {
    numarkNV.FilterRolMode.filter_roll_mode = false;
    numarkNV.FilterRolFx.filter_fx_mode = !numarkNV.FilterRolFx.filter_fx_mode;
  },

  monitor: {
    range: function range(channel, control, value, status, group) {
      if (value) {
        if (value) {
          engine.setParameter(group, 'keylock', !(
            engine.getParameter(
              group, 'keylock')));
        }
      }
    },

    load: function load(channel, control, value, status, group) {
      if (value) {
        engine.setParameter('[PreviewDeck1]',
          'LoadSelectedTrackAndPlay', 1);
      }
    }
  },

  fx: {
    fx1: function fx1(channel, control, value, status, group) {
      engine.setParameter('[EffectRack1_EffectUnit1]', 'next_chain', 1)
    },

    fx2: function fx2(channel, control, value, status, group) {
      engine.setParameter('[EffectRack1_EffectUnit2]', 'next_chain', 1)
    },

    fx3: function fx3(channel, control, value, status, group) {
      engine.setParameter('[EffectRack1_EffectUnit3]', 'next_chain', 1)
    }
  },

  pad: {
    pad01: function pad01(channel, control, value, status, group) {
      if (value) {
        switch (numarkNV.pad_mode) {
          case numarkNV.pad_mode_num.cues:
            numarkNV.shiftedButtons.padAction.cues(group, 1);
            break;
          case numarkNV.pad_mode_num.auto:
            if (engine.getParameter(group, 'beatloop_' + loop_number[7] + '_enabled')) {
              engine.setParameter(group, 'loop_enabled', 0);
              engine.setParameter(group, 'beatloop_' + loop_number[7] + '_enabled', 0);
            } else {
              engine.setParameter(group, 'beatloop_' + loop_number[7] + '_activate', 1);
            }
            break;
          case numarkNV.pad_mode_num.loop:
            engine.setParameter('[Channel1]', 'loop_in', 0);
            break;
          case numarkNV.pad_mode_num.sampler:
            numarkNV.shiftedButtons.padActioncues('[Sampler' + numarkNV.padDeckNum(group) + ']', 1)
            break;
          case numarkNV.pad_mode_num.slicer:
            engine.setParameter(group, 'beatjump_' + loop_number[7] + '_backword', 1);
            break;
          default:
            numarkNV.shiftedButtons.padAction.cues(group, 1)
        }
      }
    },

    pad02: function pad02(channel, control, value, status, group) {
      if (value) {
        switch (numarkNV.pad_mode) {
          case numarkNV.pad_mode_num.cues:
            numarkNV.shiftedButtons.padAction.cues(group, 2);
            break;
          case numarkNV.pad_mode_num.auto:
            if (engine.getParameter(group, 'beatloop_' + loop_number[6] + '_enabled')) {
              engine.setParameter(group, 'loop_enabled', 0);
              engine.setParameter(group, 'beatloop_' + loop_number[6] + '_enabled', 0);
            } else {
              engine.setParameter(group, 'beatloop_' + loop_number[6] + '_activate', 1);
            }
            break;
          case numarkNV.pad_mode_num.loop:
            engine.setParameter('[Channel1]', 'loop_out', 0)
            break;
          case numarkNV.pad_mode_num.sampler:
            numarkNV.shiftedButtons.padActioncues('[Sampler' + numarkNV.padDeckNum(group) + ']', 2)
            break;
          case numarkNV.pad_mode_num.slicer:
            engine.setParameter(group, 'beatjump_' + loop_number[6] + '_backword', 1);
            break;
          default:
            numarkNV.shiftedButtons.padAction.cues(group, 1)
        }
      }
    },

    pad03: function pad03(channel, control, value, status, group) {
      if (value) {
        switch (numarkNV.pad_mode) {
          case numarkNV.pad_mode_num.cues:
            numarkNV.shiftedButtons.padAction.cues(group, 3);
            break;
          case numarkNV.pad_mode_num.auto:
            if (engine.getParameter(group, 'beatloop_' + loop_number[5] + '_enabled')) {
              engine.setParameter(group, 'loop_enabled', 0);
              engine.setParameter(group, 'beatloop_' + loop_number[5] + '_enabled', 0);
            } else {
              engine.setParameter(group, 'beatloop_' + loop_number[5] + '_activate', 1);
            }
            break;
          case numarkNV.pad_mode_num.loop:
            engine.setParameter('[Channel1]', 'reloop_exit', 0)
            break;
          case numarkNV.pad_mode_num.sampler:
            numarkNV.shiftedButtons.padActioncues('[Sampler' + numarkNV.padDeckNum(group) + ']', 3)
            break;
          case numarkNV.pad_mode_num.slicer:
            engine.setParameter(group, 'beatjump_' + loop_number[6] + '_backword', 1);
            break;
          default:
            numarkNV.shiftedButtons.padAction.cues(group, 1)
        }
      }
    },

    pad04: function pad04(channel, control, value, status, group) {
      if (value) {
        switch (numarkNV.pad_mode) {
          case numarkNV.pad_mode_num.cues:
            numarkNV.shiftedButtons.padAction.cues(group, 4);
            break;
          case numarkNV.pad_mode_num.auto:
            if (engine.getParameter(group, 'beatloop_' + loop_number[4] + '_enabled')) {
              engine.setParameter(group, 'loop_enabled', 0);
              engine.setParameter(group, 'beatloop_' + loop_number[4] + '_enabled', 0);
            } else {
              engine.setParameter(group, 'beatloop_' + loop_number[4] + '_activate', 1);
            }
            break;
          case numarkNV.pad_mode_num.loop:
            engine.setParameter('[Channel1]', 'reloop_exit', 0)
            break;
          case numarkNV.pad_mode_num.sampler:
            numarkNV.shiftedButtons.padActioncues('[Sampler' + numarkNV.padDeckNum(group) + ']', 4)
            break;
          case numarkNV.pad_mode_num.slicer:
            engine.setParameter(group, 'beatjump_' + loop_number[4] + '_backword', 1);
            break;
          default:
            numarkNV.shiftedButtons.padAction.cues(group, 1)
        }
      }
    },

    pad05: function pad05(channel, control, value, status, group) {
      if (value) {
        switch (numarkNV.pad_mode) {
          case numarkNV.pad_mode_num.cues:
            numarkNV.shiftedButtons.padAction.cues(group, 5);
            break;
          case numarkNV.pad_mode_num.auto:
            if (engine.getParameter(group, 'beatloop_' + loop_number[3] + '_enabled')) {
              engine.setParameter(group, 'loop_enabled', 0);
              engine.setParameter(group, 'beatloop_' + loop_number[3] + '_enabled', 0);
            } else {
              engine.setParameter(group, 'beatloop_' + loop_number[3] + '_activate', 1);
            }
            break;
          case numarkNV.pad_mode_num.loop:
            engine.setParameter('[Channel' + (deck + 2) + ']', 'loop_in', 0)
            break;
          case numarkNV.pad_mode_num.sampler:
            numarkNV.shiftedButtons.padActioncues('[Sampler' + (numarkNV.padDeckNum(group) + 2) + ']', 1)
            break;
          case numarkNV.pad_mode_num.slicer:
            engine.setParameter(group, 'beatjump_' + loop_number[3] + '_backword', 1);
            break;
          default:
            numarkNV.shiftedButtons.padAction.cues(group, 1)
        }
      }
    },

    pad06: function pad06(channel, control, value, status, group) {
      if (value) {
        switch (numarkNV.pad_mode) {
          case numarkNV.pad_mode_num.cues:
            numarkNV.shiftedButtons.padAction.cues(group, 6);
            break;
          case numarkNV.pad_mode_num.auto:
            if (engine.getParameter(group, 'beatloop_' + loop_number[2] + '_enabled')) {
              engine.setParameter(group, 'loop_enabled', 0);
              engine.setParameter(group, 'beatloop_' + loop_number[2] + '_enabled', 0);
            } else {
              engine.setParameter(group, 'beatloop_' + loop_number[2] + '_activate', 1);
            }
            break;
          case numarkNV.pad_mode_num.loop:
            engine.setParameter(group, 'loop_out', 0)
            break;
          case numarkNV.pad_mode_num.sampler:
            numarkNV.shiftedButtons.padActioncues('[Sampler' + (numarkNV.padDeckNum(group) + 2) + ']', 2)
            break;
          case numarkNV.pad_mode_num.slicer:
            engine.setParameter(group, 'beatjump_' + loop_number[2] + '_backword', 1);
            break;
          default:
            numarkNV.shiftedButtons.padAction.cues(group, 6)
        }
      }
    },

    pad07: function pad07(channel, control, value, status, group) {
      if (value) {
        switch (numarkNV.pad_mode) {
          case numarkNV.pad_mode_num.cues:
            numarkNV.shiftedButtons.padAction.cues(group, 7);
            break;
          case numarkNV.pad_mode_num.auto:
            if (engine.getParameter(group, 'beatloop_' + loop_number[1] + '_enabled')) {
              engine.setParameter(group, 'loop_enabled', 0);
              engine.setParameter(group, 'beatloop_' + loop_number[1] + '_enabled', 0);
            } else {
              engine.setParameter(group, 'beatloop_' + loop_number[1] + '_activate', 1);
            }
            break;
          case numarkNV.pad_mode_num.loop:
            engine.setParameter(group, 'reloop_exit', 0)
            break;
          case numarkNV.pad_mode_num.sampler:
            numarkNV.shiftedButtons.padActioncues('[Sampler' + (numarkNV.padDeckNum(group) + 2) + ']', 3)
            break;
          case numarkNV.pad_mode_num.slicer:
            engine.setParameter(group, 'beatjump_' + loop_number[1] + '_backword', 1);
            break;
          default:
            numarkNV.shiftedButtons.padAction.cues(group, 7)
        }
      }
    },

    pad08: function pad08(channel, control, value, status, group) {
      if (value) {
        switch (numarkNV.pad_mode) {
          case numarkNV.pad_mode_num.cues:
            numarkNV.shiftedButtons.padAction.cues(group, 8);
            break;
          case numarkNV.pad_mode_num.auto:
            if (engine.getParameter(group, 'beatloop_' + loop_number[0] + '_enabled')) {
              engine.setParameter(group, 'loop_enabled', 0);
              engine.setParameter(group, 'beatloop_' + loop_number[0] + '_enabled', 0);
            } else {
              engine.setParameter(group, 'beatloop_' + loop_number[0] + '_activate', 1);
            }
            break;
          case numarkNV.pad_mode_num.loop:
            engine.setParameter(group, 'reloop_exit', 0)
            break;
          case numarkNV.pad_mode_num.sampler:
            numarkNV.shiftedButtons.padActioncues('[Sampler' + (numarkNV.padDeckNum(group) + 2) + ']', 4)
            break;
          case numarkNV.pad_mode_num.slicer:
            engine.setParameter(group, 'beatjump_' + loop_number[0] + '_backword', 1);
            break;
          default:
            numarkNV.shiftedButtons.padAction.cues(group, 8)
        }
      }
    }
  },

  padAction: {
    cues: function cues(group, padnumber) {
      if (engine.getParameter(group, 'hotcue_' + padnumber + '_enabled')) {
        engine.setParameter(group, 'hotcue_' + padnumber + '_clear', 1)
      }
    }
  }
};
/* ----------------------------------------------------------------------------- */

/* = PAD Mode Section */
/* == PAD mode change handlers */
numarkNV.padmode = {

  changeCues: function changeCues(channel, control, value, status, group) {
    if (value) {
      numarkNV.padmode = numarkNV.pad_mode_num.cues;
    }
  },

  changeAuto: function changeAuto(channel, control, value, status, group) {
    if (value) {
      numarkNV.padmode = numarkNV.pad_mode_num.auto;
    }
  },

  changeLoop: function changeLoop(channel, control, value, status, group) {
    if (value) {
      numarkNV.padmode = numarkNV.pad_mode_num.loop;
    }
  },

  changeSampler: function changeSampler(channel, control, value, status, group) {
    if (value) {
      numarkNV.padmode = numarkNV.pad_mode_num.sampler;
    }
  },

  changeSlicer: function changeSlicer(channel, control, value, status, group) {
    if (value) {
      numarkNV.padmode = numarkNV.pad_mode_num.slicer;
    }
  }
};


numarkNV.shiftButton = function shiftButton(channel, control, value, status, group) {
  // This function mapping to Shift button
  if (!value) return
  // disConnectiedMapping
  for (i = 0; i < channelsString.length - 1; i++) {
    for (j = 0; j < connectedFunctions.length - 1; j++) {
      print('j is: ' + j);
      engine.connectControl(channelsString[i], 'unShiftedButtons.' + connectedFunctions[j], true);
    }
  }
  for (i = 0; i < channelsString.length - 1; i++) {
    for (j = 0; j < connectedFunctions.length - 1; j++) {
      engine.connectControl(channelsString[i], 'ShiftedButtons.' + connectedFunctions[j]);
    }
  }
  numarkNV.activeButtons = numarkNV.shiftedButton;
}

numarkNV.releaseShiftButton = function releaseShiftButton (channel, control, value, status, group) {
  if (value) return
  // disConnectiedMapping
  for (i = 0; i < channelsString.length - 1; i++) {
    for (j = 0; j < connectedFunctions.length - 1; j++) {
      engine.connectControl(channelsString[i], 'ShiftedButtons.' + connectedFunctions[j], true);
    }
  }
  for (i = 0; i < channelsString.length - 1; i++) {
    for (j = 0; j < connectedFunctions.length - 1; j++) {
      engine.connectControl(channelsString[i], 'unShiftedButtons.' + connectedFunctions[j]);
    }
  }
  numarkNV.activeButtons = numarkNV.unShiftedButtons;
}

/* = Mixxx section */
numarkNV.init = function (id, debugging) {
  numarkNV.activeButtons = numarkNV.unShiftedButtons;
  numarkNV.ActivuPadChanger = numarkNV.padmode
  midi.sendShortMsg(0xB0, 0x64, 0xFF) // Right Display change (maybe)
  // Serato sysex
  var ControllerStatusSysex = [0xF0, 0x00, 0x20, 0x7F, 0x03, 0x01, 0xF7];
  midi.sendSysexMsg(ControllerStatusSysex, ControllerStatusSysex.length);
}

numarkNV.shutdown = function shutdown() {};
