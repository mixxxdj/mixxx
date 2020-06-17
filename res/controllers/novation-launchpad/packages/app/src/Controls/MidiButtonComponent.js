/* @flow */
import MidiComponent from './MidiComponent'

import type { LaunchpadMidiButton, MidiMessage } from '../'
import type { MidiBus } from '../MidiBus'

export default class MidiButtonComponent extends MidiComponent {
  button: LaunchpadMidiButton
  _cb: (data: MidiMessage) => void

  constructor (midibus: MidiBus, button: LaunchpadMidiButton) {
    super(midibus)
    this.midibus = midibus
    this.button = button
    this.device = midibus.device
    this._cb = (data) => {
      if (data.value) {
        this.emit('attack', data)
      } else {
        this.emit('release', data)
      }
      this.emit('midi', data)
    }
  }

  onMount () {
    super.onMount()
    this.midibus.on(this.button.def.name, this._cb)
  }

  onUnmount () {
    this.midibus.removeListener(this.button.def.name, this._cb)
    super.onUnmount()
  }
}

export const makeMidiButtonComponent = (midibus: MidiBus) => (button: LaunchpadMidiButton) => new MidiButtonComponent(midibus, button)
