
script.HIDDebug = function (message) {
    print("HID " + message);
}


// Standard target groups not resolved for controls
HIDTargetGroups = [
    '[Channel1]','[Channel2]','[Channel3]','[Channel4]',
    '[Sampler1]','[Sampler2]','[Sampler3]','[Sampler4]',
    '[Master]','[Effects]','[Playlist]','Flanger',
    '[Microphone]'
]

// Collection of bits in one parsed packet field
function HIDBitVector (length) {
    this.length = length;
    this.bits = new Object();
}

// Add a bit to the HIDBitVector at given bit index
HIDBitVector.prototype.addBit = function(group,name,index,callback) {
    var bit = new Object();
    bit.type = 'button';
    bit.group = group;
    bit.name = name;
    bit.index = index;
    bit.callback = callback;
    bit.value = undefined;
    this.bits[name] = bit;
}

// One HID input/output packet to register to HIDController
// name     name of packet
// header   list of bytes to match from beginning of packet
// length   packet length
// callback function to call when this packet is input and is received
//          callback is not meaningful for output packets
function HIDPacket (name,header,length,callback) {
    this.name = name;
    this.header = header;
    this.length = length;
    this.callback = callback;
}

HIDPacket.prototype.getControlGroup = function(name,create) {
    if (this.groups==undefined)
        this.groups = new Object();
    if (name in this.groups)
        return this.groups[name];
    if (!create)
        return undefined;

    this.groups[name] = new Object();
    return this.groups[name];
}

// Register a numeric value to parse from input packet
// Parameters:
// name      name of the field
// offset    field offset inside packet (bytes)
// pack      control packing format for parsePackedValue()
// group     mixxx group name to map, or undefined if not mapped
// control   mixxx control name to map, or undefined if not mapped
// callback  callback function to apply to the field value. If value
//           is returned, it is applied, if returns undefined value is
//           discarded expecting callback already did it's job.
// max       maximum value for encoder type fields
HIDPacket.prototype.addControl = function(group,name,offset,pack,callback,max) {
    var control_group = this.getControlGroup(group,true);
    if (control_group==undefined) {
        script.HIDDebug('ERROR creating HID packet group ' + group);
        return;
    }
    control_group[name] = {
        type: 'control',
        group: group,
        name: name,
        offset: offset,
        pack: pack,
        callback: callback,
        ignored: false,
        delta: 0,
        max: max,
        mindelta: 0,
        value: undefined
    };
}

HIDPacket.prototype.addBitmask = function(group,name,offset,index,callback,expect) {
    var control_group = this.getControlGroup(group,true);
    if (control_group==undefined) {
        script.HIDDebug('ERROR creating HID packet group ' + group);
        return;
    }
    control_group[name] = {
        type: 'bitmasked',
        group: group,
        name: name,
        offset: offset,
        index: index,
        pack: pack,
        callback: callback,
        expect: expect,
        ignored: false,
        value: undefined
    };
}

// Register a bit vector input field to HID input packet
// group    group name
// name     name of the bit vector
// offset   field offset inside packet (bytes)
// pack     control packing format for parsePackedValue()
// value    list of bits in vector
HIDPacket.prototype.addBitVector = function(group,name,offset,pack,value) {
    var control_group = this.getControlGroup(group,true);
    if (control_group==undefined) {
        script.HIDDebug('ERROR creating HID packet group ' + group);
        return;
    }
    control_group[name] = {
        type: 'bitvector',
        group: group,
        name: name,
        offset: offset,
        pack: pack,
        value: value
    }
}

// Register a LED control field (not bit) to output packet
HIDPacket.prototype.addLEDControl = function(group,name,offset,pack,callback) {
    var control_group = this.getControlGroup(group,true);
    if (control_group==undefined) {
        script.HIDDebug('ERROR creating HID packet group ' + group);
        return;
    }
    control_group[name] = {
        type: 'led',
        group: group,
        name: name,
        offset: offset,
        pack: pack,
        callback: callback,
        blink: undefined
    };
}

HIDPacket.prototype.getField = function(group,name) {
    var control_group = this.getControlGroup(group);
    var field;
    if (control_group==undefined) {
        script.HIDDebug('Could not find packet group' + group);
        return;
    }
    if (!name in control_group) {
        script.HIDDebug('Invalid packet group field' + name);
        return undefined;
    }
    field = control_group[name];
    return field;
}

// Set 'ignored' flag for field to given value
HIDPacket.prototype.setIgnored = function(group,name,ignored) {
    control = this.getField(group,name);
    if (control==undefined) {
        script.HIDDebug("ERROR setting ignored flag for " + group +' ' + name);
        return;
    }
    control.ignored = ignored;
}

// Adjust field's minimum delta value (changes smaller than this not reported)
HIDPacket.prototype.setMinDelta = function(group,name,mindelta) {
    control = this.getField(group,name);
    if (control==undefined) {
        script.HIDDebug("ERROR adjusting mindelta for " + group +' ' + name);
        return;
    }
    control.mindelta = mindelta;
}


// Parse and return the 'pack' field from field attributes. Valid values are:
//  b       signed byte
//  B       unsigned byte
//  h       short
//  H       unsigned short
//  i       integer
//  I       unsigned integer
HIDPacket.prototype.parsePackedValue = function(data,field) {
    var value = 0;
    var size = undefined;
    var signed = false;

    // Note: the controls are processed here as 32 bit numbers
    // so longs (l or L) can't be supported
    switch (field.pack) {
        case 'b': // signed byte
            size = 1; signed = true; break;
        case 'B': // unsigned byte
            size = 1; signed = false; break;
        case 'h': // short
            size = 2; signed = true; break;
        case 'H': // unsigned short
            size = 2; signed = false; break;
        case 'i': // int
            size = 4; signed = true; break;
        case 'I': // unsigned int
            size = 4; signed = false; break;
    }
    for (field_byte=0;field_byte<size;field_byte++) {
        // Handle value 2^32-1 i.e. 0
        if (data[field.offset+field_byte]==255 && field_byte==4) {
            value += 0;
        } else {
            value += data[field.offset+field_byte] * Math.pow(2,(field_byte*8));
        }
    }
    if (signed) {
        var split = Math.pow(2,size*8)/2-1;
        if (value>split) value = value-Math.pow(2,size*8);
    }
    return value;
}

// Parse bitvector field values, returning object with the named bits set.
// Value must be a valid unsigned byte to parse, with enough bits.
HIDPacket.prototype.parseBitVector = function(field,value) {
    var bits = new Object();
    var bit;
    var new_value;
    for (var name in field.value.bits) {
        new_value = value>>bit.index&1;
        if (new_value!=bit.value) {
            bit.value = new_value;
            bits[name] = bit;
        }
    }
    return bits;
}

// Fetch specified bit out of a bitmasked value
HIDPacket.prototype.parseBitmaskValue = function(field,value) {
    return value>>field.index&1;
}

// Parse input packet fields from data. Data is expected to be a
// Packet() received from HID device.
// Returns list of changed fields with new value. BitVectors are returned as
// objects you can iterate separately.
HIDPacket.prototype.parse = function(data) {
    var delta = new Object();
    var group;
    var group_name;
    var field_name;
    var bit;

    for (group_name in this.groups) {
        group = this.groups[group_name];
        for (field_name in group) {
            var field = group[field_name];
            var value = this.parsePackedValue(data,field);
            if (value == undefined) {
                script.HIDDebug("Error parsing packet field " + name);
                return;
            }
            if (field.type=='bitvector') {
                // bitvector field
                var bits = this.parseBitVector(field,value);
                for (bit in bits) {
                    var bit_value = bits[bit];
                    delta[bit] = bit_value;;
                }
            } else if (field.type=='control' && field.value!=value) {
                if (field.ignored!=true) {
                    // Value is undefined until we receive first packet
                    if (field.value!=undefined) {
                        // Check for encoder wrap around
                        if (field.value==field.max && value==0) {
                            change = 1;
                            field.delta = 1;
                        } else if (value==field.max && field.value==0) {
                            change = 1;
                            field.delta = -1;
                        } else {
                            // Check for minimum delta change
                            var change = Math.abs(value-field.value);
                            field.delta = value-field.value;
                        }
                        if (change>field.mindelta)
                            delta[field.name] = field;
                        else
                            //script.HIDDebug(field.name + " delta smaller than required: " + change);
                            continue;
                    }
                }
                field.value = value;
            } else if (field.type=='bitmasked') {
                value = this.parseBitmaskValue(field,value);
                if (field.expect!=undefined && value!=field.expect)
                    continue;
                if (field.ignored!=true) {
                    // Value is undefined until we receive first packet
                    if (field.value!=undefined)
                        delta[field.name] = field;
                }
                field.value = value;
            }
        }
    }
    return delta;
}

// Send this HID packet to device
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
    offset = this.header.length;
    for (group_name in this.groups) {
        group = this.groups[group_name];
        for (var name in group) {
            var field = group[name];
            if (field.bits!=undefined) {
                // Dump bitvector to packet
                for (bit_index=0;bit_index<field.bits.length;bit_idex++) {
                    if (bit_index>0 && bit_index%8==0)
                        offset++;
                    packet.data[offset] += field.bits[bit_index]<<bit_index%8;
                }
            } else if (field.pack=='B') {
                // Dump one byte value to packet
                packet.data[offset++] = field.value;
            } else {
                // TODO - support writing other values
            }
        }
    }
    // script.HIDDebug("Sending " + this.name + " length " + packet.length + " bytes");
    controller.send(packet.data, packet.length, 0);
}

// HID Controller with packet parser
function HIDController () {
    this.initialized = false;
    this.activeDeck = undefined;
    this.isScratchEnabled = false;

    // Scratch parameter defaults for this.scratchEnable function
    // override for custom control
    this.scratchintervalsPerRev = 128;
    this.scratchRPM = 33+1/3;
    this.scratchAlpha = 1.0/8;
    this.scratchBeta = this.scratchAlpha /32;
    this.scratchRampOnEnable = false;
    this.scratchRampOnDisable = false;

    this.ButtonStates = { released: 0, pressed: 1};
    this.LEDColors = {off: 0x0, on: 0x7f};
    // Set to value in ms to update LEDs periodically
    this.LEDUpdateInterval = undefined;

    this.modifiers = new Object();
    this.scalers = new Object();

    // Toggle buttons
    this.toggleButtons = [ 'play', 'pfl' ]

}

// Return deck number from resolved deck name
HIDController.prototype.resolveDeck = function(group) {
    if (group==undefined)
        return undefined;
    // TODO - write this with string index operations
    if (group=='[Channel1]')
        return 1;
    if (group=='[Channel2]')
        return 2;
    if (group=='[Channel3]')
        return 3;
    if (group=='[Channel4]')
        return 4;
}

// Map virtual deck names to real deck group, or undefined if name
// could not be resolved.
HIDController.prototype.resolveGroup = function(group) {
    var channel_name = /\[Channel[0-9]+\]/;
    if (group!=undefined && group.match(channel_name))
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

// Lookup scaling function for control
HIDController.prototype.lookupScalingFunction = function(name,callback) {
    if (!name in this.scalers)
        return undefined;
    return this.scalers[name];
}

// Register scaling function for a numeric control name
HIDController.prototype.registerScalingFunction = function(name,callback) {
    if (!name in this.scalers)
        return;
    this.scalers[name] = callback;
}

// Register input packet type to controller
HIDController.prototype.registerInputPacket = function(input_packet) {
    var group;
    var name;
    var control;

    if (this.InputPackets==undefined)
        this.InputPackets = new Object();
    // Find modifiers and other special cases from packet fields
    for (group in input_packet.groups) {
        for (name in input_packet.groups[group]) {
            control = input_packet.groups[group][name];
            if (control.type=='bitmap') {
                for (var bit_name in control.bits) {
                    var bit = control.bits[bit_name];
                    if (bit.group=='modifiers') {
                        // Register modifier name
                        this.registerModifier(bit.name);
                    }
                }
            }
            if (control.type=='bitmasked') {
                if (control.group=='modifiers')
                    this.registerModifier(control.name);
            }

        }
    }
    // script.HIDDebug("Registered input packet " + input_packet.name);
    this.InputPackets[input_packet.name] = input_packet;
}

HIDController.prototype.registerModifier = function(name) {
    if (name in this.modifiers) {
        script.HIDDebug("WARNING modifier already registered: " + name);
        return;
    }
    this.modifiers[name] = undefined;
}

// Register output packet type to controller
HIDController.prototype.registerOutputPacket = function(output_packet) {
    var group;
    var name;
    var control;
    // Find LEDs from packet by 'led' type
    for (group in output_packet.groups) {
        for (name in output_packet.groups[group]) {
            control = output_packet.groups[group][name];
            if (control.type!='led')
                continue;
            this.addLED(output_packet,control);
        }
    }
    if (this.OutputPackets==undefined)
        this.OutputPackets = new Object();
    // script.HIDDebug("Registered output packet " + output_packet.name);
    this.OutputPackets[output_packet.name] = output_packet;
}

// Parse a received input packet, call processDelta for results
HIDController.prototype.parsePacket = function(data,length) {
    var packet;
    var delta;

    if (this.InputPackets==undefined) {
        script.HIDDebug("No input packets registered");
        return;
    }

    for (var name in this.InputPackets) {
        packet = this.InputPackets[name];
        if (packet.length!=length) {
            script.HIDDebug("Invalid packet length" + packet.length);
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

        delta = packet.parse(data);
        if (packet.callback!=undefined) {
            packet.callback(packet,delta);
            return;
        }
        // Process named group controls
        if (packet.name=='control')
            this.processIncomingPacket(packet,delta);
        // Process generic delta packet, if callback is defined
        if (this.processDelta!=undefined)
            this.processDelta(packet,delta);
        return;
    }
    script.HIDDebug("Received unknown packet of " + length + " bytes");
}

// STUB for scratch control: you need to
HIDController.prototype.setScratchEnabled = function(group,status) {
    var deck = this.resolveDeck(group);
    if (status==true) {
        // script.HIDDebug("ENABLE scratch in group " + group + " deck " + deck);
        this.isScratchEnabled = true;
        engine.scratchEnable(deck,
            this.scratchintervalsPerRev,
            this.scratchRPM,
            this.scratchAlpha,
            this.scratchBeta,
            this.rampedScratchEnable
        );
    } else {
        // script.HIDDebug("DISABLE scratch in group " + group + " deck " + deck);
        this.isScratchEnabled = false;
        engine.scratchDisable(deck,this.rampedScratchDisable);
    }
}

// Process the Delta (modified fields) group controls from input 
// control packet if packet name is 'control'.
// Override in your class for more complicated functionality.
HIDController.prototype.processIncomingPacket = function(packet,delta) {
    var field;
    var value;
    var group;

    for (var name in delta) {
        if (this.ignoredControlChanges!=undefined) {
            if (this.ignoredControlChanges.indexOf(name)!=-1) {
                // script.HIDDebug('Ignore field ' + name);
                continue;
            }
        }
        field = delta[name];
        // print("FIELD " + field.name + " type " + field.type + " value " + field.value );
        if (field.group==undefined) {
            if (this.activeDeck!=undefined)
                group = '[Channel' + this.activeDeck + ']';
        } else {
            group = field.group;
        }
        if (field.type=='button' || field.type=='bitmasked') {
            if (group=='modifiers') {
                if (!field.name in this.modifiers) {
                    script.HIDDebug("Unknown modifier ID" + field.name);
                    continue;
                }
                if (field.value==this.ButtonStates.pressed)
                    this.modifiers[field.name] = true;
                else
                    this.modifiers[field.name] = false;
                continue;
            }
            if (field.callback) {
                field.callback(field);
                continue;
            }

            // Verify and resolve group for standard buttons
            group = field.group;
            if (HIDTargetGroups.indexOf(group)==-1) {
                if (this.resolveGroup!=undefined)
                    group = this.resolveGroup(field.group);
                if (HIDTargetGroups.indexOf(group)==-1) {
                    continue;
                }
            }

            if (field.name=='jog_touch') {
                if (group!=undefined) {
                    if (field.value==this.ButtonStates.pressed) {
                        this.setScratchEnabled(group,true);
                    } else {
                        this.setScratchEnabled(group,false);
                    }
                }
                var active_group = this.resolveGroup(field.group);

            } else if (this.toggleButtons.indexOf(field.name)!=-1) {
                script.HIDDebug("Toggle button " + field.name);
                if (field.value==this.ButtonStates.released)
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
                // script.HIDDebug("Calling field callback for " + field.name);
                value = field.callback(field);
                continue;
            }

            if (field.name=='jog_wheel') {
                // Handle jog wheel scratching transparently
                this.jog_wheel(field);
                continue;
            }

            value = field.value;
            // Verify and resolve group
            group = field.group

            if (HIDTargetGroups.indexOf(group)==-1) {
                if (this.resolveGroup!=undefined) {
                    group = this.resolveGroup(field.group);
                }
                if (HIDTargetGroups.indexOf(group)==-1) {
                    continue;
                }
            }

            scaler = this.lookupScalingFunction(name);
            if (scaler!=undefined) {
                // script.HIDDebug("Calling value scaler for " + name);
                value = scaler(value);
            }

            if (field.max) {
                // If field has max value, it is encoder

                // script.HIDDebug("CONTROL" + " type " + field.type + " group " + group + " name "  + name + " delta " + field.delta);
                engine.setValue(group,name,field.delta);
            } else {
                // script.HIDDebug("CONTROL" + " type " + field.type + " group " + group + " name "  + name + " value " + value);
                engine.setValue(group,name,value);
            }
        } else {
            script.HIDDebug('Unprocessed field ' + field.name + ' type ' + field.type );
        }
    }
}

HIDController.prototype.getOutputPacket = function(name) {
    if (!name in this.OutputPackets)
        return None;
    return this.OutputPackets[name];
}


// Default jog scratching function, override to change implementation
HIDController.prototype.jog_wheel = function(field) {
    var value = field.value;
    var scaler = undefined;

    if (this.isScratchEnabled==true) {
        var deck = this.resolveDeck(this.resolveGroup(field.group));
        if (deck==undefined)
            return;
        scaler = this.lookupScalingFunction('jog_scratch');
        if (scaler!=undefined)
            value = scaler(field.value);
        else
            script.HIDDebug("WARNING non jog_scratch scaler, you likely want one");
        engine.scratchTick(deck,value);
    } else {
        var active_group = this.resolveGroup(field.group);
        if (active_group==undefined)
            return;
        scaler = this.lookupScalingFunction('jog');
        if (scaler!=undefined)
            value = scaler(field.value);
        else
            script.HIDDebug("WARNING non jog scaler, you likely want one");
        engine.setValue(active_group,'jog',value);
    }
}

HIDController.prototype.getLEDGroup = function(name,create) {
    if (this.LEDs==undefined)
        this.LEDs = new Object();
    if (name in this.LEDs)
        return this.LEDs[name];
    if (!create)
        return undefined;
    this.LEDs[name] = new Object();
    return this.LEDs[name];
}

// Add a LED to controller's list of LEDs.
// Don't call directly, let HIDPacket.addLED call this.
HIDController.prototype.addLED = function(packet,control) {
    var led_group = this.getLEDGroup(control.group,true);
    if (led_group==undefined) {
        script.HIDDebug("ERROR: group was undefined while adding LED");
        return;
    }
    led_group[control.name] = {
        group: control.group,
        name: control.name,
        packet: packet
    };
}


// Update all output packets with LEDs on device to current state.
// If from_timer is true, you can toggle LED color for blinking
HIDController.prototype.updateLEDs = function(from_timer) {
    var led;
    var group;
    var group_name;
    var led_name;
    var led_packets = [];
    var packet;

    for (group_name in this.LEDs) {
        group = this.LEDs[group_name];
        for (led_name in group) {
            led = group[led_name];
            if (from_timer)
                this.toggleLEDBlinkState(led);
            if (led_packets.indexOf(led.packet)==-1) {
                led_packets[led_packets.length] = led.packet;
            }
        }
    }
    for (led_index=0;led_index<led_packets.length;led_index++) {
        packet = led_packets[led_index];
        packet.send();
    }
}

// Toggle color of a blinking led set by setLedBlink. Called from
// updateLEDs timer, if from_timer is true.
HIDController.prototype.toggleLEDBlinkState = function(group,name) {
    led_group = this.getLEDGroup(group);
    if (led_group==undefined) {
        // script.HIDDebug("toggleLEDBlinkState: Unknown group: " + group);
        return;
    }
    var led = led_group[name];
    if (led==undefined) {
        script.HIDDebug("toggleLEDBlinkState: Unknown LED: " + name);
        return;
    }
    var control = led.packet.groups[group][name];
    if (control.blink==undefined)
        return;
    if (control.value == this.LEDColors['off']) {
        control.value = control.blink;
    } else {
        control.value = this.LEDColors['off'];
    }
}

// Set LED state to given LEDColors value, disable blinking
HIDController.prototype.setLED = function(group,name,color) {
    led_group = this.getLEDGroup(group);
    if (led_group==undefined) {
        script.HIDDebug("toggleLEDBlinkState: Unknown group: " + group);
        return;
    }
    if (led_group==undefined) {
        script.HIDDebug("setLED: LED group not found: " + group);
        return;
    }
    var led = led_group[name];
    if (led==undefined) {
        script.HIDDebug("setLED: Unknown LED: " + name);
        return;
    }
    var control = led.packet.groups[group][name];
    // Verify color string
    if ( ! color in this.LEDColors ) {
        script.HIDDebug("Invalid LED color color: " + color);
        return;
    }
    control.value = this.LEDColors[color];
    control.blink = undefined;
    led.packet.send();
}

// Set LED to blink with given color. Reset with setLED(name,'off')
HIDController.prototype.setLEDBlink = function(group,name,blink_color) {
    led_group = this.getLEDGroup(group);
    if (led_group==undefined) {
        script.HIDDebug("setLEDBlink: LED group not found: " + group);
        return;
    }
    var led = led_group[name];
    if (led==undefined) {
        script.HIDDebug("setLEDBlink: Unknown LED: " + name);
        return;
    }
    var control = led.packet.groups[group][name];
    if ( blink_color!=undefined && ! blink_color in this.LEDColors ) {
        script.HIDDebug("Invalid LED blink color: " + blink_color);
        return;
    }
    control.value = this.LEDColors['off'];
    control.blink = this.LEDColors[blink_color];
    led.packet.send();
}

