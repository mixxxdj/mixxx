/* @flow */
import type { LaunchpadDevice } from '../../'

import type { ChannelControl, ControlMessage } from '@mixxx-launchpad/mixxx'

import { modes } from '../ModifierSidebar'
import type { Modifier } from '../ModifierSidebar'

export default (gridPosition: [number, number]) => (deck: ChannelControl) => (modifier: Modifier) => (device: LaunchpadDevice) => {
  return {
    bindings: {
      button: {
        type: 'button',
        target: gridPosition,
        attack: () => {
          modes(modifier.getState(), () => deck.reloop_exit.setValue(1))
        }
      },
      control: {
        type: 'control',
        target: deck.loop_enabled,
        update: ({ value }: ControlMessage, { bindings }: Object) => {
          if (value) {
            bindings.button.button.sendColor(device.colors.hi_green)
          } else {
            bindings.button.button.sendColor(device.colors.lo_green)
          }
        }
      }
    }
  }
}
