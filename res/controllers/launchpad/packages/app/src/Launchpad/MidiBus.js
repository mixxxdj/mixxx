/* @flow */
import EventEmitter from 'eventemitter3'

import { Buttons } from './Button'
import type { LaunchpadMidiButton } from './Button'

const callbackPrefix = '__midi'

export type MidiMessage = {
  value: number,
  button: LaunchpadMidiButton
}

export class MidiBus extends EventEmitter {
  registry: Object

  static create (registry: Object) {
    return new MidiBus(registry)
  }

  constructor (registry: Object) {
    super()
    this.registry = registry

    Object.keys(Buttons).forEach((buttonName) => {
      const button = Buttons[buttonName]
      const def = button.def
      this.registry[`${callbackPrefix}_${def.status}_${def.midino}`] = (channel, control, value, status) => {
        const message: MidiMessage = { value, button }
        this.emit(def.name, message)
      }
    })
  }
}
