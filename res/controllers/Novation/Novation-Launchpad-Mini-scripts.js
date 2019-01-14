/****************************************************************/
/*      Novation Launchpad Mini Mapping                         */
/*      For Mixxx version 1.12                                  */
/*      Author: marczis based on zestoi's work                  */
/****************************************************************/

//Common helpers
colorCode = function()
{
    return {
        black: 4,

        lo_red: 1 + 4,
        mi_red: 2 + 4,
        hi_red: 3 + 4,
        lo_green: 16 + 4,
        mi_green: 32 + 4,
        hi_green: 48 + 4,
        lo_amber: 17 + 4,
        mi_amber: 34 + 4,
        hi_amber: 51 + 4,
        hi_orange: 35 + 4,
        lo_orange: 18 + 4,
        hi_yellow: 50 + 4,
        lo_yellow: 33 + 4,

    }
};
//Define one Key
Key = Object;
Key.prototype.color = colorCode("black");
Key.prototype.x = -1;
Key.prototype.y = -1;
Key.prototype.page = -1;
Key.prototype.pressed = false;

Key.prototype.init = function(page, x, y)
{
    this.x = x;
    this.y = y;
    this.page = page;
    //print("Key created");
}

Key.prototype.setColor = function(color)
{
    //First line is special
    this.color = colorCode()[color];
    this.draw();
};

Key.prototype.draw = function()
{
    if ( this.page != NLM.page ) return;
    if ( this.y == 8 ) {
        midi.sendShortMsg(0xb0, this.x + 0x68, this.color);
        return;
    }
    midi.sendShortMsg(0x90, this.x+this.y*16, this.color);
    //midi.sendShortMsg(0xb0, 0x0, 0x28); //Enable buffer cycling
}

Key.prototype.onPush = function()
{
}

Key.prototype.onRelease = function()
{
}

Key.prototype.callback = function()
{
    if (this.pressed) {
        this.onPush();
    } else {
        this.onRelease();
    }
}

function PushKey(colordef, colorpush) {
    var that = new Key;

    that.setColor(colordef);

    that.colordef = colordef;
    that.colorpush = colorpush;

    that.onPush = function()
    {
        this.setColor(this.colorpush);
    }

    that.onRelease = function()
    {
        this.setColor(this.colordef);
    }

    return that;
}

function PushKeyBin(colordef, colorpush, group, control, pushval) {
    var that = PushKey(colordef, colorpush);

    that.onPushOrig = that.onPush;
    that.onPush = function()
    {
        engine.setValue(group, control, pushval);
        this.onPushOrig();
    }
    return that;
}

function PageSelectKey() {
    var that = new Key;

    that.onPush = function()
    {
        NLM.btns[NLM.page][8][NLM.page].setColor("black");
        NLM.page = this.y;
        NLM.btns[NLM.page][8][NLM.page].setColor("hi_amber");
        NLM.drawPage();
    }
    return that;
}

function ShiftKey() {
    var that = PushKey("lo_green", "hi_yellow");

    that.onPushOrig = that.onPush;
    that.onPush = function()
    {
        NLM.shiftstate = this.pressed;
        this.onPushOrig();
    }

    that.onReleaseOrig = that.onRelease;
    that.onRelease = function()
    {
        NLM.shiftstate = this.pressed;
        this.onReleaseOrig();
    }

    return that;
}

function HotCueKey(ctrl, deck, hotcue) {
    var that = new Key();
    that.deck = deck;
    that.hotcue = hotcue;

    that.group = "[" + ctrl + deck + "]";
    that.ctrl_act = "hotcue_" + hotcue + "_activate";
    that.ctrl_del = "hotcue_" + hotcue + "_clear";
    that.state   = "hotcue_" + hotcue + "_enabled";

    that.setled = function() {
        if (this.pressed) {
            this.setColor("hi_amber");
        } else if (engine.getValue(this.group, this.state) == 1) {
            this.setColor("lo_green");
        } else {
            this.setColor("lo_red");
        }
    }

    that.conEvent = function() {
        engine.connectControl(this.group, this.state, this.setled);
    }

    that.setled();
    that.conEvent();

    that.callback = function() {
        if (NLM.shiftstate) {
            ctrl = this.ctrl_del;
        } else {
            ctrl = this.ctrl_act;
        }

        if (this.pressed) {
            engine.setValue(this.group, ctrl, 1);
        } else {
            engine.setValue(this.group, ctrl, 0);
        }

        this.setled();
    }

    return that;
}

function PlayKey(ctrl, deck) {
    var that = new Key();
    that.group = "[" + ctrl + deck + "]";
    that.ctrl  = "play";
    that.state = "play_indicator";

    that.setled = function() {
        if (this.pressed) {
            this.setColor("hi_amber");
        } else if (engine.getValue(this.group, this.state) == 1) {
            this.setColor("hi_green");
        } else {
            this.setColor("hi_yellow");
        }
    }

    that.conEvent = function() {
        engine.connectControl(this.group, this.state, this.setled);
    }

    that.setled();
    that.conEvent();

    that.onPush = function()
    {
        engine.setValue(this.group, this.ctrl, engine.getValue(this.group, this.ctrl) == 1 ? 0 : 1);
        this.setled();
    }

    that.onRelease = function()
    {
        this.setled();
    }

    return that;
}

function LoopKey(deck, loop) {
    var that = new Key();

    that.group = "[Channel" + deck + "]";
    that.ctrl0 = "beatloop_" + loop + "_toggle";
    that.ctrl1 = "beatlooproll_" + loop + "_activate";
    that.state = "beatloop_" + loop + "_enabled";
    that.setColor("hi_yellow");

    if (LoopKey.keys == undefined) {
        LoopKey.keys = new Array;
        LoopKey.mode = 0;
    }

    LoopKey.setMode = function(mode)
    {
        LoopKey.mode = mode;
        if (mode == 1) {
            LoopKey.keys.forEach(function(e) { e.setColor("hi_orange");} );
        }
        if (mode == 0) {
            LoopKey.keys.forEach(function(e) { e.setColor("hi_yellow");} );
        }
    }

    that.callback = function()
    {
        if (LoopKey.mode == 0) {
             if (this.pressed) {
                engine.setValue(this.group, this.ctrl0, 1);
                this.setColor("hi_green");
            } else {
                if ( engine.getValue(this.group, this.state) == 1) {
                    engine.setValue(this.group, this.ctrl0, 1);
                }
                this.setColor("hi_yellow");
            }
        } else {
            if (this.pressed) {
                engine.setValue(this.group, this.ctrl1, 1);
                this.setColor("hi_green");
            } else {
                engine.setValue(this.group, this.ctrl1, 0);
                this.setColor("hi_orange");
            }
        }
    }

    LoopKey.keys.push(that);
    return that;
}

function LoopModeKey() {
    var that = new Key();
    that.setColor("lo_yellow");

    that.callback = function()
    {
        if (this.pressed) {
            if (LoopKey.mode == 0) {
                LoopKey.setMode(1);
                this.setColor("lo_orange");
            } else {
                LoopKey.setMode(0);
                this.setColor("lo_yellow");
            }
        }
    }

    return that;
}

function LoadKey(ctrl, channel) {
    var that = PushKey("hi_green","hi_amber");

    that.group   = "[" + ctrl + channel + "]";
    that.control = "LoadSelectedTrack";

    that.onPushOrig = that.onPush;

    that.onPush = function()
    {
        engine.setValue(this.group, this.control, 1);
        this.onPushOrig();
    }

    that.event = function() {
        if (engine.getValue(this.group, "play")) {
            this.colordef = "lo_red";
        } else {
            this.colordef = "hi_green";
        }
        this.setColor(this.colordef);
    }

    that.conEvent = function() {
        engine.connectControl(this.group, "play", this.event);
    }

    that.conEvent();
    return that;
}

function ZoomKey(dir) {
    var that = PushKey("lo_green", "hi_amber");

    that.dir  = dir;

    that.onPushOrig = that.onPush;
    that.onPush = function()
    {
        if ( ZoomKey.zoom < 6 && this.dir == "+" ) {
            ZoomKey.zoom++;
        }
        if ( ZoomKey.zoom > 1 && this.dir == "-") {
            ZoomKey.zoom--;
        }

        for ( ch = 1 ; ch <= NLM.numofdecks ; ch++ ) {
            //print("Zoom:" + ZoomKey.zoom);
            var group = "[Channel" + ch + "]";
            engine.setValue(group, "waveform_zoom", ZoomKey.zoom);
        }

        this.onPushOrig();
    }

    return that;
}
ZoomKey.zoom = 3;

function SeekKey(ch, pos) {
    var that = new Key();

    that.pos  = 0.125 * pos;
    that.grp = "[Channel"+ ch + "]";

    that.setled = function()
    {
        if (engine.getValue(this.grp, "playposition") >= this.pos) {
            this.setColor("hi_red");
        } else {
            this.setColor("lo_green");
        }
    }

    that.conEvent = function()
    {
        engine.connectControl(this.grp, "beat_active", this.setled);
    }

    that.conEvent();

    that.onPush = function()
    {
        engine.setValue(this.grp, "playposition", this.pos);
        SeekKey.keys[ch].forEach(function(e) { e.setled(); });
    }

    that.setled();

    if ( SeekKey.keys[ch] == undefined ) SeekKey.keys[ch] = new Array();
    SeekKey.keys[ch][pos] = that;
    return that;
}
SeekKey.keys = new Array();

//Define the controller

NLM = new Controller();
NLM.init = function()
{
        NLM.page = 0;
        NLM.shiftstate = false;
        NLM.numofdecks = engine.getValue("[Master]", "num_decks");
        // For testing NLM.numofdecks = 4;

        //Init hw
        midi.sendShortMsg(0xb0, 0x0, 0x0);
        //midi.sendShortMsg(0xb0, 0x0, 0x28); //Enable buffer cycling <-- Figure out whats wrong with this

        // select buffer 0
        midi.sendShortMsg(0xb0, 0x68, 3);
        //midi.sendShortMsg(0xb0, 0x0, 0x31);
        //print("=============================");
        //Setup btnstate which is for phy. state
        NLM.btns = new Array();
        for ( page = 0; page < 8 ; page++ ) {
            NLM.btns[page] = new Array();
            for ( x = 0 ; x < 9 ; x++ ) {
                NLM.btns[page][x] = new Array();
                for ( y = 0 ; y < 9 ; y++ ) {
                    var tmp = new Key;
                    if (x == 8) {
                        tmp = PageSelectKey();
                    }

                    if (y == 8) {
                        if (x == 0) {
                            tmp = ZoomKey("-");
                        }

                        if (x == 1) {
                            tmp = ZoomKey("+");
                        }

                        if (x == 7) {
                            tmp = ShiftKey();
                        }
                    }

                    if (y == 8 && x == 1) {

                    }

                    NLM.setupBtn(page,x,y, tmp);
                }
            }
        }
        //Set default page led
        NLM.btns[NLM.page][8][0].setColor("hi_amber");

        // ============== PAGE A ===============
        //Set ChX CueButtons
        for ( deck = 1; deck <= NLM.numofdecks; deck++ ) {
            for ( hc = 1 ; hc < 9 ; hc++ ) {
                x = hc-1;
                y = (deck-1)*2+1;
                NLM.setupBtn(0,x,y, HotCueKey("Channel", deck, hc));
            }
        }

        for ( deck = 1; deck <= NLM.numofdecks; deck++ ) {
            y = (deck-1)*2;
            //Set Chx PlayButton
            NLM.setupBtn(0,0,y, PlayKey("Channel", deck));
            //Set Chx LoopButtons
            NLM.setupBtn(0,2,y, LoopKey(deck, "0.0625"));
            NLM.setupBtn(0,3,y, LoopKey(deck, "0.125"));
            NLM.setupBtn(0,4,y, LoopKey(deck, "0.25"));
            NLM.setupBtn(0,5,y, LoopKey(deck, "0.5"));
            NLM.setupBtn(0,6,y, LoopKey(deck, "1"));
            NLM.setupBtn(0,7,y, LoopKey(deck, "2"));
        }

        NLM.setupBtn(0,2,8, LoopModeKey());

        // ============== PAGE H ===============

        // Right side, playlist scroll
        NLM.setupBtn(7,6,0, PushKeyBin("lo_amber", "hi_amber", "[Playlist]", "SelectTrackKnob", -50));
        NLM.setupBtn(7,6,1, PushKeyBin("mi_amber", "hi_amber", "[Playlist]", "SelectTrackKnob", -10));
        NLM.setupBtn(7,6,2, PushKeyBin("hi_amber", "hi_amber", "[Playlist]", "SelectPrevTrack", 1));
        NLM.setupBtn(7,6,3, PushKeyBin("hi_green", "hi_amber", "[Playlist]", "LoadSelectedIntoFirstStopped", 1));
        NLM.setupBtn(7,6,4, PushKeyBin("hi_amber", "hi_amber", "[Playlist]", "SelectNextTrack", 1));
        NLM.setupBtn(7,6,5, PushKeyBin("mi_amber", "hi_amber", "[Playlist]", "SelectTrackKnob", 10));
        NLM.setupBtn(7,6,6, PushKeyBin("lo_amber", "hi_amber", "[Playlist]", "SelectTrackKnob", 50));

        NLM.setupBtn(7,5,2, LoadKey("Channel",1));
        NLM.setupBtn(7,7,2, LoadKey("Channel",2));
        NLM.setupBtn(7,5,4, LoadKey("Channel",3));
        NLM.setupBtn(7,7,4, LoadKey("Channel",4));
        
        NLM.setupBtn(7,0,6, LoadKey("Sampler",1));
        NLM.setupBtn(7,1,6, LoadKey("Sampler",2));
        NLM.setupBtn(7,2,6, LoadKey("Sampler",3));
        NLM.setupBtn(7,3,6, LoadKey("Sampler",4));
        
        NLM.setupBtn(7,0,7, LoadKey("Sampler",5));
        NLM.setupBtn(7,1,7, LoadKey("Sampler",6));
        NLM.setupBtn(7,2,7, LoadKey("Sampler",7));
        NLM.setupBtn(7,3,7, LoadKey("Sampler",8));

        // Left side, playlists

        NLM.setupBtn(7,1,2, PushKeyBin("hi_green", "hi_amber", "[Playlist]", "SelectPrevPlaylist", 1));
        NLM.setupBtn(7,1,3, PushKeyBin("hi_yellow", "hi_amber", "[Playlist]", "ToggleSelectedSidebarItem", 1));
        NLM.setupBtn(7,1,4, PushKeyBin("hi_green", "hi_amber", "[Playlist]", "SelectNextPlaylist", 1));

        // ============== PAGE B ===============

        //SeekButtons
        for(i = 0 ; i < 8 ; i++) {
            for ( ch = 1 ; ch <= NLM.numofdecks; ch++ ) {
                NLM.setupBtn(1,i,ch*2-1, SeekKey(ch, i));
            }
        }
        
        // ============== PAGE C ===============
        
        // Add Sampler playbuttons
        for(var channel = 1 ; channel < 9 ; channel++) {
            NLM.setupBtn(2, 0, channel-1, PlayKey("Sampler", channel));
            NLM.setupBtn(2, 1, channel-1, PushKeyBin("hi_orange", "hi_red", "[Sampler" + channel + "]", "beatsync", 1));
            for(i = 1 ; i < 5 ; i++) {
                NLM.setupBtn(2, 3+i, channel-1, HotCueKey("Sampler", channel, i));
            }
        }
        



        this.drawPage();
};

NLM.setupBtn = function(page, x, y, btn)
{
    NLM.btns[page][x][y] = btn;
    NLM.btns[page][x][y].init(page, x, y);
}

NLM.shutdown = function()
{

};

NLM.incomingData = function(channel, control, value, status, group)
{
        //print("Incoming data");
        //print("cha: " + channel);
        //print("con: " + control);
        //print("val: " + value);
        //print("sta: " + status);
        //print("grp: " + group);

        //Just to make life easier

        var pressed = (value == 127);
        //Translate midi btn into index
        var y = Math.floor(control / 16);
        var x = control - y * 16;
        if ( y == 6 && x > 8 ) {
            y = 8;
            x -= 8;
        }
        if ( y == 6 && x == 8 && status == 176 ) {
            y = 8; x = 0;
        }

        print( "COO: " + NLM.page + ":" + x + ":" + y);
        NLM.btns[NLM.page][x][y].pressed = pressed;
        NLM.btns[NLM.page][x][y].callback();
};

NLM.drawPage = function() {
    for ( x = 0 ; x < 9 ; x++ ) {
        for ( y = 0 ; y < 9 ; y++ ) {
            NLM.btns[NLM.page][x][y].draw();
        }
    }
}

