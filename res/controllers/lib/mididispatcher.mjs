export class MidiDispatcher {
    constructor(noteOff) {
        this.noteOff = noteOff;
        this.inputMap = new Map();
    }
    registerInputCallback(midiBytes, callback) {
        // JavaScript is broken and believes [1,2] === [1,2] is false, so
        // JSONify the Array to make it usable as a Map key.
        const key = JSON.stringify(midiBytes);
        this.inputMap.set(key, callback);
        if (this.noteOff === true && ((midiBytes[0] & 0xF0) === 0x90)) {
            const noteOffBytes = [midiBytes[0] - 0x10, midiBytes[1]];
            const noteOffKey = JSON.stringify(noteOffBytes);
            this.inputMap.set(noteOffKey, callback);
        }
    }
    receiveData(data, timestamp) {
        const key = JSON.stringify([data[0], data[1]]);
        const callback = this.inputMap.get(key);
        if (typeof callback === 'function') {
            callback(data, timestamp);
        }
    }
}
