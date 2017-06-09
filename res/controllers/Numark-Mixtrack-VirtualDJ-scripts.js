// Numark Mixtrack (VirtualDJ) Mapping Script Functions
// Release 06/08/2017
// Scolari Mauricio mauricio@scolari.org
// http://mauricio.scolari.org
// 
// https://www.mixxx.org/wiki/doku.php/midi_scripting
// Debug function script.midiDebug()

var Mixtrack = {};
(function(root) {
    root.fname = 'Mixtrack';
    
    root.options = {
        'loopBackMode': true
    };
    
    root.groups = {
        'playlist': {
            'name': '[Playlist]'
        },
        'decks': [
            {
                'name': '[Channel1]',
                'number': 1
            },
            {
                'name': '[Channel2]',
                'number': 2
            }
        ]
    };
    
    root.hotcue_x = {
        // Deck 1
        0x5A: '1',
        0x5B: '2',
        0x5C: '3',
        
        // Deck 2
        0x5E: '1',
        0x5F: '2',
        0x60: '3'
    };
    root.hotcue_n = 3;
    
    root.blink = {
        'time_on': 500,
        'time_off': 500,
        'flash': false
    };
    
    root.loop_limit = {
        'halve': 0.03125,
        'double': 64
    };
    
    root.controls = {
        '[Playlist]': {
            'file': {
                'control': 0x72,
                'enabled': false
            },
            'folder': {
                'control': 0x73,
                'enabled': false
            },
            'SelectTrackKnob': {
                'control': 0x1A,
                'value': null
            },
            'BrowseSwitch': {
                'control': 0x4f,
                'value': null,
                'enabled': ''
            },
            'browseBack': {
                'control': null,
                'value': null,
                'enabled': false
            }
        },
        
        '[Channel1]': {
            'hotcue_delete': {
                'control': 0x59,
                'value': null,
                'enabled': false
            },
            'hotcue_1': {
                'control': 0x5A,
                'value': null,
                'enabled': false,
                'blink': false
            },
            'hotcue_2': {
                'control': 0x5B,
                'value': null,
                'enabled': false,
                'blink': false
            },
            'hotcue_3': {
                'control': 0x5C,
                'value': null,
                'enabled': false,
                'blink': false
            },
            'loop_manual': {
                'control': 0x61,
                'value': null,
                'enabled': false,
                'beatloop_size': 0,
                'blink': false
            },
            'loop_in': {
                'control': 0x53,
                'value': null,
                'blink': false
            },
            'loop_out': {
                'control': 0x54,
                'value': null,
                'blink': false,
                'timer_id': 0
            },
            'reloop_exit': {
                'control': 0x55,
                'value': null,
                'timer_id': 0
            },
            'scratch_enable': {
                'control': 0x48,
                'enabled': false
            },
            'keylock': {
                'control': 0x51,
                'enabled': false
            },
            'pfl': {
                'control': 0x65,
                'enabled': false
            },
            'jog': {
                'timer_id': 0
            },
            'cue_indicator': {
                'virtual': true,
                'value': null,
                'blink': false
            },
            'cue_default': {
                'control': 0x33,
                'value': null,
                'blink': false,
                'and_play': false
            },
            'play': {
                'control': 0x3b,
                'value': null,
                'blink': false,
                'stopposition': 0
            },
            'play_indicator': {
                'virtual': true,
                'value': null,
                'blink': false
            },
            'cue_goto': {
                'control': 0x4a,
                'value': null,
                'blink': false,
                'hot': 0
            },
            'beatsync': {
                'control': 0x40,
                'value': null,
                'beat_active_playposition_ms_min': 0,
                'beat_active_playposition_ms_max': 0
            },
            'LoadSelectedTrack': {
                'control': 0x4b,
                'value': null,
                'timer_id': 0
            }
        },
        
        '[Channel2]': {
            'hotcue_delete': {
                'control': 0x5D,
                'value': null,
                'enabled': false
            },
            'hotcue_1': {
                'control': 0x5E,
                'value': null,
                'enabled': false,
                'blink': false
            },
            'hotcue_2': {
                'control': 0x5F,
                'value': null,
                'enabled': false,
                'blink': false
            },
            'hotcue_3': {
                'control': 0x60,
                'value': null,
                'enabled': false,
                'blink': false
            },
            'loop_manual': {
                'control': 0x62,
                'value': null,
                'enabled': false,
                'beatloop_size': 0,
                'blink': false
            },
            'loop_in': {
                'control': 0x56,
                'value': null,
                'blink': false
            },
            'loop_out': {
                'control': 0x57,
                'value': null,
                'blink': false,
                'timer_id': 0
            },
            'reloop_exit': {
                'control': 0x58,
                'value': null,
                'timer_id': 0
            },
            'scratch_enable': {
                'control': 0x50,
                'enabled': false
            },
            'keylock': {
                'control': 0x52,
                'enabled': false
            },
            'pfl': {
                'control': 0x66,
                'enabled': false
            },
            'jog': {
                'timer_id': 0
            },
            'cue_default': {
                'control': 0x3c,
                'value': null,
                'blink': false,
                'and_play': false
            },
            'cue_indicator': {
                'virtual': true,
                'value': null,
                'blink': false
            },
            'play': {
                'control': 0x42,
                'value': null,
                'blink': false,
                'stopposition': 0
            },
            'play_indicator': {
                'virtual': true,
                'value': null,
                'blink': false
            },
            'cue_goto': {
                'control': 0x4c,
                'value': null,
                'blink': false,
                'hot': 0
            },
            'beatsync': {
                'control': 0x47,
                'value': null,
                'beat_active_playposition_ms_min': 0,
                'beat_active_playposition_ms_max': 0
            },
            'LoadSelectedTrack': {
                'control': 0x34,
                'value': null,
                'timer_id': 0
            }
        },
        
        // [EffectRack1_EffectUnitX]
        '[EffectRack1_EffectUnit1]': {
            'effectEnabled': {
                'control': 0x63,
                'enabled': false
            }
        },
        '[EffectRack1_EffectUnit2]': {
            'effectEnabled': {
                'control': 0x64,
                'enabled': false
            }
        }
    };
        
        
    // Called when the MIDI device is opened & set up
    root.init = function(id) {
        this.shutdown();
        
        for (var k = 0; k < this.groups['decks'].length; k++) {
            var group = this.groups['decks'][k]['name'];
            var deck = this.groups['decks'][k]['number'];
            
            // Hotcue's
            // Add event listeners
            for (var x = 1; x <= this.hotcue_n; x++) {
                engine.connectControl(
                    group, 
                    'hotcue_'+ x +'_enabled', 
                    this.fname + '.hotcue_X_enabled'
                );
            }
            
            // Key's on
            if (deck == 1) {
                // Deck 1 only
                this.key(group, 'pfl', true, true);
            }
            this.key(group, 'keylock', true, true);
            
            // LED's on only
            this.key(group, 'scratch_enable', true, false);
            this.key(group, 'loop_manual', true, false);
            
            this.controls[group]['loop_manual']['beatloop_size'] = 1;
            engine.setValue(
                group, 
                'beatloop_' + this.controls[group]['loop_manual']['beatloop_size'] + '_enabled', 
                1
            );
        }
        
        
        // [Playlist]
        this.controls['[Playlist]']['file']['enabled'] = true;
        this.controls['[Playlist]']['BrowseSwitch']['enabled'] = 'file';
        this.controls['[Playlist]']['BrowseSwitch']['control'] = this.controls['[Playlist]']['file']['control'];
        this.led(
            this.controls['[Playlist]']['file']['control'], 
            this.controls['[Playlist]']['file']['enabled']
        );
        this.controls['[Playlist]']['browseBack']['enabled'] = true;
        
        // [EffectRack1_EffectUnitX]
        script.toggleControl('[EffectRack1_EffectUnit1]', 'enabled', 0);
        script.toggleControl('[EffectRack1_EffectUnit1]', 'group_[Headphone]_enable', 1);
        script.toggleControl('[EffectRack1_EffectUnit2]', 'enabled', 0);
        script.toggleControl('[EffectRack1_EffectUnit2]', 'group_[Headphone]_enable', 1);
        script.toggleControl('[EffectRack1_EffectUnit3]', 'enabled', 0);
        script.toggleControl('[EffectRack1_EffectUnit3]', 'group_[Headphone]_enable', 1);
        script.toggleControl('[EffectRack1_EffectUnit4]', 'enabled', 0);
        script.toggleControl('[EffectRack1_EffectUnit4]', 'group_[Headphone]_enable', 1);
        
        
        // Twinkle
        this.flash();
        
        this.beat_active();
    }


    // Called when the MIDI device is closed,
    // turn off all the lights
    root.shutdown = function(id) {
        var led_lowest = 0x30;
        var led_highest = 0x73;
        
        for (var control = led_lowest; control <= led_highest; control++) {
            root.led(control, false);
        }
    }
    
    
    root.clone = function(object) {
        return (JSON.parse(JSON.stringify(object)));
    }


    root.key = function(group, key, led, action) {
        root.controls[group][key]['enabled'] = led;
        
        if (root.controls[group][key]['virtual']) {
            engine.setValue(
                group, 
                key, 
                root.controls[group][key]['enabled'] ? 1 : 0
            );
        } else {
            root.led(
                root.controls[group][key]['control'],
                root.controls[group][key]['enabled']
            );
        }
            
        if (action) {
            engine.setValue(
                group, 
                key, 
                root.controls[group][key]['enabled'] ? 1 : 0
            );
        }
    }


    root.led = function(control, active) {
        midi.sendShortMsg(
            0x90, 
            control, 
            active ? 0x64 : 0x00
        );
    }


    root.flash = function() {
        root.blink['flash'] = !root.blink['flash'];
        
        var groups = Object.keys(root.controls);
        for (var i = 0; i < groups.length; i++) {
            var group = groups[i];
            var keys = Object.keys(root.controls[group]);
            
            
            for (var j = 0; j < keys.length; j++) {
                var key = keys[j];
                
                if (root.controls[group][key]['blink']) {
                    root.led(
                        root.controls[group][key]['control'], 
                        root.blink['flash']
                    );
                }
            }
        }
        
        var timer = 0;
        if (root.blink['flash']) {
            timer = root.blink['time_on'];
        } else {
            timer = root.blink['time_off'];
        }
        engine.beginTimer(
            timer, 
            root.fname + ".flash()", 
            true
        );
    }


    root.beat_active = function() {
        for (var k = 0; k < root.groups['decks'].length; k++) {
            var beat_active = false;
            var group = root.groups['decks'][k]['name'];
            
            if (!engine.getValue(group, 'play')) {
                if (engine.getValue(group, 'file_bpm') > 0) {
                    var beat_active_playposition_ms_min = root.controls[group]['beatsync']['beat_active_playposition_ms_min'];
                    var beat_active_playposition_ms_max = root.controls[group]['beatsync']['beat_active_playposition_ms_max'];
                
                    var playposition_ms = root.playposition_ms(group);
                    var diff = Math.abs(beat_active_playposition_ms_min - playposition_ms);
                    if (beat_active_playposition_ms_min != 0 && diff > 200) {
                        beat_active_playposition_ms_min = 0;
                        beat_active_playposition_ms_max = 0;
                    }
                    
                    if (engine.getValue(group, 'beat_active')) {
                        if (beat_active_playposition_ms_max == 0) {
                            if (beat_active_playposition_ms_min) {
                                beat_active_playposition_ms_max = playposition_ms;
                                
                                var min = 0;
                                if (beat_active_playposition_ms_min > beat_active_playposition_ms_max) {
                                    min = beat_active_playposition_ms_max;
                                    beat_active_playposition_ms_max = beat_active_playposition_ms_min;
                                    beat_active_playposition_ms_min = min;
                                }
                            } else {
                                beat_active_playposition_ms_min = playposition_ms;
                            }
                        } else {
                            if (playposition_ms < beat_active_playposition_ms_min) {
                                beat_active_playposition_ms_min = playposition_ms;
                            } else if (playposition_ms > beat_active_playposition_ms_max) {
                                beat_active_playposition_ms_max = playposition_ms;
                            }
                        }
                    }
                    
                    if (playposition_ms >= beat_active_playposition_ms_min && playposition_ms <= beat_active_playposition_ms_max) {
                        beat_active = true;
                    }
                
                    root.controls[group]['beatsync']['beat_active_playposition_ms_min'] = beat_active_playposition_ms_min;
                    root.controls[group]['beatsync']['beat_active_playposition_ms_max'] = beat_active_playposition_ms_max;
                }
            }
            
            root.led(
                root.controls[group]['beatsync']['control'], 
                beat_active
            );
        }
        
        engine.beginTimer(
            20, 
            root.fname + ".beat_active()", 
            true
        );
    }
    
    root.playposition_ms = function(group) {
        var duration_ms = engine.getValue(group, 'duration') * 1000;
        var playposition_ms = duration_ms * engine.getValue(group, 'playposition');
        playposition_ms = Math.round(playposition_ms);
        
        return playposition_ms;
    }
    
    
    root.deckCurrent = function(group) {
        var result = 0;
        
        for (var k = 0; k < root.groups['decks'].length; k++) {
            if (root.groups['decks'][k]['name'] == group) {
                result = root.groups['decks'][k]['number'];
                break;
            }
        }
        
        return result;
    }


    root.SelectTrackKnob = function(channel, control, value, status, group) {
        root.controls[group]['SelectTrackKnob']['value'] = value;
        
        var direction = '';
        switch (value) {
            case 0x7F:
                direction = 'Next';
                break;
                
            case 0x01:
                direction = 'Prev';
                break;
        }
        
        var key_control = 'Select' + direction;
        switch (root.controls[group]['BrowseSwitch']['enabled']) {
            case 'file':
                key_control = key_control + 'Track';
                break;
                
            case 'folder':
                key_control = key_control + 'Playlist';
                break;
        }
        
        engine.setValue(group, key_control, 1);
    }

    root.BrowseSwitch = function(channel, control, value, status, group) {
        root.controls[group]['BrowseSwitch']['value'] = value;
        
        if (value) {
            root.browseSwitcher(group);
        }
    }

    root.browseSwitcher = function(group) {
        root.controls['[Playlist]']['file']['enabled'] = false;
        root.controls['[Playlist]']['folder']['enabled'] = false;
        
        var bba = 'file';
        switch (root.controls[group]['BrowseSwitch']['enabled']) {
            case 'file':
                bba = 'folder';
                break;
                
            case 'folder':
                bba = 'file';
                break;
        }
        
        root.controls[group][bba]['enabled'] = true;
        root.controls[group]['BrowseSwitch']['control'] = root.controls[group][bba]['control'];
        root.controls[group]['BrowseSwitch']['enabled'] = bba;
                
        root.led(
            root.controls['[Playlist]']['file']['control'], 
            root.controls['[Playlist]']['file']['enabled']
        );
        root.led(
            root.controls['[Playlist]']['folder']['control'], 
            root.controls['[Playlist]']['folder']['enabled']
        );
    }

    root.browseBack = function(channel, control, value, status, group) {
        switch (root.controls[group]['BrowseSwitch']['enabled']) {
            case 'file':
                if (value) {
                    root.browseSwitcher(group);
                    root.controls[group]['browseBack']['enabled'] = false;
                }
                break;
                
            case 'folder':
                if (root.controls[group]['browseBack']['enabled']) {
                    script.toggleControl(group, 'ToggleSelectedSidebarItem');
                } else {
                    root.controls[group]['browseBack']['enabled'] = true;
                }
                break;
        }
        
    }


    root.loopManual = function(channel, control, value, status, group) {
        if (value) {
            root.controls[group]['loop_manual']['blink'] = false;
                
            root.key(
                group, 
                'loop_manual', 
                !root.controls[group]['loop_manual']['enabled'], 
                false
            );
            
            if (root.controls[group]['loop_manual']['enabled']) {
                if (value) {
                    var loop_enabled = engine.getValue(group, 'loop_enabled');
                    
                    root.controls[group]['loop_in']['blink'] = loop_enabled;
                    root.controls[group]['loop_out']['blink'] = loop_enabled;
                    
                    root.led(
                        root.controls[group]['reloop_exit']['control'], 
                        loop_enabled
                    );
                }
            } else {
                if (value) {
                    root.controls[group]['loop_manual']['blink'] = true;
                    root.controls[group]['loop_in']['blink'] = false;
                    root.controls[group]['loop_out']['blink'] = false;
                    
                    root.led(
                        root.controls[group]['loop_in']['control'], 
                        true
                    );
                    root.led(
                        root.controls[group]['loop_out']['control'], 
                        engine.getValue(group, 'loop_enabled')
                    );
                    root.led(
                        root.controls[group]['reloop_exit']['control'], 
                        true
                    );
                }
            }
        }
    }
    
    root.loopManualClear = function(group) {
        root.controls[group]['loop_in']['blink'] = false;
        root.controls[group]['loop_out']['blink'] = false;
        
        engine.setValue(group, 'reloop_exit', 0);
        engine.setValue(group, 'hotcue_36_clear', 1);
        engine.setValue(group, 'loop_start_position', 0);
        engine.setValue(group, 'loop_in', 0);
        engine.setValue(group, 'loop_end_position', 0);
        engine.setValue(group, 'loop_out', 0);
        
        root.led(
            root.controls[group]['loop_in']['control'], 
            false
        );
        root.led(
            root.controls[group]['loop_out']['control'], 
            false
        );
        root.led(
            root.controls[group]['reloop_exit']['control'], 
            false
        );
    }

    root.loopIn = function(channel, control, value, status, group) {
        if (root.controls[group]['loop_manual']['enabled']) {
            if (value) {
                if (engine.getValue(group, 'loop_enabled')) {
                    engine.setValue(group, 'hotcue_36_goto', 1);
                } else {
                    engine.setValue(group, 'loop_in', 1);
                    engine.setValue(group, 'hotcue_36_set', 1);
                    root.led(
                        root.controls[group]['loop_in']['control'], 
                        true
                    );
                }
            }
        } else {
            if (root.controls[group]['loop_manual']['beatloop_size'] == root.loop_limit['double']) {
                if (value) {
                    engine.setValue(
                        group, 
                        'loop_double', 
                        0
                    );
                }
            }
            
            var beatloop_size = root.controls[group]['loop_manual']['beatloop_size'] * 0.5;
            if (beatloop_size >= root.loop_limit['halve']) {
                if (value) {
                    engine.setValue(
                        group, 
                        'beatloop_' + root.controls[group]['loop_manual']['beatloop_size'] + '_enabled', 
                        0
                    );
                    engine.setValue(
                        group, 
                        'beatloop_' + beatloop_size + '_enabled', 
                        1
                    );
                    root.controls[group]['loop_manual']['beatloop_size'] = beatloop_size;
                }
                script.toggleControl(group, 'loop_halve');
            }
        }
    }

    root.loopOut = function(channel, control, value, status, group) {
        if (root.controls[group]['loop_manual']['enabled']) {
            if (value) {
                if (engine.getValue(group, 'loop_in')) {
                    if (!engine.getValue(group, 'loop_enabled')) {
                        engine.setValue(group, 'loop_out', 1);
                        
                        var loop_diff_position = engine.getValue(group, 'loop_end_position') - engine.getValue(group, 'loop_start_position');
                        if (loop_diff_position <= 300) {
                            root.loopManualClear(group);
                            engine.setValue(
                                group, 
                                'reloop_exit', 
                                (loop_diff_position > 0) ? 1 : 0
                            );
                        } else {
                            root.led(root.controls[group]['loop_out']['control'], true);
                        }
                    } else {
                        engine.setValue(group, 'reloop_exit', 1);
                    }
                    
                    var loop_enabled = engine.getValue(group, 'loop_enabled');
                    root.controls[group]['loop_in']['blink'] = loop_enabled;
                    root.controls[group]['loop_out']['blink'] = loop_enabled;
                    root.led(root.controls[group]['reloop_exit']['control'], loop_enabled);
                }
            }
        } else {
            if (value) {
                var beatloop_size = root.controls[group]['loop_manual']['beatloop_size'];
                
                if (root.options['loopBackMode'] && engine.getValue(group, 'play') && !engine.getValue(group, 'loop_enabled') && beatloop_size > 2) {
                    engine.stopTimer(root.controls[group]['loop_out']['timer_id']);
                
                    engine.setValue(
                        group, 
                        'beatjump', 
                        beatloop_size * -1
                    );
                    
                    var playposition = engine.getValue(group, 'playposition');
                    root.controls[group]['loop_out']['timer_id'] = engine.beginTimer(
                        20, 
                        root.fname + ".loopAutoPlaying('" + root.fname + "', " + playposition + ", '" + group + "', " + beatloop_size + ")", 
                        true
                    );
                } else {
                    root.loopAuto(group);
                }
            }
        }
    }
    
    root.loopAutoPlaying = function(fname, playposition, group, beatloop_size) {
        if (engine.getValue(group, 'playposition') >= playposition) {
            root.controls[group]['loop_out']['timer_id'] = engine.beginTimer(
                20, 
                fname + ".loopAutoPlaying('" + fname + "', " + playposition + ", '" + group + "', " + beatloop_size + ")", 
                true
            );
        } else {
            root.loopAuto(group);
        }
    }
    
    root.loopAuto = function(group, beatloop_size) {
        var beatloop_size = root.controls[group]['loop_manual']['beatloop_size'];
        
        engine.setValue(
            group, 
            'beatloop_' + beatloop_size + '_toggle', 
            1
        );
        root.led(
            root.controls[group]['loop_out']['control'], 
            engine.getValue(group, 'loop_enabled')
        );
        engine.setValue(
            group, 
            'beatloop_' + beatloop_size + '_enabled', 
            1
        );
    }

    root.reloopExit = function(channel, control, value, status, group) {
        if (root.controls[group]['loop_manual']['enabled']) {
            if (engine.getValue(group, 'loop_start_position') > 0) {
                if (value) {
                    var loop_enabled = engine.getValue(group, 'loop_enabled');
                    
                    if (loop_enabled) {
                        engine.setValue(group, 'reloop_exit', 0);
                        engine.setValue(group, 'loop_end_position', -1);
                        engine.setValue(group, 'loop_out', 0);
                        root.led(
                            root.controls[group]['loop_out']['control'], 
                            false
                        );
                    } else {
                        if (!(engine.getValue(group, 'loop_end_position') >= 0)) {
                            var play = engine.getValue(group, 'play');
                            engine.setValue(
                                group, 
                                'hotcue_36_gotoandstop', 
                                1
                            );
                            root.controls[group]['reloop_exit']['timer_id'] = engine.beginTimer(
                                40, 
                                root.fname + ".reloopExitInOnly('" + group + "', " + play + ")", 
                                true
                            );
                        } else {
                            engine.setValue(group, 'reloop_exit', !loop_enabled);
                        }
                    }
                    
                    
                    loop_enabled = !loop_enabled;
                    root.controls[group]['loop_in']['blink'] = loop_enabled;
                    root.controls[group]['loop_out']['blink'] = loop_enabled;
                    
                    root.led(
                        root.controls[group]['loop_in']['control'], 
                        true
                    );
                    root.led(
                        root.controls[group]['reloop_exit']['control'], 
                        loop_enabled
                    );
                }
            }
        } else {
            if (root.controls[group]['loop_manual']['beatloop_size'] == root.loop_limit['halve']) {
                if (value) {
                    engine.setValue(
                        group, 
                        'loop_halve', 
                        0
                    );
                }
            }
            
            var beatloop_size = root.controls[group]['loop_manual']['beatloop_size'] * 2;
            if (beatloop_size <= root.loop_limit['double']) {
                if (value) {
                    engine.setValue(
                        group, 
                        'beatloop_' + root.controls[group]['loop_manual']['beatloop_size'] + '_enabled', 
                        0
                    );
                    engine.setValue(
                        group, 
                        'beatloop_' + beatloop_size + '_enabled', 
                        1
                    );
                    root.controls[group]['loop_manual']['beatloop_size'] = beatloop_size;
                }
                script.toggleControl(group, 'loop_double');
            }
        }
    }
    
    root.reloopExitInOnly = function(group, play) {
        engine.stopTimer(root.controls[group]['reloop_exit']['timer_id']);
        
        if (engine.getValue(group, 'play')) {
            root.controls[group]['reloop_exit']['timer_id'] = engine.beginTimer(
                40, 
                root.fname + ".reloopExitInOnly('" + group + "', " + play + ")", 
                true
            );
        } else {
            engine.setValue(
                group, 
                'beatloop_' + root.controls[group]['loop_manual']['beatloop_size'] + '_activate', 
                1
            );
            if (play) {
                engine.setValue(group, 'play', 1);
            }
        }
    }


    root.scratch_enable = function(channel, control, value, status, group) {
        if (value) {
            root.key(
                group, 
                'scratch_enable', 
                !root.controls[group]['scratch_enable']['enabled'], 
                false
            );
        }
    }

    root.scratch = function(channel, control, value, status, group) {
        if (root.controls[group]['scratch_enable']['enabled']) {
            if (value) {
                engine.stopTimer(root.controls[group]['jog']['timer_id']);
                
                var intervals_per_rev = 128;
                var rpm = 33 + (1 / 3);
                var alpha = 1.0 / 8;
                var beta = alpha / 32;
                engine.scratchEnable(
                    root.deckCurrent(group), 
                    intervals_per_rev, 
                    rpm, 
                    alpha, 
                    beta
                );
            } else {
                engine.scratchDisable(root.deckCurrent(group));
            }
        }
    }

    root.jog = function(channel, control, value, status, group) {
        var tick = 1;
        if (value >= 64) {
            tick = -1;
            value = value - 128;
        }
        
        if (engine.isScratching(root.deckCurrent(group))) {
            engine.scratchTick(root.deckCurrent(group), tick);
        } else {
            if (engine.getValue(group, 'play')) {
                engine.setValue(group, 'jog', value);
            } else {
                engine.stopTimer(root.controls[group]['jog']['timer_id']);
                
                var intervals_per_rev = 128;
                var rpm = 33 * (8 / (value * tick)) + (1 / 3);
                var alpha = 1.0 / 8;
                var beta = alpha / 32;
                engine.scratchEnable(
                    root.deckCurrent(group), 
                    intervals_per_rev, 
                    rpm, 
                    alpha, 
                    beta
                );
                
                var deck = root.deckCurrent(group);
                engine.scratchTick(deck, tick);
                root.controls[group]['jog']['timer_id'] = engine.beginTimer(
                    100, 
                    "engine.scratchDisable(" + deck + ")", 
                    true
                );
            }
        }
    }
    
    
    root.LoadSelectedTrack = function(channel, control, value, status, group) {
        script.toggleControl(group, 'LoadSelectedTrack');
        
        if (value) {
            engine.stopTimer(root.controls[group]['LoadSelectedTrack']['timer_id']);
            root.controls[group]['LoadSelectedTrack']['timer_id'] = engine.beginTimer(
                500, 
                root.fname + ".LoadSelectedTrackAndCue('" + group + "')",  
                true
            );
        }
    }
    
    root.LoadSelectedTrackAndCue = function(group) {
        if (!(engine.getValue(group, 'cue_point') > 0)) {
            engine.setValue(group, 'playposition', 0.0000001);
            engine.setValue(group, 'cue_set', 1);
            
            root.controls[group]['LoadSelectedTrack']['timer_id'] = engine.beginTimer(
                100, 
                root.fname + ".LoadSelectedTrackAndCue('" + group + "')",  
                true
            );
        } else {
            root.controls[group]['cue_goto']['hot'] = 0;
            root.cue_default_ready(group, true);
            
            for (var k = 0; k < root.groups['decks'].length; k++) {
                engine.setValue(
                    root.groups['decks'][k]['name'], 
                    'pfl', 
                    0
                );
            }
            engine.setValue(group, 'pfl', 1);
            
            if (root.controls[group]['loop_manual']['enabled']) {
                root.loopManualClear(group);
            }
            engine.setValue(
                group, 
                'beatloop_' + root.controls[group]['loop_manual']['beatloop_size'] + '_enabled', 
                1
            );
            
            var group_effect = '[EffectRack1_EffectUnit' + root.deckCurrent(group) + ']'
            root.effectAppliesClear(group_effect);
            engine.setValue(
                group_effect, 
                'group_[Headphone]_enable', 
                1
            );
        }
    }
    
    
    root.cue_default = function(channel, control, value, status, group) {
        root.controls[group]['cue_default']['value'] = value;
        
        if (value) {
            root.controls[group]['cue_default']['and_play'] = false;
            
            if (root.controls[group]['cue_goto']['value']) {
                engine.setValue(group, 'cue_gotoandstop', 1);
                root.cue_default_ready(group, true);
                return null;
            }
        }
        
        if (engine.getValue(group, 'play')) {
            if (value) {
                engine.setValue(group, 'cue_gotoandstop', 1);
                engine.beginTimer(
                    100, 
                    root.fname + ".cue_default_on_playing('" + group + "')",  
                    true
                );
            } else {
                engine.setValue(group, 'cue_default', 0);
                
                if (!root.controls[group]['cue_default']['and_play']) {
                    root.key(group, 'cue_goto', false, false);
                    root.cue_default_ready(group, true);
                } else {
                    root.cue_default_ready(group, false);
                }
            }
        } else {
            if (value) {
                engine.setValue(group, 'cue_default', 1);
                root.cue_default_ready(group, false);
            } else {
                engine.setValue(group, 'cue_default', 0);
                root.cue_default_ready(group, true);
            }
        }
    }
    
    root.cue_default_ready = function(group, active) {
        var active_inv = !active;
        
        root.controls[group]['cue_default']['blink'] = active;
        root.controls[group]['play']['blink'] = active;
        root.controls[group]['play_indicator']['blink'] = active;
        
        root.key(group, 'cue_default', false, false);
        root.key(group, 'cue_indicator', active, false);
        root.key(group, 'play', active_inv, false);
        root.key(group, 'play_indicator', active_inv, false);
        root.key(group, 'cue_goto', active_inv, false);
    }
    
    root.cue_default_on_playing = function(group) {
        engine.setValue(group, 'cue_set', 1);
    }
    
    
    root.play = function(channel, control, value, status, group) {
        if (root.controls[group]['cue_default']['value']) {
            if (value) {
                root.controls[group]['cue_default']['and_play'] = true;
                engine.setValue(group, 'play', 0);
            }
        } else {
            if (engine.getValue(group, 'play')) {
                if (value) {
                    engine.setValue(group, 'play', 0);
                    root.controls[group]['play']['stopposition'] = engine.getValue(group, 'playposition');
                    
                    root.controls[group]['cue_default']['blink'] = true;
                    root.controls[group]['play']['blink'] = true;
                    root.controls[group]['play_indicator']['blink'] = true;
                    
                    // TODO: It's not work like VirtualDJ and no there are way for now
                    engine.setValue(group, 'cue_indicator', 0);
                }
            } else {
                root.playgoto_1(value, group);
            }
        }
        
        root.playgoto_2(group);
    }
    
    root.playgoto_1 = function(value, group) {
        if (value) {
            engine.setValue(group, 'play', 1);
            root.cue_default_ready(group, false);
        }
    }    
    
    root.playgoto_2 = function(group) {
        var play = engine.getValue(group, 'play');
        root.key(group, 'play', play, false);
        root.key(group, 'cue_goto', play, false);
    }    
    
    root.cue_goto = function(channel, control, value, status, group) {
        root.controls[group]['cue_goto']['value'] = value;
        
        if (engine.getValue(group, 'play')) {
            if (value) {
                if (root.controls[group]['cue_default']['value']) {
                    root.controls[group]['cue_default']['and_play'] = true;
                    engine.setValue(group, 'play', 0);
                } else {
                    if (root.controls[group]['cue_goto']['hot']) {
                        var x = root.controls[group]['cue_goto']['hot'];
                        var hotcue_x_goto = 'hotcue_' + x + '_goto';
                        engine.setValue(group, hotcue_x_goto, 1);
                    } else {
                        if (root.controls[group]['play']['stopposition']) {
                            engine.setValue(
                                group, 
                                'playposition', 
                                root.controls[group]['play']['stopposition']
                            );
                        } else {
                            engine.setValue(group, 'cue_goto', 1);
                        }
                    }
                }
            }
        } else {
            root.playgoto_1(value, group);
        }
        
        root.playgoto_2(group);
    }
    
    
    root.beatsync = function(channel, control, value, status, group) {
        root.controls[group]['beatsync']['value'] = value;
        script.toggleControl(group, 'beatsync');
    }


    root.keylock = function(channel, control, value, status, group) {
        if (value) {
            script.toggleControl(group, 'keylock');
            root.key(
                group, 
                'keylock', 
                engine.getValue(group, 'keylock')
            );
        }
    }


    root.hotcue_X_enabled = function(value, group, control) {
        var x = control[7];
        var key = 'hotcue_' + x;
        
        root.controls[group][key]['value'] = value;
        root.controls[group][key]['enabled'] = value ? true : false;
        root.led(
            root.controls[group][key]['control'], 
            root.controls[group][key]['enabled']
        );
    }

    root.hotcue_X_toggle = function(channel, control, value, status, group) {
        var x = root.hotcue_x[control];
        var hotcue_x = 'hotcue_' + x;
        var control_name = '';

        // hotcue_X_enabled called automatically
        if (root.controls[group]['hotcue_delete']['enabled']) {
            control_name = hotcue_x + '_enabled';
            if (engine.getValue(group, control_name)){
                root.controls[group][hotcue_x]['blink'] = false;
                control_name = hotcue_x + '_clear';
                engine.setValue(group, control_name, 1);
                if (root.controls[group]['cue_goto']['hot'] == x) {
                    root.controls[group]['cue_goto']['hot'] = 0;
                }
            }
        } else {
            control_name = hotcue_x + '_enabled';
            if (engine.getValue(group, control_name)) {
                control_name = hotcue_x + '_gotoandplay';
            } else {
                control_name = hotcue_x + '_activate';
            }
            
            if (value) {
                root.controls[group]['cue_goto']['hot'] = x;
                engine.setValue(group, control_name, 1);
                
                var gotoandplay = hotcue_x + '_gotoandplay';
                if (control_name == gotoandplay) {
                    root.cue_default_ready(group, false);
                }
                
                control_name = hotcue_x + '_position';
                engine.setValue(
                    group, 
                    'cue_point', 
                    engine.getValue(group, control_name)
                );
            } else{
                engine.setValue(group, control_name, 0);
            }
        }
    }

    root.hotcueDelete = function(channel, control, value, status, group) {
        root.controls[group]['hotcue_delete']['value'] = value;
        
        var enabled = false;
        if (value === 127) {
            // do something when this button is pressed
            enabled = true;
        }
            
        root.key(
            group, 
            'hotcue_delete', 
            enabled
        );
        
        for (var x = 1; x <= root.hotcue_n; x++) {
            var hotcue_x = 'hotcue_' + x;
            var hotcue_x_enabled = hotcue_x + '_enabled';
            if (engine.getValue(group, hotcue_x_enabled)) {
                if (enabled) {
                    root.controls[group][hotcue_x]['blink'] = enabled;
                } else {
                    root.controls[group][hotcue_x]['blink'] = false;
                    root.led(
                        root.controls[group][hotcue_x]['control'], 
                        true
                    );
                }
            }
        }
    }


    root.effectEnabled = function(channel, control, value, status, group) {
        if (value) {
            script.toggleControl(group, 'enabled');
            root.key(
                group, 
                'effectEnabled', 
                engine.getValue(group, 'enabled')
            );
        }
    }

    
    root.chain_selector = function(channel, control, value, status, group) {
        engine.setValue(group, 'enabled', 0);
        root.key(
            group, 
            'effectEnabled', 
            engine.getValue(group, 'enabled')
        );
        
        engine.setValue(
            group, 
            'chain_selector', 
            (value == 0x01) ? -1 : 1
        );
    }

    
    root.effectApplies = function(channel, control, value, status, group) {
        if (value) {
            var key_control = 'group_[Master]_enable';
            if (engine.getValue(group, 'group_[Master]_enable')) {
                key_control = 'group_[Headphone]_enable';
            }
            
            root.effectAppliesClear(group);
            engine.setValue(group, key_control, 1);
        }
    }
    
    
    root.effectAppliesClear = function(group) {
        engine.setValue(group, 'group_[Master]_enable', 0);
        engine.setValue(group, 'group_[Headphone]_enable', 0);
        engine.setValue(group, 'group_[Channel1]_enable', 0);
        engine.setValue(group, 'group_[Channel2]_enable', 0);
        engine.setValue(group, 'group_[Channel3]_enable', 0);
        engine.setValue(group, 'group_[Channel4]_enable', 0);
        engine.setValue(group, 'group_[Sampler1]_enable', 0);
        engine.setValue(group, 'group_[Sampler2]_enable', 0);
        engine.setValue(group, 'group_[Sampler3]_enable', 0);
        engine.setValue(group, 'group_[Sampler4]_enable', 0);
    }
})(Mixtrack);
