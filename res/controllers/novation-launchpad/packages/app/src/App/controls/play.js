/* @flow */
import type { LaunchpadDevice } from '../../'

import type { ChannelControl, ControlMessage } from '@mixxx-launchpad/mixxx'

import { modes } from '../ModifierSidebar'
import type { Modifier } from '../ModifierSidebar'

export default (gridPosition: [number, number]) => (deck: ChannelControl) => (modifier: Modifier) => (device: LaunchpadDevice) => {
  return {
    bindings: {
      playIndicator: {
        type: 'control',
        target: deck.play_indicator,
        update: ({ value }: ControlMessage, { bindings }: Object) => {
          if (value) {
            bindings.play.button.sendColor(device.colors.hi_red)
          } else if (!value) {
            bindings.play.button.sendColor(device.colors.black)
          }
        }
      },
      play: {
        type: 'button',
        target: gridPosition,
        attack: () => {
          modes(modifier.getState(),
            () => deck.play.setValue(Number(!deck.play.getValue())),
            () => deck.start_play.setValue(1),
            () => deck.start_stop.setValue(1)
          )
        }
      }
    }
  }
}
