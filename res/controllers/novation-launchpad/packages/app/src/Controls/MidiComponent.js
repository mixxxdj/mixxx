/* @flow */
import Component from '../Component'

import type { MidiBus, MidiMessage } from '../Launchpad/MidiBus'
import type { LaunchpadMidiButton } from '../Launchpad/Button'

export type MidiComponentBuilder = (button: LaunchpadMidiButton) => MidiComponent
export const makeMidiComponent = (midibus: MidiBus) => (button: LaunchpadMidiButton) => new MidiComponent(midibus, button)

export default class MidiComponent extends Component {
  midibus: MidiBus
  button: LaunchpadMidiButton
  _cb: (data: MidiMessage) => void

  constructor (midibus: MidiBus, button: LaunchpadMidiButton) {
    super()
    this.midibus = midibus
    this.button = button
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
    this.midibus.on(this.button.def.name, this._cb)
  }

  onUnmount () {
    this.midibus.removeListener(this.button.def.name, this._cb)
  }
}
