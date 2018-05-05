/* @flow */
import EventEmitter from 'eventemitter3'

import type { LaunchpadDevice, MidiMessage } from './'

const callbackPrefix = '__midi'

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
      this.registry[`${callbackPrefix}_${def.status}_${def.midino}`] = (channel, control, value, status) => {
        const message: MidiMessage = { value, button, device: this.device }
        this.emit(def.name, message)
      }
    })
  }
}
