// Functions common to all controllers go in this file

nop = function () {}    // Only here so you don't get a syntax error on load

function script() {}
script.debug = function (channel, device, control, value, category) {
    print("Script.Debug --- channel: " + channel + " device: " + device + " control: " + control + " value: " + value+ " category: " + category);
}