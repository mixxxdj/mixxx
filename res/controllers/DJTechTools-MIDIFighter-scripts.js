// DJ Tech Tools MIDI Fighter
// By RJ Ryan (rryan@mit.edu)
// For Mixxx 1.8.0+

// These scripts are the basics of mapping buttons on the MIDI Fighter to do
// things in Mixxx. Since everyone will have their own preference for what each
// button should do, the basic mappings do nothing more than assign player 1's
// hotcues 1-8 on the top two rows and player 2's hotcues 1-8 on the bottom two
// rows.

// If no mapping is given for a button then the function
// MIDIFighter.buttonX_down will be called when the button is pressed, and
// MIDIFighter.buttonX_up will be called when the button is depressed. This
// makes it easy to extend this script with custom functionality.

// The MIDI Fighter is laid out like this (in terms of the button numbers
// referenced below:
//
//     || <-- usb wire
// ================
// |  1  2  3  4  |
// |  5  6  7  8  |
// |  9 10 11 12  |
// | 13 14 15 16  |
// | MIDI FIGHTER |
// ================

// A big thanks go to Ean Golden of DJ Tech Tools for sending me a MIDI
// Fighter. Mixxx would not have a mapping for the MIDI Fighter if it weren't
// for his generosity.

function MIDIFighter() {}

MIDIFighter.control_map = {
    0x30: 1,
    0x31: 2,
    0x32: 3,
    0x33: 4,
    0x2C: 5,
    0x2D: 6,
    0x2E: 7,
    0x2F: 8,
    0x28: 9,
    0x29: 10,
    0x2A: 11,
    0x2B: 12,
    0x24: 13,
    0x25: 14,
    0x26: 15,
    0x27: 16,
};

MIDIFighter.button_mappings = {
};

MIDIFighter.map_button = function (button, control_object) {
    MIDIFighter.button_mappings[button] = control_object;
}

MIDIFighter.init = function(id) {
    MIDIFighter.id = id;
    print("MIDI Fighter " + MIDIFighter.id + " initialized.");

    MIDIFighter.map_button(1, {'group': '[Channel1]', 'item': 'hotcue_1_activate'});
    MIDIFighter.map_button(2, {'group': '[Channel1]', 'item': 'hotcue_2_activate'});
    MIDIFighter.map_button(3, {'group': '[Channel1]', 'item': 'hotcue_3_activate'});
    MIDIFighter.map_button(4, {'group': '[Channel1]', 'item': 'hotcue_4_activate'});

    MIDIFighter.map_button(5, {'group': '[Channel1]', 'item': 'hotcue_5_activate'});
    MIDIFighter.map_button(6, {'group': '[Channel1]', 'item': 'hotcue_6_activate'});
    MIDIFighter.map_button(7, {'group': '[Channel1]', 'item': 'hotcue_7_activate'});
    MIDIFighter.map_button(8, {'group': '[Channel1]', 'item': 'hotcue_8_activate'});

    MIDIFighter.map_button(9, {'group': '[Channel2]', 'item': 'hotcue_1_activate'});
    MIDIFighter.map_button(10, {'group': '[Channel2]', 'item': 'hotcue_2_activate'});
    MIDIFighter.map_button(11, {'group': '[Channel2]', 'item': 'hotcue_3_activate'});
    MIDIFighter.map_button(12, {'group': '[Channel2]', 'item': 'hotcue_4_activate'});

    MIDIFighter.map_button(13, {'group': '[Channel2]', 'item': 'hotcue_5_activate'});
    MIDIFighter.map_button(14, {'group': '[Channel2]', 'item': 'hotcue_6_activate'});
    MIDIFighter.map_button(15, {'group': '[Channel2]', 'item': 'hotcue_7_activate'});
    MIDIFighter.map_button(16, {'group': '[Channel2]', 'item': 'hotcue_8_activate'});
}

MIDIFighter.shutdown = function() {
    print("MIDI Fighter " + MIDIFighter.id + " shutting down.");
}

MIDIFighter.button_down = function (channel, control, value, status) {
    //print("Button down " + channel + " " + control + " " + value);
    var button_number = MIDIFighter.control_map[control];
    var button_name = 'button' + button_number;
    var button_name_down = 'button' + button_number + "_down";
    if (button_number in MIDIFighter.button_mappings) {
        var control = MIDIFighter.button_mappings[button_number];
        engine.setValue(control.group, control.item, 1);
    } else if (button_name_down in MIDIFighter) {
        MIDIFighter[button_name_down]();
    } else if (button_name in MIDIFighter) {
        MIDIFighter[button_name](1);
    }
}

MIDIFighter.button_up = function (channel, control, value, status) {
    //print("Button up " + channel + " " + control + " " + value);
    var button_number = MIDIFighter.control_map[control];
    var button_name = 'button' + button_number;
    var button_name_up = 'button' + button_number + "_up";

    if (button_number in MIDIFighter.button_mappings) {
        var control = MIDIFighter.button_mappings[button_number];
        engine.setValue(control.group, control.item, 0);
    } else if (button_name_up in MIDIFighter) {
        print ("Calling " + button_name_up);
        MIDIFighter[button_name_up]();
    } else if (button_name in MIDIFighter) {
        MIDIFighter[button_name](0);
    }
}

MIDIFighter.button1_down = function() {
    // Example, if no mapping is made for button 1 then this will be called when
    // button 1 is pressed.
}

MIDIFighter.button1_up = function() {
    // Example, if no mapping is made for button 1 then this will be called when
    // button 1 is released.
}

MIDIFighter.button1 = function() {
    // Example, if no mapping is given for button 1, and no button1_up or
    // button1_down method is defined, then this function will be called with
    // the argument 1 for the button being pressed and 0 for the button being
    // released.
}
