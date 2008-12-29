// Functions common to all controllers go in this file

nop = function () {
    // Only here so you don't get a syntax error on load
}

function script() {}
script.debug = function (channel, device, control, value) {
    print("Script.Debug --- channel: " + channel + " device: " + device + " control: " + control + " value: " + value);
}


function HerculesRMX () {}
HerculesRMX.leds = { "scratch":0x29 };
HerculesRMX.scratchMode = false;
HerculesRMX.playlistJogScrollMode = false;
HerculesRMX.jogWheels = { 0x2F:"[Channel1]", 0x30:"[Channel2]" };
HerculesRMX.toggle_scratch_mode = function (channel, device, control, value) {
    if (value > 0) {
	HerculesRMX.scratchMode = !HerculesRMX.scratchMode;
        midi.sendShortMsg(0xB0, HerculesRMX.leds["scratch"] , (HerculesRMX.scratchMode ? 0x7F : 0x0), device);  // Scratch button
    }
}

HerculesRMX.up_down_arrows = function (channel, device, control, value) {
   script.debug(channel, device, control, value);
   HerculesRMX.playlistJogScrollMode = value > 0;
   switch (control) {
      case 0x2A: engine.setValue("[Playlist]","SelectPrevTrack", (value > 0)); break;
      case 0x2B: engine.setValue("[Playlist]","SelectNextTrack", (value > 0)); break;
   }
}

HerculesRMX.jog_wheel = function (channel, device, control, value) {
//  7F > 40: CCW Slow > Fast - 127 > 64 
//  01 > 3F: CW Slow > Fast - 0 > 63
//   script.debug(channel, device, control, value);
   jogValue = value >=0x7F ? value - 0x80 : value; // -64 to +63, - = CCW, + = CW
   if (HerculesRMX.playlistJogScrollMode) { // zip through library quickly
        engine.setValue("[Playlist]","SelectTrackKnob", jogValue);
   } else if (HerculesRMX.scratchMode) { // do some scratching
       // print("do scratching " + jogValue);
       engine.setValue(HerculesRMX.jogWheels[control],"scratch", jogValue/64);
   } else { // do pitch adjustment
       // print("do pitching adjust " + jogValue);
       engine.setValue(HerculesRMX.jogWheels[control],"wheel", jogValue/32)
   }
}

HerculesRMX.play = function (channel, device, control, value) {
   script.debug(channel, device, control, value);
   if (value > 0) {
     engine.setValue("[Channel1]","play", !engine.getValue("[Channel1]","play"));
   }
}

