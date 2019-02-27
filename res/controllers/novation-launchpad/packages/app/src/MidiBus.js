/* @flow */
import EventEmitter from 'eventemitter3'

import type { LaunchpadDevice, MidiMessage } from './'

const callbackPrefix = '__midi'

const leftPad = (str, padString, length) => {
  let buf = str
  while (buf.length < length) {
    buf = padString + buf
  }
  return buf
}

const hexFormat = (n, d) => '0x' + leftPad(n.toString(16).toUpperCase(), '0', d)

export class MidiBus extends EventEmitter {
  registry: Object
  device: LaunchpadDevice

  static create (registry: Object, device: LaunchpadDevice) {
    return new MidiBus(registry, device)
  }

  constructor (registry: Object, device: LaunchpadDevice) {
    super()
    this.registry = registry
    this.device = device

    Object.keys(device.buttons).forEach((buttonName) => {
      const button = device.buttons[buttonName]
      const def = button.def
      this.registry[`${callbackPrefix}_${hexFormat(def.status, 2)}_${hexFormat(def.midino, 2)}`] = (channel, control, value, status) => {
        const message: MidiMessage = { value, button, device: this.device }
        this.emit(def.name, message)
      }
    })
  }
}
