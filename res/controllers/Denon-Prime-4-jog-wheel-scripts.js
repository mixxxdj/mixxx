// eslint-disable-next-line-no-var
var Prime4Jog = {};

let wheelSide;

const denonHeader = [0x00, 0x02, 0x0B];

/*
// Convert hex value (0xYZ) to individual bytes for SysEx (0x0Y, 0x0Z)
const valueToDenonBytes = function(number) {
    return [(number >> 4) & 0xF, number & 0xF];
};
*/

/*
const colourToDenonBytes = function(colourArray) {
    const array = [];
    for (let i = 0; i < 4; i++) {
        const byte = colourArray[i];
        array.push(...valueToDenonBytes(byte));
    }
    return (array);
};
*/

/*
const changeTextColour = function(display, denonColourArray) {
    const array = [display, 0x08, 0x0b, 0x00, 0x09, 0x08];
    array.push(...colourToDenonBytes(denonColourArray));
    finalSysexWrap(array);
    return (array);
};
*/

/*
const textToSysex = function(text) {
    switch (text) {
    case "1/64":
        return 0x00;
    case "1/32":
        return 0x01;
    case "1/16":
        return 0x02;
    case "1/8":
        return 0x03;
    case "1/4":
        return 0x04;
    case "1/2":
        return 0x05;
    case "1":
        return 0x06;
    case "2":
        return 0x07;
    case "3":
        return 0x10;
    case "4":
        return 0x08;
    case "6":
        return 0x11;
    case "8":
        return 0x09;
    case "12":
        return 0x12;
    case "16":
        return 0x0a;
    case "32":
        return 0x0b;
    case "64":
        return 0x0c;
    case "A":
        return 0x0e;
    case "B":
        return 0x0f;
    case "C":
        return 0x13;
    case "D":
        return 0x14;
    default: // "--"
        return 0x0d;
    }
};
*/

/*
const displayText = function(display, text) {
    const array = [display, 0x08, 0x0A, 0x00, 0x04];
    array.push(0x00, 0x02, 0x00); //TODO: Add flexibility for background images
    array.push(textToSysex(text));
    finalSysexWrap(array);
    return (array);
};
*/

// Simplify process of sending SysEx messages
const sysex = function(sysExMsg) {
    midi.sendSysexMsg(sysExMsg, sysExMsg.length);
};

// Add Denon DJ header bytes + generic SysEx header + footer
const finalSysexWrap = function(array) {
    array.unshift(...denonHeader);
    array.unshift(0xf0);
    array.push(0xf7);
    return array;
};

// Determine which display to send SysEx messages to
const wheelSideToSysex = function(id) {
    switch (id[8]) {
    case "L":
        return 0x10;
    case "R":
        return 0x30;
    default:
        console.log("ERROR: Could not determine each jog wheel display.");
        break;
    }
};

const unlockDisplay = function(display) {
    const array = [display, 0x08, 0x10, 0x00, 0x00];
    finalSysexWrap(array);
    return (array);
};

const blackScreen = function(display) {
    const array = [display, 0x08, 0x0a, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00];
    finalSysexWrap(array);
    return (array);
};

const brightness = function(display, brightness) {
    const array = [display, 0x08, 0x7c, 0x00, 0x01, brightness];
    finalSysexWrap(array);
    return (array);
};

Prime4Jog.init = function(id, _debug) {
    wheelSide = wheelSideToSysex(id);
    sysex(unlockDisplay(wheelSide));
    //sysex(brightness(wheelSide, 0));
};

Prime4Jog.shutdown = function() {
    sysex(brightness(wheelSide, 127));
    sysex(blackScreen(wheelSide));
};
