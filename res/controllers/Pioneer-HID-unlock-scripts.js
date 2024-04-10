// eslint-disable-next-line no-var
var PioneerUnlock = {};

PioneerUnlock.init = function(id, _debugging) {
    console.info(`Pioneer Controller mapping started ${id}`);
};

PioneerUnlock.shutdown = function() {
    // nothing to do
};

/**
 * this converts a typed array from one to the other, respecting the views offset and length.
 * in theory, this is similar to a C++ reinterpret_cast
 * @param {Function} cls constructor of a TypedArray to cast to
 * @param {TypedArray} from TypedArray
 * @returns {cls} the type being cast too
 */
PioneerUnlock.castViewTo = function(cls, from) {
    if (from instanceof cls) {
        return from;
    }
    if (from instanceof ArrayBuffer) {
        return new cls(from);
    }
    const div = (cls === DataView) ? 1 : cls.BYTES_PER_ELEMENT;
    return new cls(from.buffer, from.byteOffset, Math.trunc((from.byteLength) / div));
};

PioneerUnlock.makeTLV = function(type, data) {
    console.assert(PioneerUnlock.isByte(type));
    console.assert(PioneerUnlock.isByte(data.length + 2));
    return [type, data.length + 2].concat(data);
};
/**
 *
 * @param {ArrayBuffer} data ArrayBuffer containing the start of the TLV, along with trailing data
 * @returns {Object} TODO define
 */
PioneerUnlock.readTLV = function(data) {
    const type = data[0];
    const length = data[1];

    console.assert(length <= data.length);
    const value = data.slice(2, length);

    return {type: type, length: length, value: value, rest: data.slice(length)};
};

PioneerUnlock.asciiEncode = function(string) {
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

PioneerUnlock.asciiDecode = function(buff) {
    return String.fromCodePoint(...(new Uint8Array(buff)));
};

PioneerUnlock.buffIsEmpty = function(buff) {
    return (new Uint8Array(buff).every(val => val === 0));
};


PioneerUnlock.spreadBuff = function(buff) {
    const ret = [];
    for (const byte in buff) {
        console.assert(PioneerUnlock.isByte(byte));
        ret.push(byte >> 4);
        ret.push(byte & 0x0F);
    }
    return ret;
};

PioneerUnlock.contractBuff = function(buff) {
    const view = new Uint8Array(buff);
    const ret = new Array(buff.length / 2);
    for (let i = 0; i < ret.length; i++) {
        ret[i] = (view[i*2] << 4) + view[i*2 + 1];
    }
    return ret;
};

PioneerUnlock.U32Math = {};

// PioneerUnlock.U32Math.Oxl = ([num]) => {return {
//     lo: Number.parseInt(num.slice(-4), 16), // lower 16-bit
//     hi: Number.parseInt(num.slice(-8, -4), 16) // higher 16-bit
// }};

// PioneerUnlock.U32Math.fromBuffer = (buffer) => {
//     const view = new DataView(buffer);
//     return {
//         lo: view.getUint16(1),
//         hi: view.getUint16(0)
//     }
// };

// PioneerUnlock.U32Math.losslessXor = (lhs, rhs) => {
//     return {
//         lo: lhs.lo ^ rhs.lo,
//         hi: lhs.hi ^ rhs.hi
//     };
// }

// PioneerUnlock.U32Math.losslessMult = (lhs, rhs) => {
//     const lo = (lhs.lo * rhs.lo) >>> 0;
//     const loCarry = lo >>> 16;
//     return {
//         lo: lo & 0xFFFF,
//         hi: (((lhs.hi * rhs.hi) + loCarry) >>> 0) & 0xFFFF
//     }
// }

// PioneerUnlock.U32Math.FNVhash = function (buff) {
//     // need to do 32-bit operations manually because
//     // rounding errors in 32-bit floats ruin the hash

//     let Oxl = PioneerUnlock.U32Math.Oxl;
//     let losslessMult = PioneerUnlock.U32Math.losslessMult;
//     let losslessXor = PioneerUnlock.U32Math.losslessXor;

//     const bytes = new Uint8Array(buff);
//     return bytes.reduce((hash, val) =>
//         (losslessMult(losslessXor(val, hash), Oxl`1000193`)),
//         Oxl`811c9dc5`);
// }


PioneerUnlock.U32Math.FNVhash = function(buff) {
    const bytes = new Uint8Array(buff);
    return bytes.reduce((hash, val) =>
        // appending >>> 0 to interpret result as Uint32
        (Math.imul((val ^ hash) >>> 0, 0x1000193) & 0xFFFFFFFF) >>> 0,
    0x811c9dc5);
};


// PioneerUnlock.ManufacturerUnlockSecretHiLo = {
//     PioneerDJ: PioneerUnlock.U32Math.Oxl`680131FB`,
//     Serato: PioneerUnlock.U32Math.Oxl`0D6F55AB`,
//     NativeInstruments: PioneerUnlock.U32Math.Oxl`8C5B3F5D`,
//     Atomix: PioneerUnlock.U32Math.Oxl`97779123`,
//     Algoriddim: PioneerUnlock.U32Math.Oxl`384B522B`,
//     Mixvibes: PioneerUnlock.U32Math.Oxl`9EA1A8B6`,
//     Numark: PioneerUnlock.U32Math.Oxl`889127C7`,
//     Dolby: PioneerUnlock.U32Math.Oxl`7A2B59AB`
// };
PioneerUnlock.ManufacturerUnlockSecretArray = {
    PioneerDJ: [0x68, 0x01, 0x31, 0xFB],
    Serato: [0x0D, 0x6F, 0x55, 0xAB],
    NativeInstruments: [0x8C, 0x5B, 0x3F, 0x5D],
    Atomix: [0x97, 0x77, 0x91, 0x23],
    Algoriddim: [0x38, 0x4B, 0x52, 0x2B],
    Mixvibes: [0x9E, 0xA1, 0xA8, 0xB6],
    Numark: [0x88, 0x91, 0x27, 0xC7],
    Dolby: [0x7A, 0x2B, 0x59, 0xAB]
};



PioneerUnlock.DeviceManufacturerDeviceID = {
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

PioneerUnlock.seedA = [0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF];

PioneerUnlock.keepAliveTimer = 0;

PioneerUnlock.incomingData = function(data, _length) {

    const manufacturer = "NativeInstruments";
    // TODO identify correct device properly
    const device = "DJM750MK2";

    console.trace(data);
    const view = new DataView(data);
    console.assert(view.getInt16(0) === 0x00f0);
    console.assert(view.getInt16(1) === 0x0001);
    const supertlv = PioneerUnlock.readTLV(data.slice(6));
    if (supertlv.type === 0x11) {
        console.assert(view.getInt16(2) === 0x0001);

        const firmwareVersionTlv = PioneerUnlock.readTLV(supertlv.value);
        console.assert(firmwareVersionTlv.type === 0x01);
        console.assert(firmwareVersionTlv.length === 4);
        const firmwareView = new DataView(firmwareVersionTlv.value);
        console.info(`Pioneer Firmware version: ${firmwareView.getInt8(0)}.${firmwareView.getInt8(1)}`);
        controller.sendOutputReport(0x00, [0xf0, 0x00, 0x01, 0x00, 0x01].concat(
            PioneerUnlock.makeTLV(0x12,
                // TODO idk if these are for authentication or just info, they might have to spoof
                // "licensed" DJ software for the handshake to succeed.
                PioneerUnlock.makeTLV(0x01, PioneerUnlock.asciiEncode("MixxxDJ")).concat(
                    PioneerUnlock.makeTLV(0x02, PioneerUnlock.asciiEncode("Mixxx")),
                    PioneerUnlock.makeTLV(0x03, PioneerUnlock.spreadBuff(PioneerUnlock.seedA))
                )
            )));
    } else if (supertlv.type === 0x13) {
        console.assert(view.getInt16(2) === 0x0002);

        const manufacturerTlv = PioneerUnlock.readTLV(supertlv.value);
        console.assert(manufacturerTlv.type === 0x01);
        console.info(`Manufacturer: ${PioneerUnlock.asciiDecode(manufacturerTlv.value)}`);

        const productTlv = PioneerUnlock.readTLV(manufacturerTlv.rest);
        console.assert(productTlv.type === 0x02);
        console.info(`Product: ${PioneerUnlock.asciiDecode(productTlv.value)}`);

        const hashATlv = PioneerUnlock.readTLV(productTlv.rest);
        console.assert(hashATlv.type === 0x04);
        console.assert(hashATlv.length === 0x0A);

        const seedETlv = PioneerUnlock.readTLV(hashATlv.rest);
        console.assert(seedETlv.type === 0x03);
        console.assert(seedETlv.length === 0x0D);

        const seedE = PioneerUnlock.contractBuff(seedETlv.value);

        const manufacturerSecret = PioneerUnlock.ManufacturerUnlockSecretArray[manufacturer];

        console.assert(seedE.length === manufacturerSecret.length);
        const secret = manufacturerSecret.map((val, i) => val ^ seedE[i]);


        const hashAd = PioneerUnlock.U32Math.FNVhash((new Uint8Array(PioneerUnlock.seedA.concat(secret))).buffer);
        const hashAView = new DataView(PioneerUnlock.contractBuff(hashATlv.value));
        console.assert(hashAView.getUint16(0) === hashAd.hi);
        console.assert(hashAView.getUint16(1) === hashAd.lo);

        const hashE = PioneerUnlock.U32Math.FNVhash((new Uint8Array(seedE.concat(secret))).buffer);

        controller.sendOutputReport(0x00, [0xf0, 0x00, 0x01, 0x00, 0x02].concat(
            PioneerUnlock.makeTLV(0x14,
                // I'm using the spoofed creds here.
                PioneerUnlock.makeTLV(0x01, PioneerUnlock.asciiEncode(manufacturer)).concat(
                    PioneerUnlock.makeTLV(0x02, PioneerUnlock.asciiEncode("TraktorDJ 2")),
                    PioneerUnlock.makeTLV(0x04, PioneerUnlock.spreadBuff(hashE)),
                    PioneerUnlock.makeTLV(0x05, PioneerUnlock.spreadBuff(device)),
                )
            )));
    } else if (supertlv.type === 0x15) {
        console.assert(view.getInt16(2) === 0x0002);
        console.assert(supertlv.length === 2);
        console.info("Hardware authenticated successfully");

        if (PioneerUnlock.keepAliveTimer === 0) {
            engine.stopTimer(PioneerUnlock.keepAliveTimer);
        }
        PioneerUnlock.keepAliveTimer = engine.beginTimer(400, () => {
            controller.sendOutputReport(0x00, Array(63).fill(0), true);
        });

    } else if (PioneerUnlock.buffIsEmpty(data)) {
        // reset packet received
        PioneerUnlock.handshakeState.lastStep = 0;
    }
    console.assert(PioneerUnlock.buffIsEmpty(supertlv.rest));
};
