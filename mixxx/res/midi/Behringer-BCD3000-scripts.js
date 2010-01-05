function BCD3000 () {}
BCD3000.debug = false;
BCD3000.escratch1 = "off";
BCD3000.escratch2 = "off";

//sensitivity setting
BCD3000.JogSensivity = 2;
BCD3000.ScratchSensivity = 10;

BCD3000.init = function (id) { // called when the device is opened & set up
};

BCD3000.shutdown = function (id) {
};

//Scratch, cue search and pitch bend function for chanel 1
BCD3000.jog_wheel1 = function (group, control, value, status) {

if (value >0x40){
	value = 0x41
}
else value = 0x3F

JogValue = (value - 0x40)/BCD3000.JogSensivity;
ScratchValue = (value - 0x40)*BCD3000.ScratchSensivity;
if (BCD3000.debug) print("Chanel 1 pitching adjust value:" + value + " JogValue:" + JogValue + " Scratch:" + BCD3000.escratch1);
if (BCD3000.escratch1 == "off") engine.setValue("[Channel1]","jog",JogValue);
if (BCD3000.escratch1 == "on") engine.setValue("[Channel1]","jog",ScratchValue);
};

//Scratch, cue search and pitch bend function for chanel 2
BCD3000.jog_wheel2 = function (group, control, value, status) {

JogValue = (value - 0x40)/BCD3000.JogSensivity;
ScratchValue = (value - 0x40)*BCD3000.ScratchSensivity;
if (BCD3000.debug) print("Channel 2 do pitching adjust value:" + value);
if (BCD3000.escratch2 == "off") engine.setValue("[Channel2]","jog",JogValue);
if (BCD3000.escratch2 == "on") engine.setValue("[Channel2]","jog",ScratchValue);

};

//Scratch button function for chanel 1
BCD3000.scratch1 = function (group, control, value, status) {
if (BCD3000.debug) print("Channel 1 sratch:" + status + "  " + value);
if ((BCD3000.escratch1 == "on") && (value == "0x7F")){
	BCD3000.escratch1 = "off";
	if (BCD3000.debug) print("Channel 1 sratchoff:" + BCD3000.escratch1);
	engine.setValue("[Channel1]","scratch",0x00);
	midi.sendShortMsg(0xB0,0x13,0x00);
}
else if ((BCD3000.escratch1 == "off") && (value == "0x7F")) {
	BCD3000.escratch1 = "on";
	if (BCD3000.debug) print("Channel 1 sratchon:" + BCD3000.escratch1);
	midi.sendShortMsg(0xB0,0x13,0x7F);
}
};

//Scratch button function for chanel 2
BCD3000.scratch2 = function (group, control, value, status) {
if ((BCD3000.escratch2 == "on") && (value == "0x7F")){
	BCD3000.escratch2 = "off";
	engine.setValue("[Channel1]","scratch",0x00);
	midi.sendShortMsg(0xB0,0x0B,0x00);
}
else if ((BCD3000.escratch2 == "off") && (value == "0x7F")) {
	BCD3000.escratch2 = "on";
	midi.sendShortMsg(0xB0,0x0B,0x7F);
}
};
