
function TrakProDJ() {}
TrakProDJ.id = "";
TrakProDJ.debug = false;

TrakProDJ.use_scratch = false;

TrakProDJ.deck1_browse_controls = {
    'track': false,
    'playlist': false,
};
TrakProDJ.deck2_browse_controls = {
    'track': false,
    'playlist': false,
};

TrakProDJ.deck1_scratch = {
    'active': false,
    'timer': false,
    'value': false,
};
TrakProDJ.deck2_scratch = {
    'active': false,
    'timer': false,
    'value': false,
};

TrakProDJ.ch1_buttons = {
    0x01: { 'group': '[Channel1]', 'item': 'play' },
    0x02: { 'group': '[Channel1]', 'item': 'cue_default' },
    0x03: { 'group': '[Channel1]', 'item': 'beatsync' },
    0x11: { 'group': '[Channel1]', 'item': 'pfl' },
    0x17: { 'group': '[Channel1]', 'item': 'filterLowKill' },
    0x18: { 'group': '[Channel1]', 'item': 'filterMidKill' },
    0x19: { 'group': '[Channel1]', 'item': 'filterHighKill' },
};
TrakProDJ.ch2_buttons = {
    0x01: { 'group': '[Channel2]', 'item': 'play' },
    0x02: { 'group': '[Channel2]', 'item': 'cue_default' },
    0x03: { 'group': '[Channel2]', 'item': 'beatsync' },
    0x11: { 'group': '[Channel2]', 'item': 'pfl' },
    0x17: { 'group': '[Channel2]', 'item': 'filterLowKill' },
    0x18: { 'group': '[Channel2]', 'item': 'filterMidKill' },
    0x19: { 'group': '[Channel2]', 'item': 'filterHighKill' },
};

TrakProDJ.init = function(id) {
    engine.connectControl("[Channel1]","play","TrakProDJ.play_status");
    engine.connectControl("[Channel2]","play","TrakProDJ.play_status");
};
TrakProDJ.shutdown = function(id) {};

TrakProDJ.LoadSelectedTrack = function(channel, control, value, status, group) {
    if (channel != 7) {
        print("TrakProDJ.LoadSelectedTrack: Unknown channel: " + channel);
        return
    };
    if (control == 0x1f) {
        var group = '[Channel1]';
    } else if (control == 0x20) {
        var group = '[Channel2]';
    } else {
        print("TrakProDJ.LoadSelectedTrack: Unknown control: " + control);
        return;
    }
    engine.setValue(group,'LoadSelectedTrack',1);
};

TrakProDJ.SelectTrackKnob = function(channel, control, value, status, group) {};

TrakProDJ.browse = function(channel, control, value, status, group) {
    var val = control*128 + value;
    switch (channel) {
        case 2:
            ctrl = TrakProDJ.deck1_browse_controls.playlist;
            if (ctrl == val) { return; };
            TrakProDJ.deck1_browse_controls.playlist = val;
            break;
        case 3:
            ctrl = TrakProDJ.deck1_browse_controls.track;
            if (ctrl == val) { return; };
            TrakProDJ.deck1_browse_controls.track = val;
            break
        case 4:
            ctrl = TrakProDJ.deck2_browse_controls.playlist;
            if (ctrl == val) { return; };
            TrakProDJ.deck2_browse_controls.playlist = val;
            break;
        case 5:
            ctrl = TrakProDJ.deck2_browse_controls.track;
            if (ctrl == val) { return; };
            TrakProDJ.deck2_browse_controls.track = val;
            break;
        default:
            print('browse: Unknown channel ' + channel);
            return;
    }
    if (ctrl==false) {
        change = 0;
    } else if (ctrl > val) {
        change = -1;
    } else {
        change = 1;
    }
    print(channel + ' old ' + ctrl + ' new ' + val);
    if (change==0) {
        return;
    };
    if (channel==3 || channel==5) {
        if (change == -1) {
            engine.setValue('[Playlist]','SelectPrevTrack',true);
        } else if (change == 1 ) {
            engine.setValue('[Playlist]','SelectNextTrack',true);
        }
    }
    // I don't actually want to enable playlist control for now
    return;
    if (channel==2 || channel==4) {
        if (change == -1) {
            engine.setValue('[Playlist]','SelectPrevPlaylist',true);
        } else if (change == 1 ) {
            engine.setValue('[Playlist]','SelectNextPlaylist',true);
        }
    }
};

TrakProDJ.button = function(channel, control, value, status, group) {
    if (channel == 0) {
        var cmap = TrakProDJ.ch1_buttons[control];
    } else if (channel == 1) {
        var cmap = TrakProDJ.ch2_buttons[control];

    } else {
        print ("Unknown channel: " + channel);
        return;
    }
    if (engine.getValue(cmap.group,cmap.item)==0) {
        engine.setValue(cmap.group,cmap.item,1);
    } else {
        engine.setValue(cmap.group,cmap.item,0);
    }
};

TrakProDJ.hotcue = function(channel, control, value, status, group) {
    if ( control != 0x1e ) {
        print ('Unknown hotcue control code ' + control);
        return;
    }
    channel = channel+1;
    if (channel >=1 && channel<=4) {
        var group = '[Channel1]';
    } else if (channel>=5 && channel<=8) {
        var group = '[Channel2]';
        channel = channel-4;
    } else {
        print ('Unknown hotcue control channel ' + channel);
        return;
    }
    var hotcue = "hotcue_" + channel + "_activate";
    engine.setValue(group,hotcue,1);
};

TrakProDJ.play_status = function() {
    if (engine.getValue('[Channel1]','play')==0) {
        TrakProDJ.jog_disable(1);
    }
    if (engine.getValue('[Channel2]','play')==0) {
        TrakProDJ.jog_disable(2);
    }
}

TrakProDJ.jog_enable = function(deck) {
    switch (deck) {
        case 1:
            group = '[Channel1]'
            ctrl = TrakProDJ.deck1_scratch;
            break;
        case 2:
            group = '[Channel2]'
            ctrl = TrakProDJ.deck2_scratch;
            break;
        default:
            print('enable_scratch: Unknown deck value ' + deck);
            return;
    }

    ctrl.active = true;
    if (!ctrl.timer) {
        if (TrakProDJ.use_scratch) {
            print(group + ' enable scratch');
            ctrl.timer = engine.beginTimer(20,"TrakProDJ.jog_scratch_timer(deck)");
        } else {
            print(group + ' enable rate bend');
            ctrl.timer = engine.beginTimer(20,"TrakProDJ.jog_rate_timer(deck)");
        }
    }
    if (TrakProDJ.use_scratch) {
        engine.scratchEnable(deck,128,33+1/3,1.0/8,(1.0/8)/32);
    }
}

TrakProDJ.jog_disable = function(deck) {
    switch (deck) {
        case 1:
            group = '[Channel1]';
            ctrl = TrakProDJ.deck1_scratch;
            break;
        case 2:
            group = '[Channel2]'
            ctrl = TrakProDJ.deck2_scratch;
            break;
        default:
            print('disable_scratch: Unknown deck value ' + deck);
            return;
    }
    if (ctrl.timer) {
        engine.stopTimer(ctrl.timer);
    }
    if (TrakProDJ.use_scratch) {
        engine.scratchDisable(deck);
    } else {
        engine.setValue(group,'rate_temp_down',false);
        engine.setValue(group,'rate_temp_up',false);
    }
    ctrl.value = false;
    ctrl.timer = false;
    ctrl.active = false;
}

TrakProDJ.jog_rate_timer = function(deck) {
    if (TrakProDJ.use_scratch) { return; }
    switch (deck) {
        case 1:
            group = '[Channel1]'
            ctrl = TrakProDJ.deck1_scratch;
            break;
        case 2:
            group = '[Channel2]'
            ctrl = TrakProDJ.deck2_scratch;
            break;
        default:
            print('jog_rate_timer: Unknown deck value ' + deck);
            return;
    }
    if (ctrl.active) {
        ctrl.active = false;
        return;
    }
    print(group + ' reset rate bend');
    TrakProDJ.jog_disable(deck);
}

TrakProDJ.jog_scratch_timer = function(deck) {
    if (!TrakProDJ.use_scratch) { return; }
    switch (deck) {
        case 1:
            group = '[Channel1]';
            ctrl = TrakProDJ.deck1_scratch;
            break;
        case 2:
            group = '[Channel2]';
            ctrl = TrakProDJ.deck2_scratch;
            break;
        default:
            print('jog_scratch_timer: Unknown deck value ' + deck);
            return;
    }
    if (ctrl.active) {
        ctrl.active = false;
        return;
    }
    print(group + ' disable scratch');
    TrakProDJ.jog_disable(deck);
}

TrakProDJ.jog = function(channel, control, value, status, group) {
    var val = control*128 + value;
    var change = false;
    switch (channel) {
        case 0:
            group = '[Channel1]';
            deck = 1;
            ctrl = TrakProDJ.deck1_scratch;
            if (ctrl.value != false && ctrl.value == val) { return; }
            break;
        case 1:
            group = '[Channel2]';
            deck = 2;
            ctrl = TrakProDJ.deck2_scratch;
            if (ctrl.value != false && ctrl.value == val) { return; }
            break;
        default:
            print('jog: Signal from unknown channel ' + channel);
            return;
    }
    if (!ctrl.active) {
        change = 0;
    } else if (ctrl.value > val) {
        change = -1;
    } else {
        change = 1;
    }
    ctrl.value = val;
    if (engine.getValue(group,'play') == 1) {
        if (change == 0) {
            TrakProDJ.jog_enable(deck);
        }
        if (change==-1) {
            ctrl.active = true;
            if (TrakProDJ.use_scratch) {
                engine.scratchTick(deck,-1);
            } else {
                print(group + ' rate_temp_down');
                engine.setValue(group,'rate_temp_down',true);
            }
        }
        if (change==1) {
            ctrl.active = true;
            if (TrakProDJ.use_scratch) {
                engine.scratchTick(deck,1);
            } else {
                print(group  + ' rate_temp_up');
                engine.setValue(group,'rate_temp_up',true);
            }
        }
    } else {
        if (change==0) {
            ctrl.active = true;
        }
        if (change==-1) {
            print('Seek back deck ' + deck);
            engine.setValue(group,'jog',-1);
        }
        if (change==1) {
            print('Seek fwd deck ' + deck);
            engine.setValue(group,'jog',1);
        }
    }
};

