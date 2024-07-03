/**
 * Components JS library for Mixxx
 * Documentation is on the Mixxx wiki at
 * https://github.com/mixxxdj/mixxx/wiki/Components-JS
 *
 * Copyright (C) 2017 Be <be.0@gmx.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

(function(global) {
    /**
     * @typedef ComponentOpts
     * @property {number=} midiStatus
     * @property {number=} midiNo
     * @property {string=} group
     * @property {string=} inKey
     * @property {string=} outKey
     * @property {boolean} [outConnect=true]
     * @property {boolean} [outTrigger=true]
     * @property {number} [max=127]
     * @property {number} [shiftOffset=0]
     * @property {Component.SendShifted} [sendShifted=Component.SendShifted.NO]
     */
    class Component {
        /** @param {ComponentOpts} opts */
        constructor({
            midiStatus = undefined,
            midiNo = undefined,
            group = undefined,
            inKey = undefined,
            outKey = undefined,
            outConnect = true,
            outTrigger = true,
            max = 127,
            shiftOffset = 0,
            sendShifted = Component.SendShifted.NO
        } = {}) {
            /**
             * @type {ScriptConnection[]}
             * @protected
             */
            this._connections = [];
            this.isShifted = false;

            this.midiStatus = midiStatus;
            this.midiNo = midiNo;
            this.group = group;
            this.inKey = inKey;
            this.outKey = outKey;
            this.outConnect = outConnect;
            this.outTrigger = outTrigger;
            this.max = max;
            this.shiftOffset = shiftOffset;
            this.sendShifted = sendShifted;

            if (this.outConnect) {
                this.connect();
                this.outTrigger ? this.trigger() : this.noop();
            }
        }

        noop() { /* Does nothing */ }

        inValueScale(value) {
            // Hack to get exact center of pots to return 0.5
            return (value > (this.max / 2))
                ? (value - 1) / (this.max - 1)
                : value / (this.max + 1);
        }

        /**
         * Processes a MIDI event received received from the HID/MIDI device
         *
         * By default, sets the Mixxx control {@link value} specified by {@link Component#inKey} in the group
         * {@link Component#group}.
         *
         * @callback InputCallback
         * @param {number} channel The MIDI channel
         * @param {number} control The MIDI control (MIDI data 2)
         * @param {number} value The MIDI value (MIDI byte 3)
         * @param {number} status The MIDI statut (MIDI byte 1, ex: 0x9n)
         * @param {string} group The Mixxx control group (ex: "[Master]")
         * {@see https://manual.mixxx.org/2.3/en/chapters/appendix/mixxx_controls.html}
         */
        // eslint-disable-next-line no-unused-vars
        input(channel, control, value, status, group) {
            this.inSetParameter(this.inValueScale(value));
        }

        outValueScale(value) {
            return value * this.max;
        }

        /**
         * Processes a MIDI event specified by {@link Component#outKey} received from Mixxx in the group specified
         * by {@link Component#group}
         *
         * By default, forwards the value, transformed by {@link Component#outValueScale} to the HID/MIDI controller
         * using the MIDI status and MIDI group specified by {@link Component#midiStatus} and {@link Component#midiNo}
         * @param {number} value The MIDI value (MIDI byte 3)
         * @param {number} control The Mixxx control (Ex: play_indicator)
         * @param {string} group The Mixxx control group (ex: "[Channel1]")
         * {@see https://manual.mixxx.org/2.3/en/chapters/appendix/mixxx_controls.html}
         */
        // eslint-disable-next-line no-unused-vars
        output(value, group, control) {
            this.send(this.outValueScale(value));
        }

        // common functions
        // In most cases, you should not overwrite these.
        inGetParameter() {
            return engine.getParameter(this.group, this.inKey);
        }

        inSetParameter(value) {
            engine.setParameter(this.group, this.inKey, value);
        }

        inGetValue() {
            return engine.getValue(this.group, this.inKey);
        }

        inSetValue(value) {
            engine.setValue(this.group, this.inKey, value);
        }

        inToggle() {
            this.inSetValue(!this.inGetValue());
        }

        outGetParameter() {
            return engine.getParameter(this.group, this.outKey);
        }

        outSetParameter(value) {
            engine.setParameter(this.group, this.outKey, value);
        }

        outGetValue() {
            return engine.getValue(this.group, this.outKey);
        }

        outSetValue(value) {
            engine.setValue(this.group, this.outKey, value);
        }

        outToggle() {
            this.outSetValue(!this.outGetValue());
        }

        /**
         * Override this method with a custom one to connect multiple Mixxx COs for a single Component.
         * Add the connection objects to the this.connections array so they all get disconnected just
         * by calling {@link Component#disconnect}. This can be helpful for multicolor LEDs that show a
         * different color depending on the state of different Mixxx COs. Refer to
         * {@link SamplerButton#connect} and {@link SamplerButton#output} for an example.
         */
        connect() {
            if (this.group === undefined && this.outKey === undefined) {
                return;
            }

            const connection = engine.makeConnection(this.group, this.outKey, this.output);
            if (connection !== undefined) {
                this._connections.push(connection);
            }
        }

        disconnect() {
            this._connections.forEach(it => it.disconnect());
        }

        trigger() {
            this._connections.forEach(it => it.trigger());
        }

        send(value) {
            if (this.midiStatus === undefined || this.midiNo === undefined) {
                return;
            }

            midi.sendShortMsg(this.midiStatus, this.midiNo, value);

            if (this.sendShifted === Component.SendShifted.CHANNEL) {
                midi.sendShortMsg(this.midiStatus + this.shiftOffset, this.midiNo, value);
            } else if (this.sendShifted === Component.SendShifted.CONTROL) {
                midi.sendShortMsg(this.midiStatus, this.midiNo + this.shiftOffset, value);
            }
        }
    }

    /**
     * @readonly
     * @enum {number}
     */
    Component.SendShifted = Object.freeze({
        NO: 0,
        CHANNEL: 1,
        CONTROL: 2
    });

    global.components = Object.freeze({
        Component
    });
}(this));
