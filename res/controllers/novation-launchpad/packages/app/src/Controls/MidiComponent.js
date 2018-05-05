/* @flow */
import Component from '../Component'

import type { LaunchpadDevice } from '../'
import type { MidiBus } from '../MidiBus'

export const childOfMidiComponent = (parent: MidiComponent) => new MidiComponent(parent.midibus)

export default class MidiComponent extends Component {
  midibus: MidiBus
  device: LaunchpadDevice

  constructor (midibus: MidiBus) {
    super()
    this.midibus = midibus
    this.device = midibus.device
  }

  onMount () {
    super.onMount()
  }

  onUnmount () {
    super.onUnmount()
  }
}
