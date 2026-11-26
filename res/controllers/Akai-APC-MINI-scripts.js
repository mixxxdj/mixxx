var APC = {};

/*
    AKAI APC Mini
*/

///////////////////////////////////////////////////////////////
//                         FUNCTIONS                         //
///////////////////////////////////////////////////////////////

APC.init = function(id) { // called when the MIDI device is opened & set up
    APC.id = id; // store the ID of this device for later use

    for (var i = 1; i <= 64; i++) {
        var g = "[Sampler" + i + "]";

        engine.connectControl(g, "play", "APC.deckButtonPlay");
        engine.connectControl(g, "track_loaded", "APC.deckTrackLoaded");
        engine.connectControl(g, "eject", "APC.deckTrackEjected");

        APC.deckTrackLoaded(engine.getValue(g, "track_loaded"), g, "init");
        for (var z=1; z<=500000; z++);
    }

    print("APC Mini Initialized");
};

APC.shutdown = function(id) { // called when the MIDI device is closed

    for (var i = 0; i <= 63; i++) {
        midi.sendShortMsg(0x90, i, 0x00); // switch off
    }

    print("APC Mini Shutdown: "+id);
};


APC.toggleplay = function(group, state) {
    if (state) {
        engine.setValue(group, "reverse", 0);
        engine.setValue(group, "play", false);

        print("APC Mini "+group+" STOP");
    } else {
        engine.setValue(group, "reverse", 0);
        engine.setValue(group, "play", true);

        print("APC Mini "+group+" PLAY");
    }
};

APC.deckButtonPlay = function(value, group, control) { // called when click a play button
    var deck = parseInt(group.substring(8, group.length-1)) - 1;

    print("APC Mini "+group+" on Deck Button Play - value:"+value+"   control:"+control+"   playposition:"+engine.getValue(group, "playposition"));

    if (value === 0 && engine.getValue(group, "duration") === 0) {
        print("APC Mini FAKE STOP ON "+group+" on Deck Button Play - value:"+value+"   control:"+control);
        return;
    }

    // looping deck
    if (deck < 16 && value === 1 && engine.getValue(group, "playposition") > 0.01) {
        print("triggered play for a playing looping control, stopping instead.");
        APC.toggleplay(group, true);
        value = 0;
    }

    var row = Math.floor(deck / 8);
    var col = deck % 8;
    var buttonId = ((7 - row) * 8) + col;


    if (value === 1) { // deck play on

        midi.sendShortMsg(0x90, buttonId, 0x01); // note C on with value 64 + deck

    } else { // deck play stop

        midi.sendShortMsg(0x90, buttonId, 0x03); // note C on with value 64 + deck

    }

};

APC.deckTrackLoaded = function(value, group, control) { // called when a track is loaded
    var deck = parseInt(group.substring(8, group.length-1)) - 1;

    print("APC Mini "+group+" on Deck Track Loaded - value:"+value+"   control:"+control);

    var row = Math.floor(deck / 8);
    var col = deck % 8;
    var buttonId = ((7 - row) * 8) + col;

    if (value === 1) { // deck loaded

        midi.sendShortMsg(0x90, buttonId, 0x05);

    } else {
        // Load empty track ?
        midi.sendShortMsg(0x90, buttonId, 0x00);

    }

    if (deck < 16) {
        print("APC Mini "+group+" on Deck "+deck+" Track Loaded - enabling loop");
        engine.setValue(group, "repeat", true);
    } else
        engine.setValue(group, "repeat", false);
};

APC.deckTrackEjected = function(value, group, control) { // called when a track is ejected
    var deck = parseInt(group.substring(8, group.length-1)) - 1;

    print("APC Mini "+group+"/"+control+" on Deck Track Ejected");

    var row = Math.floor(deck / 8);
    var col = deck % 8;
    var buttonId = ((7 - row) * 8) + col;

    midi.sendShortMsg(0x90, buttonId, 0x00);
};
