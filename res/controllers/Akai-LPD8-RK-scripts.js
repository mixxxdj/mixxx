function LPD8RK() {}
LPD8RK.debug=false;

//set defaults - don't change these
LPD8RK.oldHotcueBank=0;//storage for the last active hotcue bank - used for bank indicator lights
LPD8RK.LEDinterval=300;//interval in milliseconds for LED update
LPD8RK.hotcueClear=false;//if true, hitting hotcue button clears it.
LPD8RK.loopbuttonDown=false;//if true, special loop functions with knob modifiers are enabled.
LPD8RK.looplen=0x40;//default position of looplength dial - active when loop buttons are held down
LPD8RK.looptype="beatloop";//default looptype - beatloop if <64, beatlooproll otherwise
LPD8RK.loopmove=0.0125;//default loop move distance in seconds
LPD8RK.doreloop=true;//don't change this
LPD8RK.beatjumpstep=1;//default beat jump step in beats
LPD8RK.vol=new Array();//array of volumes to return to after mute - don't change
LPD8RK.hotcueBank=1;//default hotcue bank to show on PAD setting, adjustable by knob


//array of valid LED addresses
LPD8RK.validLEDS = new Array();
LPD8RK.validLEDS[0x90]={0x2b : true,0x27 : true,0x2a : true,0x26 : true,0x29 : true,0x25 : true,0x28 : true,0x24 : true};
LPD8RK.validLEDS[0x91]={0x2c : true,0x27 : true,0x2e : true,0x2a : true,0x26 : true,0x24 : true,0x25 : true,0x23 : true};
LPD8RK.validLEDS[0x92]={0x48 : true,0x41 : true,0x47 : true,0x40 : true,0x45 : true,0x3e : true,0x43 : true,0x3c : true};
LPD8RK.validLEDS[0x93]={0x30 : true,0x29 : true,0x2f : true,0x28 : true,0x2d : true,0x26 : true,0x2b : true,0x24 : true};
LPD8RK.validLEDS[0xb0]={0x01 : true,0x09 : true,0x04 : true,0x08 : true,0x03 : true,0x06 : true,0x03 : true,0x05 : true};
LPD8RK.validLEDS[0xb1]={0x01 : true,0x09 : true,0x04 : true,0x08 : true,0x03 : true,0x06 : true,0x03 : true,0x05 : true};
LPD8RK.validLEDS[0xb2]={0x01 : true,0x09 : true,0x04 : true,0x08 : true,0x03 : true,0x06 : true,0x03 : true,0x05 : true};
LPD8RK.validLEDS[0xb3]={0x01 : true,0x09 : true,0x04 : true,0x08 : true,0x03 : true,0x06 : true,0x03 : true,0x05 : true};


//map hotcue pads and leds
LPD8RK.hotcues = new Array();
//preset 1 (group, cuenum, status, ctrl)
//initialize
LPD8RK.hotcues["902b"] = new Array();
LPD8RK.hotcues["9027"] = new Array();
LPD8RK.hotcues["902a"] = new Array();
LPD8RK.hotcues["9026"] = new Array();
LPD8RK.hotcues["9029"] = new Array();
LPD8RK.hotcues["9025"] = new Array();
//bank 1
LPD8RK.hotcues["902b"][1] = new Array("[Channel1]", "1", 0x90, 0x2b);
LPD8RK.hotcues["9027"][1] = new Array("[Channel1]", "2", 0x90, 0x27);
LPD8RK.hotcues["902a"][1] = new Array("[Channel1]", "3", 0x90, 0x2a);
LPD8RK.hotcues["9026"][1] = new Array("[Channel1]", "4", 0x90, 0x26);
LPD8RK.hotcues["9029"][1] = new Array("[Channel1]", "5", 0x90, 0x29);
LPD8RK.hotcues["9025"][1] = new Array("[Channel1]", "6", 0x90, 0x25);
//bank 2
LPD8RK.hotcues["902b"][2] = new Array("[Channel1]", "7", 0x90, 0x2b);
LPD8RK.hotcues["9027"][2] = new Array("[Channel1]", "8", 0x90, 0x27);
LPD8RK.hotcues["902a"][2] = new Array("[Channel1]", "9", 0x90, 0x2a);
LPD8RK.hotcues["9026"][2] = new Array("[Channel1]", "10", 0x90, 0x26);
LPD8RK.hotcues["9029"][2] = new Array("[Channel1]", "11", 0x90, 0x29);
LPD8RK.hotcues["9025"][2] = new Array("[Channel1]", "12", 0x90, 0x25);
//bank 3
LPD8RK.hotcues["902b"][3] = new Array("[Channel1]", "13", 0x90, 0x2b);
LPD8RK.hotcues["9027"][3] = new Array("[Channel1]", "14", 0x90, 0x27);
LPD8RK.hotcues["902a"][3] = new Array("[Channel1]", "15", 0x90, 0x2a);
LPD8RK.hotcues["9026"][3] = new Array("[Channel1]", "16", 0x90, 0x26);
LPD8RK.hotcues["9029"][3] = new Array("[Channel1]", "17", 0x90, 0x29);
LPD8RK.hotcues["9025"][3] = new Array("[Channel1]", "18", 0x90, 0x25);
//bank 4
LPD8RK.hotcues["902b"][4] = new Array("[Channel1]", "19", 0x90, 0x2b);
LPD8RK.hotcues["9027"][4] = new Array("[Channel1]", "20", 0x90, 0x27);
LPD8RK.hotcues["902a"][4] = new Array("[Channel1]", "21", 0x90, 0x2a);
LPD8RK.hotcues["9026"][4] = new Array("[Channel1]", "22", 0x90, 0x26);
LPD8RK.hotcues["9029"][4] = new Array("[Channel1]", "23", 0x90, 0x29);
LPD8RK.hotcues["9025"][4] = new Array("[Channel1]", "24", 0x90, 0x25);
//bank 5
LPD8RK.hotcues["902b"][5] = new Array("[Channel1]", "25", 0x90, 0x2b);
LPD8RK.hotcues["9027"][5] = new Array("[Channel1]", "26", 0x90, 0x27);
LPD8RK.hotcues["902a"][5] = new Array("[Channel1]", "27", 0x90, 0x2a);
LPD8RK.hotcues["9026"][5] = new Array("[Channel1]", "28", 0x90, 0x26);
LPD8RK.hotcues["9029"][5] = new Array("[Channel1]", "29", 0x90, 0x29);
LPD8RK.hotcues["9025"][5] = new Array("[Channel1]", "30", 0x90, 0x25);

//##################
//preset 2 (group, cuenum, status, ctrl)
//initialize
LPD8RK.hotcues["912c"] = new Array();
LPD8RK.hotcues["9127"] = new Array();
LPD8RK.hotcues["912e"] = new Array();
LPD8RK.hotcues["912a"] = new Array();
LPD8RK.hotcues["9126"] = new Array();
LPD8RK.hotcues["9124"] = new Array();
//bank 1
LPD8RK.hotcues["912c"][1] = new Array("[Channel2]", "1", 0x91, 0x2c);
LPD8RK.hotcues["9127"][1] = new Array("[Channel2]", "2", 0x91, 0x27);
LPD8RK.hotcues["912e"][1] = new Array("[Channel2]", "3", 0x91, 0x2e);
LPD8RK.hotcues["912a"][1] = new Array("[Channel2]", "4", 0x91, 0x2a);
LPD8RK.hotcues["9126"][1] = new Array("[Channel2]", "5", 0x91, 0x26);
LPD8RK.hotcues["9124"][1] = new Array("[Channel2]", "6", 0x91, 0x24);
//bank 2
LPD8RK.hotcues["912c"][2] = new Array("[Channel2]", "7", 0x91, 0x2c);
LPD8RK.hotcues["9127"][2] = new Array("[Channel2]", "8", 0x91, 0x27);
LPD8RK.hotcues["912e"][2] = new Array("[Channel2]", "9", 0x91, 0x2e);
LPD8RK.hotcues["912a"][2] = new Array("[Channel2]", "10", 0x91, 0x2a);
LPD8RK.hotcues["9126"][2] = new Array("[Channel2]", "11", 0x91, 0x26);
LPD8RK.hotcues["9124"][2] = new Array("[Channel2]", "12", 0x91, 0x24);
//bank 3
LPD8RK.hotcues["912c"][3] = new Array("[Channel2]", "13", 0x91, 0x2c);
LPD8RK.hotcues["9127"][3] = new Array("[Channel2]", "14", 0x91, 0x27);
LPD8RK.hotcues["912e"][3] = new Array("[Channel2]", "15", 0x91, 0x2e);
LPD8RK.hotcues["912a"][3] = new Array("[Channel2]", "16", 0x91, 0x2a);
LPD8RK.hotcues["9126"][3] = new Array("[Channel2]", "17", 0x91, 0x26);
LPD8RK.hotcues["9124"][3] = new Array("[Channel2]", "18", 0x91, 0x24);
//bank 4
LPD8RK.hotcues["912c"][4] = new Array("[Channel2]", "19", 0x91, 0x2c);
LPD8RK.hotcues["9127"][4] = new Array("[Channel2]", "20", 0x91, 0x27);
LPD8RK.hotcues["912e"][4] = new Array("[Channel2]", "21", 0x91, 0x2e);
LPD8RK.hotcues["912a"][4] = new Array("[Channel2]", "22", 0x91, 0x2a);
LPD8RK.hotcues["9126"][4] = new Array("[Channel2]", "23", 0x91, 0x26);
LPD8RK.hotcues["9124"][4] = new Array("[Channel2]", "24", 0x91, 0x24);
//bank 5
LPD8RK.hotcues["912c"][5] = new Array("[Channel2]", "25", 0x91, 0x2c);
LPD8RK.hotcues["9127"][5] = new Array("[Channel2]", "26", 0x91, 0x27);
LPD8RK.hotcues["912e"][5] = new Array("[Channel2]", "27", 0x91, 0x2e);
LPD8RK.hotcues["912a"][5] = new Array("[Channel2]", "28", 0x91, 0x2a);
LPD8RK.hotcues["9126"][5] = new Array("[Channel2]", "29", 0x91, 0x26);
LPD8RK.hotcues["9124"][5] = new Array("[Channel2]", "30", 0x91, 0x24);

//##################
//preset 3 (group, cuenum, status, ctrl)
//initialize
LPD8RK.hotcues["9248"] = new Array();
LPD8RK.hotcues["9241"] = new Array();
LPD8RK.hotcues["9247"] = new Array();
LPD8RK.hotcues["9240"] = new Array();
LPD8RK.hotcues["9245"] = new Array();
LPD8RK.hotcues["923e"] = new Array();
//bank 1
LPD8RK.hotcues["9248"][1] = new Array("[Sampler1]", "1", 0x92, 0x48);
LPD8RK.hotcues["9241"][1] = new Array("[Sampler1]", "2", 0x92, 0x41);
LPD8RK.hotcues["9247"][1] = new Array("[Sampler1]", "3", 0x92, 0x47);
LPD8RK.hotcues["9240"][1] = new Array("[Sampler1]", "4", 0x92, 0x40);
LPD8RK.hotcues["9245"][1] = new Array("[Sampler1]", "5", 0x92, 0x45);
LPD8RK.hotcues["923e"][1] = new Array("[Sampler1]", "6", 0x92, 0x3e);
//bank 2
LPD8RK.hotcues["9248"][2] = new Array("[Sampler1]", "7", 0x92, 0x48);
LPD8RK.hotcues["9241"][2] = new Array("[Sampler1]", "8", 0x92, 0x41);
LPD8RK.hotcues["9247"][2] = new Array("[Sampler1]", "9", 0x92, 0x47);
LPD8RK.hotcues["9240"][2] = new Array("[Sampler1]", "10", 0x92, 0x40);
LPD8RK.hotcues["9245"][2] = new Array("[Sampler1]", "11", 0x92, 0x45);
LPD8RK.hotcues["923e"][2] = new Array("[Sampler1]", "12", 0x92, 0x3e);
//bank 3
LPD8RK.hotcues["9248"][3] = new Array("[Sampler1]", "13", 0x92, 0x48);
LPD8RK.hotcues["9241"][3] = new Array("[Sampler1]", "14", 0x92, 0x41);
LPD8RK.hotcues["9247"][3] = new Array("[Sampler1]", "15", 0x92, 0x47);
LPD8RK.hotcues["9240"][3] = new Array("[Sampler1]", "16", 0x92, 0x40);
LPD8RK.hotcues["9245"][3] = new Array("[Sampler1]", "17", 0x92, 0x45);
LPD8RK.hotcues["923e"][3] = new Array("[Sampler1]", "18", 0x92, 0x3e);
//bank 4
LPD8RK.hotcues["9248"][4] = new Array("[Sampler1]", "19", 0x92, 0x48);
LPD8RK.hotcues["9241"][4] = new Array("[Sampler1]", "20", 0x92, 0x41);
LPD8RK.hotcues["9247"][4] = new Array("[Sampler1]", "21", 0x92, 0x47);
LPD8RK.hotcues["9240"][4] = new Array("[Sampler1]", "22", 0x92, 0x40);
LPD8RK.hotcues["9245"][4] = new Array("[Sampler1]", "23", 0x92, 0x45);
LPD8RK.hotcues["923e"][4] = new Array("[Sampler1]", "24", 0x92, 0x3e);
//bank 5
LPD8RK.hotcues["9248"][5] = new Array("[Sampler1]", "25", 0x92, 0x48);
LPD8RK.hotcues["9241"][5] = new Array("[Sampler1]", "26", 0x92, 0x41);
LPD8RK.hotcues["9247"][5] = new Array("[Sampler1]", "27", 0x92, 0x47);
LPD8RK.hotcues["9240"][5] = new Array("[Sampler1]", "28", 0x92, 0x40);
LPD8RK.hotcues["9245"][5] = new Array("[Sampler1]", "29", 0x92, 0x45);
LPD8RK.hotcues["923e"][5] = new Array("[Sampler1]", "30", 0x92, 0x3e);

//##################
//preset 4 (group, cuenum, status, ctrl)
//initialize
LPD8RK.hotcues["9330"] = new Array();
LPD8RK.hotcues["9329"] = new Array();
LPD8RK.hotcues["932f"] = new Array();
LPD8RK.hotcues["9328"] = new Array();
LPD8RK.hotcues["932d"] = new Array();
LPD8RK.hotcues["9326"] = new Array();
//bank 1
LPD8RK.hotcues["9330"][1] = new Array("[Sampler2]", "1", 0x93, 0x30);
LPD8RK.hotcues["9329"][1] = new Array("[Sampler2]", "2", 0x93, 0x29);
LPD8RK.hotcues["932f"][1] = new Array("[Sampler2]", "3", 0x93, 0x2f);
LPD8RK.hotcues["9328"][1] = new Array("[Sampler2]", "4", 0x93, 0x28);
LPD8RK.hotcues["932d"][1] = new Array("[Sampler2]", "5", 0x93, 0x2d);
LPD8RK.hotcues["9326"][1] = new Array("[Sampler2]", "6", 0x93, 0x26);
//bank 2
LPD8RK.hotcues["9330"][2] = new Array("[Sampler2]", "7", 0x93, 0x30);
LPD8RK.hotcues["9329"][2] = new Array("[Sampler2]", "8", 0x93, 0x29);
LPD8RK.hotcues["932f"][2] = new Array("[Sampler2]", "9", 0x93, 0x2f);
LPD8RK.hotcues["9328"][2] = new Array("[Sampler2]", "10", 0x93, 0x28);
LPD8RK.hotcues["932d"][2] = new Array("[Sampler2]", "11", 0x93, 0x2d);
LPD8RK.hotcues["9326"][2] = new Array("[Sampler2]", "12", 0x93, 0x26);
//bank 3
LPD8RK.hotcues["9330"][3] = new Array("[Sampler2]", "13", 0x93, 0x30);
LPD8RK.hotcues["9329"][3] = new Array("[Sampler2]", "14", 0x93, 0x29);
LPD8RK.hotcues["932f"][3] = new Array("[Sampler2]", "15", 0x93, 0x2f);
LPD8RK.hotcues["9328"][3] = new Array("[Sampler2]", "16", 0x93, 0x28);
LPD8RK.hotcues["932d"][3] = new Array("[Sampler2]", "17", 0x93, 0x2d);
LPD8RK.hotcues["9326"][3] = new Array("[Sampler2]", "18", 0x93, 0x26);
//bank 4
LPD8RK.hotcues["9330"][4] = new Array("[Sampler2]", "19", 0x93, 0x30);
LPD8RK.hotcues["9329"][4] = new Array("[Sampler2]", "20", 0x93, 0x29);
LPD8RK.hotcues["932f"][4] = new Array("[Sampler2]", "21", 0x93, 0x2f);
LPD8RK.hotcues["9328"][4] = new Array("[Sampler2]", "22", 0x93, 0x28);
LPD8RK.hotcues["932d"][4] = new Array("[Sampler2]", "23", 0x93, 0x2d);
LPD8RK.hotcues["9326"][4] = new Array("[Sampler2]", "24", 0x93, 0x26);
//bank 5
LPD8RK.hotcues["9330"][5] = new Array("[Sampler2]", "25", 0x93, 0x30);
LPD8RK.hotcues["9329"][5] = new Array("[Sampler2]", "26", 0x93, 0x29);
LPD8RK.hotcues["932f"][5] = new Array("[Sampler2]", "27", 0x93, 0x2f);
LPD8RK.hotcues["9328"][5] = new Array("[Sampler2]", "28", 0x93, 0x28);
LPD8RK.hotcues["932d"][5] = new Array("[Sampler2]", "29", 0x93, 0x2d);
LPD8RK.hotcues["9326"][5] = new Array("[Sampler2]", "30", 0x93, 0x26);




//map hotcue loop and leds
LPD8RK.loops = new Array();
//preset 1 (group, beatloop_len, status, ctrl)
LPD8RK.loops["b09"] = new Array("[Channel1]", "1", 0xB0, 0x09);
LPD8RK.loops["b04"] = new Array("[Channel1]", "2", 0xB0, 0x04);
LPD8RK.loops["b08"] = new Array("[Channel1]", "4", 0xB0, 0x08);
LPD8RK.loops["b03"] = new Array("[Channel1]", "8", 0xB0, 0x03);

//preset 2 (group, beatloop_len, status, ctrl)
LPD8RK.loops["b19"] = new Array("[Channel2]", "1", 0xB1, 0x09);
LPD8RK.loops["b14"] = new Array("[Channel2]", "2", 0xB1, 0x04);
LPD8RK.loops["b18"] = new Array("[Channel2]", "4", 0xB1, 0x08);
LPD8RK.loops["b13"] = new Array("[Channel2]", "8", 0xB1, 0x03);

//preset 3 (group, beatloop_len, status, ctrl)
LPD8RK.loops["b29"] = new Array("[Sampler1]", "1", 0xB2, 0x09);
LPD8RK.loops["b24"] = new Array("[Sampler1]", "2", 0xB2, 0x04);
LPD8RK.loops["b28"] = new Array("[Sampler1]", "4", 0xB2, 0x08);
LPD8RK.loops["b23"] = new Array("[Sampler1]", "8", 0xB2, 0x03);

//preset 4 (group, beatloop_len, status, ctrl)
LPD8RK.loops["b39"] = new Array("[Sampler2]", "1", 0xB3, 0x09);
LPD8RK.loops["b34"] = new Array("[Sampler2]", "2", 0xB3, 0x04);
LPD8RK.loops["b38"] = new Array("[Sampler2]", "4", 0xB3, 0x08);
LPD8RK.loops["b33"] = new Array("[Sampler2]", "8", 0xB3, 0x03);



LPD8RK.init = function (id, debug) { // called when the device is opened & set up
    if (LPD8RK.debug){print("###init##############")};
    //soft takeovers
    engine.softTakeover("[Master]","crossfader",true);
    engine.softTakeover("[Channel1]","volume",true);
    engine.softTakeover("[Channel1]","rate",true);
    engine.softTakeover("[Channel2]","volume",true);
    engine.softTakeover("[Channel2]","rate",true);
    engine.softTakeover("[Sampler1]","volume",true);
    engine.softTakeover("[Sampler1]","rate",true);
    engine.softTakeover("[Sampler2]","volume",true);
    engine.softTakeover("[Sampler2]","rate",true);
    
    //set LED timer
    LPD8RK.ledTimer = engine.beginTimer(LPD8RK.LEDinterval, "LPD8RK.setLeds()");
};

LPD8RK.shutdown = function () {
    engine.stopTimer(LPD8RK.ledTimer);
};

LPD8RK.resetLEDTimer = function () {
    engine.stopTimer(LPD8RK.ledTimer);
    LPD8RK.setLeds()
    LPD8RK.ledTimer = engine.beginTimer(LPD8RK.LEDinterval, "LPD8RK.setLeds()");
};

LPD8RK.setLeds = function () {
    //runs repeatedly on a timer set in init()
    //has to run on a timer because you apparently can only set LEDs on the current prog selection on the LPD8
    //ie: leds on a PAD or CC other than the one you're currently using can't be set.
    //and no midi messages are sent when you change a PROG or pad.  No way of telling MIXXX which LEDs are currently active
    //so, this runs 4x a second to make sure LEDs are updated.  Kludge, but so it goes.

    //hotcues
    for (var id in LPD8RK.hotcues){
        for (var bank in LPD8RK.hotcues[id]){
            //iterate through hotcues for current bank, set hotcue leds
            var status = LPD8RK.hotcues[id][LPD8RK.hotcueBank][2];
            var ctrl = LPD8RK.hotcues[id][LPD8RK.hotcueBank][3];
            var state = engine.getValue(LPD8RK.hotcues[id][LPD8RK.hotcueBank][0], "hotcue_"+LPD8RK.hotcues[id][LPD8RK.hotcueBank][1]+"_enabled");

            //if (LPD8RK.debug){print("midi.sendShortMsg("+status+", "+ctrl+", "+state+")")};
            
            LPD8RK.lightLED(status, ctrl, state);
            };
        };
    
    //loops
    for (var id in LPD8RK.loops){
        //iterate through hotcues, set hotcue leds
        var status = LPD8RK.loops[id][2];
        var ctrl = LPD8RK.loops[id][3];
        var state = engine.getValue(LPD8RK.loops[id][0], "beatloop_"+LPD8RK.loops[id][1]+"_enabled");

        //if (LPD8RK.debug){print("midi.sendShortMsg("+status+", "+ctrl+", "+state+")")};
    
        LPD8RK.lightLED(status, ctrl, state);
        };
    
    //reloop buttons
    LPD8RK.lightLED(0xb0, 0x01, engine.getValue("[Channel1]", "loop_enabled"));    
    LPD8RK.lightLED(0xb1, 0x01, engine.getValue("[Channel2]", "loop_enabled"));    
    LPD8RK.lightLED(0xb2, 0x01, engine.getValue("[Sampler1]", "loop_enabled"));    
    LPD8RK.lightLED(0xb3, 0x01, engine.getValue("[Sampler2]", "loop_enabled"));    
    };

LPD8RK.lightLED = function (status, ctrl, state){
    //function to check for valid LED lighting messages - LPD8 seems to go into a weird nonresponsive mode when some unknown code is sent to it
    //don't know what the messed code is, but this function checks for valid LED addresses.  Hopefully this will prevent problems.
    if (LPD8RK.validLEDS[status][ctrl] !== true){
        print("######## INVALID STATUS ########");
        return false;
        } else {
        if (state > 0 || state === true){state=1;}//make sure state is valid
        midi.sendShortMsg(status, ctrl, state);
        }
    };    
    
LPD8RK.clear = function (){//enables hotcue clearing
    if (LPD8RK.debug){print("###hotcueclear##############")};
    LPD8RK.hotcueClear=true;
    };

LPD8RK.noclear = function (){//disables hotcue clearing
    LPD8RK.hotcueClear=false;
    };

LPD8RK.hotcueButton = function (channel, control, value, status, group) {
    //weird status bug workaround
    if (channel==0 && status != 0x90){status = 0x90};
    if (channel==1 && status != 0x91){status = 0x91};
    if (channel==2 && status != 0x92){status = 0x92};
    if (channel==3 && status != 0x93){status = 0x93};

    if (LPD8RK.debug){print("###hotcueButton##############")};
    if (LPD8RK.debug){print("status:"+status)};
    if (LPD8RK.debug){print("channel:"+channel)};
    if (LPD8RK.debug){print("control:"+control)};
    if (LPD8RK.debug){print("hotcuebank:"+LPD8RK.hotcueBank)};
    if (LPD8RK.debug){print("phrase:#"+status.toString(16).toLowerCase()+control.toString(16).toLowerCase()+"#")};
    //activate or clear depending on whether clear button is pressed
    var thecue = LPD8RK.hotcues[status.toString(16).toLowerCase()+control.toString(16).toLowerCase()][LPD8RK.hotcueBank];
    if (LPD8RK.hotcueClear){
        engine.setValue(thecue[0], "hotcue_"+thecue[1]+"_clear", 1);
        engine.setValue(thecue[0], "hotcue_"+thecue[1]+"_clear", 0);
        if (LPD8RK.debug){print("cleared")};
        } else {
        engine.setValue(thecue[0], "hotcue_"+thecue[1]+"_activate", 1);
        engine.setValue(thecue[0], "hotcue_"+thecue[1]+"_activate", 0);
        if (LPD8RK.debug){print("###"+status.toString(16).toLowerCase()+control.toString(16).toLowerCase()+"--activated")};
        };

    };

LPD8RK.loopButton = function (channel, control, value, status, group) {
    if (LPD8RK.debug){print(LPD8RK.looplen+"len");}
    if (LPD8RK.debug){print(LPD8RK.looptype+"type");}
    //activate beatloop
    var theloop = LPD8RK.loops[status.toString(16).toLowerCase()+control.toString(16).toLowerCase()];
    if (value>0){//button was pressed
        LPD8RK.loopbuttonDown=true;
        engine.setValue(group, LPD8RK.looptype+"_"+theloop[1]+"_activate", 1);
        } else {//button was released
        LPD8RK.loopbuttonDown=false;
        };

    };

LPD8RK.reloopButton = function (channel, control, value, status, group) {
    if (LPD8RK.debug){print(LPD8RK.looplen+"len");}
    if (LPD8RK.debug){print(LPD8RK.looptype+"type");}
    
    if (value>0){//button was pressed
        engine.stopTimer(LPD8RK.reloopTimer);
        LPD8RK.loopbuttonDown=true;
        LPD8RK.doreloop=true;
        LPD8RK.reloopTimer = engine.beginTimer(500, "LPD8RK.disablereloop()", true);
        } else {//button was released
        LPD8RK.loopbuttonDown=false;
        if (LPD8RK.doreloop===true) {engine.setValue(group, "reloop_exit", 1);};
        LPD8RK.doreloop=true;
        engine.stopTimer(LPD8RK.reloopTimer);
        };

    };

LPD8RK.disablereloop = function () {
    //timed function - fires half a second after pressing reloop.  Don't do the reloop if you hold down the button (so you can move the loop without exiting)
    LPD8RK.doreloop=false;
    };

LPD8RK.looptypeDial = function (channel, control, value, status, group) {
    //activates variable length type depending on dial position
    //beatlooproll only works in 1.11 or above - script is a placeholder, knob used to select hotcue bank for now
    //if(value>63){LPD8RK.looptype="beatlooproll";}else{LPD8RK.looptype="beatloop";}
    //if (LPD8RK.debug){print(LPD8RK.looptype);}
    };

LPD8RK.resetOldBank = function () {
    //clears stored old hotcue bank
    LPD8RK.oldHotcueBank=0;
    };

LPD8RK.hotcueBankDial = function (channel, control, value, status, group) {
    //sets which hotcue bank to display (separate this out on dedicated controller)
    
    //pause LED resets, so bank indicator lights will be visible
    engine.stopTimer(LPD8RK.ledTimer);

    //select hotcue bank
    if (value>=0 && value <=12){LPD8RK.hotcueBank=1;};
    if (value>12 && value <=46){LPD8RK.hotcueBank=2;};
    if (value>46 && value <=80){LPD8RK.hotcueBank=3;};
    if (value>80 && value <=114){LPD8RK.hotcueBank=4;};
    if (value>114 && value <=128){LPD8RK.hotcueBank=5;};

    //light up indicator light
    if (LPD8RK.oldHotcueBank != LPD8RK.hotcueBank){//check if the bank's changed.  If it has, change the LEDs - LPD8RK.oldHotcueBank != LPD8RK.hotcueBank
        for (var id in LPD8RK.hotcues){
            for (var bank in LPD8RK.hotcues[id][1]){
                var status = LPD8RK.hotcues[id][1][2];
                var ctrl = LPD8RK.hotcues[id][1][3];
                //find bank number indicator light
                if (LPD8RK.hotcues[id][1][1]==LPD8RK.hotcueBank){
                    var state = 1;
                    }else{
                    var state = 0;
                    };

                LPD8RK.lightLED(status, ctrl, state);
                };
            };
        };
    //record last hotcue bank
    LPD8RK.oldHotcueBank=LPD8RK.hotcueBank;
    //set timer to clear old bank number after 500 msec, so bank indicator light will light up
    engine.stopTimer(LPD8RK.oldbanktimer);
    LPD8RK.oldbanktimer = engine.beginTimer(500, "LPD8RK.resetOldBank()", true);    

    //set timer to restart LED updates in 500 msec
    engine.stopTimer(LPD8RK.LEDPauseTimer);
    LPD8RK.LEDPauseTimer = engine.beginTimer(LPD8RK.LEDinterval, "LPD8RK.resetLEDTimer()", true);    
    };

LPD8RK.looplenDial = function (channel, control, value, status, group) {
    //activates variable length loop depending on dial position
    LPD8RK.looplen=value;

    if (LPD8RK.loopbuttonDown !== true){return false;}//exit if no loop button down 
    else if (LPD8RK.looplen<=0x12){engine.setValue(group, LPD8RK.looptype+"_0.0625_activate", .0625);return true;} else 
    if (LPD8RK.looplen<=0x25){engine.setValue(group, LPD8RK.looptype+"_0.125_activate", .125);return true;} else 
    if (LPD8RK.looplen<=0x37){engine.setValue(group, LPD8RK.looptype+"_0.25_activate", .25);return true;} else 
    if (LPD8RK.looplen<=0x49){engine.setValue(group, LPD8RK.looptype+"_0.5_activate", .5);return true;} else 
    if (LPD8RK.looplen<=0x5b){engine.setValue(group, LPD8RK.looptype+"_1_activate", 1);return true;} else 
    if (LPD8RK.looplen<=0x6d){engine.setValue(group, LPD8RK.looptype+"_2_activate", 2);return true;} else 
    if (LPD8RK.looplen<=0x70){engine.setValue(group, LPD8RK.looptype+"_4_activate", 4);return true;} else    
    if (LPD8RK.looplen<=0x7f){engine.setValue(group, LPD8RK.looptype+"_8_activate", 8);return true;};    
    };

LPD8RK.loopminus = function (channel, control, value, status, group) {
    //shrinks loop or moves loop back
    if (LPD8RK.loopbuttonDown !== true){engine.setValue(group, "loop_halve", 1);engine.setValue(group, "loop_halve", 0); return false;}//shrink loop if no loop button down 
    else if (engine.getValue(group, "loop_start_position")>=0 && engine.getValue(group, "loop_end_position")>=0 ){
        //move loop
        var interval =    LPD8RK.loopmove*engine.getValue(group, "track_samples")/engine.getValue(group, "duration");
        var start = engine.getValue(group, "loop_start_position");
        var end = engine.getValue(group, "loop_end_position");
        engine.setValue(group, "loop_start_position", start-interval);
        engine.setValue(group, "loop_end_position", end-interval);
        return true;
        };
    };

LPD8RK.loopplus = function (channel, control, value, status, group) {
    //grows loop or moves loop forward
    if (LPD8RK.loopbuttonDown !== true){engine.setValue(group, "loop_double", 1); engine.setValue(group, "loop_double", 0); return false;}//shrink loop if no loop button down 
    else if (engine.getValue(group, "loop_start_position")>=0 && engine.getValue(group, "loop_end_position")>=0 ){
        //move loop
        var interval =    LPD8RK.loopmove*engine.getValue(group, "track_samples")/engine.getValue(group, "duration");
        var start = engine.getValue(group, "loop_start_position");
        var end = engine.getValue(group, "loop_end_position");
        engine.setValue(group, "loop_start_position", start+interval);
        engine.setValue(group, "loop_end_position", end+interval);
        return true;
        };
    };

LPD8RK.beatjump = function (channel, control, value, status, group) {
    //jumps back certain number of beats depending on knob modifier
    var curpos = engine.getValue(group, "playposition")*engine.getValue(group, "track_samples");
    var numbeats = LPD8RK.beatjumpstep;
    var backseconds = numbeats*(1/(engine.getValue(group, "bpm")/60));
    var backsamples = backseconds*engine.getValue(group, "track_samples")/engine.getValue(group, "duration");
    var newpos = curpos-(backsamples+engine.getValue("Master", "latency"));
    
    if (LPD8RK.debug){print("backseconds: "+backseconds);}
    if (LPD8RK.debug){print("backsamples: "+backsamples);}
    if (LPD8RK.debug){print("curpos: "+curpos);}
    if (LPD8RK.debug){print("newpos: "+newpos);}
    if (LPD8RK.debug){print("numbeats: "+numbeats);}
    
    engine.setValue(group, "playposition", newpos/engine.getValue(group, "track_samples"));
    };

LPD8RK.beatjumpDial = function (channel, control, value, status, group) {
    //activates variable length loop depending on dial position
    if(value>=0 && value <=127){
        if (value<=1){LPD8RK.beatjumpstep=.25; return true;} else 
        if (value<=31){LPD8RK.beatjumpstep=.5; return true;} else     
        if (value<=63){LPD8RK.beatjumpstep=1; return true;} else     
        if (value<=94){LPD8RK.beatjumpstep=2; return true;} else     
        if (value<=125){LPD8RK.beatjumpstep=4; return true;} else     
        if (value<=127){LPD8RK.beatjumpstep=8; return true;};
        };    
    };

LPD8RK.mute = function (group) {
    //toggles mute, then returns to previous volume
    storedvol=LPD8RK.vol[group];
    curvol=engine.getValue(group, "volume");
    engine.softTakeover(group,"volume",false);
    if (curvol==0){//is muted.  unmute.
        engine.setValue(group, "volume", storedvol);
        }else{//is not muted.  mute.
        engine.setValue(group, "volume", 0);
        LPD8RK.vol[group]=curvol;
        };
    engine.softTakeover(group,"volume",true);
    
    if (LPD8RK.debug){print("MUTE");}
    };

LPD8RK.progChng = function (channel, control, value, status, group) {
    if (LPD8RK.debug){print("###PROG CHANGE###");}
    //workaround because prog chng buttons don't seem to like to work unless they're linked to scripts
    if (control==0x07){LPD8RK.toggleplay(group, engine.getValue(group, "play")); return true;} else 
    if (control==0x03){engine.setValue(group, "cue_default", true); engine.setValue(group, "cue_default", false); return true;} else 
    if (control==0x06){engine.setValue(group, "beatsync", true); engine.setValue(group, "beatsync", false); return true;} else 
    if (control==0x02){LPD8RK.togglepfl(group, engine.getValue(group, "pfl")); return true;} else 
    if (control==0x05){LPD8RK.togglereverse(group, engine.getValue(group, "reverse")); return true;} else 
    if (control==0x04){LPD8RK.toggleback(group, engine.getValue(group, "back")); return true;} else 
    if (control==0x00){LPD8RK.togglefwd(group, engine.getValue(group, "fwd")); return true;}
    if (control==0x01){LPD8RK.mute(group); return true;}
    };

LPD8RK.toggleplay = function (group, state) {
    if (state==true){engine.setValue(group, "reverse",0);engine.setValue(group, "play", false);} else {engine.setValue(group, "reverse",0);engine.setValue(group, "play", true);};
    };

LPD8RK.togglepfl = function (group, state) {
    if (state==true){engine.setValue(group, "pfl", false);} else {engine.setValue(group, "pfl", true);};
    };

LPD8RK.togglereverse = function (group, state) {
    if (state==true){engine.setValue(group, "reverse", false);} else {engine.setValue(group, "reverse", true);};    
    };

LPD8RK.toggleback = function (group, state) {
    if (state==true){engine.setValue(group, "back", false);} else {engine.setValue(group, "back", true);};    
    };

LPD8RK.togglefwd = function (group, state) {
    if (state==true){engine.setValue(group, "fwd", false);} else {engine.setValue(group, "fwd", true);};    
    };

LPD8RK.softXfade = function (channel, control, value, status, group) {
    engine.setValue(group, "crossfader", (value/64)-1);
    };

LPD8RK.softVolume = function (channel, control, value, status, group) {
    engine.setValue(group, "volume", (value/127));
    };

LPD8RK.softRate = function (channel, control, value, status, group) {
    engine.setValue(group, "rate", (value/64)-1);
    };


LPD8RK.test = function (channel, control, value, status, group) {
    print("channel: "+channel);
    print("control: "+control.toString(16));
    print("value: "+value);
    print("status: "+status);
    print("group: "+group);
    print("test: ##"+status.toString(16)+control.toString(16)+"##");
    print("loopbuttondown: ##"+LPD8RK.loopbuttonDown+"##");
    print("LPD8RK.looplen: ##"+LPD8RK.looplen+"##");
    print("LPD8RK.looplen.tostring: ##"+LPD8RK.looplen.toString(16)+"##");
    print("LPD8RK.looptype: ##"+LPD8RK.looptype+"##");
    print("test: ##"+2+"##");
    };


