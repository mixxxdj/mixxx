// eslint-disable-next-line no-var, no-redeclare
var pcon = {};

// pcon.init = function(id, _debugging) {
//     console.info(`Pioneer Controller mapping started ${id}`);
// };

// pcon.shutdown = function() {
//     // nothing to do
// };

// /**
//  * this converts a typed array from one to the other, respecting the views offset and length.
//  * in theory, this is similar to a C++ reinterpret_cast
//  * @param {Function} cls constructor of a TypedArray to cast to
//  * @param {TypedArray} from TypedArray
//  * @returns {cls} the type being cast too
//  */
// pcon.castViewTo = function(cls, from) {
//     if (from instanceof cls) {
//         return from;
//     }
//     if (from instanceof ArrayBuffer) {
//         return new cls(from);
//     }
//     if (from instanceof Array) {
//         return pcon.castViewTo(cls, new Uint8Array(from));
//     }
//     if (cls === DataView) {
//         return new DataView()
//     }
//     const div =  ? 1 : cls.BYTES_PER_ELEMENT;
//     return new cls(from.buffer, from.byteOffset, Math.trunc((from.byteLength) / div));
// };

pcon.util = {
    isByte: function(size) {
        return size >= 0x00 && size <= 0xFF;
    },
    isASCII: function(byte) {
        return byte >= 0x20 && byte <= 0x7E;
    },
    chunk: function(arr, size) {
        const ret = [];
        for (let chunk = 0; chunk <  arr.length;) {
            ret.push(Array.from(arr.slice(chunk, chunk+=size)));
        }
        return ret;
    }
};

pcon.debug = {
    hexDump: function(buff) {
        return pcon.util.chunk(buff, 16).map(chunk => {
            const asciiRep = chunk.map(byte => pcon.util.isASCII(byte) ? String.fromCharCode(byte) : "•").join("");
            const bufHex = chunk.map(byte => pcon.util.isByte(byte) ? byte.toString(16).padStart(2, "0") : "  ").join(" ");
            const charsPerByte = 3;
            const pad = Math.min(buff.length, 16);
            const strBuffer = `${bufHex.slice(0, 8*charsPerByte)} ${bufHex.slice(8*charsPerByte, 16*charsPerByte)}`.padEnd(pad*charsPerByte);
            return `${strBuffer} | ${asciiRep}`;
        })
            .join("\n");
    }
};


pcon.makeTLV = function(type, data) {
    console.assert(pcon.util.isByte(type));
    console.assert(pcon.util.isByte(data.length + 2));
    return [type, data.length + 2].concat(data);
};
/**
 *
 * @param {Uint8Array} data ArrayBuffer containing the start of the TLV, along with trailing data
 * @returns {Uint8Array} TODO define
 */
pcon.readTLV = function(data) {
    if (data.length < 2) {
        return undefined;
    }
    const type = data[0];
    const length = data[1];

    console.assert(length <= data.length);
    const value = data.slice(2, length);

    console.debug(type.toString(16));
    console.debug(length.toString(16));
    console.debug(value);

    return {type: type, length: length, value: value, rest: data.slice(length)};
};

pcon.asciiEncode = function(string) {
    const ret = [];
    for (let i = 0; i < string.length; i++) {
        ret.push(string.codePointAt(i) & 0xFF);
    }
    return ret;
};
/**
 *
 * @param {ArrayBuffer} buff
 * @returns
 */

pcon.asciiDecode = function(buff) {
    return String.fromCodePoint(...(new Uint8Array(buff)));
};

pcon.buffIsEmpty = function(buff) {
    return (new Uint8Array(buff).every(val => val === 0));
};


pcon.spreadBuff = function(buff) {
    const ret = [];
    for (const byte of buff) {
        console.assert(pcon.util.isByte(byte));
        ret.push(byte >>> 4);
        ret.push(byte & 0x0F);
    }
    return ret;
};

pcon.contractBuff = function(buff) {
    console.assert(buff.length % 2 === 0);
    const view = new Uint8Array(buff);
    const ret = new Uint8Array(buff.length / 2);
    for (let i = 0; i < ret.length; i++) {
        ret[i] = (view[i*2] << 4) + view[i*2 + 1];
    }
    return ret;
};

pcon.U32Math = {};

// pcon.U32Math.Oxl = ([num]) => {return {
//     lo: Number.parseInt(num.slice(-4), 16), // lower 16-bit
//     hi: Number.parseInt(num.slice(-8, -4), 16) // higher 16-bit
// }};

// pcon.U32Math.fromBuffer = (buffer) => {
//     const view = new DataView(buffer);
//     return {
//         lo: view.getUint16(1),
//         hi: view.getUint16(0)
//     }
// };

// pcon.U32Math.losslessXor = (lhs, rhs) => {
//     return {
//         lo: lhs.lo ^ rhs.lo,
//         hi: lhs.hi ^ rhs.hi
//     };
// }

// pcon.U32Math.losslessMult = (lhs, rhs) => {
//     const lo = (lhs.lo * rhs.lo) >>> 0;
//     const loCarry = lo >>> 16;
//     return {
//         lo: lo & 0xFFFF,
//         hi: (((lhs.hi * rhs.hi) + loCarry) >>> 0) & 0xFFFF
//     }
// }

// pcon.U32Math.FNVhash = function (buff) {
//     // need to do 32-bit operations manually because
//     // rounding errors in 32-bit floats ruin the hash

//     let Oxl = pcon.U32Math.Oxl;
//     let losslessMult = pcon.U32Math.losslessMult;
//     let losslessXor = pcon.U32Math.losslessXor;

//     const bytes = new Uint8Array(buff);
//     return bytes.reduce((hash, val) =>
//         (losslessMult(losslessXor(val, hash), Oxl`1000193`)),
//         Oxl`811c9dc5`);
// }


pcon.U32Math.FNVhash = function(buff) {
    const bytes = new Uint8Array(buff);
    return bytes.reduce((hash, val) =>
        // appending >>> 0 to interpret result as Uint32
        (Math.imul((val ^ hash) >>> 0, 0x1000193) & 0xFFFFFFFF) >>> 0,
    0x811c9dc5);
};


// pcon.ManufacturerUnlockSecretHiLo = {
//     PioneerDJ: pcon.U32Math.Oxl`680131FB`,
//     Serato: pcon.U32Math.Oxl`0D6F55AB`,
//     NativeInstruments: pcon.U32Math.Oxl`8C5B3F5D`,
//     Atomix: pcon.U32Math.Oxl`97779123`,
//     Algoriddim: pcon.U32Math.Oxl`384B522B`,
//     Mixvibes: pcon.U32Math.Oxl`9EA1A8B6`,
//     Numark: pcon.U32Math.Oxl`889127C7`,
//     Dolby: pcon.U32Math.Oxl`7A2B59AB`
// };
pcon.ManufacturerUnlockSecretArray = {
    PioneerDJ: [0x68, 0x01, 0x31, 0xFB],
    Serato: [0x0D, 0x6F, 0x55, 0xAB],
    NativeInstruments: [0x8C, 0x5B, 0x3F, 0x5D],
    Atomix: [0x97, 0x77, 0x91, 0x23],
    Algoriddim: [0x38, 0x4B, 0x52, 0x2B],
    Mixvibes: [0x9E, 0xA1, 0xA8, 0xB6],
    Numark: [0x88, 0x91, 0x27, 0xC7],
    Dolby: [0x7A, 0x2B, 0x59, 0xAB]
};



pcon.DeviceManufacturerDeviceID = {
    NativeInstruments: {
        CDJ2000NXS2: [0x02, 0xF2, 0xF4, 0x51, 0x0E, 0xD3, 0x0A, 0x2C, 0x10, 0x42],
        DJM900NXS2: [0x43, 0xF4, 0xCD, 0xEE, 0x2C, 0x07, 0xC9, 0x59, 0x07, 0x42],
        CDJTOUR1: [0x76, 0x8D, 0xCE, 0xDA, 0x0D, 0xD7, 0xF0, 0x10, 0x43, 0x4E],
        DJMTOUR1: [0x50, 0x28, 0x76, 0x73, 0x8F, 0x3D, 0xC7, 0xCF, 0x1D, 0xA7],
        XDJ1000MK2: [0xAB, 0xA7, 0x78, 0xD3, 0xB5, 0x06, 0xE2, 0x2E, 0xBB, 0xAE],
        DJM450: [0x08, 0xEF, 0x3F, 0x2F, 0x1E, 0x7A, 0x90, 0x17, 0xF6, 0xAF],
        DJM250MK2: [0xD2, 0xF5, 0x8B, 0x61, 0x2C, 0x62, 0x06, 0x81, 0x91, 0x39],
        DJM750MK2: [0x86, 0x6E, 0x33, 0xBD, 0x04, 0x85, 0x2E, 0x71, 0xED, 0x21],
        DJMV10: [0x3C, 0x83, 0xDB, 0x25, 0x9C, 0x76, 0xFE, 0x8D, 0xB2, 0xAB]
    },
    Serato: {
        CDJ2000NXS2: [0xC0, 0xBD, 0x94, 0x0F, 0xB7, 0x00, 0xBB, 0x8F, 0xC7, 0x5B],
        DJM900NXS2: [0xD5, 0x57, 0xCE, 0x8F, 0x5E, 0xF0, 0x7B, 0xC2, 0x7E, 0x98],
        DJMV10: [0x1D, 0x3E, 0xAC, 0x55, 0xC6, 0x13, 0xD2, 0x97, 0x62, 0x3D],
        XDJZX: [0x2D, 0x99, 0x85, 0x8D, 0x0A, 0x5A, 0x91, 0x3D, 0x4E, 0xBD],
    },
    PioneerDJ: {
        CDJ2000NXS2: [0x02, 0xF2, 0xF4, 0x51, 0x0E, 0xD3, 0x0A, 0x2C, 0x10, 0x42],
    }
};

pcon.DeviceSysexCode = {
    DJM250MK2: [0x00, 0x00, 0x00, 0x17, 0x00],
    DJM750MK2: [0x00, 0x00, 0x01, 0x0B, 0x00],
};

pcon.seedA = [0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF];

pcon.keepAliveTimer = 0;
pcon.handshakeState = {};

pcon.send = {
    // https://swiftb0y.github.io/CDJHidProtocol/hid-analysis/startup.html#_sysex_extended_header
    sysex: function(deviceId, inner) {
        const data = [0xf0, 0x00, 0x40, 0x05].concat(deviceId, inner, [0xF7]);
        console.debug(`outgoing:\n${pcon.debug.hexDump(data)}`);
        midi.sendSysexMsg(data);
    },
    hid: function(deck, type, inner) {
        if (inner.length <= 62) {
            pcon.send.hidsimple(deck, type, inner);
        } else {
            const bytesPerFragment = (0x40 - 6);
            const fragments = Math.ceil(inner.length / bytesPerFragment);
            for (let index = 0; index < fragments; index++) {
                pcon.send.hidextended(deck, type, index, fragments, inner.slice(bytesPerFragment*index, bytesPerFragment*(index + 1)));
            }
        }
    },
    // https://swiftb0y.github.io/CDJHidProtocol/hid-analysis/startup.html#_simple_header
    hidsimple: function(deck, type, inner) {
        // TODO verify this is a an output report, not a feature report
        controller.sendOutputReport(deck, [type].concat(inner));
    },
    // https://swiftb0y.github.io/CDJHidProtocol/hid-analysis/startup.html#_extended_header
    hidextended: function(deck, type, index, num, inner) {
        // num & index in little-endian
        // TODO verify this is a an output report, not a feature report
        controller.sendOutputReport(deck, [type, index & 0xFF, index >> 8, num & 0xFF,  num >> 8].concat(inner));
    }
};
/**
 * @readonly
 * @enum {symbol}
 */
pcon.protocol = {
    HID: Symbol(),
    SYSEX: Symbol(),
};



pcon.parseHeader = {
    sysex: function(data) {
        const view = new DataView(data.buffer);
        console.assert(view.getUint8(0) === 0xf0);
        console.assert(view.getUint8(1) === 0x00);
        const manufacturerId = view.getUint16(2);
        console.debug(manufacturerId);
        console.assert(manufacturerId === 0x4005);
        const usbPid = (new DataView(pcon.contractBuff(data.slice(4, 8)).buffer)).getUint16(0);
        const deck = view.getUint8(8);
        console.assert(view.getUint8(data.length - 1) === 0xF7);
        const inner = data.slice(9, -1); // snip trailing sysex EOX
        return {
            usbPid: usbPid,
            deck: deck,
            inner: inner
        };
    },
    hid: function() {
        // TODO, also handle reassembly and the different header formats
    }
};

// run this to start the sysex handshake init
pcon.sysexGreet = function() {
    // TODO don't hardcode this
    pcon.send.sysex(pcon.DeviceSysexCode.DJM750MK2, [0x50, 0x01]);
};

/**
 *
 * @param {ArrayBuffer} data BinaryData from hardware
 * @param {pcon.protocol} protocol whether to communicate via HID or sysex (ugly hack for now)
 * @returns {bool} whether the packet was auth-related and handled by this function
 */

pcon.handleAuth = function(data, protocol) {

    const send = (stage, inner) => {
        if (protocol === pcon.protocol.HID) {
            pcon.send.hidextended(0, 0xf0, 1, stage, inner);
        } else {
            // TODO don't hardcode this
            pcon.send.sysex(pcon.DeviceSysexCode.DJM750MK2, inner);
        }
    };

    const manufacturer = "NativeInstruments";

    console.debug(pcon.debug.hexDump(data));
    const payload = (protocol === pcon.protocol.SYSEX) ? pcon.parseHeader.sysex(data).inner : pcon.parseHeader.hid(data).inner;
    // console.assert(view.getInt16(0) === 0x00f0);
    // console.assert(view.getInt16(1) === 0x0001);
    const supertlv = pcon.readTLV(payload);
    console.debug(supertlv.type);
    console.debug(supertlv.length);
    if (supertlv.type === 0x11) {
        // console.assert(view.getInt16(2) === 0x0001);

        if (supertlv.length > 2) {
            // firmware version only seems to be contained in HID messages?
            const firmwareVersionTlv = pcon.readTLV(supertlv.value);
            console.assert(firmwareVersionTlv.type === 0x01);
            console.assert(firmwareVersionTlv.length === 4);
            const firmwareView = new DataView(firmwareVersionTlv.value.buffer);
            console.info(`Pioneer Firmware version: ${firmwareView.getInt8(0)}.${firmwareView.getInt8(1)}`);
        }
        send(1, pcon.makeTLV(0x12,
            // probably already need to send the spoofed creds here
            pcon.makeTLV(0x01, pcon.asciiEncode(manufacturer)).concat(
                pcon.makeTLV(0x02, pcon.asciiEncode("TraktorDJ 2")),
                pcon.makeTLV(0x03, pcon.spreadBuff(pcon.seedA))
            )));
        return true;
    } else if (supertlv.type === 0x13) {
        // console.assert(view.getInt16(2) === 0x0002);

        const manufacturerTlv = pcon.readTLV(supertlv.value);
        console.assert(manufacturerTlv.type === 0x01);
        console.info(`Manufacturer: ${pcon.asciiDecode(manufacturerTlv.value)}`);

        const productTlv = pcon.readTLV(manufacturerTlv.rest);
        console.assert(productTlv.type === 0x02);
        pcon.handshakeState.device = pcon.asciiDecode(productTlv.value);
        console.info(`Product: ${pcon.handshakeState.device}`);

        const hashATlv = pcon.readTLV(productTlv.rest);
        console.assert(hashATlv.type === 0x04);
        console.assert(hashATlv.length === 0x0A);

        const seedETlv = pcon.readTLV(hashATlv.rest);
        console.assert(seedETlv.type === 0x03);
        // length seems to change between devices/protocol?
        // console.assert(seedETlv.length === 0x0D);

        const seedE = pcon.contractBuff(seedETlv.value);

        console.debug(`seedE:\n${pcon.util.hexDump(seedE)}`);

        const manufacturerSecret = pcon.ManufacturerUnlockSecretArray[manufacturer];

        console.assert(seedE.length === manufacturerSecret.length);
        const secret = manufacturerSecret.map((val, i) => val ^ seedE[i]);
        console.debug(`secret:\n${pcon.util.hexDump(secret)}`);


        const hashAd = pcon.U32Math.FNVhash((new Uint8Array(pcon.seedA.concat(secret))).buffer);
        const hashAView = new DataView(pcon.contractBuff(hashATlv.value).buffer);
        console.debug(`verify hash: ${hashAd}`);
        console.debug(`hashA ${hashAView.getUint32(0)}`);
        console.assert(hashAView.getUint32(0) === hashAd);

        // TODO optimize?
        const E = Array.from(seedE).concat(secret);
        console.debug(pcon.debug.hexDump(E));
        const hashE = pcon.U32Math.FNVhash((new Uint8Array(E)).buffer);

        console.debug(pcon.debug.hexDump(hashE));

        send(2, pcon.makeTLV(0x14,
            // I'm using the spoofed creds here.
            // TODO don't hardcode traktor here.
            pcon.makeTLV(0x01, pcon.asciiEncode(manufacturer)).concat(
                pcon.makeTLV(0x02, pcon.asciiEncode("TraktorDJ 2")),
                pcon.makeTLV(0x04, pcon.spreadBuff(hashE)),
                pcon.makeTLV(0x05, pcon.spreadBuff(pcon.DeviceManufacturerDeviceID[manufacturer][pcon.handshakeState.device])),
            )
        ));
        return true;
    } else if (supertlv.type === 0x15) {
        // console.assert(view.getInt16(2) === 0x0002);
        console.assert(supertlv.length === 2);
        console.info("Hardware authenticated successfully");

        if (pcon.keepAliveTimer !== 0) {
            engine.stopTimer(pcon.keepAliveTimer);
        }
        pcon.keepAliveTimer = engine.beginTimer(400, () => {
            if (protocol === pcon.protocol.HID) {
                // TODO confirm sendOutputReport is appropriate instead of setFeatureReport
                controller.sendOutputReport(0x00, Array(63).fill(0), true);
            } else if (protocol === pcon.protocol.SYSEX) {
                send([0x50, 0x01]);
            }
        });
        return true;
    } else if (pcon.buffIsEmpty(data)) {
        pcon.handshakeState = {};
        return true;
    }
    console.assert(pcon.buffIsEmpty(supertlv.rest));
    return false;
};
