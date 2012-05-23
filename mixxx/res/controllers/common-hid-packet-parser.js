
// Common HID script debugging function. Just to get logging with 'HID' prefix.
script.HIDDebug = function (message) { print("HID " + message); }

// Standard target groups not resolved for controls. This is used by HID packet
// parser to recognize group parameters we should try sending to mixxx.
HIDTargetGroups = [
    '[Channel1]','[Channel2]','[Channel3]','[Channel4]',
    '[Sampler1]','[Sampler2]','[Sampler3]','[Sampler4]',
    '[Master]','[Effects]','[Playlist]','[Flanger]',
    '[Microphone]'
]

//
// HID Bit Vector Class
//
// Collection of bits in one parsed packet field. These objects are
// created by HIDPacket addControl and addLED and should not be
// created manually.
function HIDBitVector () {
    // Nothing to initialize. Do NOT add any attributes to this object!
    // Attributes here are the 'bits' addressed by HIDBitVector[bit_name]
}

// Add a control button bit to the HIDBitVector at given bit index
HIDBitVector.prototype.addBit = function(group,name,index) {
    var bit = new Object();
    bit.type = 'button';
    bit.id = group+'.'+name;
    bit.group = group;
    bit.name = name;
    bit.index = index;
    bit.callback = undefined;
    bit.value = undefined;
    this[bit.id] = bit;
}

// Add a LED control bit to the HIDBitVector at given bit index
HIDBitVector.prototype.addLED = function(group,name,index) {
    var bit = new Object();
    bit.type = 'led';
    bit.id = group+'.'+name;
    bit.group = group;
    bit.name = name;
    bit.index = index;
    bit.active_group = undefined;
    bit.callback = undefined;
    bit.value = undefined;
    bit.blink = undefined;
    this[bit.id] = bit;
}

//
// HID Packet Class
//
// One HID input/output packet to register to HIDController
// name     name of packet
// header   list of bytes to match from beginning of packet
// length   packet length
// callback function to call when this packet type is input
//          and is received. If packet callback is set, the
//          packet is not parsed by delta functions.
//          callback is not meaningful for output packets
function HIDPacket (name,header,length,callback) {
    this.name = name;
    this.header = header;
    this.length = length;
    this.callback = callback;

    // Size of various 'pack' values in bytes
    this.packSizes = { b: 1, B: 1, h: 2, H: 2, i: 4, I: 4 };
    this.signedPackFormats = [ 'b', 'h', 'i'];
}

// Pack a field value to the packet.
// Can only pack bits and byte values, patches welcome.
HIDPacket.prototype.pack = function(packet,field) {
    if (field.type=='bitvector') {
        for (bit_id in field.value) {
            var bit = field.value[bit_id];
            var bit_offset = field.offset+bit.index%8;
            packet.data[bit_offset] += bit.value<<bit.index%8;
        }
        return;
    }
    if (!(field.pack in this.packSizes)) {
        script.HIDDebug("Error packing field: unknown pack value " + field.pack);
        return;
    }
    var signed = false;
    if (this.signedPackFormats.indexOf(field.pack)!=-1)
        signed = true;

    // TODO - implement packing anything else but unsigned byte
    packet.data[field.offset] = field.value;

}

// Parse and return field value matching the 'pack' field from field attributes.
// Valid values are:
//  b       signed byte
//  B       unsigned byte
//  h       signed short
//  H       unsigned short
//  i       signed integer
//  I       unsigned integer
HIDPacket.prototype.unpack = function(data,field) {
    var value = 0;
    
    if (!(field.pack in this.packSizes)) {
        script.HIDDebug("ERROR parsing packed value: invalid pack format " + field.pack);
        return;
    }
    var bytes = this.packSizes[field.pack];
    var signed = false;
    if (this.signedPackFormats.indexOf(field.pack)!=-1)
        signed = true;
    

    for (field_byte=0;field_byte<bytes;field_byte++) {
        if (data[field.offset+field_byte]==255 && field_byte==4)
            value += 0;
        else
            value += data[field.offset+field_byte] * Math.pow(2,(field_byte*8));
    }
    if (signed==true) {
        var max_value = Math.pow(2,bytes*8);
        var split = max_value/2-1;
        if (value>split) 
            value = value-max_value;
    }
    return value;
}

// Lookup HID packet group matching name.
// Create group if create is true
HIDPacket.prototype.lookupGroup = function(name,create) {
    if (this.groups==undefined)
        this.groups = new Object();
    if (name in this.groups)
        return this.groups[name];
    if (!create)
        return undefined;

    this.groups[name] = new Object();
    return this.groups[name];
}

// Lookup HID packet field matching given offset and pack type
// Returns undefined if no patching field can be found.
HIDPacket.prototype.lookupFieldByOffset = function(offset,pack) {
    if (!(pack in this.packSizes)) {
        script.HIDDebug("Unknown pack string " + pack);
        return undefined;
    }
    var end_offset = offset + this.packSizes[pack];
    if (end_offset>this.length) {
        script.HIDDebug("Invalid offset+pack range " +
            offset + '-' + end_offset +
            " for " +  this.length + " byte packet"
        );
        return undefined;
    }
    var group = undefined;
    var field = undefined;
    for (var group_name in this.groups) {
        group = this.groups[group_name];
        for (var field_id in group) {
            field = group[field_id];
            if (field.offset>=offset && end_offset<=field.end_offset) {
                return field;
            }
        }
    }
    return undefined;
}

// Return a field by group and name from the packet,
// Returns undefined if field could not be found
HIDPacket.prototype.lookupField = function(group,name) {
    var field_id = group+'.'+name;
    if (!(group in this.groups)) {
        script.HIDDebug("PACKET " + this.name + " group not found " + group);
        return undefined;
    }
    var control_group = this.groups[group];
    if (field_id in control_group)
        return control_group[field_id];

    // Lookup for bit fields in bitvector matching field name
    for (var group_name in this.groups) {
        var control_group = this.groups[group_name];
        for (field_id in control_group) {
            var field = control_group[field_id];
            if (field.type!='bitvector')
                continue
            for (bit_id in field.value) {
                var bit = field.value[bit_id];
                if (bit.id==bit_id) {
                    return field;
                }
                print("BIT ID " + bit.id + " FIELD ID " + bit_id);
            }
        }
    }

    return undefined;
}

// Register a numeric value to parse from input packet
// Parameters:
// group     control group name
// name      name of the field
// offset    field offset inside packet (bytes)
// pack      control packing format for unpack()
// bitmask   bitmask size, bit offset for buttons, undefined for controls
// isEncoder indicates if this is an encoder which should be wrapped and delta reported
// callback  callback function to apply to the field value, or undefined for no callback
//
HIDPacket.prototype.addControl = function(group,name,offset,pack,bitmask,isEncoder) {
    var control_group = this.lookupGroup(group,true);
    var bitvector = undefined;
    if (control_group==undefined) {
        script.HIDDebug('ERROR creating HID packet group ' + group);
        return;
    }
    if (!(pack in this.packSizes)) {
        script.HIDDebug('Unknown pack value ' + pack);
        return;
    }

    var field = this.lookupFieldByOffset(offset,pack);
    if (field!=undefined) {
        if (bitmask==undefined) {
            script.HIDDebug("ERROR trying to overwrite non-bitmask control " + group + " " + name);
            return;
        }
        bitvector = field.value;
        bitvector.addBit(group,name,bitmask);
        return;
    }

    // Define new field to add
    field = new Object();
    field.id = group+'.'+name;    
    field.group = group;
    field.name = name;
    field.pack = pack;
    field.offset = offset;
    field.end_offset = offset + this.packSizes[field.pack];
    field.bitmask = bitmask;
    field.isEncoder = isEncoder;
    field.callback = undefined;
    field.sof_takeover = false;
    field.ignored = false;

    var packet_max_value = Math.pow(2,this.packSizes[field.pack]*8);
    if (this.signedPackFormats.indexOf(pack)!=-1) {
        field.min = 0 - (packet_max_value/2)+1;
        field.max = (packet_max_value/2)-1;
    } else {
        field.min = 0;
        field.max = packet_max_value-1;
    }

    if (bitmask==undefined || bitmask==packet_max_value) {
        field.type = 'control';
        field.value = undefined;
        field.delta = 0;
        field.mindelta = 0;
    } else {
        // Create a new bitvector field and add the bit to that
        // TODO - accept controls with bitmask < packet_max_value
        field_name = 'bitvector_' + offset;
        field.type = 'bitvector';
        field.name = field_name;
        field.id = group+'.'+field_name;
        bitvector = new HIDBitVector();
        bitvector.addBit(group,name,bitmask);
        field.value = bitvector;
        field.delta = undefined;
        field.sof_takeover = undefined;
        field.mindelta = undefined;
    }

    // Add the new field to the packet
    control_group[field.id] = field;
}

// Register a LED control field or LED control bit to output packet
// LED control field:   LED field with no bitmask, controls LED with multiple values
// LED control bit:     LED with with bitmask, controls LED with a single bit
// It is recommended to define callbacks after packet creationg with
// registerCallback instaed of adding it directly here. But you can do it.
HIDPacket.prototype.addLED = function(group,name,offset,pack,bitmask,callback) {
    var control_group = this.lookupGroup(group,true);
    var field = undefined;
    var bitvector = undefined;
    var field_id = group+'.'+name;

    if (control_group==undefined) {
        return;
    }
    if (!(pack in this.packSizes)) {
        script.HIDDebug("ERROR: unknown LED control pack value " + pack);
        return;
    }

    // Check if we are adding a LED bit to existing bitvector
    field = this.lookupFieldByOffset(offset,pack);
    if (field!=undefined) {
        if (bitmask==undefined) {
            script.HIDDebug("ERROR: trying to overwrite non-bitmask control " + group + " " + name);
            return;
        }
        bitvector = field.value;
        bitvector.addBit(group,name,bitmask);
        return;
    }

    field = new Object();
    field.id = field_id;
    field.group = group;
    field.name = name;
    field.active_group = undefined;
    field.pack = pack;
    field.offset = offset;
    field.end_offset = offset + this.packSizes[field.pack];
    field.bitmask = bitmask;
    field.callback = callback;
    field.blink = undefined;

    var packet_max_value = Math.pow(2,this.packSizes[field.pack]*8);
    if (bitmask==undefined || bitmask==packet_max_value) {
        field.type = 'led';
        field.value = undefined;
        field.delta = undefined;
        field.mindelta = undefined;
    } else {
        // Create new LED bitvector control field, add bit to it
        // rewrite name to use bitvector instead
        field_name = 'bitvector_' + offset;
        field.type = 'bitvector';
        field.id = group+'.'+field_name;
        field.name = field_name;
        bitvector = new HIDBitVector();
        bitvector.addLED(group,name,bitmask);
        field.value = bitvector;
        field.delta = undefined;
        field.mindelta = undefined;
    }

    // Add LED to HID packet
    control_group[field.id] = field;

}

// Register a callback to field or bitvector's bit
// Does not make sense for LED fields but you can do that.
HIDPacket.prototype.registerCallback = function(group,name,callback) {
    var field = this.lookupField(group,name);
    var field_id = group+'.'+name;
    if (field==undefined) {
        script.HIDDebug("ERROR in registerCallback: field for " +field_id+ " not found");
        return;
    }
    if (field.type=='bitvector') {
        for (var bit_id in field.value) {
            var bit = field.value[bit_id];
            if (bit_id!=field_id)
                continue;
            bit.callback = callback;
        }
    } else {
        field.callback = callback;
    }
}

// Set 'ignored' flag for field to given value (true or false)
// If field is ignored, it is not reported in 'delta' objects.
HIDPacket.prototype.setIgnored = function(group,name,ignored) {
    var field = this.lookupField(group,name);
    if (field==undefined) {
        script.HIDDebug("ERROR setting ignored flag for " + group +' ' + name);
        return;
    }
    field.ignored = ignored;
}

// Adjust field's minimum delta value.
// Input value changes smaller than this are not reported in delta
HIDPacket.prototype.setMinDelta = function(group,name,mindelta) {
    field = this.lookupField(group,name);
    if (field==undefined) {
        script.HIDDebug("ERROR adjusting mindelta for " + group +' ' + name);
        return;
    }
    if (field.type=='bitvector') {
        script.HIDDebug("ERROR setting mindelta for bitvector packet does not make sense");
        return;
    }
    field.mindelta = mindelta;
}

// Parse bitvector field values, returning object with the named bits set.
// Value must be a valid unsigned byte to parse, with enough bits.
// Returns list of modified bits (delta)
HIDPacket.prototype.parseBitVector = function(field,value) {
    var bits = new Object();
    var bit;
    var new_value;
    for (var bit_id in field.value) {
        bit = field.value[bit_id];
        new_value = value>>bit.index&1;
        if (new_value!=bit.value) {
            bit.value = new_value;
            bits[bit_id] = bit;
        }
    }
    return bits;
}

// Parse input packet fields from data.
// Data is expected to be a Packet() received from HID device.
// Returns list of changed fields with new value.
// BitVectors are returned as bits you can iterate separately.
HIDPacket.prototype.parse = function(data) {
    var field_changes = new Object();
    var group;
    var group_name;
    var field;
    var field_name;
    var field_id;
    var bit;
    var bit_value;

    for (group_name in this.groups) {
        group = this.groups[group_name];
        for (field_id in group) {
            field = group[field_id];

            var value = this.unpack(data,field);
            if (value == undefined) {
                script.HIDDebug("Error parsing packet field value for " + field_id);
                return;
            }

            if (field.type=='bitvector') {
                var bits = this.parseBitVector(field,value);
                for (bit in bits) {
                    bit_value = bits[bit];
                    field_changes[bit] = bit_value;;
                }

            } else if (field.type=='control') {
                if (field.value==value)
                    continue;
                if (field.ignored==true || field.value==undefined) {
                    field.value = value;
                    continue
                }
                if (field.isEncoder) {
                    if (field.value==field.max && value==field.min) {
                        change = 1;
                        field.delta = 1;
                    } else if (value==field.max && field.value==field.min) {
                        change = 1;
                        field.delta = -1;
                    } else {
                        change = 1;
                        field.delta = value-field.value;
                    }
                    field.value = value;
                } else {
                    var change = Math.abs(value-field.value);
                    field.delta = value-field.value;
                }
                if (field.mindelta==undefined || change>field.mindelta) {
                    field_changes[field.name] = field;
                    field.value = value;
                }
            }
        }
    }
    return field_changes;
}

// Send this HID packet to device.
// First the header bytes are copied to beginning of packet, then
// field object values are packed to the HID packet according to the
// field type.
HIDPacket.prototype.send = function() {
    var offset = 0;
    var i;
    var group_name;
    var group;
    var name;
    var packet = new Packet(this.length);

    for (header_byte=0;header_byte<this.header.length;header_byte++) {
        packet.data[header_byte] = this.header[header_byte];
    }

    for (var group_name in this.groups) {
        group = this.groups[group_name];
        for (var field_id in group) {
            var field = group[field_id];
            this.pack(packet,field);
        }
    }
    controller.send(packet.data, packet.length, 0);
}

//
// HID Controller Class
//
// HID Controller with packet parser
// Global attributes include:
//
// initialized          by default false, you should set this to true when
//                      controller is found and everything is OK
// activeDeck           by default undefined, used to map the virtual deck
//                      names 'deck','deck1' and 'deck2' to actual [ChannelX]
// isScratchEnabled     set to true, when button 'jog_touch' is active
// buttonStates         valid state values for buttons, should contain fields
//                      released (default 0) and pressed (default 1)
// LEDColors            possible LED colors named, must contain 'off' value
// deckLEDColors        Which colors to use for each deck. Default 'on' for first
//                      four decks. Values are like {1: 'red', 2: 'green' }
//                      and must reference valid LEDColors fields.
// LEDUpdateInterval    By default undefined. If set, it's a value for timer
//                      executed every n ms to update LEDs with updateLEDs()
// modifiers            Object containing 'modifier' key names registered with
//                      registerModifier()
// toggleButtons        List of button names you wish to act as 'toggle', i.e.
//                      pressing the button and releasing toggles state of the
//                      control and does not set it off again when released.
//
// Scratch variables (initialized with 'common' defaults, you can override):
// scratchintervalsPerRev   Intervals value for scratch_enable
// scratchRPM               RPM value for scratch_enable
// scratchAlpha             Alpha value for scratch_enable
// scratchBeta              Beta value for scratch_enable
// scratchRampOnEnable      If 'ramp' is used when enabling scratch
// scratchRampOnDisable     If 'ramp' is used when disabling scratch
function HIDController () {
    this.initialized = false;
    this.activeDeck = undefined;


    // Scratch parameter defaults for this.scratchEnable function
    // override for custom control
    this.isScratchEnabled = false;
    this.scratchintervalsPerRev = 128;
    this.scratchRPM = 33+1/3;
    this.scratchAlpha = 1.0/8;
    this.scratchBeta = this.scratchAlpha /32;
    this.scratchRampOnEnable = false;
    this.scratchRampOnDisable = false;

    this.buttonStates = { released: 0, pressed: 1};
    this.LEDColors = {off: 0x0, on: 0x7f};
    // Override to set specific colors for multicolor button LED per deck
    this.deckLEDColors = {1: 'on', 2: 'on', 3: 'on', 4: 'on'};
    // Set to value in ms to update LEDs periodically
    this.LEDUpdateInterval = undefined;

    this.modifiers = new Object();
    this.scalers = new Object();
    // Toggle buttons
    this.toggleButtons = [ 'play', 'pfl', 'keylock' ]
}

// Initialize our packet data and callbacks. This does not seem to
// work when executed from here, but we keep a stub just in case.
HIDController.prototype.initializePacketData = function() { }

// Return deck number from deck name. Deck name can't be virtual deck name
// in this function call.
HIDController.prototype.resolveDeck = function(group) {
    if (group==undefined)
        return undefined;
    var result = group.match(/\[Channel[0-9]+\]/);
    if (!result)
        return undefined;
    var str = group.replace(/\[Channel/,"");
    return str.substring(0,str.length-1);
}

// Return the group name from given deck number.
HIDController.prototype.resolveDeckGroup = function(deck) {
    if (deck==undefined)
        return undefined;
    return "[Channel"+deck+"]";
}

// Map virtual deck names to real deck group. If group is already
// a real mixxx group value, just return it as it without mapping.
HIDController.prototype.resolveGroup = function(group) {
    var channel_name = /\[Channel[0-9]+\]/;
    if (group!=undefined && group.match(channel_name) )
        return group;
    if (group=='deck' || group==undefined) {
        if (this.activeDeck==undefined)
            return undefined;
        return '[Channel' + this.activeDeck + ']';
    }
    if (this.activeDeck==1 || this.activeDeck==2) {
        if (group=='deck1') return '[Channel1]';
        if (group=='deck2') return '[Channel2]';
    }
    if (this.activeDeck==3 || this.activeDeck==4) {
        if (group=='deck3') return '[Channel3]';
        if (group=='deck4') return '[Channel4]';
    }
    return undefined;
}

// Find LED output control matching give group and name
// Returns undefined if LED can't be found.
HIDController.prototype.resolveLED = function(group,name) {
    var led_id = group+'.'+name;
    if (this.ledgroups==undefined) {
        this.ledgroups = new Object();
    }
    if (!(group in this.ledgroups)) {
        this.ledgroups[group] = new Object();
    }
    var led_group = this.ledgroups[group];
    if (!(led_id in led_group))
        return undefined;
    return led_group[led_id];
}

// Find input packet matching given name.
// Returns undefined if input packet name is not registered.
HIDController.prototype.resolveInputPacket = function(name) {
    if (!(name in this.InputPackets))
        return undefined;
    return this.InputPackets[name];
}

// Find output packet matching given name
// Returns undefined if output packet name is not registered.
HIDController.prototype.resolveOutputPacket = function(name) {
    if (!(name in this.OutputPackets))
        return undefined;
    return this.OutputPackets[name];
}

// Lookup scaling function for control
// Returns undefined if function is not registered.
HIDController.prototype.lookupScalingFunction = function(name,callback) {
    if (!(name in this.scalers))
        return undefined;
    return this.scalers[name];
}

// Add a LED to controller's list of LEDs.
// THIS IS PRIVATE METHOD!
// DO NOT call directly, let HIDPacket.addLED call this for you
HIDController.prototype.addLED = function(packet,field) {
    var group = field.group;
    var name = field.name;

    var controller_led = this.resolveLED(group,name);
    if (controller_led!=undefined) {
        script.HIDDebug("ERROR Duplicate LED specification for " + field.id);
        return;
    }
    var leds = this.ledgroups;
    if (!(group in leds)) {
        script.HIDDebug("ERROR: group was undefined while adding LED");
        return;
    }
    if (!(group in this.ledgroups)) {
        script.HIDDebug("LED group was not defined when adding LED");
        return;
    }
    var led_group = this.ledgroups[group];
    var led = new Object();
    led.packet = packet;
    led.field = field;
    led_group[field.id] = led;
}

// Update all output packets with LEDs on device to current state.
// If from_timer is true, you can toggle LED color for blinking with timer.
HIDController.prototype.updateLEDs = function(from_timer) {
    var led_packets = [];
    for (var group_name in this.ledgroups) {
        var group = this.ledgroups[group_name];
        for (var led_id in group) {
            var led = group[led_id];
            if (from_timer==true)
                this.toggleLEDBlinkState(group_name,led.field.name);
            if (led.packet==undefined) {
                script.HIDDebug("BUG: Invalid LED, no packet defined");
                continue;
            }
            if (led_packets.indexOf(led.packet)==-1)
                led_packets[led_packets.length] = led.packet;
        }
    }
    for (led_index=0;led_index<led_packets.length;led_index++) {
        var packet = led_packets[led_index];
        packet.send();
    }
}

// Disconnect LEDs for given virtual deck from mixxx engine.
// This must be called before swapping virtual decks.
HIDController.prototype.disconnectDeckLEDs = function() {
    var virtual_groups = ['deck','deck1','deck2'];
    for (var group_name in this.ledgroups) {
        if (virtual_groups.indexOf(group_name)==-1)
            continue;
        var group = this.ledgroups[group_name];
        var active_group = this.resolveGroup(group_name);
        var active = this.resolveDeck(active_group);
        if (active==undefined) {
            script.HIDDebug("Active deck could not resolved for LED " + led_name);
            return;
        }
        if (!(active in this.deckLEDColors)) {
            script.HIDDebug("Not a valid deckLEDColors deck number index: " + active);
            return;
        }
        var active_color = this.deckLEDColors[active];
        for (var led_id in group) {
            var led = group[led_id];
            engine.connectControl("[Channel"+active+"]",led.field.name,"EksOtus.activeLEDUpdateWrapper",true);
        }
    }
}

// Connect LEDs for given virtual deck to mixxx engine.
// This must be called after swapping virtual decks.
HIDController.prototype.connectDeckLEDs = function() {
    var virtual_groups = ['deck','deck1','deck2'];
    for (var group_name in this.ledgroups) {
        if (virtual_groups.indexOf(group_name)==-1)
            continue;
        var group = this.ledgroups[group_name];
        var active_group = this.resolveGroup(group_name);
        var active = this.resolveDeck(active_group);
        if (active==undefined) {
            script.HIDDebug("Active deck could not resolved for LED " + led_name);
            return;
        }
        if (!(active in this.deckLEDColors)) {
            script.HIDDebug("Not a valid deckLEDColors deck number index: " + active);
            return;
        }
        var active_color = this.deckLEDColors[active];
        for (var led_id in group) {
            var led = group[led_id];
            engine.connectControl("[Channel"+active+"]",led.field.name,"EksOtus.activeLEDUpdateWrapper");
        }
    }
}

// Update LEDs for all active virtual decks
HIDController.prototype.updateActiveDeckLEDs = function() {
    var virtual_groups = ['deck','deck1','deck2'];
    for (var group_name in this.ledgroups) {
        // Only alter LEDs in virtual groups 'deck', 'deck1' and 'deck2'
        if (virtual_groups.indexOf(group_name)==-1)
            continue;
        var group = this.ledgroups[group_name];
        var active_group = this.resolveGroup(group_name);
        var active = this.resolveDeck(active_group);
        if (active==undefined) {
            script.HIDDebug("Active deck could not resolved for LED " + led_name);
            return;
        }
        if (!(active in this.deckLEDColors)) {
            script.HIDDebug("Not a valid deckLEDColors deck number index: " + active);
            return;
        }
        var active_color = this.deckLEDColors[active];

        for (var led_id in group) {
            var led = group[led_id];
            var color = undefined;
            var state = engine.getValue(active_group,led.field.name);
            if (engine.getValue(active_group,led.field.name)) {
                color = active_color;
            } else {
                color = this.LEDColors['off'];
            }

            led.field.value = this.LEDColors[color];
            led.field.blink = undefined;
        }
    }
    this.updateLEDs();
}

// Toggle color of a blinking led set by setLedBlink.
// Called from updateLEDs timer when it's in timer mode.
HIDController.prototype.toggleLEDBlinkState = function(group,name) {
    var led = this.resolveLED(group,name);
    if (led==undefined) {
        script.HIDDebug("toggleLEDBlinkState: Unknown LED group " +group+'.'+name);
        return;
    }
    var field = led.field;
    if (field.blink==undefined)
        return;
    if (field.value == this.LEDColors['off']) {
        field.value = field.blink;
    } else {
        field.value = this.LEDColors['off'];
    }
}

// Set LED state to given LEDColors value, disable blinking as side effect.
HIDController.prototype.setLED = function(group,name,color,send_packet) {
    if (!(color in this.LEDColors)) {
        script.HIDDebug("Invalid LED color: " + color);
        return;
    }
    var led = this.resolveLED(group,name);
    if (led==undefined) {
        script.HIDDebug("setLED: Unknown LED group " +group+'.'+name);
        return;
    }
    var field = led.field;
    field.value = this.LEDColors[color];
    field.blink = undefined;
    if (send_packet==undefined || send_packet==true)
        led.packet.send();
}

// Set LED to blink with given color. Reset with setLED(name,'off')
// We only support blinking between 'off' and color, not between 2 colors.
HIDController.prototype.setLEDBlink = function(group,name,blink_color) {
    if (blink_color!=undefined && !(blink_color in this.LEDColors)) {
        script.HIDDebug("setLEDBlink: Invalid LED blink color: " + color);
        return;
    }
    var led = this.resolveLED(group,name);
    if (led==undefined) {
        script.HIDDebug("setLEDBlink: Unknown LED group " +group+'.'+name);
        return;
    }
    var field = led.field;
    field.value = this.LEDColors[blink_color];
    field.blink = this.LEDColors[blink_color];
    led.packet.send();
}

// Register packet's field callback.
// If packet has callback, it is still parsed but no field processing is done,
// callback is called directly after unpacking fields from packet.
HIDController.prototype.registerInputCallback = function(packet,group,name,callback) {
    var input_packet = this.resolveInputPacket(packet);
    if (input_packet==undefined) {
        script.HIDDebug("Input packet not found " + packet);
        return;
    }
    // script.HIDDebug("Registered callback for " + group+'.'+name + " to input packet " + input_packet.name);
    input_packet.registerCallback(group,name,callback);
}

// Register scaling function for a control name
// This does not check if given control name is valid
HIDController.prototype.registerScalingFunction = function(name,callback) {
    if (name in this.scalers)
        return;
    this.scalers[name] = callback;
}

// Register input packet type to controller.
// Input packets can be responses from device to queries, or control data information
// The default control data packet must be called 'control' to allow automatic processing.
HIDController.prototype.registerInputPacket = function(input_packet) {
    var group;
    var name;
    var field;

    if (this.InputPackets==undefined)
        this.InputPackets = new Object();
    // Find modifiers and other special cases from packet fields
    for (group in input_packet.groups) {
        for (var field_id in input_packet.groups[group]) {
            field = input_packet.groups[group][field_id];
            if (field.type=='bitvector') {
                for (var bit_id in field.value) {
                    var bit = field.value[bit_id];
                    if (bit.group=='modifiers') {
                        // Register modifier name
                        this.registerModifier(bit.name);
                    }
                }
            }
        }
    }
    this.InputPackets[input_packet.name] = input_packet;
}

// Register output packet type to controller
// There are no special LED control output packets, just register LEDs to any
// valid packet and we detect them here.
// This module only supports sending bitvector values and byte fields to device.
// If you need other data structures, patches are welcome, or you can just do it
// manually in your script without registering the packet.
HIDController.prototype.registerOutputPacket = function(output_packet) {
    var group;
    var name;
    var field;
    // Find LEDs from packet by 'led' type
    for (group in output_packet.groups) {
        for (var field_id in output_packet.groups[group]) {
            field = output_packet.groups[group][field_id];
            if (field.type!='led')
                continue;
            this.addLED(output_packet,field);
        }
    }
    if (this.OutputPackets==undefined)
        this.OutputPackets = new Object();
    this.OutputPackets[output_packet.name] = output_packet;
}

// Register a modifier button to controller.
// Modifiers are set to 'true' while pressed, and 'false' or undefined when not active
HIDController.prototype.registerModifier = function(name) {
    if (name in this.modifiers) {
        script.HIDDebug("ERROR: modifier already registered: " + name);
        return;
    }
    // script.HIDDebug("Registered modifier " + name);
    this.modifiers[name] = undefined;
}

// Parse a received input packet fields with 'unpack' calls to fields
// Calls packet callback and returns, if packet callback was defined
// Calls processIncomingPacket and processes automated events there.
// If defined, calls processDelta for results after processing automated fields
HIDController.prototype.parsePacket = function(data,length) {
    var packet;
    var changed_data;

    if (this.InputPackets==undefined) {
        return;
    }

    for (var name in this.InputPackets) {
        packet = this.InputPackets[name];
        if (packet.length!=length) {
            continue;
        }
        // Check for packet header match against data
        for (var header_byte=0;header_byte<packet.header.length;header_byte++) {
            if (packet.header[header_byte]!=data[header_byte]) {
                packet=undefined;
                break;
            }
        }
        if (packet==undefined)
            continue;

        changed_data = packet.parse(data);
        if (packet.callback!=undefined) {
            packet.callback(packet,changed_data);
            return;
        }
        // Process named group controls
        if (packet.name=='control')
            this.processIncomingPacket(packet,changed_data);
        // Process generic changed_data packet, if callback is defined
        if (this.processDelta!=undefined)
            this.processDelta(packet,changed_data);
        return;
    }
    script.HIDDebug("Received unknown packet of " + length + " bytes");
}

// Process the modified field values (delta) from input packet fields for
// input control packet, if packet name is 'control'.
//
// Button field processing
// - Sets modifiers from buttons
// - Calls button callbacks, if defined
// - Finally tries to run matching engine.setValue() function for buttons in default
// mixxx groups, honoring toggleButtons and other button details. Not done if a callback
// was defined for button.
//
// Control field processing
// - Calls scaling functions for control fields, if defined for field. Scaling function for
// encoders (isEncoder attribute is true) scales field delta instead of raw value.
// - Calls callback functions for control fields, if defined for field
// - Finally tries run matching engine.setValue() function for control fields
// in default mixxx groups. Not done if a callback was defined.
//
HIDController.prototype.processIncomingPacket = function(packet,delta) {
    var field;
    var value;
    var group;

    for (var name in delta) {
        if (this.ignoredControlChanges!=undefined) {
            if (this.ignoredControlChanges.indexOf(name)!=-1)
                continue;
        }
        field = delta[name];
        if (field.group==undefined) {
            if (this.activeDeck!=undefined)
                group = '[Channel' + this.activeDeck + ']';
        } else {
            group = field.group;
        }
        if (field.type=='button') {
            if (group=='modifiers') {
                if (!field.id in this.modifiers) {
                    script.HIDDebug("Unknown modifier ID" + field.name);
                    continue;
                }
                if (field.value==this.buttonStates.pressed)
                    this.modifiers[field.name] = true;
                else
                    this.modifiers[field.name] = false;
                continue;
            }
            if (field.callback!=undefined) {
                field.callback(field);
                continue;
            }

            // Verify and resolve group for standard buttons
            group = field.group;
            if (HIDTargetGroups.indexOf(group)==-1) {
                if (this.resolveGroup!=undefined)
                    group = this.resolveGroup(field.group);
                if (HIDTargetGroups.indexOf(group)==-1) {
                    if (this.activeDeck!=undefined)
                        script.HIDDebug("Error resolving button ID " + field.id);
                    continue;
                }
            }

            if (field.name=='jog_touch') {
                if (group!=undefined) {
                    if (field.value==this.buttonStates.pressed) {
                        this.setScratchEnabled(group,true);
                    } else {
                        this.setScratchEnabled(group,false);
                    }
                }
                var active_group = this.resolveGroup(field.group);

            } else if (this.toggleButtons.indexOf(field.name)!=-1) {
                if (field.value==this.buttonStates.released)
                    continue;
                if (engine.getValue(group,field.name)) {
                    if (field.name=='play')
                        engine.setValue(group,'stop',true);
                    else
                        engine.setValue(group,field.name,false);
                } else {
                    engine.setValue(group,field.name,true);
                }
            } else if (engine.getValue(group,field.name)==false) {
                engine.setValue(group,field.name,true);

            } else {
                engine.setValue(group,field.name,false);
            }

        } else if (field.type=='control') {
            if (field.callback!=undefined) {
                value = field.callback(field);
                continue;
            }

            if (field.name=='jog_wheel') {
                // Handle jog wheel scratching transparently
                this.jog_wheel(field);
                continue;
            }

            // Verify and resolve group
            group = field.group;
            if (HIDTargetGroups.indexOf(group)==-1) {
                if (this.resolveGroup!=undefined) {
                    group = this.resolveGroup(field.group);
                }
                if (HIDTargetGroups.indexOf(group)==-1) {
                    continue;
                }
            }

            value = field.value;
            scaler = this.lookupScalingFunction(name);
            if (field.isEncoder==true) {
                var field_delta = field.delta;
                if (scaler!=undefined)
                   field_delta = scaler(group,name,field_delta);
                engine.setValue(group,name,field_delta);
            } else {
                if (scaler!=undefined)
                   value = scaler(group,name,value);
                engine.setValue(group,name,value);
            }
        }
    }
}

// Processing of the 'jog_touch' special button name, which is used to detect
// when scratching should be enabled.
// Deck is resolved from group with 'resolveDeck'

// Enabling scratching (press 'jog_touch' button)
// Sets the internal 'isScratchEnabled' attribute to true, and calls scratchEnable
// with the scratch attributes (see class defination)

// Disabling scratching (release 'jog_touch' button)
// Sets the internal 'isScratchEnabled attribute to false, and calls scratchDisable
// to end scratching mode
//
HIDController.prototype.setScratchEnabled = function(group,status) {
    var deck = this.resolveDeck(group);
    if (status==true) {
        this.isScratchEnabled = true;
        engine.scratchEnable(deck,
            this.scratchintervalsPerRev,
            this.scratchRPM,
            this.scratchAlpha,
            this.scratchBeta,
            this.rampedScratchEnable
        );
    } else {
        this.isScratchEnabled = false;
        engine.scratchDisable(deck,this.rampedScratchDisable);
    }
}

// Default jog scratching function. Used to handle jog move events from special
// input control field called 'jog_wheel'. Handles both 'scratch' and 'jog' mixxx
// functions, depending on isScratchEnabled value above (see setScratchEnabled())
//
// Since most controllers require value scaling for jog and scratch functions, you
// are warned if following scaling function names are not registered:
//
// jog          Scaling function from 'jog_wheel' for rate bend events with mixxx 'jog'
//              function. Should return value range suitable for 'jog', whatever you
//              wish it to do.
// jog_scratch  Scaling function from 'jog_wheel' for scratch movements with mixxx
//              'scratchTick' function. Should return -1,0,1 or small ranges of integers
//              both negative and positive values.
//
HIDController.prototype.jog_wheel = function(field) {
    var scaler = undefined;
    var active_group = this.resolveGroup(field.group);
    var value = undefined;
    if (field.isEncoder)
        value = field.delta;
    else
        value = field.value;

    if (this.isScratchEnabled==true) {
        var deck = this.resolveDeck(active_group);
        if (deck==undefined)
            return;
        scaler = this.lookupScalingFunction('jog_scratch');
        if (scaler!=undefined)
            value = scaler(active_group,'jog_scratch',value);
        else
            script.HIDDebug("WARNING non jog_scratch scaler, you likely want one");
        engine.scratchTick(deck,value);
    } else {
        if (active_group==undefined)
            return;
        scaler = this.lookupScalingFunction('jog');
        if (scaler!=undefined)
            value = scaler(active_group,'jog',value);
        else
            script.HIDDebug("WARNING non jog scaler, you likely want one");
        engine.setValue(active_group,'jog',value);
    }
}

