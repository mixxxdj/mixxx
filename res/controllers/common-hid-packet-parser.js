// fixing names to to be camelcase would break the API
// so disable it for the entire file for now.
/* eslint-disable camelcase */

/**
 * Common HID script debugging function. Just to get logging with 'HID' prefix.
 * @deprecated Use console.log instead
 * @param {any} message Message to be printed on controller debug console output
 */
this.HIDDebug = function(message) {
    console.log(`HID ${message}`);
};
/**
 * creates a `DataView` from any ArrayBuffer, TypedArray
 * or plain Array (clamped to 8-bit width).
 * @param {number[] | ArrayBuffer | Int8Array | Uint8Array | Uint8ClampedArray | Int16Array | Uint16Array | Int32Array | Uint32Array } bufferLike Object that can be represented as a sequence of bytes
 * @returns {DataView} dataview over the bufferLike object
 */
const createDataView = function(bufferLike) {
    return new DataView((() => {
        if (Array.isArray(bufferLike)) {
            return new Uint8ClampedArray(bufferLike).buffer;
        } else if (ArrayBuffer.isView(bufferLike)) {
            return bufferLike.buffer;
        } else {
            return bufferLike;
        }
    })());
};

/**
 * Callback function to call when, the packet represents an HID InputReport, and new data for this
 * InputReport are received. If a packet callback is defined and the data for the InputReport are
 * received, the complete report data are sent to the callback function after field values are
 * parsed, without calling any packet field parsing functions.
 * @callback packetCallback
 * @param {HIDPacket} packet The packet that represents the InputReport
 * @param {Object.<string, packetField | bitObject>} changed_data The data received from the device
 */
/**
 * Callback function to call when, the value of a modifier control changed
 * @callback modifierCallback
 * @param {boolean} Value of the modifier control
 */
/**
 * Callback function to call when, data for specified filed in the packet is updated.
 * @callback fieldChangeCallback
 * @param {packetField|bitObject} Object that describes a field/bit inside of a packet, which can often
 *     mapped to a Mixxx control.
 */

/**
 * Callback function, which will be called every time, the value of the connected control changes.
 * @callback controlCallback
 * @param {number} value New value of the control
 * @param {string} group Mixxx control group name e.g. "[Channel1]"
 * @param {string} name Mixxx control name "pregain"
 * @returns {any} Value
 */

/**
 * In almost every case, a HID controller sends data values with input fields which are not directly
 * suitable for Mixxx control values. To solve this issue, HIDController contains function to scale
 * the input value to suitable range automatically before calling any field processing functions.
 * Scalers can be registered with HIDController.setScaler.
 *
 * The ScallingCallback function can also have a boolean property .useSetParameter, if:
 * - 'false' or 'undefined', engine.setValue is used
 * - 'true' engine.setParameter is used
 * @callback scalingCallback
 * @param {string} group Control group name e.g. "[Channel1]"
 * @param {string} name Control name "pregain"
 * @param {number} value Value to be scaled
 * @returns {number} Scaled value
 */

/**
 * Callback function to call when, jog wheel scratching got enabled or disabled by
 * the button with the special name 'jog_touch'
 * @callback scratchingCallback
 * @param {boolean} isScratchEnabled True, when button 'jog_touch' is active
 */

/**
 * @typedef packetField
 * @type {Object}
 * @property {HIDPacket} packet
 * @property {string} id Group and control name separated by a dot
 * @property {string} group
 * @property {string} name
 * @property {string} mapped_group Mapped group, must be a valid Mixxx control group name e.g. "[Channel1]"
 * @property {string} mapped_name Name of mapped control, must be a valid Mixxx control name "vu_meter"
 * @property {controlCallback} mapped_callback
 * @property {string} pack Control packing format for unpack(), one of b/B, h/H, i/I
 * @property {number} offset Position of the first byte in the packet in bytes (first byte is 0)
 * @property {number} end_offset Position of the last byte in the packet in bytes ({@link packetField.offset} + packet size)
 * @property {number} bitmask
 * @property {boolean} isEncoder
 * @property {fieldChangeCallback} callback
 * @property {boolean} soft_takeover
 * @property {boolean} ignored
 * @property {fieldChangeCallback} auto_repeat
 * @property {number} auto_repeat_interval
 * @property {number} min
 * @property {number} max
 * @property {('bitvector'|'control'|'output')} type Must be either:
 *              - 'bitvector'       If value is of type HIDBitVector
 *              - 'control'         If value is a number
 *              - 'output'
 * @property {HIDBitVector|boolean|number} value
 * @property {number} delta
 * @property {number} mindelta
 * @property {number} toggle
 */

/**
 * @typedef bitObject
 * @type {Object}
 * @property {HIDPacket} packet
 * @property {string} id Group and control name separated by a dot
 * @property {string} group
 * @property {string} name
 * @property {string} mapped_group Mapped group, must be a valid Mixxx control group name e.g. "[Channel1]"
 * @property {string} mapped_name Name of mapped control, must be a valid Mixxx control name "cue_indicator"
 * @property {controlCallback} mapped_callback
 * @property {number} bitmask
 * @property {number} bit_offset
 * @property {fieldChangeCallback} callback
 * @property {fieldChangeCallback} auto_repeat
 * @property {number} auto_repeat_interval
 * @property {('button'|'output')} type Must be either:
 *              - 'button'
 *              - 'output'
 * @property {number} value
 * @property {number} toggle
 */

/**
 * HID Bit Vector Class
 *
 * Collection of bits in one parsed packet field. These objects are
 * created by HIDPacket addControl and addOutput and should not be
 * created manually.
 */
// @ts-ignore Same identifier for class and instance needed for backward compatibility
class HIDBitVector {
    constructor() {
        /**
         * Number of bitObjects in bits array
         * @type {number}
         */
        this.size = 0;
        /**
         * Object of bitObjects, referred by a string of group and control name separated by a dot
         * @type {Object.<string, bitObject>}
         */
        this.bits = {};
    }
    /**
     * Get the index of the least significant bit that is 1 in `bitmask`
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
     * @param {string} group Control group name e.g. "[Channel1]"
     * @param {string} name Control name e.g. "play"
     * @param {number} bitmask A bitwise mask of up to 32 bit. All bits set to'1' in this mask are
     *     considered.
     */
    addBitMask(group, name, bitmask) {
        /** @type {bitObject} */
        const bit = {};
        bit.type = "button";
        bit.packet = undefined;
        bit.id = `${group}.${name}`;
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
     * @param {string} group Control group name e.g. "[Channel1]"
     * @param {string} name Control name e.g. "play"
     * @param {number} bitmask A bitwise mask of up to 32 bit. All bits set to'1' in this mask are
     *     considered.
     */
    addOutputMask(group, name, bitmask) {
        /** @type {bitObject} */
        const bit = {};
        bit.type = "output";
        bit.packet = undefined;
        bit.id = `${group}.${name}`;
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
 * e.g. a shift button can be defined as modifier for the behavior of other controls.
 *
 * Wraps all defined modifiers to one object with uniform API.
 * Don't call directly, this is available as HIDController.modifiers
 */
// @ts-ignore Same identifier for class and instance needed for backward compatibility
class HIDModifierList {
    constructor() {
        /**
         * Actual value of the modifier
         * @type {Object.<string, boolean>}
         */
        this.modifiers = Object();

        /**
         * Function to be called after modifier value changes
         * @type {Object.<string, modifierCallback>}
         */
        this.callbacks = Object();
    }
    /**
     * Add a new modifier to controller.
     * @param {string} name Name of modifier
     */
    add(name) {
        if (name in this.modifiers) {
            console.warn(`HIDModifierList.add - Modifier already defined: ${name}`);
            return;
        }
        this.modifiers[name] = undefined;
    }
    /**
     * Set modifier value
     * @param {string} name Name of modifier
     * @param {boolean} value Value to be set
     */
    set(name, value) {
        if (!(name in this.modifiers)) {
            console.error(`HIDModifierList.set - Unknown modifier: ${name}`);
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
     * @param {string} name Name of modifier
     * @returns {boolean} Value of modifier
     */
    get(name) {
        if (!(name in this.modifiers)) {
            console.error(`HIDModifierList.get - Unknown modifier: ${name}`);
            return false;
        }
        return this.modifiers[name];
    }
    /**
     * Set modifier callback function
     * @param {string} name Name of reference in HIDModifierList
     * @param {modifierCallback} callback Function to be called after modifier value changes
     */
    setCallback(name, callback) {
        if (!(name in this.modifiers)) {
            console.error(`HIDModifierList.setCallback - Unknown modifier: ${name}`);
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
 */
// @ts-ignore Same identifier for class and instance needed for backward compatibility
class HIDPacket {
    /**
     * @param {string} name Name of packet (it makes sense to refer the HID report type and HID
     * ReportID here e.g. 'InputReport_0x02' or 'OutputReport_0x81')
     * @param {number} reportId ReportID of the packet. If the device does not use ReportIDs this
     * must be 0. [default = 0]
     * @param {packetCallback} callback function to call when the packet type represents an InputReport,
     * and a new report is received. If packet callback is set, the packet is not parsed by delta
     * functions. Note, that a callback is not meaningful for output packets.
     * @param {number[]} header (optional) List of bytes to match from beginning of packet.
     * Do NOT put the report ID in this - use the reportId parameter instead.
     */
    constructor(name, reportId = 0, callback = undefined, header = []) {
        /**
         * Name of packet
         * @type {string}
         */
        this.name = name;

        /**
         * ReportID of the packet. If the device does not use ReportIDs this must be 0.
         * @type {number}
         */
        this.reportId = reportId;

        /**
         * Function to call when the packet type represents an InputReport, and a new report is received.
         * @type {packetCallback}
         */
        this.callback = callback;

        /**
         * List of bytes to match from beginning of packet
         * @type {number[]}
         */
        this.header = header;

        /**
         * Object of groups, referred by the group string
         * @type {Object.<string, Object.<string, any>>}
         */
        this.groups = {};

        /**
         * Length of packet in bytes
         * @type {number}
         */
        this.length = this.header.length;

        /**
         * Size of the 'pack' types in bytes
         * @type {Object.<string, number>}
         */
        this.packSizes = {b: 1, B: 1, h: 2, H: 2, i: 4, I: 4};
        this.signedPackFormats = ["b", "h", "i"];
    }

    /**
     * Pack a field value to the packet.
     * Can only pack bits and byte values, patches welcome.
     * @todo Implement multi byte bit vector outputs
     * @param {Uint8Array} data Data to be send as OutputReport to the device
     * @param {packetField} field Object that describes a field inside of a packet, which can often
     *     mapped to a Mixxx control.
     */
    pack(data, field) {
        if (!(field.pack in this.packSizes)) {
            console.error(`HIDPacket.pack - Parsing packed value: invalid pack format ${field.pack}`);
            return;
        }
        if (field.type === "bitvector") {
            const bitVector = /** @type {HIDBitVector} */ (field.value);
            if (this.packSizes[field.pack] > 1) {
                console.error("HIDPacket.pack - Packing multibyte bit vectors not yet supported");
                return;
            }
            HIDController.fastForIn(bitVector.bits, (bit) => {
                data[field.offset] |= bitVector.bits[bit].value;
            }
            );
            return;
        }

        const value = Number((field.value !== undefined) ? field.value : 0);

        if (value < field.min || value > field.max) {
            console.error(`HIDPacket.pack - ${field.id} packed value out of range: ${value}`);
            return;
        }

        const dataView = createDataView(data);
        switch (field.pack) {
        case "b":
            dataView.setInt8(field.offset, value);
            break;
        case "B":
            dataView.setUint8(field.offset, value);
            break;
        case "h":
            dataView.setInt16(field.offset, value, true);
            break;
        case "H":
            dataView.setUint16(field.offset, value, true);
            break;
        case "i":
            dataView.setInt32(field.offset, value, true);
            break;
        case "I":
            dataView.setUint32(field.offset, value, true);
            break;
        default:
              // Impossible, because range checked at beginning of the function
        }
    }
    /**
     * Parse and return field value matching the 'pack' field from field attributes.
     * Valid field packing types are:
     * - b       signed byte
     * - B       unsigned byte
     * - h       signed short
     * - H       unsigned short
     * - i       signed integer
     * - I       unsigned integer
     * @param {number[] | ArrayBuffer | Int8Array | Uint8Array | Uint8ClampedArray | Int16Array | Uint16Array | Int32Array | Uint32Array} data Data received as InputReport from the device
     * @param {packetField} field Object that describes a field inside of a packet, which can often
     *     mapped to a Mixxx control.
     * @returns {number} Value for the field in data, represented according the fields packing type
     */
    unpack(data, field) {
        const dataView = createDataView(data);
        switch (field.pack) {
        case "b":
            return dataView.getInt8(field.offset);
        case "B":
            return dataView.getUint8(field.offset);
        case "h":
            return dataView.getInt16(field.offset, true);
        case "H":
            return dataView.getUint16(field.offset, true);
        case "i":
            return dataView.getInt32(field.offset, true);
        case "I":
            return dataView.getUint32(field.offset, true);
        default:
            console.error(`HIDPacket.unpack - Parsing packed value: invalid pack format ${field.pack}`);
            return undefined;
        }
    }
    /**
     * Find HID packet group matching name.
     * Create group if create is true
     * @param {string} name Name of the group
     * @param {boolean} [create] If true, group will be created
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
     * @param {number} offset The field's offset from the start of the packet in bytes:
     *                        - For HID devices which don't use ReportIDs, the data bytes starts at
     * position 0
     *                        - For HID devices which use ReportIDs to enumerate the reports, the
     * data bytes starts at position 1
     * @param {string} pack Is one of the field packing types:
     *              - b       signed byte       (Int8)
     *              - B       unsigned byte     (Uint8)
     *              - h       signed short      (Int16  Little-Endian)
     *              - H       unsigned short    (Uint16 Little-Endian)
     *              - i       signed integer    (Int32  Little-Endian)
     *              - I       unsigned integer  (Uint32 Little-Endian)
     * @returns {packetField} Returns matching field or undefined if no matching field can be found.
     */
    getFieldByOffset(offset, pack) {
        if (!(pack in this.packSizes)) {
            console.error(`HIDPacket.getFieldByOffset - Unknown pack string ${pack}`);
            return undefined;
        }
        const end_offset = offset + this.packSizes[pack];

        for (const group_name in this.groups) {
            const group = this.groups[group_name];
            for (const field_id in group) {
                /** @type {packetField} */
                const field = group[field_id];
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
     * @param {string} group Control group name e.g. "[Channel1]"
     * @param {string} name Control name e.g. "play"
     * @returns {packetField} Field
     */
    getField(group, name) {
        const field_id = `${group}.${name}`;
        if (!(group in this.groups)) {
            console.error(`HIDPacket.getField - Packet ${this.name} group not found ${group}`);
            return undefined;
        }

        const control_group1 = this.groups[group];
        if (field_id in control_group1) {
            return control_group1[field_id];
        }

        // Lookup for bit fields in bitvector matching field name
        for (const group_name in this.groups) {
            const control_group2 = this.groups[group_name];
            for (const field_name in control_group2) {
                const field = control_group2[field_name];
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
     * @param {string} group Control group name e.g. "[Channel1]"
     * @param {string} name Control name e.g. "play"
     * @returns {bitObject} Reference to a bit in a bitvector field
     */
    lookupBit(group, name) {
        const field = this.getField(group, name);
        if (field === undefined) {
            console.error(`HIDPacket.lookupBit - Bitvector match not found: ${group}.${name}`);
            return undefined;
        }
        if (field.type !== "bitvector") {
            console.error(`HIDPacket.lookupBit - Control doesn't refer a field of type bitvector: ${group}.${name}`);
            return undefined;
        }
        const bitVector = /** @type {HIDBitVector} */ (field.value);
        const bit_id = `${group}.${name}`;

        // Fast loop implementation over bitvector.bits object
        const bitVectorKeyArr = Object.keys(bitVector.bits);
        let bitVectorKeyIdx = bitVectorKeyArr.length;
        while (bitVectorKeyIdx--) {
            const bit = bitVector.bits[bitVectorKeyArr[bitVectorKeyIdx]];
            if (bit.id === bit_id) {
                return bit;
            }
        }
        console.error("HIDPacket.lookupBit - BUG: bit not found after successful field lookup");
        return undefined;
    }
    /**
     * Remove a control registered. Normally not needed
     * @param {string} group Control group name e.g. "[Channel1]"
     * @param {string} name Control name e.g. "play"
     */
    removeControl(group, name) {
        const control_group = this.getGroup(group);
        if (!(name in control_group)) {
            console.warn(`HIDPacket.removeControl - Field not in control group ${group}: ${name}`);
            return;
        }
        delete control_group[name];
    }
    /**
     * Register a numeric value to parse from input packet
     *
     * 'group' and 'name' form the ID of the field, if it matches a valid Mixxx control name,
     * the system attempts to attach it directly to the correct field.
     * @param {string} group Control group name e.g. "[Channel1]"
     * @param {string} name Control name e.g. "play"
     * @param {number} offset The field's offset from the start of the packet in bytes:
     *                        - For HID devices which don't use ReportIDs, the data bytes starts at
     * position 0
     *                        - For HID devices which use ReportIDs to enumerate the reports, the
     * data bytes starts at position 1
     * @param {string} pack Is one of the field packing types:
     *              - b       signed byte       (Int8)
     *              - B       unsigned byte     (Uint8)
     *              - h       signed short      (Int16  Little-Endian)
     *              - H       unsigned short    (Uint16 Little-Endian)
     *              - i       signed integer    (Int32  Little-Endian)
     *              - I       unsigned integer  (Uint32 Little-Endian)
     * @param {number} [bitmask]  A bitwise mask of up to 32 bit. All bits set to'1' in this mask are
     *     considered.
     *           Note: For controls that use full bytes (8bit, 16bit, ...), you can set this to
     * undefined NOTE: Parsing bitmask with multiple bits is not supported yet.
     * @param {boolean} [isEncoder] indicates if this is an encoder which should be wrapped and delta
     *     reported
     * @param {fieldChangeCallback} [callback] Callback function for the control
     */
    addControl(group, name, offset, pack, bitmask, isEncoder, callback) {
        const control_group = this.getGroup(group, true);
        if (control_group === undefined) {
            console.error(`HIDPacket.addControl - Creating HID packet group ${group}`);
            return;
        }
        if (!(pack in this.packSizes)) {
            console.error(`HIDPacket.addControl - Unknown pack value ${pack}`);
            return;
        }

        const fieldByOffset = this.getFieldByOffset(offset, pack);
        if (fieldByOffset !== undefined) {
            if (bitmask === undefined) {
                console.error(`HIDPacket.addControl - Registering offset ${offset} pack ${pack}`);
                console.error(`HIDPacket.addControl - Trying to overwrite non-bitmask control ${group} ${name}`);
                return;
            }
            if (fieldByOffset.type !== "bitvector") {
                console.error(`HIDPacket.addControl - Field is not of type bitvector: ${group}.${name}`);
                return;
            } else {
                const bitVector = /** @type {HIDBitVector} */ (fieldByOffset.value);
                bitVector.addBitMask(group, name, bitmask);
                if (callback !== undefined) {
                    if (typeof callback !== "function") {
                        console.error(
                            `HIDPacket.addControl - Callback provided for ${group}.${name} is not a function.`);
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
        field.id = `${group}.${name}`;
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

        const packet_max_value = Math.pow(2, this.packSizes[field.pack] * 8) - 1;
        const signed = this.signedPackFormats.includes(field.pack);
        if (signed) {
            field.min = 0 - ((packet_max_value + 1) / 2) + 1;
            field.max = ((packet_max_value + 1) / 2) - 1;
        } else {
            field.min = 0;
            field.max = packet_max_value;
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
            const field_name = `bitvector_${offset}`;
            field.type = "bitvector";
            field.name = field_name;
            field.id = `${group}.${field_name}`;
            const bitvector = new HIDBitVector();
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
                    `HIDPacket.addControl - Callback provided for ${group}.${name} is not a function.`);
                return;
            }
            this.setCallback(group, name, callback);
        }
    }
    /**
     * Register a Output control field or Output control bit to output packet
     * Output control field:
     * Output field with no bitmask, controls Output with multiple values
     * Output control bit:
     * Output with with bitmask, controls Output with a single bit
     *
     * It is recommended to define callbacks after packet creation with
     * setCallback instead of adding it directly here. But you can do it.
     * @param {string} group Control group name e.g. "[Channel1]"
     * @param {string} name Control name "vu_meter"
     * @param {number} offset The field's offset from the start of the packet in bytes:
     *                        - For HID devices which don't use ReportIDs, the data bytes starts at
     * position 0
     *                        - For HID devices which use ReportIDs to enumerate the reports, the
     * data bytes starts at position 1
     * @param {string} pack Is one of the field packing types:
     *              - b       signed byte       (Int8)
     *              - B       unsigned byte     (Uint8)
     *              - h       signed short      (Int16  Little-Endian)
     *              - H       unsigned short    (Uint16 Little-Endian)
     *              - i       signed integer    (Int32  Little-Endian)
     *              - I       unsigned integer  (Uint32 Little-Endian)
     * @param {number} [bitmask] A bitwise mask of up to 32 bit. All bits set to'1' in this mask are
     *     considered.
     * @param {fieldChangeCallback} [callback] Callback function for the control
     */
    addOutput(group, name, offset, pack, bitmask, callback) {
        const control_group = this.getGroup(group, true);
        const field_id = `${group}.${name}`;

        if (control_group === undefined) {
            return;
        }
        if (!(pack in this.packSizes)) {
            console.error(`HIDPacket.addOutput - Unknown Output control pack value ${pack}`);
            return;
        }

        // Adjust offset by 1 because the reportId was previously considered part of the payload
        // but isn't anymore and we can't be bothered to adjust every single script manually
        offset -= 1;

        // Check if we are adding a Output bit to existing bitvector
        const fieldByOffset = this.getFieldByOffset(offset, pack);
        if (fieldByOffset !== undefined) {
            if (bitmask === undefined) {
                console.error(`HIDPacket.addOutput - Overwrite non-bitmask control ${group}.${name}`);
                return;
            }
            if (fieldByOffset.type !== "bitvector") {
                console.error(`HIDPacket.addOutput - Field is not of type bitvector: ${group}.${name}`);
                return;
            }
            const bitVector = /** @type {HIDBitVector} */ (fieldByOffset.value);
            bitVector.addOutputMask(group, name, bitmask);
            if (this.length < offset) { this.length = offset; }
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
            const field_name = `bitvector_${offset}`;
            field.type = "bitvector";
            field.id = `${group}.${field_name}`;
            field.name = field_name;
            const bitvector = new HIDBitVector();
            bitvector.size = field.max;
            bitvector.addOutputMask(group, name, bitmask);
            field.value = bitvector;
            field.delta = undefined;
            field.mindelta = undefined;
        }

        // Add Output to HID packet
        if (this.length < field.end_offset) { this.length = field.end_offset; }
        control_group[field.id] = field;
    }
    /**
     * Register a callback to field or a bit vector bit.
     * Does not make sense for Output fields but you can do that.
     * @param {string} group Control group name e.g. "[Channel1]"
     * @param {string} name Control name e.g. "play"
     * @param {fieldChangeCallback} callback Callback function for the control
     */
    setCallback(group, name, callback) {
        const field = this.getField(group, name);
        const field_id = `${group}.${name}`;
        if (callback === undefined) {
            console.error(`HIDPacket.setCallback - Callback to add was undefined for ${field_id}`);
            return;
        }
        if (field === undefined) {
            console.error(`HIDPacket.setCallback - Field for ${field_id} not found`);
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
            console.error(`HIDPacket.setCallback - Bit not found ${field_id}`);
        } else {
            field.callback = callback;
        }
    }
    /**
     * This function can be set in script code to ignore a field you don't want to be processed but
     * still wanted to define, to make packet format complete from specifications. If field is
     * ignored, it is not reported in 'delta' objects.
     * @param {string} group Control group name e.g. "[Channel1]"
     * @param {string} name Control name "pregain"
     * @param {boolean} ignored 'ignored' flag for field to given value (true or false)
     */
    setIgnored(group, name, ignored) {
        const field = this.getField(group, name);
        if (field === undefined) {
            console.error(`HIDPacket.setIgnored - Setting ignored flag for ${group} ${name}`);
            return;
        }
        field.ignored = ignored;
    }
    /**
     * Adjust field's minimum delta value.
     * Input value changes smaller than this are not reported in delta
     * @param {string} group Control group name e.g. "[Channel1]"
     * @param {string} name Control name "pregain"
     * @param {number} mindelta Minimum delta value.
     */
    setMinDelta(group, name, mindelta) {
        const field = this.getField(group, name);
        if (field === undefined) {
            console.error(`HIDPacket.setMinDelta - Adjusting mindelta for ${group} ${name}`);
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
     * @param {packetField} field Object that describes a field inside of a packet, which can often
     *     mapped to a Mixxx control.
     * @param {number} value Value must be a valid unsigned byte to parse, with enough bits.
     * @returns {Object.<string, bitObject>} List of modified bits (delta),
     *                                       referred by a string of group and control name separated by a dot
     */
    parseBitVector(field, value) {
        /**
         * Object of bitObjects, referred by a string of group and control name separated by a dot
         * @type {Object.<string, bitObject>}
         */
        const bits = {};

        if (field.type !== "bitvector") {
            console.error("HIDPacket.parseBitVector - Field isn't of type bitvector");
            return undefined;
        }
        const bitVector = /** @type {HIDBitVector} */ (field.value);

        HIDController.fastForIn(bitVector.bits, (bit_id) => {
            const bit = bitVector.bits[bit_id];
            const new_value = (bit.bitmask & value) >> bit.bit_offset;
            if (bit.value !== undefined && bit.value !== new_value) {
                bits[bit_id] = bit;
            }
            bit.value = new_value;
        }
        );
        return bits;
    }
    /**
     * Parse input packet fields from data.
     * Data is expected to be a Packet() received from HID device.
     * BitVectors are returned as bits you can iterate separately.
     * @param {number[] | ArrayBuffer | Int8Array | Uint8Array | Uint8ClampedArray | Int16Array | Uint16Array | Int32Array | Uint32Array} data Data received as InputReport from the device
     * @returns {Object.<string, packetField | bitObject>} List of changed fields with new value.
     */
    parse(data) {
        /**
         * Object of packetField or bitObjects, referred by a string of group and control name separated by a dot
         * @type {Object.<string, packetField | bitObject>}
         */
        const field_changes = {};

        // Fast loop implementation over this.groups object
        const groupKeyArr = Object.keys(this.groups);
        let groupKeyIdx = groupKeyArr.length;
        while (groupKeyIdx--) {
            const group = this.groups[groupKeyArr[groupKeyIdx]];

            // Fast loop implementation over group object
            const fieldKeyArr = Object.keys(group);
            let fieldKeyIdx = fieldKeyArr.length;
            while (fieldKeyIdx--) {
                const field = group[fieldKeyArr[fieldKeyIdx]];

                if (field === undefined) {
                    continue;
                }

                const value = this.unpack(data, field);
                if (value === undefined) {
                    console.error(`HIDPacket.parse - Parsing packet field value for ${group}.${field}`);
                    return;
                }

                if (field.type === "bitvector") {
                    // Bitvector deltas are checked in parseBitVector
                    const changedBits = this.parseBitVector(field, value);


                    HIDController.fastForIn(changedBits, (changedBit) => {
                        field_changes[changedBit] = changedBits[changedBit];
                    }
                    );

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
     * @param {boolean} [debug] Enables debug output to console
     */
    send(debug) {
        const data = new Uint8Array(this.length);

        if (this.header !== undefined) {
            for (let header_byte = 0; header_byte < this.header.length; header_byte++) {
                data[header_byte] = this.header[header_byte];
            }
        }

        HIDController.fastForIn(this.groups, (group_name) => {
            const group = this.groups[group_name];

            HIDController.fastForIn(group, (field_name) => {
                const field = group[field_name];

                this.pack(data, field);
            }
            );
        }
        );

        if (debug) {
            let packet_string = "";
            for (const d in data) {
                if (data[d] < 0x10) {
                    // Add padding for bytes smaller than 10
                    packet_string += "0";
                }
                packet_string += data[d].toString(16) + " ";
            }
            console.log(`Sending packet with Report ID ${this.reportId}: ${packet_string}`);
        }
        controller.sendOutputReport(this.reportId, data.buffer);
    }
}
// Add class HIDPacket to the Global JavaScript object
// @ts-ignore Same identifier for class and instance needed for backward compatibility
this.HIDPacket = HIDPacket;


/**
 * HID Controller Class with packet parser
 */
// @ts-ignore Same identifier for class and instance needed for backward compatibility
class HIDController {
    constructor() {

        /**
         * - By default 'false'
         * - Should be set 'true', when controller is found and everything is OK
         * @type {boolean}
         */
        this.initialized = false;

        /**
         * @type {number}
         */
        this.activeDeck = undefined;

        /**
         * HIDPackets representing HID InputReports, by packet name
         * @type {Object.<string, HIDPacket>}
         */
        this.InputPackets = {};

        /**
         * HIDPackets representing HID OutputReports, by packet name
         * @type {Object.<string, HIDPacket>}
         */
        this.OutputPackets = {};

        /**
         * A map to determine the output Bit or bytewise field by group and name,
         * across all OutputPackets
         * @type {Map<string,bitObject|packetField>}
         */
        this.OutputFieldLookup = new Map();

        /**
         * Default input packet name: can be modified for controllers
         * which can swap modes (wiimote for example)
         * @type {string}
         */
        this.defaultPacket = "control";

        // Callback functions called by deck switching. Undefined by default
        this.disconnectDeck = undefined;
        this.connectDeck = undefined;

        // Scratch parameter defaults for this.scratchEnable function
        // override for custom control
        /**
         * Set to true, when button 'jog_touch' is active
         * @type {boolean}
         */
        this.isScratchEnabled = false;

        /**
         * The resolution of the jogwheel HID control (in intervals per revolution)
         * - Default is 128
         * @type {number}
         */
        this.scratchintervalsPerRev = 128;

        /**
         * The speed of the imaginary record at 0% pitch - in revolutions per minute (RPM)
         * - Default 33+1/3 - adjust for comfort
         * @type {number}
         */
        this.scratchRPM = 33 + 1 / 3;

        /**
         * The alpha coefficient of the filter
         * - Default is 1/8 (0.125) - start tune from there
         * @type {number}
         */
        this.scratchAlpha = 1.0 / 8;

        /**
         * The beta coefficient of the filter
         * - Default is scratchAlpha/32 - start tune from there
         * @type {number}
         */
        this.scratchBeta = this.scratchAlpha / 32;

        /**
         * - Set 'true' to ramp the deck speed down.
         * - Set 'false' to stop instantly (default)
         * @type {boolean}
         */
        this.scratchRampOnEnable = false;

        /**
         * - Set 'true' to ramp the deck speed up.
         * - Set 'false' to jump to normal play speed instantly (default)
         * @type {boolean}
         */
        this.scratchRampOnDisable = false;

        /**
         * Callback function to call when, jog wheel scratching got enabled or disabled
         * @type {scratchingCallback}
         */
        this.enableScratchCallback = undefined;

        /**
         * List of valid state values for buttons, should contain fields:
         * - 'released' (default 0)
         * - 'pressed' (default 1)
         */
        this.buttonStates = {released: 0, pressed: 1};

        /**
         * List of named output colors to send
         * - must contain 'off' value
         */
        this.LEDColors = {off: 0x0, on: 0x7f};

        /**
         * List of button names you wish to act as 'toggle'
         *
         * i.e. pressing the button and releasing toggles state of the control and doesn't set it off again when released.
         */
        this.toggleButtons = [
            "play", "pfl", "keylock", "quantize", "reverse", "slip_enabled",
            "group_[Channel1]_enable", "group_[Channel2]_enable",
            "group_[Channel3]_enable", "group_[Channel4]_enable"
        ];

        /**
         * List of colors to use for each deck
         * - Default is 'on' for first four decks.
         *
         * Override to set specific colors for multicolor button output per deck:
         * - Values are like {1: 'red', 2: 'green' } and must reference valid OutputColors fields.
         */
        this.deckOutputColors = {1: "on", 2: "on", 3: "on", 4: "on"};

        //
        /**
         * Used to map the virtual deck names 'deck', 'deck1' or 'deck2' to actual [ChannelX]
         * @type {string[]}
         */
        this.virtualDecks = ["deck", "deck1", "deck2", "deck3", "deck4"];

        /**
         * Mapping of automatic deck switching with switchDeck function
         */
        this.deckSwitchMap = {1: 2, 2: 1, 3: 4, 4: 3, undefined: 1};

        //
        /**
         * Set to value in ms to update Outputs periodically
         * - By default undefined.
         * - If set, it's a value for timer executed every n ms to update Outputs with updateOutputs()
         * @deprecated This is unused and updateOutputs() doesn't exist - Remove?
         * @type {number}
         */
        this.OutputUpdateInterval = undefined;

        /**
         * Reference to HIDModifierList object
         * @type {HIDModifierList}
         */
        this.modifiers = new HIDModifierList();

        /**
         * Object of scaling function callbacks by name
         * @type {Object.<string, scalingCallback>}
         */
        this.scalers = {};

        /**
         * Object of engine timer IDs of running auto repeat timers
         * Key is a user specified timer_id.
         * Used only in the controller.startAutoRepeatTimer code stubs of Sony-SixxAxis.js and Nintendo-Wiimote.js.
         * @type {Object.<number, number>}
         */
        this.timers = {};

        /**
         * Auto repeat interval default for fields, where not specified individual (in milliseconds)
         * @default 100
         * @type {number}
         */
        this.auto_repeat_interval = 100;

        /**
         * @deprecated Use {@link postProcessDelta} instead
         * (not used in any official mapping)
         * @type {packetCallback}
         */
        this.processDelta = undefined;

        /**
         * Callback that is executed after parsing incoming packet
         * (see Traktor-Kontrol-F1-scripts.js for an example)
         * @type {packetCallback}
         */
        this.postProcessDelta = undefined;

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
     * Return deck number from deck name. Deck name can't be virtual deck name
     * in this function call.
     * @param {string} group Control group name e.g. "[Channel1]"
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
     * @param {number} deck Number of deck
     * @returns {string} Group name of the deck (e.g. Channel2 for deck number 2)
     */
    resolveDeckGroup(deck) {
        if (deck === undefined) {
            return undefined;
        }
        return `[Channel${deck}]`;
    }
    /**
     * Map virtual deck names ("deck, "deck1", "deck2") to real deck group. If group is already a
     * real mixxx group value, just return it as it without mapping.
     * @param {string} group Control group name e.g. "[Channel1]" or "deck" or "deck1".
     * @returns {string} Channel
     */
    resolveGroup(group) {
        if (group === "deck" || group === undefined) {
            if (this.activeDeck === undefined) {
                return undefined;
            }
            return `[Channel${this.activeDeck}]`;
        }
        if (group === "deck1") {
            if (this.activeDeck === 1 || this.activeDeck === 2) {
                return "[Channel1]";
            }
            if (this.activeDeck === 3 || this.activeDeck === 4) {
                return "[Channel3]";
            }
            return undefined;
        } else if (group === "deck2") {
            if (this.activeDeck === 1 || this.activeDeck === 2) {
                return "[Channel2]";
            }
            if (this.activeDeck === 3 || this.activeDeck === 4) {
                return "[Channel4]";
            }
            return undefined;
        }
        return group;
    }
    /**
     * Find Output control matching give group and name
     * @param {string} m_group Mapped group, must be a valid Mixxx control group name e.g. "[Channel1]"
     * @param {string} m_name Name of mapped control, must be a valid Mixxx control name "vu_meter"
     * @returns {bitObject|packetField} Bit or bytewise field - Returns undefined if output field
     *     can't be found.
     */
    getOutputField(m_group, m_name) {
        return this.OutputFieldLookup.get([m_group, m_name].toString());
    }
    /**
     * Find input packet matching given name.
     * Returns undefined if input packet name is not registered.
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
     * @param {string} packet The name of the input packet e.g. 'InputReport_0x02'
     * @param {string} group Control group name e.g. "[Channel1]"
     * @param {string} name Control name "pregain"
     * @param {fieldChangeCallback} callback Callback function for the control
     */
    setCallback(packet, group, name, callback) {
        const input_packet = this.getInputPacket(packet);
        if (input_packet === undefined) {
            console.error(`HIDController.setCallback - Input packet not found ${packet}`);
            return;
        }
        input_packet.setCallback(group, name, callback);
    }
    /**
     * Register scaling function for a control name
     * This does not check if given control name is valid
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
     * @param {string} name Reference of the scaling function in scalers list of HIDController
     * @param {any} _callback Unused
     * @returns {scalingCallback} Scaling function. Returns undefined if function is not
     *     registered.
     */
    getScaler(name, _callback = undefined) {
        if (!(name in this.scalers)) {
            return undefined;
        }
        return this.scalers[name];
    }
    /**
     * Change type of a previously defined field to modifier and register it
     * @param {string} group Control group name e.g. "[Channel1]"
     * @param {string} name Control name
     * @param {string} modifier Name of the modifier e.g. 'shiftbutton'
     */
    linkModifier(group, name, modifier) {
        const packet = this.getInputPacket(this.defaultPacket);
        if (packet === undefined) {
            console.error(`HIDController.linkModifier - Creating modifier: input packet ${this.defaultPacket} not found`);
            return;
        }
        const bit_id = `${group}.${name}`;
        const bitField = packet.lookupBit(group, name);
        if (bitField === undefined) {
            console.error(`HIDController.linkModifier - Bit field not found: ${bit_id}`);
            return;
        }
        bitField.group = "modifiers";
        bitField.name = modifier;
        this.modifiers.set(modifier, Boolean(bitField.value));
    }

    /**
     * @todo Implement unlinking of modifiers
     * @param {string} _group Unused
     * @param {string} _name Unused
     * @param {string} _modifier Unused
     */
    unlinkModifier(_group, _name, _modifier) {
        console.warn("HIDController.unlinkModifier - Unlinking of modifiers not yet implemented");
    }

    /**
     * Link a previously declared HID control to actual mixxx control
     * @param {string} group Control group name
     * @param {string} name Control name
     * @param {string} m_group Mapped group, must be a valid Mixxx control group name e.g. "[Channel1]"
     * @param {string} m_name Name of mapped control, must be a valid Mixxx control name "pregain"
     * @param {fieldChangeCallback} callback Callback function for the control
     */
    linkControl(group, name, m_group, m_name, callback) {
        const packet = this.getInputPacket(this.defaultPacket);
        if (packet === undefined) {
            console.error(`HIDController.linkControl - Creating modifier: input packet ${this.defaultPacket} not found`);
            return;
        }
        const field = packet.getField(group, name);
        if (field === undefined) {
            console.error(`HIDController.linkControl - Field not found: ${group}.${name}`);
            return;
        }
        if (field.type !== "bitvector") {
            field.mapped_group = m_group;
            field.mapped_name = m_name;
            if (callback !== undefined) {
                field.callback = callback;
            }
        } else {
            const bitField = packet.lookupBit(group, name);
            if (field === undefined) {
                console.error(`HIDController.linkControl - Bit not found: ${group}.${name}`);
                return;
            }
            bitField.mapped_group = m_group;
            bitField.mapped_name = m_name;
            if (callback !== undefined) {
                bitField.callback = callback;
            }
        }
    }
    /**
     * @todo Implement unlinking of controls
     * @param {string} _group Mixxx control group name e.g. "[Channel1]"
     * @param {string} _name  Mixxx control name "pregain"
     */
    unlinkControl(_group, _name) {}
    /**
     * Register HID input packet type to controller.
     * Input packets can be responses from device to queries, or control
     * data details. The default control data packet must be named in
     * variable this.defaultPacket to allow automatic processing.
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
                        // Fill lookup map
                        if (bit.mapped_group && bit.mapped_name) {
                            this.OutputFieldLookup.set([bit.mapped_group, bit.mapped_name].toString(), bit);
                        }
                        if (bit.group && bit.name) {
                            this.OutputFieldLookup.set([bit.group, bit.name].toString(), bit);
                        }
                    }
                } else {
                    if (field.mapped_group && field.mapped_name) {
                        this.OutputFieldLookup.set([field.mapped_group, field.mapped_name].toString(), field);
                    }
                    if (field.group && field.name) {
                        this.OutputFieldLookup.set([field.group, field.name].toString(), field);
                    }
                }
            }
        }
    }
    /**
     * Parse a packet representing an HID InputReport, and processes each field with "unpack":
     * - Calls packet callback and returns, if packet callback was defined
     * - Calls processIncomingPacket and processes automated events there.
     * - If defined, calls postProcessDelta for results after processing automated fields
     * @param {number[] | ArrayBuffer | Int8Array | Uint8Array | Uint8ClampedArray | Int16Array | Uint16Array | Int32Array | Uint32Array} data The data received from an HID InputReport.
     *                        In case of HID devices, which use ReportIDs to enumerate the reports,
     * the ReportID is stored in the first byte and the data start at the second byte
     * @param {number} [length] Length of the data array in bytes
     */
    parsePacket(data, length) {
        if (this.InputPackets === undefined) {
            return;
        }

        // Fast loop implementation over this.InputPackets object
        const InputPacketsKeyArr = Object.keys(this.InputPackets);
        let InputPacketsIdx = InputPacketsKeyArr.length;
        while (InputPacketsIdx--) {
            /** @type {HIDPacket} */
            let packet = this.InputPackets[InputPacketsKeyArr[InputPacketsIdx]];

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
            const changed_data = packet.parse(data);
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
        console.warn(`HIDController.parsePacket - Received unknown packet of ${length} bytes`);
        for (const i in data) {
            console.log(`BYTE ${data[i]}`);
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
     * in default mixxx groups, honoring toggleButtons and other button
     * details. Not done if a callback was defined for button.
     *
     * Control (Numeric value) field processing
     * - Calls scaling functions for control fields, if defined for field.
     * Scaling function for encoders (isEncoder attribute is true) scales
     * field delta instead of raw value.
     * - Calls callback functions for control fields, if defined for field
     * - Finally tries run matching engine.setValue() function for control
     * fields in default mixxx groups. Not done if a callback was defined.
     * @param {any} packet Unused
     * @param {Object.<string, packetField | bitObject>} delta
     */
    processIncomingPacket(packet, delta) {

        HIDController.fastForIn(delta, (function(field_name) {
            // @ts-ignore ignoredControlChanges should be defined in the users mapping
            // see EKS-Otus.js for an example
            if (this.ignoredControlChanges !== undefined &&
                    // @ts-ignore
                    this.ignoredControlChanges.indexOf(field_name) !== -1) {
                return; // continue loop - by returning to fastForIn
            }
            const field = delta[field_name];
            if (field.type === "button") {
                // Button/Boolean field
                this.processButton(field);
            } else if (field.type === "control") {
                // Numeric value field
                this.processControl(field);
            } else {
                console.warn(`HIDController.processIncomingPacket - Unknown field ${field.name} type ${field.type}`);
            }
        }).bind(this)); // Qt < 6.2.4 : .bind(this) needed because of QTBUG-95677
    }
    /**
     * Get active group for this field
     * @param {packetField|bitObject} field Object that describes a field inside of a packet, which can often
     *     mapped to a Mixxx control.
     * @returns {string} Group
     */
    getActiveFieldGroup(field) {
        if (field.mapped_group !== undefined) {
            return this.resolveGroup(field.mapped_group);
        }
        const group = field.group;
        if (group === "deck" || group === "deck1" || group === "deck2") {
            return group;
        }
        return this.resolveGroup(group);
    }
    /**
     * Get active control name from field
     * @param {packetField|bitObject} field Object that describes a field inside of a packet, which can often
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
     * @param {bitObject} field Object that describes a field inside of a packet, which can often
     *     mapped to a Mixxx control.
     */
    processButton(field) {
        const group = this.getActiveFieldGroup(field);
        const control = this.getActiveFieldControl(field);

        if (group === undefined) {
            console.warn(
                `HIDController.processButton - Could not resolve group from ${field.group} ${field.mapped_group} ${field.name} ${field.mapped_name}`);
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
            const timer_id = `auto_repeat_${field.id}`;
            if (field.value) {
                // @ts-ignore startAutoRepeatTimer needs to be implemented in the users mapping
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
                    engine.setValue(group, "stop", 1);
                } else {
                    engine.setValue(group, control, Number(false));
                }
            } else {
                engine.setValue(group, control, Number(true));
            }
            return;
        }
        if (field.auto_repeat && field.value === this.buttonStates.pressed) {
            console.log(`Callback for ${field.group}`);
            engine.setValue(group, control, field.auto_repeat(field));
        } else if (engine.getValue(group, control) === 0) {
            engine.setValue(group, control, Number(true));
        } else {
            engine.setValue(group, control, Number(false));
        }
    }
    /**
     * Process given control field, triggering events
     * @param {packetField} field Object that describes a field inside of a packet, which can often
     *     mapped to a Mixxx control.
     */
    processControl(field) {
        const group = this.getActiveFieldGroup(field);
        const control = this.getActiveFieldControl(field);

        if (group === undefined) {
            console.error(
                `HIDController.processControl - Could not resolve group from ${field.group} ${field.mapped_group} ${field.name} ${field.mapped_name}`);
            return;
        }
        if (field.type === "bitvector") {
            console.error(`HIDController.processControl  - Control refers a field of type bitvector: ${group}.${control}`);
            return;
        }

        if (field.callback !== undefined) {
            /*value =*/field.callback(field);
            return;
        }
        if (group === "modifiers") {
            this.modifiers.set(control, Boolean(field.value));
            return;
        }
        if (control === "jog_wheel") {
            // Handle jog wheel scratching transparently
            this.jog_wheel(field);
            return;
        }
        // Call value scaler if defined and send mixxx signal
        let value = field.value;
        const scaler = this.getScaler(control);
        if (field.isEncoder) {
            let field_delta = field.delta;
            if (scaler !== undefined) {
                field_delta = scaler(group, control, field_delta);
            }
            engine.setValue(group, control, field_delta);
        } else {
            if (scaler !== undefined) {
                value = scaler(group, control, Number(value));
                // See the Traktor S4 script for how to use this.  If the scaler function has this
                // parameter set to true, we use the effects-engine setParameter call instead of
                // setValue.
                if (scaler.useSetParameter) {
                    engine.setParameter(group, control, value);
                    return;
                }
            }
            engine.setValue(group, control, Number(value));
        }
    }
    /**
     * Toggle control state from toggle button
     * @param {string} group Control group name e.g. "[Channel1]"
     * @param {string} control Name of the control (button)
     * @param {number} value Value defined in this.buttonStates
     */
    toggle(group, control, value) {
        if (value === this.buttonStates.released) {
            return;
        }
        const status = Boolean(engine.getValue(group, control)) !== true;
        engine.setValue(group, control, Number(status));
    }
    /**
     * Toggle play/pause state
     * @param {string} group Control group name e.g. "[Channel1]"
     * @param {packetField} field Object that describes a field inside of a packet, which can often
     *     mapped to a Mixxx control.
     */
    togglePlay(group, field) {
        if (field.value === this.buttonStates.released) {
            return;
        }
        const status = !(engine.getValue(group, "play"));
        if (!status) {
            engine.setValue(group, "stop", Number(true));
        } else {
            engine.setValue(group, "play", Number(true));
        }
    }
    /**
     * Processing of the 'jog_touch' special button name, which is used to detect
     * when scratching should be enabled.
     * Deck is resolved from group with 'resolveDeck'
     * @param {string} group Control group name e.g. "[Channel1]"
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
     * Scaling function from 'jog_wheel' for rate bend events with mixxx 'jog'
     * function. Should return value range suitable for 'jog', whatever you
     * wish it to do.
     * jog_scratch
     * Scaling function from 'jog_wheel' for scratch movements with mixxx
     * 'scratchTick' function. Should return -1,0,1 or small ranges of integers
     * both negative and positive values.
     * @param {packetField} field Object that describes a field inside of a packet, which can often
     *     mapped to a Mixxx control.
     */
    jog_wheel(field) {
        const active_group = this.getActiveFieldGroup(field);
        if (field.type === "bitvector") {
            console.error("HIDPacket.jog_wheel - Setting field.value of type for bitvector packet does not make sense");
            return;
        }
        let value;
        if (field.isEncoder) {
            value = Number(field.delta);
        } else {
            value = Number(field.value);
        }
        if (this.isScratchEnabled) {
            const deck = this.resolveDeck(active_group);
            if (deck === undefined) {
                return;
            }
            const jogScratchScaler = this.getScaler("jog_scratch");
            if (jogScratchScaler !== undefined) {
                value = jogScratchScaler(active_group, "jog_scratch", value);
            } else {
                console.warn("HIDController.jog_wheel - Non jog_scratch scaler, you likely want one");
            }
            engine.scratchTick(deck, value);
        } else {
            if (active_group === undefined) {
                return;
            }
            const jogScaler = this.getScaler("jog");
            if (jogScaler !== undefined) {
                value = jogScaler(active_group, "jog", value);
            } else {
                console.warn("HIDController.jog_wheel - Non jog scaler, you likely want one");
            }
            engine.setValue(active_group, "jog", value);
        }
    }
    /**
     * Stops the specified auto repeat timer
     * @param {string} timer_id Reference of the timer to stop
     */
    stopAutoRepeatTimer(timer_id) {
        if (this.timers[timer_id]) {
            engine.stopTimer(this.autorepeat[timer_id]);
            delete this.timers[timer_id];
        } else {
            // console.warn(`HIDController.stopAutoRepeatTimer - No such autorepeat timer: ${timer_id}`);
        }
    }
    /**
     * Toggle field autorepeat on or off
     * @param {string} group
     * @param {string} name
     * @param {fieldChangeCallback} callback Callback function for the control
     * @param {number} interval
     */
    setAutoRepeat(group, name, callback, interval) {
        const packet = this.getInputPacket(this.defaultPacket);
        const field = packet.getField(group, name);
        if (field === undefined) {
            console.error(`HIDController.setAutoRepeat - Field not found ${group}.${name}`);
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
        const packet = this.InputPackets[this.defaultPacket];
        for (const group_name in packet.groups) {
            const group = packet.groups[group_name];
            for (const field_name in group) {
                const field = group[field_name];
                if (field.type !== "bitvector") {
                    if (field.auto_repeat) {
                        this.processControl(field);
                    }
                    continue;
                }
                for (const bit_name in field.value.bits) {
                    const bit = field.value.bits[bit_name];
                    if (bit.auto_repeat) {
                        this.processButton(bit);
                    }
                }
            }
        }
    }
    /**
     * Toggle active deck and update virtual output field control mappings
     * @param {number} deck Number of deck
     */
    switchDeck(deck) {
        if (deck === undefined) {
            if (this.activeDeck === undefined) {
                deck = 1;
            } else {
                // This is unusable: num_decks has always minimum 4 decks
                // var totalDecks = engine.getValue("[App]", "num_decks");
                // deck = (this.activeDeck+1) % totalDecks;
                deck = this.deckSwitchMap[this.activeDeck];
                if (deck === undefined) {
                    deck = 1;
                }
            }
        }
        const new_group = this.resolveDeckGroup(deck);
        console.log(`Switching to deck ${deck} group ${new_group}`);
        if (this.disconnectDeck !== undefined) {
            this.disconnectDeck();
        }
        for (const packet_name in this.OutputPackets) {
            const packet = this.OutputPackets[packet_name];
            for (const group_name in packet.groups) {
                const group = packet.groups[group_name];
                for (const field_name in group) {
                    const field = group[field_name];
                    if (field.type === "bitvector") {
                        for (const bit_id in field.value.bits) {
                            const bit = field.value.bits[bit_id];
                            if (this.virtualDecks.indexOf(bit.mapped_group) === -1) {
                                continue;
                            }
                            const bitControlGroup = this.resolveGroup(bit.mapped_group);
                            if (bitControlGroup === undefined) {
                                console.warn("HIDController.switchDeck: resolvedGroup(bit.mapped_group) returned undefined");
                            }
                            engine.connectControl(
                                bitControlGroup, bit.mapped_name, bit.mapped_callback, true);
                            engine.makeConnection(new_group, bit.mapped_name, bit.mapped_callback);
                            const value = engine.getValue(new_group, bit.mapped_name);
                            console.log(`Bit ${bit.group}.${bit.name} value ${value}`);
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
                    const fieldControlGroup = this.resolveGroup(field.mapped_group);
                    if (fieldControlGroup === undefined) {
                        console.warn("HIDController.switchDeck: resolvedGroup(field.mapped_group) returned undefined");
                    }
                    engine.connectControl(
                        fieldControlGroup, field.mapped_name, field.mapped_callback, true);
                    engine.makeConnection(new_group, field.mapped_name, field.mapped_callback);
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
     * @param {string} group Control group name
     * @param {string} name  Control name
     * @param {string} m_group Mapped group, must be a valid Mixxx control group name e.g. "[Channel1]"
     * @param {string} m_name Name of mapped control, must be a valid Mixxx control name "vu_meter"
     * @param {controlCallback} callback Callback function for the control
     */
    linkOutput(group, name, m_group, m_name, callback) {
        const field = this.getOutputField(group, name);
        if (field === undefined) {
            console.error(`HIDController.linkOutput - Linked output not found: ${group}.${name}`);
            return;
        }
        if (field.mapped_group !== undefined) {
            console.warn(`HIDController.linkOutput - Output already linked: ${field.mapped_group}`);
            return;
        }
        const controlgroup = this.resolveGroup(m_group);
        if (controlgroup === undefined) {
            console.warn("HIDController.linkOutput: resolvedGroup(m_group) returned undefined");
        }
        field.mapped_group = m_group;
        field.mapped_name = m_name;
        field.mapped_callback = callback;
        engine.makeConnection(controlgroup, m_name, callback);
        if (engine.getValue(controlgroup, m_name)) {
            this.setOutput(m_group, m_name, this.LEDColors.on);
        } else {
            this.setOutput(m_group, m_name, this.LEDColors.off);
        }
    }
    /**
     * Unlink a virtual HID Output from mixxx control
     * @param {string} group Mixxx control group name e.g. "[Channel1]"
     * @param {string} name Mixxx control name "vu_meter"
     * @param {controlCallback} callback Callback function for the control
     */
    unlinkOutput(group, name, callback) {
        const field = this.getOutputField(group, name);
        if (field === undefined) {
            console.warn(`HIDController.unlinkOutput - Output to be unlinked not found: ${group}.${name}`);
            return;
        }
        if (field.mapped_group === undefined || field.mapped_name === undefined) {
            console.warn(`HIDController.unlinkOutput - Output to be unlinked not mapped: ${group}.${name}`);
            return;
        }
        const controlgroup = this.resolveGroup(field.mapped_group);
        if (controlgroup === undefined) {
            console.warn("HIDController.unlinkOutput: resolvedGroup(field.mapped_group) returned undefined");
        }
        engine.connectControl(controlgroup, field.mapped_name, callback, true);
        field.mapped_group = undefined;
        field.mapped_name = undefined;
        field.mapped_callback = undefined;
    }
    /**
     * Set output state to given value
     * @param {string} group Control group name e.g. "[Channel1]"
     * @param {string} name Control name "cue_indicator"
     * @param {number|boolean} value Value to set as new output state of the control
     * @param {boolean} [send_packet] If true, the packet (an HID OutputReport) is send
     *     immediately
     */
    setOutput(group, name, value, send_packet) {
        const field = this.getOutputField(group, name);
        if (field === undefined) {
            console.error(`HIDController.setOutput - Unknown field: ${group}.${name}`);
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
     * @param {string} group Control group name e.g. "[Channel1]"
     * @param {string} name Control name "cue_indicator"
     * @param toggle_value
     */
    setOutputToggle(group, name, toggle_value) {
        const field = this.getOutputField(group, name);
        if (field === undefined) {
            console.error(`HIDController.setOutputToggle - Unknown field ${group}.${name}`);
            return;
        }
        field.value = toggle_value << field.bit_offset;
        field.toggle = toggle_value << field.bit_offset;
        field.packet.send();
    }
    /**
     * Fast loop implementation over object
     *
     * Don't use 'continue' and 'break' don't work as in normal loops,
     * because body is a function
     * 'return' statements in the body function behaves as 'continue' in normal loops
     * @param {Object.<string, any>} object
     * @param {function (string):void } body
     */
    static fastForIn(object, body) {
        const objKeyArray = Object.keys(object);
        let objKeyArrayIdx = objKeyArray.length;
        while (objKeyArrayIdx--) {
            body(objKeyArray[objKeyArrayIdx]);
        }
    }
}
// Add class HIDController to the Global JavaScript object
// @ts-ignore Same identifier for class and instance needed for backward compatibility
this.HIDController = HIDController;
