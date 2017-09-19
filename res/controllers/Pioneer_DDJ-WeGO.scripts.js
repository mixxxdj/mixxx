//notes:
//this video has been helpful in going over the basics of the deck:
// https://www.youtube.com/watch?v=SmV1Yn3BGdU
//
// this vidoo shows how the lights are intended to be displayed
// https://www.youtube.com/watch?v=zishhAytC-Y
//
// unknown midi messages
// 0x9C 0x79 [0-127] - appears to reset the controller
// 0x9C 0x7B [0-127] - appears to reset the controller
//
// lights Deck A
// wheel lighting effects:
// ctrlA 0xB4, 0x00, nn:
//   FX3 0xB4, 0x06 [0|1-15|16-31|32-63|64-127] value makes a difference here.
//   FX2 0xB4, 0x04, [0|1-15|16-31|32-63|64-127] value makes a difference here.
//   FX1 0xB4, 0x02, [0|1-15|16-31|32-63|64-127] value makes a difference here.
// ctrlB 0xB4 0x07, nn - see below for details
//
//Deck C control codes same as above but
// ctrlA " , 0x08, "
// fx1   " , 0x0A, "
// fx2   " , 0x0C, "
// fx3   " , 0x0E, "
// ctrlB " , 0x0F, "
//
//Deck BD works the same with status channel 0x05
//
// ctrl[AB] details:
//  0-15 = blue with flashing red for a kind of purply cycling colour
// 16-31 = from black to red leading blue spiral ccw into 3/4 blue.
// 32-47 = from black to red leading blue spiral ccw into 1/2 blue.
// 48-63 = from black to red leading blue spiral ccw into 1/4 blue.
//    64 = green
// 65-79 = from black to red leading green spiral cw into 1/4 green.
// 80-95 = from black to red leading greeny yellow spiral cw into 1/2 greeny
// yellow.
// 96-111 = from black to red leading yellow spiral cw into 3/4 green.
// 112-115 = yellow with flashing orange
// 116-119 = less yellow more orange
// 120-124 = flashing orange.
//

function PioneerDDJWeGO() {}

PioneerDDJWeGO.message1 = [
'External Effects',
'================',
'JOG FX: events to use when linking up external software',
'on1 = turns the button light on',
'on2 = on1 and turns on the wheel lighting effects',
'p = 4|5 = Deck[AC]|Deck[BD]',
'n = [0-2]',
'DeckAB FX(n+1) light: send{ 0x9p, 0x43+n, [off:0x00|on1:0x01|on2:0x7F] }',
'              button: down{ 0x9p, 0x43+n,   0x7F },  up{ 0x8p, 0x43+n,   0x40 }',
'  [shift]     button: down{ 0x9p, 0x4D+n,   0x7F },  up{ 0x8p, 0x4D+n,   0x40 }',
'                spin:  MSB{ 0xBp, 0x02+n*2, 0x.. }, LSB{ 0xBp, 0x22+n*2, 0x.. }',
'  [shift]       spin:  MSB{ 0xBp, 0x12+n*2, 0x.. }, LSB{ 0xBp, 0x32+n*2, 0x.. }',
'',
'DeckCD FX(n+1) light: send{ 0x9p, 0x48+n, [off:0x00|on1:0x01|on2:0x7F] }',
'              button: down{ 0x9p, 0x48+n,   0x7F },  up{ 0x8p, 0x48+n,   0x40 }',
'  [shift]     button: down{ 0x9p, 0x52+n,   0x7F },  up{ 0x8p, 0x52+n,   0x40 }',
'                spin:  MSB{ 0xBp, 0x0A+n*2, 0x.. }, LSB{ 0xBp, 0x2A+n*2, 0x.. }',
'  [shift]       spin:  MSB{ 0xBp, 0x1A+n*2, 0x.. }, LSB{ 0xBp, 0x3A+n*2, 0x.. }',
];

PioneerDDJWeGO.settings = {
    'blinkingSync': true,
    'autoLoopSize': 32,
    'fxExternal': false,
// ctrl & fx modes: disabled=0x00, button light=0x01, effects:=0x7f
    'ctrlAmode': 0x7F,
    'ctrlBmode': 0x7F,
    'fxMode': 0x7F
};

PioneerDDJWeGO.init = function(id) {
    print( "Initialising Pioneer DDJ-WeGO controller mapping script");

    PioneerDDJWeGO.scratchSettings = {
        'alpha': 1.0 / 8,
        'beta': 1.0 / 8 / 32,
        'jogResolution': 480,
        'vinylSpeed': 33 + 1/3,
        'safeScratchTimeout': 20
    };

    // controller state
    PioneerDDJWeGO.activeDeck = [ true, true, false, false ]; // [A,B,C,D]
    // ctrlA,B and FX button states, 0x00, 0x01, 0x7F
    PioneerDDJWeGO.ctrlA = [0x00, 0x00, 0x00, 0x00];
    PioneerDDJWeGO.ctrlB = [0x00, 0x00, 0x00, 0x00];
    PioneerDDJWeGO.fx1 = [0x00, 0x00, 0x00, 0x00];
    PioneerDDJWeGO.fx2 = [0x00, 0x00, 0x00, 0x00];
    PioneerDDJWeGO.fx3 = [0x00, 0x00, 0x00, 0x00];

    //make mixxx match
    engine.setValue( '[QuickEffectRack1_[Channel1]_Effect1]', 'enabled', false );
    engine.setValue( '[QuickEffectRack1_[Channel2]_Effect1]', 'enabled', false );
    engine.setValue( '[QuickEffectRack1_[Channel3]_Effect1]', 'enabled', false );
    engine.setValue( '[QuickEffectRack1_[Channel4]_Effect1]', 'enabled', false );

    PioneerDDJWeGO.vuMeters = {
        '[Channel1]': 0,
        '[Channel2]': 0,
        '[Channel3]': 0,
        '[Channel4]': 0,
    };

    PioneerDDJWeGO.channelGroups = {
       '[Channel1]': 0x00,
       '[Channel2]': 0x01,
       '[Channel3]': 0x02,
       '[Channel4]': 0x03
    };

    PioneerDDJWeGO.jogfxGroups = {
        '[JogFX1]': 0x04,
        '[JogFX2]': 0x05,
    };

    PioneerDDJWeGO.samplerGroups = {
        '[Sampler1]': 0x00,
        '[Sampler2]': 0x00,
        '[Sampler3]': 0x00,
        '[Sampler4]': 0x00,
        '[Sampler5]': 0x01,
        '[Sampler6]': 0x01,
        '[Sampler7]': 0x01,
        '[Sampler8]': 0x01,
        '[Sampler9]': 0x02,
        '[Sampler10]': 0x02,
        '[Sampler11]': 0x02,
        '[Sampler12]': 0x02,
        '[Sampler13]': 0x03,
        '[Sampler14]': 0x03,
        '[Sampler15]': 0x03,
        '[Sampler16]': 0x03
    };

    PioneerDDJWeGO.deckLeds = {
        'cue': 0x0C,
        'play': 0x0B,
        'sync': 0x58,
        'hotCues': 0x2E, // += [0-3]
        'samplers' : 0x3C, // += [0246], alternate numbers are [shift] samplers
        'rotate': 0x6E,
        'jogfx': 0x28
    };

    PioneerDDJWeGO.jogfxLeds = {
        'ctrlA': 0x42,
        'fx1': 0x43,
        'fx2': 0x44,
        'fx3': 0x45,
        'ctrlB': 0x46,
        'ctrlA_': 0x47,
        'fx1_': 0x48,
        'fx2_': 0x49,
        'fx3_': 0x4A,
        'ctrlB_': 0x4B,
    };

    PioneerDDJWeGO.channel7leds = {
        'pfl': 0x54 // += [Channeln]
    };

    // there are volume indicators in the centre of the deck with five lights
    // each based off these starting points
    PioneerDDJWeGO.channel12Leds = {
        "meters" : 0x28, // += [Channeln] * 5 + [0-4]
    };

    PioneerDDJWeGO.setAllSoftTakeover( true );
    PioneerDDJWeGO.initDeck("[Channel1]");
    PioneerDDJWeGO.initDeck("[Channel2]");
    PioneerDDJWeGO.initDeck("[Channel3]");
    PioneerDDJWeGO.initDeck("[Channel4]");

    // setup the samplers
    for( var i = 1; i <= 16; i++ ){
        var sampler = "[Sampler" + i + "]";
        engine.connectControl( sampler, "play_indicator",
            "PioneerDDJWeGO.samplerLeds", false );
    }

    //FX led control
    if(! PioneerDDJWeGO.settings.fxExternal ){
    for( var j = 0; j < 4; j++ ){
    for( var i = 0; i < 3; i++ ){
        engine.connectControl(
            '[EffectRack1_EffectUnit' + (j+1) + '_Effect' + (i+1) + ']',
            'enabled',
            'PioneerDDJWeGO.fxLeds',
            false );
    }}} else {
        for( i in PioneerDDJWeGO.message1 )
            print( PioneerDDJWeGO.message1[ i ] );
    }

};

PioneerDDJWeGO.shutdown = function(id) {
    PioneerDDJWeGO.setAllSoftTakeover( false );
};

PioneerDDJWeGO.bindDeckControlConnections = function( group, binding ){
    var i,
        controlsToFunctions = {
            'play_indicator': 'PioneerDDJWeGO.playLed',
            'play': 'PioneerDDJWeGO.rotateLed',
            'cue_indicator': 'PioneerDDJWeGO.cueLed',
            'pfl': 'PioneerDDJWeGO.headphoneCueLed',
            'PeakIndicator': 'PioneerDDJWeGO.peakLeds',
            'VuMeter': 'PioneerDDJWeGO.levelLeds',
            'LoadSelectedTrack': 'PioneerDDJWeGO.loadLeds',
            'eject': 'PioneerDDJWeGO.loadLeds',
            'loop_enabled': 'PioneerDDJWeGO.loopLed',
            'pitch': 'PioneerDDJWeGO.ctrlAWheelLeds'
        };

    if (PioneerDDJWeGO.blinkingSync) {
        controlsToFunctions.beat_active = 'PioneerDDJWeGO.syncLed';
    } else {
        controlsToFunctions.sync_enabled = 'PioneerDDJWeGO.syncLed';
    }

    for( i = 1; i <= 4; i++ ){
        controlsToFunctions['hotcue_' + i + '_enabled'] = 'PioneerDDJWeGO.hotCueLeds';
    }

    script.bindConnections( group, controlsToFunctions, binding);

    //ctrlA button light is handled manually as there is no pitch enable/disable
    //ctrlA wheel turn is handled above by the pitch control.
    //ctrlB Button
    engine.connectControl(
        '[QuickEffectRack1_' + group + '_Effect1]',
        'enabled',
        'PioneerDDJWeGO.ctrlBLed',
        binding );
    //ctrlB wheel Turn
    engine.connectControl(
        '[QuickEffectRack1_' + group + ']',
        'super1',
        'PioneerDDJWeGO.ctrlBWheelLeds',
        binding );
        
};


PioneerDDJWeGO.setDeckSoftTakeover = function (channel, binding) {
    engine.softTakeover(channel, "volume", binding);
    engine.softTakeover(channel, "rate", binding);
    engine.softTakeover(channel, "filterHigh", binding);
    engine.softTakeover(channel, "filterMid", binding);
    engine.softTakeover(channel, "filterLow", binding);
};

PioneerDDJWeGO.setAllSoftTakeover = function (binding) {
    var channelIndex;
    for (channelIndex = 1; channelIndex <= 4; channelIndex++) {
        PioneerDDJWeGO.setDeckSoftTakeover('[Channel' + channelIndex + ']', binding);
    }
    engine.softTakeover( '[Master]', 'crossfader', binding );
    engine.softTakeover( '[Master]', 'headMix', binding );
};

PioneerDDJWeGO.initDeck = function (group) {
    PioneerDDJWeGO.bindDeckControlConnections(group, false);
};

///////////////////////////////////////////////////////////////
//                          ACTIONS                          //
///////////////////////////////////////////////////////////////

PioneerDDJWeGO.deckToggle = function(channel, control, value)
{
    if( value === 0 )return; //ignore note off
    if( PioneerDDJWeGO.activeDeck[+channel] === false )
    {
        PioneerDDJWeGO.activeDeck[+channel] = true;
        PioneerDDJWeGO.activeDeck[+channel - 2] = false;
        PioneerDDJWeGO.activeDeck[+channel - 2] = false;
        midi.sendShortMsg( 0x90 + +channel , 0x72, 127 );
    } else {
        PioneerDDJWeGO.activeDeck[+channel] = false;
        PioneerDDJWeGO.activeDeck[+channel - 2] = true;
        midi.sendShortMsg( 0x90 + +channel, 0x72, 0 );
    }
};


PioneerDDJWeGO.sampleButtons = function (channel, control, value, status, group)
{
    if( value ){
        if(! engine.getValue( group, 'track_samples' ) ){
            engine.setValue( group, 'LoadSelectedTrack', true );
            return;
        }
        if(! engine.getValue( group, "play" ) ){
            engine.setValue( group, "cue_gotoandplay", true );
        } else {
            engine.setValue( group, "cue_gotoandstop", true );
        }
    }
};

// The button that enables/disables scratching
PioneerDDJWeGO.wheelTouch = function (channel, control, value, status, group) {
    if ((status & 0xF0) === 0x90) {    // If button down
        if (value === 0x7F) {  // Some wheels send 0x90 on press and release, so you need to check the value
            engine.scratchEnable(
                channel + 1,
                PioneerDDJWeGO.scratchSettings.jogResolution,
                PioneerDDJWeGO.scratchSettings.vinylSpeed,
                PioneerDDJWeGO.scratchSettings.alpha,
                PioneerDDJWeGO.scratchSettings.beta
            );
        } else {    // If button up
            engine.scratchDisable( channel + 1 );
        }
    }
};

// The wheel that actually controls the scratching
PioneerDDJWeGO.wheelTurn = function (channel, control, value, status, group) {
    var newValue = value - 64;
    engine.scratchTick( channel + 1, newValue);
};

PioneerDDJWeGO.ctrlAButton = function( channel, control, value, status, group )
{
    if(! value )return;
    var deck = PioneerDDJWeGO.channelGroups[ group ];

    PioneerDDJWeGO.ctrlA[ deck ] = !PioneerDDJWeGO.ctrlA[ deck ];
    PioneerDDJWeGO.ctrlALed(
        PioneerDDJWeGO.ctrlA[ deck ],
        group,
        control
    );
};

PioneerDDJWeGO.ctrlBButton = function( channel, control, value, status, group )
{
    if(! value )return;
    var deck = PioneerDDJWeGO.channelGroups[ group ];

    PioneerDDJWeGO.ctrlB[ deck ] = !PioneerDDJWeGO.ctrlB[ deck ];
    midi.sendShortMsg(
        0x90 + channel,
        control,
        PioneerDDJWeGO.ctrlB[ deck ] ? PioneerDDJWeGO.settings.ctrlBmode : 0x00
    );
}

PioneerDDJWeGO.fx1Button = function( channel, control, value, status, group )
{
    if(! value )return;
    var deck = PioneerDDJWeGO.channelGroups[ group ];

    PioneerDDJWeGO.fx1[ deck ] = !PioneerDDJWeGO.fx1[ deck ];
    midi.sendShortMsg(
        0x90 + channel,
        control,
        PioneerDDJWeGO.fx1[ deck ] ? PioneerDDJWeGO.settings.fxMode : 0x00
    );
// old functionality, ill see if i ca stick it in
    // [EffectRack1_EffectUnit1_Effect1],enabled
}

PioneerDDJWeGO.fx2Button = function( channel, control, value, status, group )
{
    if(! value )return;
    var deck = PioneerDDJWeGO.channelGroups[ group ];

    PioneerDDJWeGO.fx2[ deck ] = !PioneerDDJWeGO.fx2[ deck ];
    midi.sendShortMsg(
        0x90 + channel,
        control,
        PioneerDDJWeGO.fx2[ deck ] ? PioneerDDJWeGO.settings.fxMode : 0x00
    );
}

PioneerDDJWeGO.fx3Button = function( channel, control, value, status, group )
{
    if(! value )return;
    var deck = PioneerDDJWeGO.channelGroups[ group ];

    PioneerDDJWeGO.fx3[ deck ] = !PioneerDDJWeGO.fx3[ deck ];
    midi.sendShortMsg(
        0x90 + channel,
        control,
        PioneerDDJWeGO.fx3[ deck ] ? PioneerDDJWeGO.settings.fxMode : 0x00
    );
}


PioneerDDJWeGO.autoLoopTurn = function( channel, control, value, status, group )
{
    if( value > 0x40 ){
        engine.setValue( group, 'loop_halve', true  );
        engine.setValue( group, 'loop_halve', false );
    } else {
        engine.setValue( group, 'loop_double', true );
        engine.setValue( group, 'loop_double', false);
    }
}

PioneerDDJWeGO.autoLoopButton = function( channel, control, value, status, group )
{
    if(! value )return;
    if( engine.getValue( group, 'loop_enabled' ) ){
        engine.setValue( group, 'reloop_exit', true )
        engine.setValue( group, 'reloop_exit', false )
    } else {
        var size = PioneerDDJWeGO.settings.autoLoopSize;
        engine.setValue( group, 'beatloop_' + size + '_activate', true )
        engine.setValue( group, 'beatloop_' + size + '_activate', false )
    }
}


PioneerDDJWeGO.ctrlfxTurn = function( channel, control, value, status, group ){
    midi.sendShortMsg(
        status,
        control,
        value);
}

///////////////////////////////////////////////////////////////
//                             LEDS                          //
///////////////////////////////////////////////////////////////

PioneerDDJWeGO.headphoneCueLed = function (value, group, control) {
    midi.sendShortMsg(
        0x96,
        0x54 + PioneerDDJWeGO.channelGroups[ group ],
        value ? 0x7F : 0x00 );
};

PioneerDDJWeGO.playLed = function (value, group, control) {
    midi.sendShortMsg(
        0x90 + PioneerDDJWeGO.channelGroups[ group ],
        PioneerDDJWeGO.deckLeds.play,
        value ? 0x7F : 0x00
    );
};

PioneerDDJWeGO.rotateLed = function (value, group, control) {
    midi.sendShortMsg(
        0x90 + PioneerDDJWeGO.channelGroups[ group ],
        PioneerDDJWeGO.deckLeds.rotate,
        value ? 0x7F : 0x00
    );
};

PioneerDDJWeGO.cueLed = function (value, group, control) {
    midi.sendShortMsg(
        0x90 + PioneerDDJWeGO.channelGroups[ group ],
        PioneerDDJWeGO.deckLeds.cue,
        value ? 0x7F : 0x00
    );
};

PioneerDDJWeGO.syncLed = function (value, group, control) {
    midi.sendShortMsg(
        0x90 + PioneerDDJWeGO.channelGroups[ group ],
        PioneerDDJWeGO.deckLeds.sync,
        value ? 0x7F : 0x00
    );
};


PioneerDDJWeGO.loopLed = function( value, group, control ){
    midi.sendShortMsg(
        0x9B,
        0x10 + PioneerDDJWeGO.channelGroups[ group ],
        value ? 0x7F : 0x00 );
}

PioneerDDJWeGO.samplerLeds = function (value, group, control) {
    var samplerNum;

    for( samplerNum = 0; samplerNum < 16; samplerNum++ ){
        if( group === '[Sampler' + (samplerNum + 1) + ']' ){
            midi.sendShortMsg(
                0x90 + PioneerDDJWeGO.samplerGroups[group],
                PioneerDDJWeGO.deckLeds.samplers + (samplerNum % 4) * 2,
                value ? 0x7F : 0x00
            );
        }
    }
};

PioneerDDJWeGO.hotCueLeds = function (value, group, control) {
    var hotCueNum;

    for( hotCueNum = 1; hotCueNum <= 4; hotCueNum++ ){
        if( control === 'hotcue_' + hotCueNum + '_enabled' ){
            midi.sendShortMsg(
                0x90 + PioneerDDJWeGO.channelGroups[ group ],
                PioneerDDJWeGO.deckLeds.hotCues + hotCueNum - 1,
                value ? 0x7F : 0x00
            );
        }
    }
};

PioneerDDJWeGO.levelLeds = function( level ,group, control ){
    var split = PioneerDDJWeGO.vuMeters[ group ];
    var channel = PioneerDDJWeGO.channelGroups[ group ];

    if( level > (0.10 + (0.2 * split)) ){
        midi.sendShortMsg( 0x9C, 0x28 + (channel * 5) + split, 0x7F );
        if( split < 5 )PioneerDDJWeGO.vuMeters[ group ]++;
    }

    if( level < (0.10 + (0.2 * (split - 1))) ){
        midi.sendShortMsg( 0x9C, 0x28 + (channel * 5) + split - 1, 0x00 );
        if( split > 0 )PioneerDDJWeGO.vuMeters[ group ]--;
    }
};

PioneerDDJWeGO.peakLeds = function (value, group, control) {
    print( "Unnasigned: peakLeds( " + value + ", " + group + ", " + control + " )" );
};

PioneerDDJWeGO.loadLeds = function (value, group, control ){
    if(! value ) return;
    if( control === "LoadSelectedTrack"){
        midi.sendShortMsg(
            0x9B,
            0x0C + PioneerDDJWeGO.channelGroups[ group ],
            0x7F );
    }
    if( control === "eject" ){
        midi.sendShortMsg(
            0x9B,
            0x0C + PioneerDDJWeGO.channelGroups[ group ],
            0x00 );
    }
};

PioneerDDJWeGO.fxLeds = function( value, group, control ){

    for( var unit = 0; unit < 4; unit++ ){
    for( var effect = 0; effect < 3; effect++ ){
        if( group === '[EffectRack1_EffectUnit' + (unit+1)
                    + '_Effect' + (effect+1) + ']' )
        {
            var channel = 0x94 + (unit % 2);
            var ledNumber = 0x43 + effect + 5 * ( unit > 1 ? 1 : 0 );
            midi.sendShortMsg(
                channel,
                ledNumber,
                value ? PioneerDDJWeGO.settings.fxMode : 0x00
            );
        }
    }}

};

PioneerDDJWeGO.ctrlALed = function( value, group, control ){
    var channel = PioneerDDJWeGO.channelGroups[ group ];
    var lights = PioneerDDJWeGO.jogfxLeds;
    midi.sendShortMsg(
        0x94 + channel % 2,
        channel < 2 ? 0x42 : 0x47,
        value ? PioneerDDJWeGO.settings.ctrlBmode : 0x00
    );

}

PioneerDDJWeGO.ctrlAWheelLeds = function( value, group, control ){
    var channel = PioneerDDJWeGO.channelGroups[ group ];
    midi.sendShortMsg(
        0xB4 + channel % 2,
        channel < 2 ? 0x00 : 0x08,
        Math.round( (value + 6) * 10.66 )
    );
}

PioneerDDJWeGO.ctrlBLed = function( value, group, control) {
    for( var i = 0; i < 4; i++ ){
        if( group === '[QuickEffectRack1_[Channel' + (i+1) + ']_Effect1]' ){
            midi.sendShortMsg(
                0x94 + i % 2,
                i < 2 ? 0x46 : 0x4B,
                value ? PioneerDDJWeGO.settings.ctrlBmode : 0x00
            );
            break;
        }
    }
}

PioneerDDJWeGO.ctrlBWheelLeds = function( value, group, control ){
    for( var i = 0; i < 4; i++ ){
        if( group === '[QuickEffectRack1_[Channel' + (i+1) + ']]'){
            midi.sendShortMsg(
                0xB4 + i % 2,
                i < 2 ? 0x07 : 0x0F,
                Math.round( value * 127)
            );
            break;
        }
    }
}
// just a place to keep this for when I need it instead of re-writing it.
//print("<function>Led( " + value + ", " + group + ", " + control + " )" );
//
/*print( "<control><action>("
        + channel + ", "
        + control + ", "
        + value + ", "
        + status + ", "
        + group + " )"
    );
*/
