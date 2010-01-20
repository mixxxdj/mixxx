function BehringerBCD3000 () {}
BehringerBCD3000.debug = false;
BehringerBCD3000.escratch1 = "off";
BehringerBCD3000.escratch2 = "off";

//sensitivity setting
BehringerBCD3000.JogSensivity = 2;
BehringerBCD3000.ScratchSensivity = 10;

BehringerBCD3000.init = function (id) { // called when the device is opened & set up
};

BehringerBCD3000.shutdown = function () {
};

//Scratch, cue search and pitch bend function for channel 1
BehringerBCD3000.jog_wheel1 = function (group, control, value, status) {

    JogValue = (value - 0x40)/BehringerBCD3000.JogSensivity;
    ScratchValue = (value - 0x40)*BehringerBCD3000.ScratchSensivity;
    if (BehringerBCD3000.debug) print("Channel 1 pitching adjust value:" + value + " JogValue:" + JogValue + " Scratch:" + BehringerBCD3000.escratch1);
    if (BehringerBCD3000.escratch1 == "off") engine.setValue("[Channel1]","jog",JogValue);
    if (BehringerBCD3000.escratch1 == "on") engine.setValue("[Channel1]","jog",ScratchValue);
};

//Scratch, cue search and pitch bend function for channel 2
BehringerBCD3000.jog_wheel2 = function (group, control, value, status) {

    JogValue = (value - 0x40)/BehringerBCD3000.JogSensivity;
    ScratchValue = (value - 0x40)*BehringerBCD3000.ScratchSensivity;
    if (BehringerBCD3000.debug) print("Channel 2 pitching adjust value:" + value);
    if (BehringerBCD3000.escratch2 == "off") engine.setValue("[Channel2]","jog",JogValue);
    if (BehringerBCD3000.escratch2 == "on") engine.setValue("[Channel2]","jog",ScratchValue);
};

//Scratch button function for channel 1
BehringerBCD3000.scratch1 = function (group, control, value, status) {
    if (BehringerBCD3000.debug) print("Channel 1 scratch:" + status + "  " + value);
    if ((BehringerBCD3000.escratch1 == "on") && (value == "0x7F")){
        BehringerBCD3000.escratch1 = "off";
        if (BehringerBCD3000.debug) print("Channel 1 scratchoff:" + BehringerBCD3000.escratch1);
        engine.setValue("[Channel1]","scratch",0x00);
        midi.sendShortMsg(0xB0,0x13,0x00);
    }
    else if ((BehringerBCD3000.escratch1 == "off") && (value == "0x7F")) {
        BehringerBCD3000.escratch1 = "on";
        if (BehringerBCD3000.debug) print("Channel 1 scratchon:" + BehringerBCD3000.escratch1);
        midi.sendShortMsg(0xB0,0x13,0x7F);
    }
};

//Scratch button function for channel 2
BehringerBCD3000.scratch2 = function (group, control, value, status) {
    if ((BehringerBCD3000.escratch2 == "on") && (value == "0x7F")){
        BehringerBCD3000.escratch2 = "off";
        engine.setValue("[Channel1]","scratch",0x00);
        midi.sendShortMsg(0xB0,0x0B,0x00);
    }
    else if ((BehringerBCD3000.escratch2 == "off") && (value == "0x7F")) {
        BehringerBCD3000.escratch2 = "on";
        midi.sendShortMsg(0xB0,0x0B,0x7F);
    }
};
