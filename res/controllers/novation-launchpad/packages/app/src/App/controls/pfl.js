/* @flow */
import type { LaunchpadDevice, MidiMessage } from '../../'
import type { ChannelControl, ControlMessage } from '@mixxx-launchpad/mixxx'

import { modes } from '../ModifierSidebar'
import type { Modifier } from '../ModifierSidebar'

export default (gridPosition: [number, number]) => (deck: ChannelControl) => (modifier: Modifier) => (device: LaunchpadDevice) => (device: LaunchpadDevice) => {
  return {
    bindings: {
      pfl: {
        type: 'control',
        target: deck.pfl,
        update: ({ value }: ControlMessage, { bindings }: Object) => value
          ? bindings.button.button.sendColor(device.colors.hi_green)
          : bindings.button.button.sendColor(device.colors.black)
      },
      button: {
        type: 'button',
        target: gridPosition,
        attack: (message: MidiMessage, { bindings }: Object) => modes(modifier.getState(),
          () => bindings.pfl.setValue(Number(!bindings.pfl.getValue())))
      }
    }
  }
}
