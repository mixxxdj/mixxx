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

        /*
        flash_lo_red: 1,
        flash_mi_red: 2,
        flash_hi_red: 3,
        flash_lo_green: 16,
        flash_mi_green: 32,
        flash_hi_green: 48,
        flash_lo_amber: 17,
        flash_mi_amber: 34,
        flash_hi_amber: 51,
        flash_hi_orange: 35,
        flash_lo_orange: 18,
        flash_hi_yellow: 50,
        flash_lo_yellow: 33
        TODO fix these*/
    }
};

//Different kind of callbacks for the buttons.
DefaultCallback = function(nlm, key)
{
    this.nlm = nlm;
    this.key = key;
}
DefaultCallback.prototype.f = function()
{
    /*
    if ( this.key.pressed ) return;
    print("Key[" + this.key.x + ":" + this.key.y + "]");
    if ( this.key.color == colorCode()["hi_red"] ) {
        this.key.setColor("hi_green");
    } else {
        this.key.setColor("hi_red");
    }
    */
}

PageSelectCallback = function(nlm, key)
{
    this.nlm = nlm;
    this.key = key;
}
PageSelectCallback.prototype.f = function()
{
    if (this.key.pressed) {
        this.nlm.btns[this.nlm.page][8][this.nlm.page].setColor("black");
        this.nlm.page = this.key.y;
        this.nlm.btns[this.nlm.page][8][this.nlm.page].setColor("hi_amber");
    }
    this.nlm.drawPage();
}

PushBtnCallback = function(key, group, control, vdef, vpress, colordef, colorpress)
{
    this.key = key;

    this.group      = group;
    this.control    = control;
    this.vdef       = vdef;
    this.vpress     = vpress;
    this.colordef   = colordef;
    this.colorpress = colorpress;

    this.key.setColor(colordef);
}

PushBtnCallback.prototype.f = function()
{
    if (this.key.pressed) {
        engine.setValue(this.group, this.control, this.vpress);
        this.key.setColor(this.colorpress);
    } else {
        engine.setValue(this.group, this.control, this.vdef);
        this.key.setColor(this.colordef);
    }
}

ShiftCallback = function(nlm, key)
{
    this.key = key;
    this.nlm = nlm;

    this.key.setColor("lo_green");
}

ShiftCallback.prototype.f = function()
{
    this.nlm.shiftstate = this.key.pressed;
    if (this.key.pressed) {
        this.key.setColor("hi_yellow");
    } else {
        this.key.setColor("lo_green");
    }
}

HotCueActCallback = function(nlm, key, deck, hotcue)
{
    this.nlm   = nlm;
    this.group = "[Channel" + deck + "]";
    this.ctrl_act = "hotcue_" + hotcue + "_activate";
    this.ctrl_del = "hotcue_" + hotcue + "_clear";
    this.state   = "hotcue_" + hotcue + "_enabled";
    this.key = key;

    this.setled();
    engine.connectControl(this.group, this.state, this.setled);
}

HotCueActCallback.prototype.setled = function()
{
    if (this.key.pressed) {
        this.key.setColor("hi_amber");
    } else if (engine.getValue(this.group, this.state) == 1) {
        this.key.setColor("lo_green");
    } else {
        this.key.setColor("lo_red");
    }
}

HotCueActCallback.prototype.f = function()
{
    if (this.nlm.shiftstate) {
        ctrl = this.ctrl_del;
    } else {
        ctrl = this.ctrl_act;
    }

    if (this.key.pressed) {
        engine.setValue(this.group, ctrl, 1);
    } else {
        engine.setValue(this.group, ctrl, 0);
    }

    this.setled();
}

PlayCallback = function(key, deck)
{
    this.group = "[Channel" + deck + "]";
    this.ctrl  = "play";
    this.state = "play_indicator";
    this.key   = key;
    this.setled();
    engine.connectControl(this.group, this.state, this.setled);
}

PlayCallback.prototype.setled = function()
{
    if (this.key.pressed) {
        this.key.setColor("hi_amber");
    } else if (engine.getValue(this.group, this.state) == 1) {
        this.key.setColor("hi_green");
    } else {
        this.key.setColor("hi_yellow");
    }
}

PlayCallback.prototype.f = function()
{
    if (this.key.pressed) {
        engine.setValue(this.group, this.ctrl, engine.getValue(this.group, this.ctrl) == 1 ? 0 : 1);
    }
    this.setled();
}

LoopCallback = function(key, deck, loop)
{
    this.key = key;
    this.group = "[Channel" + deck + "]";
    this.ctrl0 = "beatloop_" + loop + "_toggle";
    this.ctrl1 = "beatlooproll_" + loop + "_activate";
    this.state = "beatloop_" + loop + "_enabled";
    this.key.setColor("hi_yellow");
    if (LoopCallback.keys == undefined) {
        LoopCallback.keys = new Array;
    }

    LoopCallback.keys.push(key);
}

LoopCallback.mode = 0;
LoopCallback.setMode = function(mode)
{
    LoopCallback.mode = mode;
    if (mode == 1) {
        LoopCallback.keys.forEach(function(e) { e.setColor("hi_orange");} );
    }
    if (mode == 0) {
        LoopCallback.keys.forEach(function(e) { e.setColor("hi_yellow");} );
    }
}

LoopCallback.prototype.f = function()
{
    if (LoopCallback.mode == 0) {
         if (this.key.pressed) {
            engine.setValue(this.group, this.ctrl0, 1);
            this.key.setColor("hi_green");
        } else {
            if ( engine.getValue(this.group, this.state) == 1) {
                engine.setValue(this.group, this.ctrl0, 1);
            }
            this.key.setColor("hi_yellow");
        }
    } else {
        if (this.key.pressed) {
            engine.setValue(this.group, this.ctrl1, 1);
            this.key.setColor("hi_green");
        } else {
            engine.setValue(this.group, this.ctrl1, 0);
            this.key.setColor("hi_orange");
        }
    }
}

LoopModeCallback = function(key)
{
    this.key = key;
    this.key.setColor("lo_yellow");
}

LoopModeCallback.prototype.f = function()
{
    if (this.key.pressed) {
        if (LoopCallback.mode == 0) {
            LoopCallback.setMode(1);
            this.key.setColor("lo_orange");
        } else {
            LoopCallback.setMode(0);
            this.key.setColor("lo_yellow");
        }
    }
}

//Define the controller

NLM = new Controller();
NLM.init = function()
{
        this.page = 0;
        this.shiftstate = false;

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
                    tmp.init(x,y);

                    //Setup shift
                    if (y == 8 && x == 7) {
                        tmp.callback = new ShiftCallback(NLM, tmp);
                    } else
                    //Setup Page selectors
                    if (x == 8) {
                        tmp.callback = new PageSelectCallback(NLM, tmp);
                    } else {
                        tmp.callback = new DefaultCallback(NLM, tmp);
                    }

                    NLM.btns[page][x][y] = tmp;
//                    NLM.setColor(x, y, "hi_yellow");
                }
            }
        }
        //Set default page led
        NLM.btns[NLM.page][8][0].setColor("hi_amber");

        //Set ChX CueButtons
        for ( deck = 1; deck < 5; deck++ ) {
            for ( hc = 1 ; hc < 9 ; hc++ ) {
                x = hc-1;
                y = (deck-1)*2+1;
                NLM.btns[0][x][y].callback = new HotCueActCallback(NLM, NLM.btns[0][x][y], deck, hc);
            }
        }

        for ( deck = 1; deck < 5; deck++ ) {
            y = (deck-1)*2;
            //Set Chx PlayButton
            NLM.btns[0][0][y].callback = new PlayCallback(NLM.btns[0][0][y], deck);
            //Set Chx LoopButtons
            NLM.btns[0][2][y].callback = new LoopCallback(NLM.btns[0][2][y], deck, "0.0625");
            NLM.btns[0][3][y].callback = new LoopCallback(NLM.btns[0][3][y], deck, "0.125");
            NLM.btns[0][4][y].callback = new LoopCallback(NLM.btns[0][4][y], deck, "0.25");
            NLM.btns[0][5][y].callback = new LoopCallback(NLM.btns[0][5][y], deck, "0.5");
            NLM.btns[0][6][y].callback = new LoopCallback(NLM.btns[0][6][y], deck, "1");
            NLM.btns[0][7][y].callback = new LoopCallback(NLM.btns[0][7][y], deck, "2");
        }

        NLM.btns[0][2][8].callback = new LoopModeCallback(NLM.btns[0][2][8]);
};


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

        print( "COO: " + y + ":" + x);
        NLM.btns[NLM.page][x][y].pressed = pressed;
        NLM.btns[NLM.page][x][y].callback.f();
};

NLM.drawPage = function() {
    for ( x = 0 ; x < 9 ; x++ ) {
        for ( y = 0 ; y < 9 ; y++ ) {
            NLM.btns[NLM.page][x][y].draw();
        }
    }
}

//Define one Key
Key = Object;
Key.prototype.color = colorCode("black");
Key.prototype.x = 0;
Key.prototype.y = 0;
Key.prototype.pressed = false;

Key.prototype.init = function(x,y)
{
    this.x = x;
    this.y = y;
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
    if ( this.y == 8 ) {
        midi.sendShortMsg(0xb0, this.x + 0x68, this.color);
        return;
    }
    midi.sendShortMsg(0x90, this.x+this.y*16, this.color);
    //midi.sendShortMsg(0xb0, 0x0, 0x28); //Enable buffer cycling
}

Key.prototype.callback = Object;
