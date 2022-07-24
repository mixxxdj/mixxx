/* global controller */

/**
 * Common HID script debugging function. Just to get logging with 'HID' prefix.
 *
 * @deprecated Use console.log instead
 * @param {any} message Message to be printed on controller debug console output
 */
this.HIDDebug = function(message) {
    console.log("HID " + message);
};

/**
 * Callback function to call when, the packet represents an HID InputReport, and new data for this
 * InputReport are received. If a packet callback is defined and the data for the InputReport are
 * received, the complete report data are sent to the callback function after field values are
 * parsed, without calling any packet field parsing functions.
 *
 * @callback packetCallback
 * @param {HIDPacket} packet The packet that represents the InputReport
 * @param {number[]} changed_data The data received from the device
 */
/**
 * Callback function to call when, data for specified filed in the packet is updated.
 *
 * @callback controlCallback
 * @param {packetField} field Object that describes a field inside of a packet, which can often
 *     mapped to a Mixxx control.
 */
/**
 * In almost every case, a HID controller sends data values with input fields which are not directly
 * suitable for Mixxx control values. To solve this issue, HIDController contains function to scale
 * the input value to suitable range automatically before calling any field processing functions.
 * Scalers can be registered with HIDController.registerScalingFunction(group,name,callback) in
 * HIDController.
 *
 * @callback scalingCallback
 * @param {string} group Defines the group name for the field. The group can be any string, but if
 *     it matches a valid Mixxx control group name, it is possible to map a field to a control or
 *     output without any additional code.
 * @param {string} name Is the name of the control for the field. The name can be any string, but if
 *     it matches a valid Mixxx control name in the group defined for field, the system attempts to
 *     attach it directly to the correct field. Together group and name form the ID of the field
 *     (group.name)
 * @param {number} value Value to be scaled
 * @returns {number} Scaled value
 */
/**
 * Callback function to call when, jog wheel scratching got enabled or disabled by
 * the button with the special name 'jog_touch'
 *
 * @callback scratchingCallback
 * @param {boolean} isScratchEnabled True, when button 'jog_touch' is active
 */
/**
 * @typedef packetField
 * @type {object}
 * @property {HIDPacket} packet
 * @property {string} id Group and control name separated by a dot
 * @property {string} group
 * @property {string} name
 * @property {string} mapped_group
 * @property {string} mapped_name
 * @property {controlCallback} mapped_callback
 * @property {object} pack Control packing format for unpack(), one of b/B, h/H, i/I
 * @property {number} offset
 * @property {number} end_offset
 * @property {number} bitmask
 * @property {boolean} isEncoder
 * @property {controlCallback} callback
 * @property {boolean} soft_takeover
 * @property {boolean} ignored
 * @property {controlCallback} auto_repeat
 * @property {number} auto_repeat_interval
 * @property {number} min
 * @property {number} max
 * @property {('bitvector'|'button'|'control'|'output')} type Must be either:
 *              - 'bitvector'       If value is of type HIDBitVector
 *              - 'button'          If value is a boolean
 *              - 'control'         If value is a number
 *              - 'output'
 * @property {HIDBitVector|boolean|number} value
 * @property {number} delta
 * @property {number} mindelta
 * @property {number} toggle
 */

/**
 * @typedef bitObject
 * @type {object}
 * @property {HIDPacket} packet
 * @property {string} id Group and control name separated by a dot
 * @property {string} group
 * @property {string} name
 * @property {string} mapped_group
 * @property {string} mapped_name
 * @property {number} bitmask
 * @property {number} bit_offset
 * @property {controlCallback} callback
 * @property {controlCallback} auto_repeat
 * @property {number} auto_repeat_interval
 * @property {('button'|'output')} type Must be either:
 *              - 'button'
 *              - 'output'
 * @property {boolean} value
 * @property {number} toggle
 */

/**
 * HID Bit Vector Class
 *
 * Collection of bits in one parsed packet field. These objects are
 * created by HIDPacket addControl and addOutput and should not be
 * created manually.
 *
 * @property {number} size
 * @property {bitObject[]} bits
 */
// @ts-ignore Same identifier for class and instance needed for backward compatibility
class HIDBitVector {
    constructor() {
        this.size = 0;
        this.bits = {};
    }
    /**
     * Get the index of the least significant bit that is 1 in `bitmask`
     *
     * @param {number} bitmask A bitwise mask of up to 32 bit. All bits set to'1' in this mask are
     *     considered.
     * @returns {number} Index of the least significant bit that is 1 in `bitmask`
     */
    getOffset(bitmask) {
        bitmask >>>= 0;  // ensures coercion to Uint32
        // The previous implementation should have returned 32 for an empty bitmask, instead it
        // returned 0
        if (bitmask === 0) {
            return 0;
        }                     // skipping this step would make it return -1
        bitmask &= -bitmask;  // equivalent to `bitmask = bitmask & (~bitmask + 1)`
        return 31 - Math.clz32(bitmask);
    }
    /**
     * Add a control bitmask to the HIDBitVector
     *
     * @param {string} group Defines the group name for the field. The group can be any string, but
     *     if it matches a valid Mixxx control group name, it is possible to map a field to a
     *     control without any additional code. Defines the group name for the field. The group can
     *     be any string, but if it matches a valid Mixxx control group name, it is possible to map
     *     a field to a control or output without any additional code
     * @param {string} name Is the name of the control for the field. The name can be any string,
     *     but if it matches a valid Mixxx control name in the group defined for field, the system
     *     attempts to attach it directly to the correct field. Together group and name form the ID
     *     of the field (group.name)
     * @param {number} bitmask A bitwise mask of up to 32 bit. All bits set to'1' in this mask are
     *     considered.
     */
    addBitMask(group, name, bitmask) {
        /** @type {bitObject} */
        const bit = {};
        bit.type = "button";
        bit.packet = undefined;
        bit.id = group + "." + name;
        bit.group = group;
        bit.name = name;
        bit.mapped_group = undefined;
        bit.mapped_name = undefined;
        bit.bitmask = bitmask;
        bit.bit_offset = this.getOffset(bitmask);
        bit.callback = undefined;
        bit.value = undefined;
        bit.auto_repeat = undefined;
        bit.auto_repeat_interval = undefined;
        this.bits[bit.id] = bit;
    }
    /**
     * Add an output control bitmask to the HIDBitVector
     *
     * @param {string} group Defines the group name for the field. The group can be any string, but
     *     if it matches a valid Mixxx control group name, it is possible to map a field to an
     *     output without any additional code.
     * @param {string} name Is the name of the control for the field. The name can be any string,
     *     but if it matches a valid Mixxx control name in the group defined for field, the system
     *     attempts to attach it directly to the correct field. Together group and name form the ID
     *     of the field (group.name)
     * @param {number} bitmask A bitwise mask of up to 32 bit. All bits set to'1' in this mask are
     *     considered.
     */
    addOutputMask(group, name, bitmask) {
        /** @type {bitObject} */
        const bit = {};
        bit.type = "output";
        bit.packet = undefined;
        bit.id = group + "." + name;
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
}
// Add class HIDBitVector to the Global JavaScript object
// @ts-ignore Same identifier for class and instance needed for backward compatibility
this.HIDBitVector = HIDBitVector;


/**
 * HID Modifiers object
 *
 * Wraps all defined modifiers to one object with uniform API.
 * Don't call directly, this is available as HIDController.modifiers
 */
// @ts-ignore Same identifier for class and instance needed for backward compatibility
class HIDModifierList {
    constructor() {
        this.modifiers = Object();
        this.callbacks = Object();
    }
    /**
     * Add a new modifier to controller.
     *
     * @param {string} name Name of modifier
     */
    add(name) {
        if (name in this.modifiers) {
            console.warn("HIDModifierList.add - Modifier already defined: " + name);
            return;
        }
        this.modifiers[name] = undefined;
    }
    /**
     * Set modifier value
     *
     * @param {string} name Name of modifier
     * @param {number|boolean} value Value to be set
     */
    set(name, value) {
        if (!(name in this.modifiers)) {
            console.error("HIDModifierList.set - Unknown modifier: " + name);
            return;
        }
        this.modifiers[name] = value;
        if (name in this.callbacks) {
            const callback = this.callbacks[name];
            callback(value);
        }
    }
    /**
     * Get modifier value
     *
     * @param {string} name Name of modifier
     * @returns {number|boolean} Value of modifier
     */
    get(name) {
        if (!(name in this.modifiers)) {
            console.error("HIDModifierList.get - Unknown modifier: " + name);
            return false;
        }
        return this.modifiers[name];
    }
    /**
     * Set modifier callback function
     *
     * @param {string} name Name of reference in HIDModifierList
     * @param {packetCallback} callback Function to be called after modifier value changes
     */
    setCallback(name, callback) {
        if (!(name in this.modifiers)) {
            console.error("HIDModifierList.setCallback - Unknown modifier: " + name);
            return;
        }
        this.callbacks[name] = callback;
    }
}
// Add class HIDModifierList to the Global JavaScript object
// @ts-ignore Same identifier for class and instance needed for backward compatibility
this.HIDModifierList = HIDModifierList;


/**
 * HID Packet object
 *
 * An HIDPacket represents one HID report of type InputReport or OutputReport (FeatureReports are
 * currently not supported)
 *
 * Each HIDPacket must be registered to HIDController.
 *
 * @param {string} name Name of packet (it makes sense to refer the HID report type and HID
 *     Report-ID here e.g. 'InputReport_0x02' or 'OutputReport_0x81')
 * @param {number} reportId ReportID of the packet. If the device does not use ReportIDs this must
 *     be 0. [default = 0]
 * @param {packetCallback} callback function to call when the packet type represents an InputReport
 *     an a new report is received. If packet callback is set, the
 *          packet is not parsed by delta functions.
 *          callback is not meaningful for output packets
 * @param {number[]} header   (optional) list of bytes to match from beginning
 *          of packet. Do NOT put the report ID in this; use
 *          the reportId parameter instead.
 */
// @ts-ignore Same identifier for class and instance needed for backward compatibility
class HIDPacket {
    constructor(name, reportId = 0, callback, header) {
        this.name = name;
        this.header = header;
        this.callback = callback;
        this.reportId = reportId;
        this.groups = {};

        // Size of various 'pack' values in bytes
        this.packSizes = {b: 1, B: 1, h: 2, H: 2, i: 4, I: 4};
        this.signedPackFormats = ["b", "h", "i"];
    }

    /**
     * Pack a field value to the packet.
     * Can only pack bits and byte values, patches welcome.
     *
     * @todo Implement multi byte bit vector outputs
     * @param {number[]} data Data received as InputReport from the device
     * @param {packetField} field Object that describes a field inside of a packet, which can often
     *     mapped to a Mixxx control.
     */
    pack(data, field) {
        if (!(field.pack in this.packSizes)) {
            console.error("HIDPacket.pack - Parsing packed value: invalid pack format " + field.pack);
            return;
        }
        const bytes = this.packSizes[field.pack];
        const signed = this.signedPackFormats.includes(field.pack);
        if (field.type === "bitvector") {
            const bitVector = /** @type {HIDBitVector} */ (field.value);
            if (bytes > 1) {
                console.error("HIDPacket.pack - Packing multibyte bit vectors not yet supported");
                return;
            }
            for (const bit_id in bitVector.bits) {
                const bit = bitVector.bits[bit_id];
                data[field.offset] = data[field.offset] | bit.value;
            }
            return;
        }

        const value = Number((field.value !== undefined) ? field.value : 0);

        if (value < field.min || value > field.max) {
            console.error("HIDPacket.pack - " + field.id + " packed value out of range: " + value);
            return;
        }

        for (let byte_index = 0; byte_index < bytes; byte_index++) {
            const index = field.offset + byte_index;
            if (signed) {
                if (value >= 0) {
                    data[index] = (value >> (byte_index * 8)) & 255;
                } else {
                    data[index] = 255 - ((-(value + 1) >> (byte_index * 8)) & 255);
                }
            } else {
                data[index] = (value >> (byte_index * 8)) & 255;
            }
        }
    }
    /**
     * Parse and return field value matching the 'pack' field from field attributes.
     * Valid field packing types are:
     *  - b       signed byte
     *  - B       unsigned byte
     *  - h       signed short
     *  - H       unsigned short
     *  - i       signed integer
     *  - I       unsigned integer
     *
     * @param {number[]} data Data received as InputReport from the device
     * @param {packetField} field Object that describes a field inside of a packet, which can often
     *     mapped to a Mixxx control.
     * @returns {number} Value for the field in data, represented according the fields packing type
     */
    unpack(data, field) {
        let value = 0;

        if (!(field.pack in this.packSizes)) {
            console.error("HIDPacket.unpack - Parsing packed value: invalid pack format " + field.pack);
            return;
        }
        const bytes = this.packSizes[field.pack];
        const signed = this.signedPackFormats.includes(field.pack);

        for (let field_byte = 0; field_byte < bytes; field_byte++) {
            if (data[field.offset + field_byte] === 255 && field_byte === 4) {
                value += 0;
            } else {
                value += data[field.offset + field_byte] * Math.pow(2, (field_byte * 8));
            }
        }
        if (signed) {
            const max_value = Math.pow(2, bytes * 8);
            const split = max_value / 2 - 1;
            if (value > split) {
                value = value - max_value;
            }
        }
        return value;
    }
    /**
     * Find HID packet group matching name.
     * Create group if create is true
     *
     * @param {string} name Name of the group
     * @param {boolean} [create=false] If true, group will be created
       @returns {any} Group Returns group or undefined, when group is not existing and create is set
         to false
     */
    getGroup(name, create) {
        if (this.groups === undefined) {
            this.groups = {};
        }
        if (name in this.groups) {
            return this.groups[name];
        }
        if (!create) {
            return undefined;
        }
        this.groups[name] = {};
        return this.groups[name];
    }
    /**
     * Lookup HID packet field matching given offset and pack type
     *
     * @param {number} offset The field's offset from the start of the packet in bytes:
     *                        - For HID devices which don't use ReportIDs, the data bytes starts at
     * position 0
     *                        - For HID devices which use ReportIDs to enumerate the reports, the
     * data bytes starts at position 1
     * @param {object} pack Is one of the field packing types:
     *              - b       signed byte
     *              - B       unsigned byte
     *              - h       signed short
     *              - H       unsigned short
     *              - i       signed integer
     *              - I       unsigned integer
     * @returns {packetField} Returns matching field or undefined if no matching field can be found.
     */
    getFieldByOffset(offset, pack) {
        if (!(pack in this.packSizes)) {
            console.error("HIDPacket.getFieldByOffset - Unknown pack string " + pack);
            return undefined;
        }
        const end_offset = offset + this.packSizes[pack];
        let group;
        let field;
        for (const group_name in this.groups) {
            group = this.groups[group_name];
            for (const field_id in group) {
                field = group[field_id];
                // Same field offset
                if (field.offset === offset) {
                    return field;
                }
                // 7-8 8-9
                // Offset for smaller packet inside multibyte field
                if (field.offset < offset && field.end_offset >= end_offset) {
                    return field;
                }
                // Packet offset starts inside field, may overflow
                if (field.offset < offset && field.end_offset > offset) {
                    return field;
                }
                // Packet start before field, ends or overflows field
                if (field.offset > offset && field.offset < end_offset) {
                    return field;
                }
            }
        }
        return undefined;
    }
    /**
     * Return a field by group and name from the packet,
     * Returns undefined if field could not be found
     *
     * @param {string} group Defines the group name for the field. The group can be any string, but
     *     if it matches a valid Mixxx control group name, it is possible to map a field to a
     *     control or output without any additional code.
     * @param {string} name Is the name of the control for the field. The name can be any string,
     *     but if it matches a valid Mixxx control name in the group defined for field, the system
     *     attempts to attach it directly to the correct field. Together group and name form the ID
     *     of the field (group.name)
     * @returns {packetField} Field
     */
    getField(group, name) {
        const field_id = group + "." + name;
        if (!(group in this.groups)) {
            console.error("HIDPacket.getField - Packet " + this.name + " group not found " + group);
            return undefined;
        }

        let control_group = this.groups[group];
        if (field_id in control_group) {
            return control_group[field_id];
        }

        // Lookup for bit fields in bitvector matching field name
        for (const group_name in this.groups) {
            control_group = this.groups[group_name];
            for (const field_name in control_group) {
                const field = control_group[field_name];
                if (field === undefined || field.type !== "bitvector") {
                    continue;
                }
                for (const bit_name in field.value.bits) {
                    const bit = field.value.bits[bit_name];
                    if (bit.id === field_id) {
                        return field;
                    }
                }
            }
        }
        // Field not found
        return undefined;
    }
    /**
     * Return reference to a bit in a bitvector field
     *
     * @param {string} group Defines the group name for the field. The group can be any string, but
     *     if it matches a valid Mixxx control group name, it is possible to map a field to a
     *     control or output without any additional code.
     * @param {string} name Is the name of the control for the field. The name can be any string,
     *     but if it matches a valid Mixxx control name in the group defined for field, the system
     *     attempts to attach it directly to the correct field. Together group and name form the ID
     *     of the field (group.name)
     * @returns {packetField} Reference to a bit in a bitvector field
     */
    lookupBit(group, name) {
        const field = this.getField(group, name);
        if (field === undefined) {
            console.error("HIDPacket.lookupBit - Bitvector match not found: " + group + "." + name);
            return undefined;
        }
        if (field.type !== "bitvector") {
            console.error("HIDPacket.lookupBit - Control doesn't refer a field of type bitvector: " + group + "." + name);
            return undefined;
        }
        const bitVector = /** @type {HIDBitVector} */ (field.value);
        const bit_id = group + "." + name;
        for (const bit_name in bitVector.bits) {
            const bit = bitVector.bits[bit_name];
            if (bit.id === bit_id) {
                return bit;
            }
        }
        console.error("HIDPacket.lookupBit - BUG: bit not found after successful field lookup");
        return undefined;
    }
    /**
     * Remove a control registered. Normally not needed
     *
     * @param {string} group Defines the group name for the field. The group can be any string, but
     *     if it matches a valid Mixxx control group name, it is possible to map a field to a
     *     control or output without any additional code.
     * @param {string} name Is the name of the control for the field. The name can be any string,
     *     but if it matches a valid Mixxx control name in the group defined for field, the system
     *     attempts to attach it directly to the correct field. Together group and name form the ID
     *     of the field (group.name)
     */
    removeControl(group, name) {
        const control_group = this.getGroup(group);
        if (!(name in control_group)) {
            console.warn("HIDPacket.removeControl - Field not in control group " + group + ": " + name);
            return;
        }
        delete control_group[name];
    }
    /**
     * Register a numeric value to parse from input packet
     *
     * @param {string} group Defines the group name for the field. The group can be any string, but
     *     if it matches a valid Mixxx control group name, it is possible to map a field to a
     *     control or output without any additional code.     control group name
     * @param {string} name Is the name of the control for the field. The name can be any string,
     *     but if it matches a valid Mixxx control name in the group defined for field, the system
     *     attempts to attach it directly to the correct field. Together group and name form the ID
     *     of the field (group.name)
     * @param {number} offset The field's offset from the start of the packet in bytes:
     *                        - For HID devices which don't use ReportIDs, the data bytes starts at
     * position 0
     *                        - For HID devices which use ReportIDs to enumerate the reports, the
     * data bytes starts at position 1
     * @param {object} pack Is one of the field packing types:
     *              - b       signed byte
     *              - B       unsigned byte
     *              - h       signed short
     *              - H       unsigned short
     *              - i       signed integer
     *              - I       unsigned integer
     * @param {number} bitmask  A bitwise mask of up to 32 bit. All bits set to'1' in this mask are
     *     considered.
     *           Note: For controls that use full bytes (8bit, 16bit, ...), you can set this to
     * undefined NOTE: Parsing bitmask with multiple bits is not supported yet.
     * @param {boolean} isEncoder indicates if this is an encoder which should be wrapped and delta
     *     reported
     * @param {controlCallback} callback Callback function for the control
     */
    addControl(group, name, offset, pack, bitmask, isEncoder, callback) {
        const control_group = this.getGroup(group, true);
        let bitvector = undefined;
        if (control_group === undefined) {
            console.error("HIDPacket.addControl - Creating HID packet group " + group);
            return;
        }
        if (!(pack in this.packSizes)) {
            console.error("HIDPacket.addControl - Unknown pack value " + pack);
            return;
        }

        const fieldByOffset = this.getFieldByOffset(offset, pack);
        if (fieldByOffset !== undefined) {
            if (bitmask === undefined) {
                console.error("HIDPacket.addControl - Registering offset " + offset + " pack " + pack);
                console.error("HIDPacket.addControl - Trying to overwrite non-bitmask control " + group + " " + name);
                return;
            }
            if (fieldByOffset.type !== "bitvector") {
                console.error("HIDPacket.addControl - Field is not of type bitvector: " + group + "." + name);
                return;
            } else {
                const bitVector = /** @type {HIDBitVector} */ (fieldByOffset.value);
                bitVector.addBitMask(group, name, bitmask);
                if (callback !== undefined) {
                    if (typeof callback !== "function") {
                        console.error(
                            "HIDPacket.addControl - Callback provided for " + group + "." + name +
                            " is not a function.");
                        return;
                    }
                    this.setCallback(group, name, callback);
                }
                return;
            }
        }

        /** @type {packetField} */
        const field = {};
        field.packet = undefined;
        field.id = group + "." + name;
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

        const packet_max_value = Math.pow(2, this.packSizes[field.pack] * 8);
        const signed = this.signedPackFormats.includes(field.pack);
        if (signed) {
            field.min = 0 - (packet_max_value / 2) + 1;
            field.max = (packet_max_value / 2) - 1;
        } else {
            field.min = 0;
            field.max = packet_max_value - 1;
        }

        if (bitmask === undefined || bitmask === packet_max_value) {
            field.type = "control";
            field.value = undefined;
            field.delta = 0;
            field.mindelta = 0;
        } else {
            // bitmask is only defined for fields which are not expected to handle all bits in the
            // control field. For fields with bitmasks, you can define same offset and pack multiple
            // times with different bitmask values to get for example all 8 bits of a buttons state
            // byte to different control fields in addControl input packet command. Masking multiple
            // bits should work but has not been as widely tested.
            const signed = this.signedPackFormats.includes(field.pack);
            if (signed) {
                console.error("HIDPacket.addControl - Registering bitvector: signed fields not supported");
                return;
            }
            // Create a new bitvector field and add the bit to that
            // TODO - accept controls with bitmask < packet_max_value
            const field_name = "bitvector_" + offset;
            field.type = "bitvector";
            field.name = field_name;
            field.id = group + "." + field_name;
            bitvector = new HIDBitVector();
            bitvector.size = field.max;
            bitvector.addBitMask(group, name, bitmask);
            field.value = bitvector;
            field.delta = undefined;
            field.soft_takeover = undefined;
            field.mindelta = undefined;
        }

        // Add the new field to the packet
        control_group[field.id] = field;

        if (callback !== undefined) {
            if (typeof callback !== "function") {
                console.error(
                    "HIDPacket.addControl - Callback provided for " + group + "." + name +
                    " is not a function.");
                return;
            }
            this.setCallback(group, name, callback);
        }
    }
    /**
     * Register a Output control field or Output control bit to output packet
     * Output control field:
     *    Output field with no bitmask, controls Output with multiple values
     * Output control bit:
     *    Output with with bitmask, controls Output with a single bit
     *
     * It is recommended to define callbacks after packet creation with
     * setCallback instead of adding it directly here. But you can do it.
     *
     * @param {string} group Defines the group name for the field. The group can be any string, but
     *     if it matches a valid Mixxx control group name, it is possible to map a field to a
     *     control or output without any additional code.
     * @param {string} name Is the name of the control for the field. The name can be any string,
     *     but if it matches a valid Mixxx control name in the group defined for field, the system
     *     attempts to attach it directly to the correct field. Together group and name form the ID
     *     of the field (group.name)
     * @param {number} offset The field's offset from the start of the packet in bytes:
     *                        - For HID devices which don't use ReportIDs, the data bytes starts at
     * position 0
     *                        - For HID devices which use ReportIDs to enumerate the reports, the
     * data bytes starts at position 1
     * @param {object} pack Is one of the field packing types:
     *              - b       signed byte
     *              - B       unsigned byte
     *              - h       signed short
     *              - H       unsigned short
     *              - i       signed integer
     *              - I       unsigned integer
     * @param {number} bitmask A bitwise mask of up to 32 bit. All bits set to'1' in this mask are
     *     considered.
     * @param {controlCallback} [callback=undefined] Callback function for the control
     */
    addOutput(group, name, offset, pack, bitmask, callback) {
        const control_group = this.getGroup(group, true);
        const field_id = group + "." + name;

        if (control_group === undefined) {
            return;
        }
        if (!(pack in this.packSizes)) {
            console.error("HIDPacket.addOutput - Unknown Output control pack value " + pack);
            return;
        }

        // Adjust offset by 1 because the reportId was previously considered part of the payload
        // but isn't anymore and we can't be bothered to adjust every single script manually
        offset -= 1;

        // Check if we are adding a Output bit to existing bitvector
        const fieldByOffset = this.getFieldByOffset(offset, pack);
        if (fieldByOffset !== undefined) {
            if (bitmask === undefined) {
                console.error("HIDPacket.addOutput - Overwrite non-bitmask control " + group + "." + name);
                return;
            }
            if (fieldByOffset.type !== "bitvector") {
                console.error("HIDPacket.addOutput - Field is not of type bitvector: " + group + "." + name);
                return;
            }
            const bitVector = /** @type {HIDBitVector} */ (fieldByOffset.value);
            bitVector.addOutputMask(group, name, bitmask);
            return;
        }

        /** @type {packetField} */
        const field = {};
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

        const packet_max_value = Math.pow(2, this.packSizes[field.pack] * 8);
        const signed = this.signedPackFormats.includes(field.pack);
        if (signed) {
            field.min = 0 - (packet_max_value / 2) + 1;
            field.max = (packet_max_value / 2) - 1;
        } else {
            field.min = 0;
            field.max = packet_max_value - 1;
        }
        if (bitmask === undefined || bitmask === packet_max_value) {
            field.type = "output";
            field.value = undefined;
            field.delta = undefined;
            field.mindelta = undefined;
        } else {
            // Create new Output bitvector control field, add bit to it
            // rewrite name to use bitvector instead
            const field_name = "bitvector_" + offset;
            field.type = "bitvector";
            field.id = group + "." + field_name;
            field.name = field_name;
            const bitvector = new HIDBitVector();
            bitvector.size = field.max;
            bitvector.addOutputMask(group, name, bitmask);
            field.value = bitvector;
            field.delta = undefined;
            field.mindelta = undefined;
        }

        // Add Output to HID packet
        control_group[field.id] = field;
    }
    /**
     * Register a callback to field or a bit vector bit.
     * Does not make sense for Output fields but you can do that.
     *
     * @param {string} group Defines the group name for the field. The group can be any string, but
     *     if it matches a valid Mixxx control group name, it is possible to map a field to a
     *     control or output without any additional code.
     * @param {string} name Is the name of the control for the field. The name can be any string,
     *     but if it matches a valid Mixxx control name in the group defined for field, the system
     *     attempts to attach it directly to the correct field. Together group and name form the ID
     *     of the field (group.name)
     * @param {controlCallback} callback Callback function for the control
     */
    setCallback(group, name, callback) {
        const field = this.getField(group, name);
        const field_id = group + "." + name;
        if (callback === undefined) {
            console.error("HIDPacket.setCallback - Callback to add was undefined for " + field_id);
            return;
        }
        if (field === undefined) {
            console.error("HIDPacket.setCallback - Field for " + field_id + " not found");
            return;
        }
        if (field.type === "bitvector") {
            const bitVector = /** @type {HIDBitVector} */ (field.value);
            for (const bit_id in bitVector.bits) {
                const bit = bitVector.bits[bit_id];
                if (bit_id !== field_id) {
                    continue;
                }
                bit.callback = callback;
                return;
            }
            console.error("HIDPacket.setCallback - Bit not found " + field_id);
        } else {
            field.callback = callback;
        }
    }
    /**
     * This function can be set in script code to ignore a field you don't want to be processed but
     * still wanted to define, to make packet format complete from specifications. If field is
     * ignored, it is not reported in 'delta' objects.
     *
     * @param {string} group Defines the group name for the field. The group can be any string, but
     *     if it matches a valid Mixxx control group name, it is possible to map a field to a
     *     control or output without any additional code.
     * @param {string} name Is the name of the control for the field. The name can be any string,
     *     but if it matches a valid Mixxx control name in the group defined for field, the system
     *     attempts to attach it directly to the correct field. Together group and name form the ID
     *     of the field (group.name)
     * @param {boolean} ignored 'ignored' flag for field to given value (true or false)
     */
    setIgnored(group, name, ignored) {
        const field = this.getField(group, name);
        if (field === undefined) {
            console.error("HIDPacket.setIgnored - Setting ignored flag for " + group + " " + name);
            return;
        }
        field.ignored = ignored;
    }
    /**
     * Adjust field's minimum delta value.
     * Input value changes smaller than this are not reported in delta
     *
     * @param {string} group Defines the group name for the field. The group can be any string, but
     *     if it matches a valid Mixxx control group name, it is possible to map a field to a
     *     control or output without any additional code.
     * @param {string} name Is the name of the control for the field. The name can be any string,
     *     but if it matches a valid Mixxx control name in the group defined for field, the system
     *     attempts to attach it directly to the correct field. Together group and name form the ID
     *     of the field (group.name)
     * @param {number} mindelta Minimum delta value.
     */
    setMinDelta(group, name, mindelta) {
        const field = this.getField(group, name);
        if (field === undefined) {
            console.error("HIDPacket.setMinDelta - Adjusting mindelta for " + group + " " + name);
            return;
        }
        if (field.type === "bitvector") {
            console.error("HIDPacket.setMinDelta - Setting mindelta for bitvector packet does not make sense");
            return;
        }
        field.mindelta = mindelta;
    }
    /**
     * Parse bitvector field values, returning object with the named bits set.
     *
     * @param {packetField} field Object that describes a field inside of a packet, which can often
     *     mapped to a Mixxx control.
     * @param {number} value Value must be a valid unsigned byte to parse, with enough bits.
     * @returns {bitObject[]} List of modified bits (delta)
     */
    parseBitVector(field, value) {
        /** @type bitObject[]*/
        const bits = new Array();
        let bit;
        let new_value;
        if (field.type !== "bitvector") {
            console.error("HIDPacket.parseBitVector - Field isn't of type bitvector");
            return undefined;
        }
        const bitVector = /** @type {HIDBitVector} */ (field.value);
        for (const bit_id in bitVector.bits) {
            bit = bitVector.bits[bit_id];
            new_value = (bit.bitmask & value) >> bit.bit_offset;
            if (bit.value !== undefined && bit.value !== new_value) {
                bits[bit_id] = bit;
            }
            bit.value = new_value;
        }
        return bits;
    }
    /**
     * Parse input packet fields from data.
     * Data is expected to be a Packet() received from HID device.
     * BitVectors are returned as bits you can iterate separately.
     *
     * @param {number[]} data Data received as InputReport from the device
     * @returns List of changed fields with new value.
     */
    parse(data) {
        const field_changes = {};
        let group;
        let group_name;
        let field;
        let field_id;

        for (group_name in this.groups) {
            group = this.groups[group_name];
            for (field_id in group) {
                field = group[field_id];
                if (field === undefined) {
                    continue;
                }

                const value = this.unpack(data, field);
                if (value === undefined) {
                    console.error("HIDPacket.parse - Parsing packet field value for " + field_id);
                    return;
                }

                if (field.type === "bitvector") {
                    // Bitvector deltas are checked in parseBitVector
                    const changed_bits = this.parseBitVector(field, value);
                    for (const bit_name in changed_bits) {
                        field_changes[bit_name] = changed_bits[bit_name];
                    }

                } else if (field.type === "control") {
                    if (field.value === value && field.mindelta !== undefined) {
                        continue;
                    }
                    if (field.ignored || field.value === undefined) {
                        field.value = value;
                        continue;
                    }
                    let change = 0;
                    if (field.isEncoder) {
                        if (field.value === field.max && value === field.min) {
                            change = 1;
                            field.delta = 1;
                        } else if (value === field.max && field.value === field.min) {
                            change = 1;
                            field.delta = -1;
                        } else {
                            change = 1;
                            field.delta = value - field.value;
                        }
                        field.value = value;
                    } else {
                        change = Math.abs(value - field.value);
                        field.delta = value - field.value;
                    }
                    if (field.mindelta === undefined || change > field.mindelta) {
                        field_changes[field.id] = field;
                        field.value = value;
                    }
                }
            }
        }
        return field_changes;
    }
    /**
     * Send this HID packet to device.
     * First the header bytes are copied to beginning of packet, then
     * field object values are packed to the HID packet according to the
     * field type.
     *
     * @param {boolean} [debug=false] Enables debug output to console
     */
    send(debug) {
        const data = [];

        if (this.header !== undefined) {
            for (let header_byte = 0; header_byte < this.header.length; header_byte++) {
                data[header_byte] = this.header[header_byte];
            }
        }

        for (const group_name in this.groups) {
            const group = this.groups[group_name];
            for (const field_name in group) {
                this.pack(data, group[field_name]);
            }
        }

        if (debug) {
            let packet_string = "";
            for (const d in data) {
                if (data[d] < 0x10) {
                    // Add padding for bytes smaller than 10
                    packet_string += "0";
                }
                packet_string += data[d].toString(16) + " ";
            }
            console.log("Sending packet with Report ID " + this.reportId + ": " + packet_string);
        }
        controller.send(data, data.length, this.reportId);
    }
}
// Add class HIDPacket to the Global JavaScript object
// @ts-ignore Same identifier for class and instance needed for backward compatibility
this.HIDPacket = HIDPacket;


/**
 * HID Controller Class
 *
 * HID Controller with packet parser
 * Global attributes include:
 *
 * @property {boolean} initialized          by default false, you should set this to true when
 *                      controller is found and everything is OK
 * @property {string} activeDeck           by default undefined, used to map the virtual deck
 *                      names 'deck','deck1' and 'deck2' to actual [ChannelX]
 * @property {boolean} isScratchEnabled     set to true, when button 'jog_touch' is active
 * @property buttonStates         valid state values for buttons, should contain fields
 *                      released (default 0) and pressed (default 1)
 * @property LEDColors            possible Output colors named, must contain 'off' value
 * @property deckOutputColors        Which colors to use for each deck. Default 'on' for first
 *                      four decks. Values are like {1: 'red', 2: 'green' }
 *                      and must reference valid OutputColors fields.
 * @property {number} OutputUpdateInterval    By default undefined. If set, it's a value for timer
 *                      executed every n ms to update Outputs with updateOutputs()
 * @property {HIDModifierList} modifiers            Reference to HIDModifierList object
 * @property toggleButtons        List of button names you wish to act as 'toggle', i.e.
 *                      pressing the button and releasing toggles state of the
 *                      control and does not set it off again when released.
 *
 * Scratch variables (initialized with 'common' defaults, you can override):
 * @property {number} scratchintervalsPerRev   Intervals value for scratch_enable
 * @property {number} scratchRPM               RPM value for scratch_enable
 * @property {number} scratchAlpha             Alpha value for scratch_enable
 * @property {number} scratchBeta              Beta value for scratch_enable
 * @property {boolean} scratchRampOnEnable     Set true to ramp the deck speed down. Set false to stop instantly [default = false]
 * @property {boolean} scratchRampOnDisable    Set true to ramp the deck speed up. Set false to jump to normal play speed instantly [default = false]
 * @property {scratchingCallback} enableScratchCallback Callback function to call when, jog wheel scratching got enabled or disabled
 * @property {number} auto_repeat_interval     Auto repeat interval default for fields, where not
 * specified individual
 */
// @ts-ignore Same identifier for class and instance needed for backward compatibility
class HIDController {
    constructor() {
        this.initialized = false;
        this.activeDeck = undefined;

        this.InputPackets = {};
        this.OutputPackets = {};
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
        this.scratchRPM = 33 + 1 / 3;
        this.scratchAlpha = 1.0 / 8;
        this.scratchBeta = this.scratchAlpha / 32;
        this.scratchRampOnEnable = false;
        this.scratchRampOnDisable = false;

        this.enableScratchCallback = undefined;

        // Button states available
        this.buttonStates = {released: 0, pressed: 1};
        // Output color values to send
        this.LEDColors = {off: 0x0, on: 0x7f};
        // Toggle buttons
        this.toggleButtons = [
            "play", "pfl", "keylock", "quantize", "reverse", "slip_enabled",
            "group_[Channel1]_enable", "group_[Channel2]_enable",
            "group_[Channel3]_enable", "group_[Channel4]_enable"
        ];

        // Override to set specific colors for multicolor button Output per deck
        this.deckOutputColors = {1: "on", 2: "on", 3: "on", 4: "on"};
        // Mapping of automatic deck switching with deckSwitch function
        this.virtualDecks = ["deck", "deck1", "deck2", "deck3", "deck4"];
        this.deckSwitchMap = {1: 2, 2: 1, 3: 4, 4: 3, undefined: 1};

        // Standard target groups available in mixxx. This is used by
        // HID packet parser to recognize group parameters we should
        // try sending to mixxx.
        this.valid_groups = [
            "[Channel1]",
            "[Channel2]",
            "[Channel3]",
            "[Channel4]",
            "[Sampler1]",
            "[Sampler2]",
            "[Sampler3]",
            "[Sampler4]",
            "[Sampler5]",
            "[Sampler6]",
            "[Sampler7]",
            "[Sampler8]",
            "[Master]",
            "[PreviewDeck1]",
            "[Effects]",
            "[Playlist]",
            "[Flanger]",
            "[Microphone]",
            "[EffectRack1_EffectUnit1]",
            "[EffectRack1_EffectUnit2]",
            "[EffectRack1_EffectUnit3]",
            "[EffectRack1_EffectUnit4]",
            "[InternalClock]"
        ];

        // Set to value in ms to update Outputs periodically
        this.OutputUpdateInterval = undefined;

        this.modifiers = new HIDModifierList();
        this.scalers = {};
        this.timers = {};

        this.auto_repeat_interval = 100;
    }
    /** Function to close the controller object cleanly */
    close() {
        for (const name in this.timers) {
            const timer = this.timers[name];
            if (timer === undefined) {
                continue;
            }
            engine.stopTimer(timer);
            this.timers[name] = undefined;
        }
    }
    /**
     * Initialize our packet data and callbacks. This does not seem to
     * work when executed from here, but we keep a stub just in case.
     */
    initializePacketData() {}
    /**
     * Return deck number from deck name. Deck name can't be virtual deck name
     * in this function call.
     *
     * @param {string} group Defines the group name for the field. The group can be any string, but
     *     if it matches a valid Mixxx control group name, it is possible to map a field to a
     *     control or output without any additional code.
     * @returns {number} Number of deck
     */
    resolveDeck(group) {
        if (group === undefined) {
            return undefined;
        }
        const result = group.match(/\[Channel[0-9]+\]/);
        if (!result) {
            return undefined;
        }
        const str = group.replace(/\[Channel/, "");
        return Number(str.substring(0, str.length - 1));
    }
    /**
     * Return the group name from given deck number.
     *
     * @param {number} deck Number of deck
     * @returns {string} Group name of the deck (e.g. Channel2 for deck number 2)
     */
    resolveDeckGroup(deck) {
        if (deck === undefined) {
            return undefined;
        }
        return "[Channel" + deck + "]";
    }
    /**
     * Map virtual deck names to real deck group. If group is already
     * a real mixxx group value, just return it as it without mapping.
     *
     * @param {string} group Defines the group name for the field. The group can be any string, but
     *     if it matches a valid Mixxx control group name, it is possible to map a field to a
     *     control or output without any additional code.
     * @returns {string} Channel
     */
    resolveGroup(group) {
        const channel_name = /\[Channel[0-9]+\]/;
        if (group !== undefined && group.match(channel_name)) {
            return group;
        }
        if (this.valid_groups.indexOf(group) !== -1) {
            return group;
        }
        if (group === "deck" || group === undefined) {
            if (this.activeDeck === undefined) {
                return undefined;
            }
            return "[Channel" + this.activeDeck + "]";
        }
        if (this.activeDeck === 1 || this.activeDeck === 2) {
            if (group === "deck1") { return "[Channel1]"; }
            if (group === "deck2") { return "[Channel2]"; }
        }
        if (this.activeDeck === 3 || this.activeDeck === 4) {
            if (group === "deck1") { return "[Channel3]"; }
            if (group === "deck2") { return "[Channel4]"; }
        }
        return undefined;
    }
    /**
     * Find Output control matching give group and name
     *
     * @todo The current implementation of this often called function is very slow and does not
     * scale, due to several nested loops.
     * @param {string} m_group Defines the group name for the field. The group can be any string,
     *     but if it matches a valid Mixxx control group name, it is possible to map a field to a
     *     control or output without any additional code.
     * @param {string} m_name Is the name of the control for the field. The name can be any string,
     *     but if it matches a valid Mixxx control name in the group defined for field, the system
     *     attempts to attach it directly to the correct field. Together group and name form the ID
     *     of the field (group.name)
     * @returns {bitObject|packetField} Bit or bytewise field - Returns undefined if output field
     *     can't be found.
     */
    getOutputField(m_group, m_name) {
        for (const packet_name in this.OutputPackets) {
            const packet = this.OutputPackets[packet_name];
            for (const group_name in packet.groups) {
                const group = packet.groups[group_name];
                for (const field_name in group) {
                    const field = group[field_name];
                    if (field.type === "bitvector") {
                        for (const bit_id in field.value.bits) {
                            const bit = field.value.bits[bit_id];
                            if (bit.mapped_group === m_group && bit.mapped_name === m_name) {
                                return bit;
                            }
                            if (bit.group === m_group && bit.name === m_name) {
                                return bit;
                            }
                        }
                        continue;
                    }
                    if (field.mapped_group === m_group && field.mapped_name === m_name) {
                        return field;
                    }
                    if (field.group === m_group && field.name === m_name) {
                        return field;
                    }
                }
            }
        }
        return undefined;
    }
    /**
     * Find input packet matching given name.
     * Returns undefined if input packet name is not registered.
     *
     * @param {string} name Name of packet (it makes sense to refer the HID report type and HID
     *     Report-ID here e.g. 'InputReport_0x02')
     * @returns {HIDPacket} The input packet
     */
    getInputPacket(name) {
        if (!(name in this.InputPackets)) {
            return undefined;
        }
        return this.InputPackets[name];
    }
    /**
     * Find output packet matching given name
     * Returns undefined if output packet name is not registered.
     *
     * @param {string} name Name of packet (it makes sense to refer the HID report type and HID
     *     Report-ID here e.g. 'OutputReport_0x81')
     * @returns {HIDPacket} The output packet
     */
    getOutputPacket(name) {
        if (!(name in this.OutputPackets)) {
            return undefined;
        }
        return this.OutputPackets[name];
    }
    /**
     * Set input packet callback afterwards
     *
     * @param {string} packet The name of the input packet e.g. 'InputReport_0x02'
     * @param {packetCallback} callback Callback function for the control
     */
    setPacketCallback(packet, callback) {
        const input_packet = this.getInputPacket(packet);
        input_packet.callback = callback;
    }
    /**
     * Register packet field's callback.
     * If packet has callback, it is still parsed but no field processing is done,
     * callback is called directly after unpacking fields from packet.
     *
     * @param {string} packet The name of the input packet e.g. 'InputReport_0x02'
     * @param {string} group Defines the group name for the field. The group can be any string, but
     *     if it matches a valid Mixxx control group name, it is possible to map a field to a
     *     control or output without any additional code.
     * @param {string} name Is the name of the control for the field. The name can be any string,
     *     but if it matches a valid Mixxx control name in the group defined for field, the system
     *     attempts to attach it directly to the correct field. Together group and name form the ID
     *     of the field (group.name)
     * @param {controlCallback} callback Callback function for the control
     */
    setCallback(packet, group, name, callback) {
        const input_packet = this.getInputPacket(packet);
        if (input_packet === undefined) {
            console.error("HIDController.setCallback - Input packet not found " + packet);
            return;
        }
        input_packet.setCallback(group, name, callback);
    }
    /**
     * Register scaling function for a control name
     * This does not check if given control name is valid
     *
     * @param {string} name Reference of the scaling function in scalers list of HIDController
     * @param {scalingCallback} callback Scaling function
     */
    setScaler(name, callback) {
        if (name in this.scalers) {
            return;
        }
        this.scalers[name] = callback;
    }
    /**
     * Lookup scaling function for control
     *
     * @param {string} name Reference of the scaling function in scalers list of HIDController
     * @param _callback Unused
     * @returns  {scalingCallback} Scaling function. Returns undefined if function is not
     *     registered.
     */
    getScaler(name, _callback) {
        if (!(name in this.scalers)) {
            return undefined;
        }
        return this.scalers[name];
    }
    /**
     * Change type of a previously defined field to modifier and register it
     *
     * @param {string} group Defines the group name for the field. The group can be any string, but
     *     if it matches a valid Mixxx control group name, it is possible to map a field to a
     *     control or output without any additional code.
     * @param {string} name Is the name of the control for the field. The name can be any string,
     *     but if it matches a valid Mixxx control name in the group defined for field, the system
     *     attempts to attach it directly to the correct field. Together group and name form the ID
     *     of the field (group.name)
     * @param {any} modifier
     */
    linkModifier(group, name, modifier) {
        const packet = this.getInputPacket(this.defaultPacket);
        if (packet === undefined) {
            console.error("HIDController.linkModifier - Creating modifier: input packet " + this.defaultPacket + " not found");
            return;
        }
        const bit_id = group + "." + name;
        const field = packet.lookupBit(group, name);
        if (field === undefined) {
            console.error("HIDController.linkModifier - Bit field not found: " + bit_id);
            return;
        }
        field.group = "modifiers";
        field.name = modifier;
        this.modifiers.set(modifier);
    }
    /**
     * @todo Implement unlinking of modifiers
     * @param {string} _group Unused
     * @param {string} _name Unused
     * @param _modifier Unused
     */
    unlinkModifier(_group, _name, _modifier) {
        console.warn("HIDController.unlinkModifier - Unlinking of modifiers not yet implemented");
    }
    /**
     * Link a previously declared HID control to actual mixxx control
     *
     * @param {string} group Defines the group name for the field. The group can be any string, but
     *     if it matches a valid Mixxx control group name, it is possible to map a field to a
     *     control or output without any additional code.
     * @param {string} name Is the name of the control for the field. The name can be any string,
     *     but if it matches a valid Mixxx control name in the group defined for field, the system
     *     attempts to attach it directly to the correct field. Together group and name form the ID
     *     of the field (group.name)
     * @param {string} m_group Mapped group
     * @param {string} m_name Mapped name
     * @param {controlCallback} callback Callback function for the control
     */
    linkControl(group, name, m_group, m_name, callback) {
        let field;
        const packet = this.getInputPacket(this.defaultPacket);
        if (packet === undefined) {
            console.error("HIDController.linkControl - Creating modifier: input packet " + this.defaultPacket + " not found");
            return;
        }
        field = packet.getField(group, name);
        if (field === undefined) {
            console.error("HIDController.linkControl - Field not found: " + group + "." + name);
            return;
        }
        if (field.type === "bitvector") {
            field = packet.lookupBit(group, name);
            if (field === undefined) {
                console.error("HIDController.linkControl - Bit not found: " + group + "." + name);
                return;
            }
        }
        field.mapped_group = m_group;
        field.mapped_name = m_name;
        if (callback !== undefined) {
            field.callback = callback;
        }
    }
    /**
     * @todo Implement unlinking of controls
     * @param {string} _group Defines the group name for the field. The group can be any string, but
     *     if it matches a valid Mixxx control group name, it is possible to map a field to a
     *     control or output without any additional code.
     * @param {string} _name  Is the name of the control for the field. The name can be any string,
     *     but if it matches a valid Mixxx control name in the group defined for field, the system
     *     attempts to attach it directly to the correct field. Together group and name form the ID
     *     of the field (group.name)
     */
    unlinkControl(_group, _name) {}
    /**
     * Register HID input packet type to controller.
     * Input packets can be responses from device to queries, or control
     * data details. The default control data packet must be named in
     * variable this.defaultPacket to allow automatic processing.
     *
     * @param {HIDPacket} packet The input packet to register
     */
    registerInputPacket(packet) {
        // Find modifiers and other special cases from packet fields
        for (const group_name in packet.groups) {
            const group = packet.groups[group_name];
            for (const field_name in group) {
                const field = group[field_name];
                field.packet = packet;
                if (field.type === "bitvector") {
                    for (const bit_id in field.value.bits) {
                        const bit = field.value.bits[bit_id];
                        bit.packet = packet;
                        if (bit.group === "modifiers") {
                            this.modifiers.add(bit.name);
                        }
                    }
                } else {
                    if (field.group === "modifiers") {
                        this.modifiers.add(field.name);
                    }
                }
            }
        }
        this.InputPackets[packet.name] = packet;
    }
    /**
     * Register HID output packet type to controller
     * There are no special Output control output packets, just register Outputs to any
     * valid packet and we detect them here.
     * This module only supports sending bitvector values and byte fields to device.
     * If you need other data structures, patches are welcome, or you can just do it
     * manually in your script without registering the packet.
     *
     * @param {HIDPacket} packet The output packet to register
     */
    registerOutputPacket(packet) {
        this.OutputPackets[packet.name] = packet;
        // Link packet to all fields
        for (const group_name in packet.groups) {
            const group = packet.groups[group_name];
            for (const field_name in group) {
                const field = group[field_name];
                field.packet = packet;
                if (field.type === "bitvector") {
                    for (const bit_id in field.value.bits) {
                        const bit = field.value.bits[bit_id];
                        bit.packet = packet;
                    }
                }
            }
        }
    }
    /**
     * Parse a packet representing an HID InputReport, and processes each field with "unpack":
     *  - Calls packet callback and returns, if packet callback was defined
     *  - Calls processIncomingPacket and processes automated events there.
     *  - If defined, calls processDelta for results after processing automated fields
     *
     * @param {number[]} data The data received from an HID InputReport.
     *                        In case of HID devices, which use ReportIDs to enumerate the reports,
     * the ReportID is stored in the first byte and the data start at the second byte
     * @param {number} length Length of the data array in bytes
     */
    parsePacket(data, length) {
        /** @type {HIDPacket} */
        let packet;
        let changed_data;
        if (this.InputPackets === undefined) {
            return;
        }
        for (const name in this.InputPackets) {
            packet = this.InputPackets[name];

            // When the device uses ReportIDs to enumerate the reports, hidapi
            // prepends the report ID to the data sent to Mixxx. If the device
            // only has a single report type, the HIDPacket constructor sets the
            // reportId as 0. In this case, hidapi only sends the data of the
            // report to Mixxx without a report ID.
            if (packet.reportId !== 0 && packet.reportId !== data[0]) {
                continue;
            }

            if (packet.header !== undefined) {
                for (let header_byte = 0; header_byte < packet.header.length; header_byte++) {
                    if (packet.header[header_byte] !== data[header_byte]) {
                        packet = undefined;
                        break;
                    }
                }
                if (packet === undefined) {
                    continue;
                }
            }
            changed_data = packet.parse(data);
            if (packet.callback !== undefined) {
                packet.callback(packet, changed_data);
                return;
            }
            // Process named group controls
            if (packet.name === this.defaultPacket) {
                this.processIncomingPacket(packet, changed_data);
            }
            // Process generic changed_data packet, if callback is defined
            if (this.processDelta !== undefined) {
                this.processDelta(packet, changed_data);
            }
            if (this.postProcessDelta !== undefined) {
                this.postProcessDelta(packet, changed_data);
            }
            return;
        }
        console.warn("HIDController.parsePacket - Received unknown packet of " + length + " bytes");
        for (const i in data) {
            console.log("BYTE " + data[i]);
        }
    }
    /**
     * Process the modified field values (delta) from input packet fields for
     * input control packet, if packet name is in this.defaultPacket.
     *
     * Button (Boolean value) field processing:
     * - Sets modifiers from buttons
     * - Calls button callbacks, if defined
     * - Finally tries to run matching engine.setValue() function for buttons
     *   in default mixxx groups, honoring toggleButtons and other button
     *   details. Not done if a callback was defined for button.
     *
     * Control (Numeric value) field processing
     * - Calls scaling functions for control fields, if defined for field.
     *   Scaling function for encoders (isEncoder attribute is true) scales
     *   field delta instead of raw value.
     * - Calls callback functions for control fields, if defined for field
     * - Finally tries run matching engine.setValue() function for control
     *   fields in default mixxx groups. Not done if a callback was defined.
     *
     * @param packet Unused
     * @param delta
     */
    processIncomingPacket(packet, delta) {
        /** @type {packetField} */
        let field;
        for (const name in delta) {
            if (this.ignoredControlChanges !== undefined &&
                this.ignoredControlChanges.indexOf(name) !== -1) {
                continue;
            }
            field = delta[name];
            if (field.type === "button") {
                // Button/Boolean field
                this.processButton(field);
            } else if (field.type === "control") {
                // Numeric value field
                this.processControl(field);
            } else {
                console.warn("HIDController.processIncomingPacket - Unknown field " + field.name + " type " + field.type);
            }
        }
    }
    /**
     * Get active group for this field
     *
     * @param {packetField} field Object that describes a field inside of a packet, which can often
     *     mapped to a Mixxx control.
     * @returns {string} Group
     */
    getActiveFieldGroup(field) {
        if (field.mapped_group !== undefined) {
            return this.resolveGroup(field.mapped_group);
        }
        const group = field.group;
        if (group === undefined) {
            if (this.activeDeck !== undefined) {
                return "[Channel" + this.activeDeck + "]";
            }
        }
        if (this.valid_groups.indexOf(group) !== -1) {
            // console.log("Resolving group " + group);
            return this.resolveGroup(group);
        }
        return group;
    }
    /**
     * Get active control name from field
     *
     * @param {packetField} field Object that describes a field inside of a packet, which can often
     *     mapped to a Mixxx control.
     * @returns {string} Name of field
     */
    getActiveFieldControl(field) {
        if (field.mapped_name !== undefined) {
            return field.mapped_name;
        } else {
            return field.name;
        }
    }
    /**
     * Process given button field, triggering events
     *
     * @param {packetField} field Object that describes a field inside of a packet, which can often
     *     mapped to a Mixxx control.
     */
    processButton(field) {
        const group = this.getActiveFieldGroup(field);
        const control = this.getActiveFieldControl(field);

        if (group === undefined) {
            console.warn(
                "HIDController.processButton - Could not resolve group from "
                + field.group + " " + field.mapped_group + " "
                + field.name + " " + field.mapped_name);
            return;
        }

        if (group === "modifiers") {
            if (field.value !== 0) {
                this.modifiers.set(control, true);
            } else {
                this.modifiers.set(control, false);
            }
            return;
        }
        if (field.auto_repeat) {
            const timer_id = "auto_repeat_" + field.id;
            if (field.value) {
                this.startAutoRepeatTimer(timer_id, field.auto_repeat_interval);
            } else {
                this.stopAutoRepeatTimer(timer_id);
            }
        }
        if (field.callback !== undefined) {
            field.callback(field);
            return;
        }
        if (control === "jog_touch") {
            if (group !== undefined) {
                if (field.value === this.buttonStates.pressed) {
                    this.enableScratch(group, true);
                } else {
                    this.enableScratch(group, false);
                }
            }
            return;
        }
        if (this.toggleButtons.indexOf(control) !== -1) {
            if (field.value === this.buttonStates.released) {
                return;
            }
            if (engine.getValue(group, control)) {
                if (control === "play") {
                    engine.setValue(group, "stop", true);
                } else {
                    engine.setValue(group, control, false);
                }
            } else {
                engine.setValue(group, control, true);
            }
            return;
        }
        if (field.auto_repeat && field.value === this.buttonStates.pressed) {
            console.log("Callback for " + field.group);
            engine.setValue(group, control, field.auto_repeat(field));
        } else if (engine.getValue(group, control) === false) {
            engine.setValue(group, control, true);
        } else {
            engine.setValue(group, control, false);
        }
    }
    /**
     * Process given control field, triggering events
     *
     * @param {packetField} field Object that describes a field inside of a packet, which can often
     *     mapped to a Mixxx control.
     */
    processControl(field) {
        let value;
        const group = this.getActiveFieldGroup(field);
        const control = this.getActiveFieldControl(field);

        if (group === undefined) {
            console.warn(
                "HIDController.processControl - Could not resolve group from "
                + field.group + " " + field.mapped_group + " "
                + field.name + " " + field.mapped_name);
            return;
        }

        if (field.callback !== undefined) {
            value = field.callback(field);
            return;
        }
        if (group === "modifiers") {
            this.modifiers.set(control, field.value);
            return;
        }
        if (control === "jog_wheel") {
            // Handle jog wheel scratching transparently
            this.jog_wheel(field);
            return;
        }
        // Call value scaler if defined and send mixxx signal
        value = field.value;
        const scaler = this.getScaler(control);
        if (field.isEncoder) {
            let field_delta = field.delta;
            if (scaler !== undefined) {
                field_delta = scaler(group, control, field_delta);
            }
            engine.setValue(group, control, field_delta);
        } else {
            if (scaler !== undefined) {
                value = scaler(group, control, value);
                // See the Traktor S4 script for how to use this.  If the scaler function has this
                // parameter set to true, we use the effects-engine setParameter call instead of
                // setValue.
                if (scaler.useSetParameter) {
                    engine.setParameter(group, control, value);
                    return;
                }
            }
            engine.setValue(group, control, value);
        }
    }
    /**
     * Toggle control state from toggle button
     *
     * @param {string} group Defines the group name for the field. The group can be any string, but
     *     if it matches a valid Mixxx control group name, it is possible to map a field to a
     *     control or output without any additional code.
     * @param control
     * @param value
     */
    toggle(group, control, value) {
        if (value === this.buttonStates.released) {
            return;
        }
        const status = Boolean(engine.getValue(group, control)) !== true;
        engine.setValue(group, control, status);
    }
    /**
     * Toggle play/pause state
     *
     * @param {string} group Defines the group name for the field. The group can be any string, but
     *     if it matches a valid Mixxx control group name, it is possible to map a field to a
     *     control or output without any additional code.
     * @param {packetField} field Object that describes a field inside of a packet, which can often
     *     mapped to a Mixxx control.
     */
    togglePlay(group, field) {
        if (field.value === this.buttonStates.released) {
            return;
        }
        const status = !(engine.getValue(group, "play"));
        if (!status) {
            engine.setValue(group, "stop", true);
        } else {
            engine.setValue(group, "play", true);
        }
    }
    /**
     * Processing of the 'jog_touch' special button name, which is used to detect
     * when scratching should be enabled.
     * Deck is resolved from group with 'resolveDeck'
     *
     * @param {string} group Defines the group name for the field. The group can be any string, but
     *     if it matches a valid Mixxx control group name, it is possible to map a field to a
     *     control or output without any additional code.
     * @param {boolean} status Enable or Disable scratching:
     * - true enables scratching (press 'jog_touch' button)
     * Sets the internal 'isScratchEnabled' attribute to true, and calls scratchEnable
     * with the scratch attributes (see class definition)
     *
     * - false disables scratching (release 'jog_touch' button)
     * Sets the internal 'isScratchEnabled attribute to false, and calls scratchDisable
     * to end scratching mode
     */
    enableScratch(group, status) {
        const deck = this.resolveDeck(group);
        if (status) {
            this.isScratchEnabled = true;
            engine.scratchEnable(deck,
                this.scratchintervalsPerRev,
                this.scratchRPM,
                this.scratchAlpha,
                this.scratchBeta,
                this.scratchRampOnEnable
            );
            if (this.enableScratchCallback !== undefined) {
                this.enableScratchCallback(true);
            }
        } else {
            this.isScratchEnabled = false;
            engine.scratchDisable(deck, this.scratchRampOnDisable);
            if (this.enableScratchCallback !== undefined) {
                this.enableScratchCallback(false);
            }
        }
    }
    /**
     * Default jog scratching function. Used to handle jog move events from special
     * input control field called 'jog_wheel'. Handles both 'scratch' and 'jog' mixxx
     * functions, depending on isScratchEnabled value above (see enableScratch())
     *
     * Since most controllers require value scaling for jog and scratch functions,
     * you are warned if following scaling function names are not registered:
     *
     * jog
     *      Scaling function from 'jog_wheel' for rate bend events with mixxx 'jog'
     *      function. Should return value range suitable for 'jog', whatever you
     *      wish it to do.
     * jog_scratch
     *      Scaling function from 'jog_wheel' for scratch movements with mixxx
     *      'scratchTick' function. Should return -1,0,1 or small ranges of integers
     *      both negative and positive values.
     *
     * @param {packetField} field Object that describes a field inside of a packet, which can often
     *     mapped to a Mixxx control.
     */
    jog_wheel(field) {
        let scaler = undefined;
        const active_group = this.getActiveFieldGroup(field);
        let value = undefined;
        if (field.isEncoder) {
            value = field.delta;
        } else {
            value = field.value;
        }
        if (this.isScratchEnabled) {
            const deck = this.resolveDeck(active_group);
            if (deck === undefined) {
                return;
            }
            scaler = this.getScaler("jog_scratch");
            if (scaler !== undefined) {
                value = scaler(active_group, "jog_scratch", value);
            } else {
                console.warn("HIDController.jog_wheel - Non jog_scratch scaler, you likely want one");
            }
            engine.scratchTick(deck, value);
        } else {
            if (active_group === undefined) {
                return;
            }
            scaler = this.getScaler("jog");
            if (scaler !== undefined) {
                value = scaler(active_group, "jog", value);
            } else {
                console.warn("HIDController.jog_wheel - Non jog scaler, you likely want one");
            }
            engine.setValue(active_group, "jog", value);
        }
    }
    /**
     * Stops the specified auto repeat timer
     *
     * @param {string} timer_id Reference of the timer to stop
     */
    stopAutoRepeatTimer(timer_id) {
        if (this.timers[timer_id]) {
            engine.stopTimer(this.timers[timer_id]);
            delete this.timers[timer_id];
        } else {
            // console.warn("HIDController.stopAutoRepeatTimer - No such autorepeat timer: " + timer_id);
        }
    }
    /**
     * Toggle field autorepeat on or off
     *
     * @param {string} group
     * @param {string} name
     * @param {controlCallback} callback Callback function for the control
     * @param {number} interval
     */
    setAutoRepeat(group, name, callback, interval) {
        const packet = this.getInputPacket(this.defaultPacket);
        const field = packet.getField(group, name);
        if (field === undefined) {
            console.error("HIDController.setAutoRepeat - Field not found " + group + "." + name);
            return;
        }
        field.auto_repeat = callback;
        if (interval) {
            field.auto_repeat_interval = interval;
        } else {
            field.auto_repeat_interval = this.auto_repeat_interval;
        }
        if (callback) {
            callback(field);
        }
    }
    /**
     * Callback for auto repeat timer to send again the values for
     * buttons and controls marked as 'auto_repeat'
     * Timer must be defined from actual controller side, because of
     * callback call namespaces and 'this' reference
     */
    autorepeatTimer() {
        let group_name;
        let group;
        let field;
        let field_name;
        let bit_name;
        let bit;
        const packet = this.InputPackets[this.defaultPacket];
        for (group_name in packet.groups) {
            group = packet.groups[group_name];
            for (field_name in group) {
                field = group[field_name];
                if (field.type !== "bitvector") {
                    if (field.auto_repeat) {
                        this.processControl(field);
                    }
                    continue;
                }
                for (bit_name in field.value.bits) {
                    bit = field.value.bits[bit_name];
                    if (bit.auto_repeat) {
                        this.processButton(bit);
                    }
                }
            }
        }
    }
    /**
     * Toggle active deck and update virtual output field control mappings
     *
     * @param {number} deck Number of deck
     */
    switchDeck(deck) {
        let packet;
        let field;
        let controlgroup;
        if (deck === undefined) {
            if (this.activeDeck === undefined) {
                deck = 1;
            } else {
                // This is unusable: num_decks has always minimum 4 decks
                // var totalDecks = engine.getValue("[Master]","num_decks");
                // deck = (this.activeDeck+1) % totalDecks;
                deck = this.deckSwitchMap[this.activeDeck];
                if (deck === undefined) {
                    deck = 1;
                }
            }
        }
        const new_group = this.resolveDeckGroup(deck);
        console.log("Switching to deck " + deck + " group " + new_group);
        if (this.disconnectDeck !== undefined) {
            this.disconnectDeck();
        }
        for (const packet_name in this.OutputPackets) {
            packet = this.OutputPackets[packet_name];
            for (const group_name in packet.groups) {
                const group = packet.groups[group_name];
                for (const field_name in group) {
                    field = group[field_name];
                    if (field.type === "bitvector") {
                        for (const bit_id in field.value.bits) {
                            const bit = field.value.bits[bit_id];
                            if (this.virtualDecks.indexOf(bit.mapped_group) === -1) {
                                continue;
                            }
                            controlgroup = this.resolveGroup(bit.mapped_group);
                            engine.connectControl(
                                controlgroup, bit.mapped_name, bit.mapped_callback, true);
                            engine.connectControl(new_group, bit.mapped_name, bit.mapped_callback);
                            const value = engine.getValue(new_group, bit.mapped_name);
                            console.log("Bit " + bit.group + "." + bit.name + " value " + value);
                            if (value) {
                                this.setOutput(
                                    bit.group, bit.name,
                                    this.LEDColors[this.deckOutputColors[deck]]);
                            } else {
                                this.setOutput(
                                    bit.group, bit.name,
                                    this.LEDColors.off
                                );
                            }
                        }
                        continue;
                    }
                    // Only move outputs of virtual decks
                    if (this.virtualDecks.indexOf(field.mapped_group) === -1) {
                        continue;
                    }
                    controlgroup = this.resolveGroup(field.mapped_group);
                    engine.connectControl(
                        controlgroup, field.mapped_name, field.mapped_callback, true);
                    engine.connectControl(new_group, field.mapped_name, field.mapped_callback);
                    const value = engine.getValue(new_group, field.mapped_name);
                    if (value) {
                        this.setOutput(
                            field.group, field.name,
                            this.LEDColors[this.deckOutputColors[deck]]
                        );
                    } else {
                        this.setOutput(
                            field.group, field.name,
                            this.LEDColors.off
                        );
                    }
                }
            }
        }
        this.activeDeck = deck;
        if (this.connectDeck !== undefined) {
            this.connectDeck();
        }
    }
    /**
     * Link a virtual HID Output to mixxx control
     *
     * @param {string} group Defines the group name for the field. The group can be any string, but
     *     if it matches a valid Mixxx control group name, it is possible to map a field to a
     *     control or output without any additional code.
     * @param {string} name  Is the name of the control for the field. The name can be any string,
     *     but if it matches a valid Mixxx control name in the group defined for field, the system
     *     attempts to attach it directly to the correct field. Together group and name form the ID
     *     of the field (group.name)
     * @param {string} m_group Mapped group
     * @param {string} m_name Name of mapped control
     * @param {controlCallback} callback Callback function for the control
     */
    linkOutput(group, name, m_group, m_name, callback) {
        const field = this.getOutputField(group, name);
        if (field === undefined) {
            console.error("HIDController.linkOutput - Linked output not found: " + group + "." + name);
            return;
        }
        if (field.mapped_group !== undefined) {
            console.warn("HIDController.linkOutput - Output already linked: " + field.mapped_group);
            return;
        }
        const controlgroup = this.resolveGroup(m_group);
        field.mapped_group = m_group;
        field.mapped_name = m_name;
        field.mapped_callback = callback;
        engine.connectControl(controlgroup, m_name, callback);
        if (engine.getValue(controlgroup, m_name)) {
            this.setOutput(m_group, m_name, true);
        } else {
            this.setOutput(m_group, m_name, false);
        }
    }
    /**
     * Unlink a virtual HID Output from mixxx control
     *
     * @param {string} group Defines the group name for the field. The group can be any string, but
     *     if it matches a valid Mixxx control group name, it is possible to map a field to a
     *     control or output without any additional code.
     * @param {string} name  Is the name of the control for the field. The name can be any string,
     *     but if it matches a valid Mixxx control name in the group defined for field, the system
     *     attempts to attach it directly to the correct field. Together group and name form the ID
     *     of the field (group.name)
     * @param {controlCallback} callback Callback function for the control
     */
    unlinkOutput(group, name, callback) {
        const field = this.getOutputField(group, name);
        if (field === undefined) {
            console.warn("HIDController.unlinkOutput - Output to be unlinked not found: " + group + "." + name);
            return;
        }
        if (field.mapped_group === undefined || field.mapped_name === undefined) {
            console.warn("HIDController.unlinkOutput - Output to be unlinked not mapped: " + group + "." + name);
            return;
        }
        const controlgroup = this.resolveGroup(field.mapped_group);
        engine.connectControl(controlgroup, field.mapped_name, callback, true);
        field.mapped_group = undefined;
        field.mapped_name = undefined;
        field.mapped_callback = undefined;
    }
    /**
     * Set output state to given value
     *
     * @param {string} group Defines the group name for the field. The group can be any string, but
     *     if it matches a valid Mixxx control group name, it is possible to map a field to a
     *     control or output without any additional code.
     * @param {string} name  Is the name of the control for the field. The name can be any string,
     *     but if it matches a valid Mixxx control name in the group defined for field, the system
     *     attempts to attach it directly to the correct field. Together group and name form the ID
     *     of the field (group.name)
     * @param {number|boolean} value Value to set as new output state of the control
     * @param {boolean} [send_packet=false] If true, the packet (an HID OutputReport) is send
     *     immediately
     */
    setOutput(group, name, value, send_packet) {
        const field = this.getOutputField(group, name);
        if (field === undefined) {
            console.error("HIDController.setOutput - Unknown field: " + group + "." + name);
            return;
        }
        field.value = Number(value) << field.bit_offset;
        field.toggle = Number(value) << field.bit_offset;
        if (send_packet) {
            field.packet.send();
        }
    }
    /**
     * Set Output to toggle between two values. Reset with setOutput(name,'off')
     *
     * @param {string} group Defines the group name for the field. The group can be any string, but
     *     if it matches a valid Mixxx control group name, it is possible to map a field to a
     *     control or output without any additional code.
     * @param {string} name  Is the name of the control for the field. The name can be any string,
     *     but if it matches a valid Mixxx control name in the group defined for field, the system
     *     attempts to attach it directly to the correct field. Together group and name form the ID
     *     of the field (group.name)
     * @param toggle_value
     */
    setOutputToggle(group, name, toggle_value) {
        const field = this.getOutputField(group, name);
        if (field === undefined) {
            console.error("HIDController.setOutputToggle - Unknown field " + group + "." + name);
            return;
        }
        field.value = toggle_value << field.bit_offset;
        field.toggle = toggle_value << field.bit_offset;
        field.packet.send();
    }
}
// Add class HIDController to the Global JavaScript object
// @ts-ignore Same identifier for class and instance needed for backward compatibility
this.HIDController = HIDController;
