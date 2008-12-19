// Functions common to all controllers go in this file

nop = function () {
    // Only here so you don't get a syntax error on load
}

function script() {}
script.debug = function (channel, device, control, value) {
    print("Script.Debug --- channel: " + channel + " device: " + device + " control: " + control + " value: " + value);
}


function HerculesRMX () {}
HerculesRMX.leds = { "scratch": 0x29 };
HerculesRMX.toggle_scratch_mode = function (channel, device, control, value) {
//    script.debug(channel, device, control, value);
    if (value > 0) {
        var scratch_mode = engine.getValue("[Channel1]","scratch") == 0 && engine.getValue("[Channel2]","scratch") == 0 ? 0x7F : 0;
//        print("scratch mode to " + scratch_mode);
        engine.setValue("[Channel1]","scratch", scratch_mode);
        engine.setValue("[Channel2]","scratch", scratch_mode);
        midi.sendShortMsg(0xB0, HerculesRMX.leds["scratch"] , scratch_mode, device);  // Scratch button
    }
}


