
// Common HID script debugging function. Just to get logging with 'HID' prefix.
HIDDebug = function (message) { print("HID " + message); }

// HID Bit Vector Class
//
// Collection of bits in one parsed packet field. These objects are
// created by HIDPacket addControl and addOutput and should not be
// created manually.
function HIDBitVector () {
    this.size = 0;
    this.bits = new Object();
}

// Return bit offset based on bitmask
HIDBitVector.prototype.getOffset = function(bitmask) {
    for (var i=0;i<32;i++) 
        if ( (1&bitmask>>i)!=0 )
            return i;
    return 0;
}

// Add a control bitmask to the HIDBitVector 
HIDBitVector.prototype.addBitMask = function(group,name,bitmask) {
    var bit = new Object();
    bit.type = "button";
    bit.packet = undefined;
    bit.id = group+"."+name;
    bit.group = group;
    bit.name = name;
    bit.mapped_group = undefined;
    bit.mapped_name = undefined;
    bit.bitmask = bitmask;
    bit.bitmask = bitmask;
    bit.bit_offset = this.getOffset(bitmask);
    bit.callback = undefined;
    bit.value = undefined;
    bit.auto_repeat = undefined;
    bit.auto_repeat_interval = undefined;
    this.bits[bit.id] = bit;
}

// Add a Output control bitmask to the HIDBitVector 
HIDBitVector.prototype.addOutputMask = function(group,name,bitmask) {
    var bit = new Object();
    bit.type = "output";
    bit.packet = undefined;
    bit.id = group+"."+name;
    bit.group = group;
    bit.name = name;
    bit.mapped_group = undefined;
    bit.mapped_name = undefined;
    bit.bitmask = bitmask;
    bit.bit_offset = this.getOffset(bitmask);
    bit.callback = undefined;
    bit.value = undefined;
    bit.toggle = undefined;
    this.bits[bit.id] = bit;
}

// HID Modifiers object
// 
// Wraps all defined modifiers to one object with uniform API.
// Don't call directly, this is available as HIDController.modifiers
function HIDModifierList() {
    this.modifiers = Object();
    this.callbacks = Object();
}

// Add a new modifier to controller. 
HIDModifierList.prototype.add = function(name) {
    if (name in this.modifiers) {
        HIDDebug("Modifier already defined: " + name);
        return;
    }
    this.modifiers[name] = undefined;
}

// Set modifier value
HIDModifierList.prototype.set = function(name,value) {
    if ((!name in this.modifiers)) {
        HIDDebug("Unknown modifier: " + name);
        return;
    }
    this.modifiers[name] = value;
    if (name in this.callbacks) {
        var callback = this.callbacks[name];
        callback(value);
    }
}

// Get modifier value
HIDModifierList.prototype.get = function(name) {
    if (!(name in this.modifiers)) {
        HIDDebug("Unknown modifier: " + name);
        return false;
    }
    return this.modifiers[name];
}

// Set modifier callback (update function after modifier state changes)
HIDModifierList.prototype.setCallback = function(name,callback) {
    if ((!name in this.modifiers)) {
        HIDDebug("Unknonwn modifier: " + name);
        return;
    }
    this.callbacks[name] = callback;
}

//
// HID Packet object 
//
// One HID input/output packet to register to HIDController
// name     name of packet
// header   list of bytes to match from beginning of packet
// length   packet length
// callback function to call when this packet type is input
//          and is received. If packet callback is set, the
//          packet is not parsed by delta functions.
//          callback is not meaningful for output packets
function HIDPacket(name,header,length,callback) {
    this.name = name;
    this.header = header;
    this.length = length;
    this.callback = callback;

    this.groups = new Object();

    // Size of various 'pack' values in bytes
    this.packSizes = { b: 1, B: 1, h: 2, H: 2, i: 4, I: 4 };
    this.signedPackFormats = [ "b", "h", "i"];
}

// Pack a field value to the packet.
// Can only pack bits and byte values, patches welcome.
HIDPacket.prototype.pack = function(packet,field) {
    var value = 0;
    if (!(field.pack in this.packSizes)) {
        HIDDebug("ERROR parsing packed value: invalid pack format " + field.pack);
        return;
    }
    var bytes = this.packSizes[field.pack];
    var signed = false;
    if (this.signedPackFormats.indexOf(field.pack)!=-1)
        signed = true;

    if (field.type=="bitvector") {
        // TODO - fix multi byte bit vector outputs
        if (bytes>1) {
            HIDDebug("ERROR: packing multibyte bit vectors not yet supported");
            return;
        }
        for (bit_id in field.value.bits) {
            var bit = field.value.bits[bit_id];
            packet.data[field.offset] = packet.data[field.offset] | bit.value;
        }
        return;
    }

    value = (field.value!=undefined) ? field.value : 0;  

    if (value<field.min || value>field.max) {
        HIDDebug("ERROR " + field.id + " packed value out of range: " + value);
        return;
    }

    for (var byte_index=0;byte_index<bytes;byte_index++) {
        var index = field.offset+byte_index;
        if (signed) {
            if (value>=0) {
                packet.data[index] = (value>>(byte_index*8)) & 255;
            } else {
                packet.data[index] = 255 - ((-(value+1)>>(byte_index*8)) & 255);
            }
        } else {
            packet.data[index] = (value>>(byte_index*8)) & 255;
        }
    }

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
        HIDDebug("ERROR parsing packed value: invalid pack format " + field.pack);
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
    if (signed) {
        var max_value = Math.pow(2,bytes*8);
        var split = max_value/2-1;
        if (value>split) 
            value = value-max_value;
    }
    return value;
}

// Find HID packet group matching name.
// Create group if create is true
HIDPacket.prototype.getGroup = function(name,create) {
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
HIDPacket.prototype.getFieldByOffset = function(offset,pack) {
    if (!(pack in this.packSizes)) {
        HIDDebug("Unknown pack string " + pack);
        return undefined;
    }
    var end_offset = offset + this.packSizes[pack];
    if (end_offset>this.length) {
        HIDDebug("Invalid offset+pack range " +
            offset + "-" + end_offset +
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
            // Same field offset
            if (field.offset==offset) 
                return field;
            // 7-8 8-9
            // Offset for smaller packet inside multibyte field
            if (field.offset<offset && field.end_offset>=end_offset)
                return field;
            // Packet offset starts inside field, may overflow
            if (field.offset<offset && field.end_offset>offset)
                return field;
            // Packet start before field, ends or overflows field
            if (field.offset>offset && field.offset<end_offset)
                return field;
        }
    }
    return undefined;
}

// Return a field by group and name from the packet,
// Returns undefined if field could not be found
HIDPacket.prototype.getField = function(group,name) {
    var field_id = group+"."+name;
    if (!(group in this.groups)) {
        HIDDebug("PACKET " + this.name + " group not found " + group);
        return undefined;
    }

    var control_group = this.groups[group];
    if (field_id in control_group)
        return control_group[field_id];

    // Lookup for bit fields in bitvector matching field name
    for (var group_name in this.groups) {
        var control_group = this.groups[group_name];
        for (field_name in control_group) {
            var field = control_group[field_name];
            if (field==undefined || field.type!="bitvector")
                continue
            for (bit_name in field.value.bits) {
                var bit = field.value.bits[bit_name];
                if (bit.id==field_id) {
                    return field;
                }
            }
        }
    }
    // Field not found
    return undefined;
}

// Return reference to a bit in a bitvector field
HIDPacket.prototype.lookupBit = function(group,name) {
    var field = this.getField(group,name);
    if (field==undefined) {
        HIDDebug("Bitvector match not found: "+group+"."+name);
        return undefined;
    }
    var bit_id = group+"."+name;
    for (bit_name in field.value.bits) {
        var bit = field.value.bits[bit_name];
        if (bit.id==bit_id)
            return bit;
    }
    HIDDebug("BUG: bit not found after successful field lookup");
    return undefined;
}

// Remove a control registered. Normally not needed
HIDPacket.prototype.removeControl = function(group,name) {
    var control_group = this.getGroup(group);
    if (!(name in control_group)) {
        HIDDebug("Field not in control group " + group + ": " + name);
        return;
    }
    delete control_group[name];
}

// Register a numeric value to parse from input packet
// Parameters:
// group     control group name
// name      name of the field
// offset    field offset inside packet (bytes)
// pack      control packing format for unpack()
// bitmask   bitmask size, undefined for byte(s) controls 
//           NOTE: Parsing bitmask with multiple bits is not supported yet. 
// isEncoder indicates if this is an encoder which should be wrapped and delta reported
// callback  callback function to apply to the field value, or undefined for no callback
//
HIDPacket.prototype.addControl = function(group,name,offset,pack,bitmask,isEncoder) {
    var control_group = this.getGroup(group,true);
    var bitvector = undefined;
    if (control_group==undefined) {
        HIDDebug("ERROR creating HID packet group " + group);
        return;
    }
    if (!(pack in this.packSizes)) {
        HIDDebug("Unknown pack value " + pack);
        return;
    }

    var field = this.getFieldByOffset(offset,pack);
    if (field!=undefined) {
        if (bitmask==undefined) {
            HIDDebug("ERROR registering offset " +offset+ " pack " + pack);
            HIDDebug(
                "ERROR trying to overwrite non-bitmask control " + group + " " + name
            );
            return;
        }
        bitvector = field.value;
        bitvector.addBitMask(group,name,bitmask);
        return;
    }

    field = new Object();
    field.packet = undefined;
    field.id = group+"."+name;    
    field.group = group;
    field.name = name;
    field.mapped_group = undefined;
    field.mapped_name = undefined;
    field.pack = pack;
    field.offset = offset;
    field.end_offset = offset + this.packSizes[field.pack];
    field.bitmask = bitmask;
    field.isEncoder = isEncoder;
    field.callback = undefined;
    field.soft_takeover = false;
    field.ignored = false;
    field.auto_repeat = undefined;
    field.auto_repeat_interval = undefined;

    var packet_max_value = Math.pow(2,this.packSizes[field.pack]*8);
    if (this.signedPackFormats.indexOf(pack)!=-1) {
        field.min = 0 - (packet_max_value/2)+1;
        field.max = (packet_max_value/2)-1;
    } else {
        field.min = 0;
        field.max = packet_max_value-1;
    }

    if (bitmask==undefined || bitmask==packet_max_value) {
        field.type = "control";
        field.value = undefined;
        field.delta = 0;
        field.mindelta = 0;
    } else {
        if (this.signedPackFormats.indexOf(pack)!=-1) {
            HIDDebug("ERROR registering bitvector: signed fields not supported");
            return;
        }
        // Create a new bitvector field and add the bit to that
        // TODO - accept controls with bitmask < packet_max_value
        field_name = "bitvector_" + offset;
        field.type = "bitvector";
        field.name = field_name;
        field.id = group+"."+field_name;
        bitvector = new HIDBitVector(field.max);
        bitvector.size = field.max;
        bitvector.addBitMask(group,name,bitmask);
        field.value = bitvector;
        field.delta = undefined;
        field.soft_takeover = undefined;
        field.mindelta = undefined;
    }

    // Add the new field to the packet
    control_group[field.id] = field;
}

// Register a Output control field or Output control bit to output packet
// Output control field: 
//    Output field with no bitmask, controls Output with multiple values
// Output control bit:
//    Output with with bitmask, controls Output with a single bit
//
// It is recommended to define callbacks after packet creationg with
// setCallback instead of adding it directly here. But you can do it.
HIDPacket.prototype.addOutput = function(group,name,offset,pack,bitmask,callback) {
    var control_group = this.getGroup(group,true);
    var field = undefined;
    var bitvector = undefined;
    var field_id = group+"."+name;

    if (control_group==undefined) {
        return;
    }
    if (!(pack in this.packSizes)) {
        HIDDebug("ERROR: unknown Output control pack value " + pack);
        return;
    }

    // Check if we are adding a Output bit to existing bitvector
    field = this.getFieldByOffset(offset,pack);
    if (field!=undefined) {
        if (bitmask==undefined) {
            HIDDebug(
                "ERROR: overwrite non-bitmask control " + group+"."+name
            );
            return;
        }
        bitvector = field.value;
        bitvector.addOutputMask(group,name,bitmask);
        return;
    }

    field = new Object();
    field.id = field_id;
    field.group = group;
    field.name = name;
    field.mapped_group = undefined;
    field.mapped_name = undefined;
    field.pack = pack;
    field.offset = offset;
    field.end_offset = offset + this.packSizes[field.pack];
    field.bitmask = bitmask;
    field.callback = callback;
    field.toggle = undefined;

    var packet_max_value = Math.pow(2,this.packSizes[field.pack]*8);
    if (this.signedPackFormats.indexOf(pack)!=-1) {
        field.min = 0 - (packet_max_value/2)+1;
        field.max = (packet_max_value/2)-1;
    } else {
        field.min = 0;
        field.max = packet_max_value-1;
    }
    if (bitmask==undefined || bitmask==packet_max_value) {
        field.type = "output";
        field.value = undefined;
        field.delta = undefined;
        field.mindelta = undefined;
    } else {
        // Create new Output bitvector control field, add bit to it
        // rewrite name to use bitvector instead
        field_name = "bitvector_" + offset;
        field.type = "bitvector";
        field.id = group+"."+field_name;
        field.name = field_name;
        bitvector = new HIDBitVector();
        bitvector.size = field.max;
        bitvector.addOutputMask(group,name,bitmask);
        field.value = bitvector;
        field.delta = undefined;
        field.mindelta = undefined;
    }

    // Add Output to HID packet
    control_group[field.id] = field;
}

// Register a callback to field or a bit vector bit.
// Does not make sense for Output fields but you can do that.
HIDPacket.prototype.setCallback = function(group,name,callback) {
    var field = this.getField(group,name);
    var field_id = group+"."+name;
    if (callback==undefined) {
        HIDDebug("Callback to add was undefined for " + field_id);
        return;
    }
    if (field==undefined) {
        HIDDebug("setCallback: field for " +field_id+ " not found"
        );
        return;
    }
    if (field.type=="bitvector") {
        for (var bit_id in field.value.bits) {
            var bit = field.value.bits[bit_id];
            if (bit_id!=field_id)
                continue;
            bit.callback = callback;
            return;
        }
        HIDDebug("ERROR: BIT NOT FOUND " + field_id);
    } else {
        field.callback = callback;
    }
}

// Set 'ignored' flag for field to given value (true or false)
// If field is ignored, it is not reported in 'delta' objects.
HIDPacket.prototype.setIgnored = function(group,name,ignored) {
    var field = this.getField(group,name);
    if (field==undefined) {
        HIDDebug("ERROR setting ignored flag for " + group +" " + name);
        return;
    }
    field.ignored = ignored;
}

// Adjust field's minimum delta value.
// Input value changes smaller than this are not reported in delta
HIDPacket.prototype.setMinDelta = function(group,name,mindelta) {
    field = this.getField(group,name);
    if (field==undefined) {
        HIDDebug("ERROR adjusting mindelta for " + group +" " + name);
        return;
    }
    if (field.type=="bitvector") {
        HIDDebug("ERROR setting mindelta for bitvector packet does not make sense");
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
    for (var bit_id in field.value.bits) {
        bit = field.value.bits[bit_id];
        new_value = (bit.bitmask&value)>>bit.bit_offset;
        if (bit.value!=undefined && bit.value!=new_value)
            bits[bit_id] = bit;
        bit.value = new_value;
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
            if (field==undefined)
                continue;

            var value = this.unpack(data,field);
            if (value == undefined) {
                HIDDebug("Error parsing packet field value for " + field_id);
                return;
            }

            if (field.type=="bitvector") {
				// Bitvector deltas are checked in parseBitVector
                var changed_bits = this.parseBitVector(field,value);
                for (bit_name in changed_bits) 
                    field_changes[bit_name] = changed_bits[bit_name];

            } else if (field.type=="control") {
                if (field.value==value)
                    continue;
                if (field.ignored || field.value==undefined) {
                    field.value = value;
                    continue;
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
    var packet = new Packet(this.length);

    for (header_byte=0;header_byte<this.header.length;header_byte++) {
        packet.data[header_byte] = this.header[header_byte];
    }

    for (var group_name in this.groups) {
        var group = this.groups[group_name];
        for (var field_name in group) {
            var field = group[field_name];
            if (field.type=="bitvector") {
                for (var bit_id in field.value.bits) {
                    var bit = field.value.bits[bit_id];
                }
            }
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
// LEDColors            possible Output colors named, must contain 'off' value
// deckOutputColors        Which colors to use for each deck. Default 'on' for first
//                      four decks. Values are like {1: 'red', 2: 'green' }
//                      and must reference valid OutputColors fields.
// OutputUpdateInterval    By default undefined. If set, it's a value for timer
//                      executed every n ms to update Outputs with updateOutputs()
// modifiers            Reference to HIDModifierList object
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

    this.InputPackets = new Object();
    this.OutputPackets = new Object();
    // Default input control packet name: can be modified for controllers 
    // which can swap modes (wiimote for example)
    this.defaultPacket = "control";

    // Callback functions called by deck switching. Undefined by default
    this.disconnectDeck = undefined;
    this.connectDeck = undefined;

    // Scratch parameter defaults for this.scratchEnable function
    // override for custom control
    this.isScratchEnabled = false;
    this.scratchintervalsPerRev = 128;
    this.scratchRPM = 33+1/3;
    this.scratchAlpha = 1.0/8;
    this.scratchBeta = this.scratchAlpha /32;
    this.scratchRampOnEnable = false;
    this.scratchRampOnDisable = false;

    // Button states available
    this.buttonStates = { released: 0, pressed: 1};
    // Output color values to send 
    this.LEDColors = {off: 0x0, on: 0x7f};
    // Toggle buttons
    this.toggleButtons = [ "play", "pfl", "keylock", "quantize", "reverse" ];

    // Override to set specific colors for multicolor button Output per deck
    this.deckOutputColors = {1: "on", 2: "on", 3: "on", 4: "on"};
    // Mapping of automatic deck switching with deckSwitch function
    this.virtualDecks = ["deck","deck1","deck2","deck3","deck4"];
    this.deckSwitchMap = { 1: 2, 2: 1, 3: 4, 4: 3, undefined: 1 };

    // Standard target groups available in mixxx. This is used by 
    // HID packet parser to recognize group parameters we should 
    // try sending to mixxx.
    this.valid_groups = [
    "[Channel1]","[Channel2]","[Channel3]","[Channel4]",
    "[Sampler1]","[Sampler2]","[Sampler3]","[Sampler4]",
    "[Master]","[Effects]","[Playlist]","[Flanger]",
    "[Microphone]"
]


    // Set to value in ms to update Outputs periodically
    this.OutputUpdateInterval = undefined;

    this.modifiers = new HIDModifierList();
    this.scalers = new Object();
    this.timers = new Object();

    this.auto_repeat_interval = 100;
}

// Function to close the controller object cleanly
HIDController.prototype.close = function() {
    for (var name in this.timers) {
        var timer = this.timers[name];
        if (timer==undefined)
            continue;
        engine.stopTimer(timer);
        this.timers[name] = undefined;
    }
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
    if (this.valid_groups.indexOf(group)!=-1) {
        return group;
    }
    if (group=="deck" || group==undefined) {
        if (this.activeDeck==undefined)
            return undefined;
        return "[Channel" + this.activeDeck + "]";
    }
    if (this.activeDeck==1 || this.activeDeck==2) {
        if (group=="deck1") return "[Channel1]";
        if (group=="deck2") return "[Channel2]";
    }
    if (this.activeDeck==3 || this.activeDeck==4) {
        if (group=="deck1") return "[Channel3]";
        if (group=="deck2") return "[Channel4]";
    }
    return undefined;
}

// Find Output control matching give group and name
// Returns undefined if output field can't be found.
HIDController.prototype.getOutputField = function(m_group,m_name) {
    for (var packet_name in this.OutputPackets) {
        var packet = this.OutputPackets[packet_name];
        for (var group_name in packet.groups) {
            var group = packet.groups[group_name];
            for (var field_name in group) {
                var field = group[field_name];
                if (field.type=="bitvector") {
                    for (var bit_id in field.value.bits) {
                        var bit = field.value.bits[bit_id];
                        if (bit.mapped_group==m_group && bit.mapped_name==m_name)
                            return bit;
                        if (bit.group==m_group && bit.name==m_name)
                            return bit;
                    }
                    continue;
                }
                if (field.mapped_group==m_group && field.mapped_name==m_name)
                    return field;
                if (field.group==m_group && field.name==m_name)
                    return field;
            }
        }
    }
    return undefined;
}

// Find input packet matching given name.
// Returns undefined if input packet name is not registered.
HIDController.prototype.getInputPacket = function(name) {
    if (!(name in this.InputPackets))
        return undefined;
    return this.InputPackets[name];
}

// Find output packet matching given name
// Returns undefined if output packet name is not registered.
HIDController.prototype.getOutputPacket = function(name) {
    if (!(name in this.OutputPackets))
        return undefined;
    return this.OutputPackets[name];
}

// Set input packet callback afterwards
HIDController.prototype.setPacketCallback = function(packet,callback) {
    var input_packet = this.getInputPacket(packet);
    input_packet.callback = callback;
}

// Register packet field's callback.
// If packet has callback, it is still parsed but no field processing is done,
// callback is called directly after unpacking fields from packet.
HIDController.prototype.setCallback = function(packet,group,name,callback) {
    var input_packet = this.getInputPacket(packet);
    if (input_packet==undefined) {
        HIDDebug("Input packet not found " + packet);
        return;
    }
    input_packet.setCallback(group,name,callback);
}

// Register scaling function for a control name
// This does not check if given control name is valid
HIDController.prototype.setScaler = function(name,callback) {
    if (name in this.scalers)
        return;
    this.scalers[name] = callback;
}

// Lookup scaling function for control
// Returns undefined if function is not registered.
HIDController.prototype.getScaler = function(name,callback) {
    if (!(name in this.scalers))
        return undefined;
    return this.scalers[name];
}

// Change type of a previously defined field to modifier and register it
HIDController.prototype.linkModifier = function(group,name,modifier) {
    var packet = this.getInputPacket(this.defaultPacket);
    if (packet==undefined) {
        HIDDebug(
            "ERROR creating modifier: input packet "+this.defaultPacket+" not found"
        );
        return;
    }
    var bit_id = group+"."+name;
    var field = packet.lookupBit(group,name);
    if (field==undefined) {
        HIDDebug("BIT field not found: " + bit_id);
        return;
    }
    field.group = "modifiers";
    field.name = modifier;
    this.modifiers.set(modifier);
}

// TODO - implement unlinking of modifiers
HIDController.prototype.unlinkModifier = function(group,name,modifier) {
    HIDDebug("Unlinking of modifiers not yet implemented");
}

// Link a previously declared HID control to actual mixxx control
HIDController.prototype.linkControl = function(group,name,m_group,m_name,callback) {
    var field;
    var packet = this.getInputPacket(this.defaultPacket);
    if (packet==undefined) {
        HIDDebug("ERROR creating modifier: input packet "+this.defaultPacket+" not found");
        return;
    }
    field = packet.getField(group,name);
    if (field==undefined) {
        HIDDebug("Field not found: " + group+"."+name);
        return;
    }
    if (field.type=="bitvector") {
        field = packet.lookupBit(group,name);
        if (field==undefined) {
            HIDDebug("bit not found: " + group+"."+name);
            return;
        }
    } 
    field.mapped_group = m_group;
    field.mapped_name = m_name;
    if (callback!=undefined)
        field.callback = callback;
}

// TODO - implement unlinking of controls
HIDController.prototype.unlinkControl = function(group,name) {

}

// Register HID input packet type to controller.
// Input packets can be responses from device to queries, or control 
// data details. The default control data packet must be named in
// variable this.defaultPacket to allow automatic processing.
HIDController.prototype.registerInputPacket = function(packet) {
    var name;
    // Find modifiers and other special cases from packet fields
    for (var group_name in packet.groups) {
        var group = packet.groups[group_name];
        for (var field_name in group) {
            var field = group[field_name];
            field.packet = packet;
            if (field.type=="bitvector") {
                for (var bit_id in field.value.bits) {
                    var bit = field.value.bits[bit_id];
                    bit.packet = packet;
                    if (bit.group=="modifiers") 
                        this.modifiers.add(bit.name);
                }
            } else {
                if (field.group=="modifiers") 
                    this.modifiers.add(field.name);
            }
        }
    }
    this.InputPackets[packet.name] = packet;
}

// Register HID output packet type to controller
// There are no special Output control output packets, just register Outputs to any
// valid packet and we detect them here.
// This module only supports sending bitvector values and byte fields to device.
// If you need other data structures, patches are welcome, or you can just do it
// manually in your script without registering the packet.
HIDController.prototype.registerOutputPacket = function(packet) {
    this.OutputPackets[packet.name] = packet;
    // Link packet to all fields
    for (var group_name in packet.groups) {
        var group = packet.groups[group_name];
        for (var field_name in group) {
            var field = group[field_name];
            field.packet = packet;
            if (field.type=="bitvector") {
                for (var bit_id in field.value.bits) {
                    var bit = field.value.bits[bit_id];
                    bit.packet = packet;
                }
            }
        }
    }
}

// Parse a received input packet fields with "unpack" calls to fields
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
        if (packet.name==this.defaultPacket)
            this.processIncomingPacket(packet,changed_data);
        // Process generic changed_data packet, if callback is defined
        if (this.processDelta!=undefined)
            this.processDelta(packet,changed_data);
        if (this.postProcessDelta!=undefined)
            this.postProcessDelta(packet,changed_data);
        return;
    }
    HIDDebug("Received unknown packet of " + length + " bytes");
    for (var i in data) HIDDebug("BYTE " + data[i]);
}

// Process the modified field values (delta) from input packet fields for
// input control packet, if packet name is in this.defaultPacket.
//
// Button field processing:
// - Sets modifiers from buttons
// - Calls button callbacks, if defined
// - Finally tries to run matching engine.setValue() function for buttons 
//   in default mixxx groups, honoring toggleButtons and other button 
//   details. Not done if a callback was defined for button.
//
// Control field processing
// - Calls scaling functions for control fields, if defined for field. 
//   Scaling function for encoders (isEncoder attribute is true) scales 
//   field delta instead of raw value.
// - Calls callback functions for control fields, if defined for field
// - Finally tries run matching engine.setValue() function for control 
//   fields in default mixxx groups. Not done if a callback was defined.
HIDController.prototype.processIncomingPacket = function(packet,delta) {
    var field;
    for (var name in delta) {
        if (this.ignoredControlChanges!=undefined 
            && this.ignoredControlChanges.indexOf(name) != -1)
                continue;
        field = delta[name];
        if (field.type=="button") 
            this.processButton(field);
        else if (field.type=="control") 
            this.processControl(field);
        else
            HIDDebug("Unknown field " + field.name + " type " + field.type);
    }
}

// Get active group for this field
HIDController.prototype.getActiveFieldGroup = function(field) {
    if (field.mapped_group!=undefined) {
        return this.resolveGroup(field.mapped_group);
    }
    group = field.group;
    if (group==undefined) {
        if (this.activeDeck!=undefined)
            return "[Channel" + this.activeDeck + "]";
    }
    if (this.valid_groups.indexOf(group)!=-1) {
        HIDDebug("Resolving group " + group);
        return this.resolveGroup(group);
    }
    return group; 
}

// Get active control name from field
HIDController.prototype.getActiveFieldControl = function(field) {
    if (field.mapped_name!=undefined)
        return field.mapped_name;
    return field.name;
}

// Process given button field, triggering events
HIDController.prototype.processButton = function(field) {
    var control;
    var value;
    var group = this.getActiveFieldGroup(field);
    var control = this.getActiveFieldControl(field);

    if (group==undefined) {
        HIDDebug("processButton: Could not resolve group from " 
            + field.group + " " + field.mapped_group + " "
            + field.name + " " + field.mapped_name
        );
        return;
    }

    if (group=="modifiers") {
        if (field.value!=0)
            this.modifiers.set(control,true);
        else
            this.modifiers.set(control,false);
        return;
    }
    if (field.auto_repeat) {
        timer_id = "auto_repeat_" + field.id;
        if (field.value) {
            this.startAutoRepeatTimer(timer_id,field.auto_repeat_interval);
        } else {
            this.stopAutoRepeatTimer(timer_id);
        }
    }
    if (field.callback!=undefined) {
        field.callback(field);
        return;        
    }
    if (control=="jog_touch") {
        if (group!=undefined) {
            if (field.value==this.buttonStates.pressed) 
                this.enableScratch(group,true);
            else 
                this.enableScratch(group,false);
        }
        return;
    }
    if (this.toggleButtons.indexOf(control)!=-1) {
        if (field.value==this.buttonStates.released)
            return;
        if (engine.getValue(group,control)) {
            if (control=="play")
                engine.setValue(group,"stop",true);
            else
                engine.setValue(group,control,false);
        } else {
            engine.setValue(group,control,true);
        }
        return;
    }
    if (field.auto_repeat && field.value==this.buttonStates.pressed) {
        HIDDebug("Callback for " + field.group);
        engine.setValue(group,control,field.auto_repeat(field));
    } else if (engine.getValue(group,control)==false) {
        engine.setValue(group,control,true);
    } else {
        engine.setValue(group,control,false);
    }
}

// Process given control field, triggering events
HIDController.prototype.processControl = function(field) {
    var value;
    var group = this.getActiveFieldGroup(field);
    var control = this.getActiveFieldControl(field);

    if (group==undefined) {
        HIDDebug("processControl: Could not resolve group from " 
            + field.group + " " + field.mapped_group + " "
            + field.name + " " + field.mapped_name
        );
        return;
    }

    if (field.callback!=undefined) {
        value = field.callback(field);
        return;
    }
    if (group=="modifiers") {
        this.modifiers.set(control,field.value);
        return;
    }
    if (control=="jog_wheel") {
        // Handle jog wheel scratching transparently
        this.jog_wheel(field);
        return;
    }
    // Call value scaler if defined and send mixxx signal
    value = field.value;
    scaler = this.getScaler(control);
    if (field.isEncoder) {
        var field_delta = field.delta;
        if (scaler!=undefined)
            field_delta = scaler(group,control,field_delta);
        engine.setValue(group,control,field_delta);
    } else {
        if (scaler!=undefined)
            value = scaler(group,control,value);
        engine.setValue(group,control,value);
    }
}

// Toggle control state from toggle button
HIDController.prototype.toggle = function(group,control,value) {
    if (value==this.buttonStates.released)
        return;
    var status = (engine.getValue(group,control)==true) ? false : true;
    engine.setValue(group,control,status);
}

// Toggle play/pause state
HIDController.prototype.togglePlay = function(group,field) {
    if (field.value==this.buttonStates.released)
        return;
    var status = (engine.getValue(group,"play")) ? false : true;
    if (!status)
        engine.setValue(group,"stop",true);
    else
        engine.setValue(group,"play",true);
}

// Processing of the 'jog_touch' special button name, which is used to detect
// when scratching should be enabled.
// Deck is resolved from group with 'resolveDeck'

// Enabling scratching (press 'jog_touch' button)
// Sets the internal 'isScratchEnabled' attribute to true, and calls scratchEnable
// with the scratch attributes (see class defination)
//
// Disabling scratching (release 'jog_touch' button)
// Sets the internal 'isScratchEnabled attribute to false, and calls scratchDisable
// to end scratching mode
HIDController.prototype.enableScratch = function(group,status) {
    var deck = this.resolveDeck(group);
    if (status) {
        this.isScratchEnabled = true;
        engine.scratchEnable(deck,
            this.scratchintervalsPerRev,
            this.scratchRPM,
            this.scratchAlpha,
            this.scratchBeta,
            this.rampedScratchEnable
        );
        if (this.enableScratchCallback!=undefined) this.enableScratchCallback(true);
    } else {
        this.isScratchEnabled = false;
        engine.scratchDisable(deck,this.rampedScratchDisable);
        if (this.enableScratchCallback!=undefined) this.enableScratchCallback(false);
    }
}

// Default jog scratching function. Used to handle jog move events from special
// input control field called 'jog_wheel'. Handles both 'scratch' and 'jog' mixxx
// functions, depending on isScratchEnabled value above (see enableScratch())
//
// Since most controllers require value scaling for jog and scratch functions, 
// you  are warned if following scaling function names are not registered:
//
// jog
//      Scaling function from 'jog_wheel' for rate bend events with mixxx 'jog'
//      function. Should return value range suitable for 'jog', whatever you
//      wish it to do.
// jog_scratch
//      Scaling function from 'jog_wheel' for scratch movements with mixxx
//      'scratchTick' function. Should return -1,0,1 or small ranges of integers
//      both negative and positive values.
//
HIDController.prototype.jog_wheel = function(field) {
    var scaler = undefined;
    var active_group = this.getActiveFieldGroup(field);
    var value = undefined;
    if (field.isEncoder)
        value = field.delta;
    else
        value = field.value;
    if (this.isScratchEnabled) {
        var deck = this.resolveDeck(active_group);
        if (deck==undefined)
            return;
        scaler = this.getScaler("jog_scratch");
        if (scaler!=undefined)
            value = scaler(active_group,"jog_scratch",value);
        else
            HIDDebug("WARNING non jog_scratch scaler, you likely want one");
        engine.scratchTick(deck,value);
    } else {
        if (active_group==undefined)
            return;
        scaler = this.getScaler("jog");
        if (scaler!=undefined)
            value = scaler(active_group,"jog",value);
        else
            HIDDebug("WARNING non jog scaler, you likely want one");
        engine.setValue(active_group,"jog",value);
    }
}

HIDController.prototype.stopAutoRepeatTimer = function(timer_id) {
    if (this.timers[timer_id]) {
        engine.stopTimer(this.timers[timer_id]);
        delete this.timers[timer_id];
    } else {
        //HIDDebug("No such autorepeat timer: " + timer_id);
    }
}

// Toggle field autorepeat on or off
HIDController.prototype.setAutoRepeat = function(group,name,callback,interval) {
    var packet = this.getInputPacket(this.defaultPacket);
    var field = packet.getField(group,name);
    if (field!=undefined && field.type=="bitvector")
        var field = packet.lookupBit(group,name);
    if (field==undefined) {
        HIDDebug("setAutoRepeat: field not found " +group+"."+name);
        return;
    }
    field.auto_repeat = callback;
    if (interval)
        field.auto_repeat_interval = interval;
    else
        field.auto_repeat_interval = controller.auto_repeat_interval;
    if (callback)
        callback(field);
}   

// Callback for auto repeat timer to send again the values for
// buttons and controls marked as 'auto_repeat'
// Timer must be defined from actual controller side, because of
// callback call namespaces and 'this' reference
HIDController.prototype.autorepeatTimer = function() {
    var group_name;
    var group;
    var field;
    var field_name;
    var bit_name;
    var bit;
    var packet = this.InputPackets[this.defaultPacket];
    for (group_name in packet.groups) {
        group = packet.groups[group_name];
        for (field_name in group) {
            field = group[field_name];
            if (field.type!="bitvector") {
                if (field.auto_repeat)
                    this.processControl(field);
                continue
            }
            for (bit_name in field.value.bits) {
                bit = field.value.bits[bit_name];
                if (bit.auto_repeat)
                    this.processButton(bit);
            }
        }
    }
}

// Toggle active deck and update virtual output field control mappings
HIDController.prototype.switchDeck = function(deck) {
    var packet;
    var field;
    var group;
    var controlgroup;
    if (deck==undefined) {
        if (this.activeDeck==undefined) {
            deck = 1;
        } else {
            // This is unusable: num_decks has always minimum 4 decks
            // var totalDecks = engine.getValue("[Master]","num_decks");
            // deck = (this.activeDeck+1) % totalDecks;
            deck = this.deckSwitchMap[this.activeDeck];
            if (deck==undefined)
                deck=1;
        }
    }
    new_group = this.resolveDeckGroup(deck);
    HIDDebug("Switching to deck " + deck + " group " + new_group);
    if (this.disconnectDeck!=undefined)
        this.disconnectDeck();
    for (var packet_name in this.OutputPackets) {
        packet = this.OutputPackets[packet_name];
        var send_packet = false;
        for (var group_name in packet.groups) {
            var group = packet.groups[group_name];
            for (var field_name in group) {
                field = group[field_name];
                if (field.type=="bitvector") {
                    for (var bit_id in field.value.bits) {
                        var bit = field.value.bits[bit_id];
                        if (this.virtualDecks.indexOf(bit.mapped_group)==-1)
                            continue;
                        send_packet = true;
                        controlgroup = this.resolveGroup(bit.mapped_group);
                        engine.connectControl(controlgroup,bit.mapped_name,bit.mapped_callback,true);
                        engine.connectControl(new_group,bit.mapped_name,bit.mapped_callback);
                        var value = engine.getValue(new_group,bit.mapped_name);
                        HIDDebug("BIT "+bit.group+"."+bit.name + " value " + value);
                        if (value)
                            this.setOutput(
                                bit.group,bit.name,
                                this.LEDColors[this.deckOutputColors[deck]]
                            )
                        else 
                            this.setOutput(
                                bit.group,bit.name,
                                this.LEDColors.off
                            )
                    }
                    continue;
                }
                // Only move outputs of virtual decks
                if (this.virtualDecks.indexOf(field.mapped_group)==-1)
                    continue;
                send_packet = true;
                controlgroup = this.resolveGroup(field.mapped_group);
                engine.connectControl(controlgroup,field.mapped_name,field.mapped_callback,true);
                engine.connectControl(new_group,field.mapped_name,field.mapped_callback);
                var value = engine.getValue(new_group,field.mapped_name);
                if (value)
                    this.setOutput(
                        field.group,field.name,
                        this.LEDColors[this.deckOutputColors[deck]]
                    )
                else 
                    this.setOutput(
                        field.group,field.name,
                        this.LEDColors.off
                    )
            }
        }
    }
    this.activeDeck = deck;
    if (this.connectDeck!=undefined)
        this.connectDeck();
}

// Link a virtual HID Output to mixxx control
HIDController.prototype.linkOutput = function(group,name,m_group,m_name,callback) {
    var controlgroup = undefined;
    var field = this.getOutputField(group,name);
    if (field==undefined) {
        HIDDebug("Linked output not found: " + group+"."+name);
        return;
    }
    if (field.mapped_group!=undefined) {
        HIDDebug("Output already linked: " + led.id);
        return;
    }
    controlgroup = this.resolveGroup(m_group);
    field.mapped_group = m_group;
    field.mapped_name = m_name;
    field.mapped_callback = callback;
    engine.connectControl(controlgroup,m_name,callback);
    if (engine.getValue(controlgroup,m_name)) 
        this.setOutput(m_group,m_name,"on");
    else
        this.setOutput(m_group,m_name,"off");
}

// Unlink a virtual HID Output from mixxx control
HIDController.prototype.unlinkOutput = function(group,name,callback) {
    var field = this.getOutputField(group,name);
    var controlgroup;
    if (field==undefined) {
        HIDDebug("Unlinked output not found: " + group+"."+name);
        return;
    }
    if (field.mapped_group==undefined || field.mapped_name==undefined) {
        HIDDebug("Unlinked output not mapped: " + group+"."+name);
        return;
    }
    controlgroup = this.resolveGroup(field.mapped_group);
    engine.connectControl(controlgroup,field.mapped_name,callback,true);
    field.mapped_group = undefined;
    field.mapped_name = undefined;
    field.mapped_callback = undefined;
}

// Set output state to given value
HIDController.prototype.setOutput = function(group,name,value,send_packet) {
    var field = this.getOutputField(group,name);
    if (field==undefined) {
        HIDDebug("setOutput: unknown field: " + group+"."+name);
        return;
    }
    field.value = value<<field.bit_offset;
    field.toggle = value<<field.bit_offset;
    if (send_packet)
        field.packet.send();
}

// Set Output to toggle between two values. Reset with setOutput(name,'off')
HIDController.prototype.setOutputToggle = function(group,name,toggle_value) {
    var field = this.getOutputField(group,name);
    if (field==undefined) {
        HIDDebug("setOutputToggle: unknown field " +group+"."+name);
        return;
    }
    field.value = toggle_value<<field.bit_offset;
    field.toggle = toggle_value<<field.bit_offset;
    field.packet.send();
}

// Manual packing test functions 
//var packet = new HIDPacket("test",[0x1,0x2],6);
//var field;
//packet.addOutput("test","ushort",2,"H");
//packet.addOutput("test","short",4,"h");
//field = packet.getField("test","ushort");
//print("FIELD " + field.id + " MIN " + field.min + " MAX " + field.max);
//field.value = 1024;
//field = packet.getField("test","short");
//field.value = -32767;
//print("FIELD " + field.id + " MIN " + field.min + " MAX " + field.max);
//var out = { "length": packet.length, "data": []};
//for (var i=0;i<packet.header.length;i++) {
//    out.data[i] = i;
//}
//for (var group_name in packet.groups) {
//    var group = packet.groups[group_name];
//    for (var field_name in group) {
//        var field = group[field_name];
//        print("PACKING " + field.id);
//        packet.pack(out,field);
//    }
//}
//for (var i=0;i<out.length;i++) { print("BYTE " +i+ " VALUE " +out.data[i]); }

