/**
* MidiDispatcher routes incoming MIDI messages to registered callbacks. Register callbacks with
* the setInputCallback method in your controller script module's init function and call
* MidiDispatcher's receiveMidiData method in your controller script module's receiveData function.
*
* This class is not designed to handle System Exclusive or MIDI Clock messages. For those, implement logic
* specific to your controller in the controller module's receiveData function before calling
* MidiDispatcher.receiveData.
*/
export class MidiDispatcher {
    /**
     * @param {bool} noteOff - When setting the callback for a Note On message, also map the corresponding Note Off
     * message to the same callback. Defaults to true.
     */
    constructor(noteOff) {
        if (noteOff === undefined) {
            noteOff = true;
        }
        this.noteOff = noteOff;
        this.inputMap = new Map();
    }
    /**
     * Set the callback for an incoming MIDI message.
     * @param {Array} midiBytes - Array of numbers indicating the beginning of the MIDI messages. In most cases,
     * this should be the first two bytes of the MIDI messages, for example [0x93, 0x27]
     *
     * Program change (starting with 0xC) and aftertouch (starting with 0xD) messages are distinguished by only
     * their first byte, so in those cases the Array should only have one number.
     *
     * @param {function} callback - The callback that will be called by the receiveData method when a MIDI message
     * matching midiBytes is received from the controller.
     */
    setInputCallback(midiBytes, callback) {
        if (!Array.isArray(midiBytes)) {
            throw new Error('MidiDispatcher.setInputCallback midiBytes must be an Array, received ' + midiBytes);
        }
        if (typeof midiBytes[0] !== 'number') {
            throw new Error('MidiDispatcher.setInputCallback midiBytes must be an Array of numbers, received ' + midiBytes);
        }
        if (typeof callback !== 'function') {
            throw new Error('MidiDispatcher.setInputCallback callback must be a function, received ' + callback);
        }
        // JavaScript is broken and believes [1,2] === [1,2] is false, so
        // JSONify the Array to make it usable as a Map key.
        const key = JSON.stringify(midiBytes);
        this.inputMap.set(key, callback);
        // If passed a Note On message, also map the corresponding Note Off
        // message to the same callback.
        if (this.noteOff === true && ((midiBytes[0] & 0xF0) === 0x90)) {
            const noteOffBytes = [midiBytes[0] - 0x10, midiBytes[1]];
            const noteOffKey = JSON.stringify(noteOffBytes);
            this.inputMap.set(noteOffKey, callback);
        }
    }
    /**
     * Receive incoming MIDI data from a controller and execute the callback registered to that
     * MIDI message.
     * @param {Array} data - The incoming MIDI data.
     * @param {number} timestamp - The timestamp that Mixxx received the MIDI data at.
     */
    handleMidiInput(data, timestamp) {
        let key;
        // MIDI messages starting with 0xC (program change) or 0xD (aftertouch) messages are only
        // two bytes long and distinguished by their first byte.
        // https://www.midi.org/specifications-old/item/table-2-expanded-messages-list-status-bytes
        if ((data[0] & 0xF0) == 0xC0 || (data[0] & 0xF0) === 0xD0) {
            key = JSON.stringify([data[0]]);
        } else {
            key = JSON.stringify([data[0], data[1]]);
        }

        const callback = this.inputMap.get(key);
        if (typeof callback === 'function') {
            callback(data, timestamp);
        }
    }
}
