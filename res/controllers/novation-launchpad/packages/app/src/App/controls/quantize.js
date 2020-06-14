/* @flow */
import type { LaunchpadDevice, MidiMessage } from '../../'

import type { ChannelControl, ControlMessage } from '@mixxx-launchpad/mixxx'

import { modes } from '../ModifierSidebar'
import type { Modifier } from '../ModifierSidebar'

export default (gridPosition: [number, number]) => (deck: ChannelControl) => (modifier: Modifier) => (device: LaunchpadDevice) => {
  return {
    bindings: {
      quantize: {
        type: 'control',
        target: deck.quantize,
        update: ({ value }: ControlMessage, { bindings }: Object) => value
          ? bindings.button.button.sendColor(device.colors.hi_orange)
          : bindings.button.button.sendColor(device.colors.black)
      },
      button: {
        type: 'button',
        target: gridPosition,
        attack: (message: MidiMessage, { bindings }: Object) => modes(modifier.getState(),
          () => bindings.quantize.setValue(Number(!bindings.quantize.getValue())))
      }
    }
  }
}
