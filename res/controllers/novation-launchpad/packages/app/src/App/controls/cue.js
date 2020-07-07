/* @flow */

import type { ChannelControl, ControlMessage } from '@mixxx-launchpad/mixxx'

import { modes, retainAttackMode } from '../ModifierSidebar'
import type { Modifier } from '../ModifierSidebar'
import type { LaunchpadDevice } from '../../'

export default (gridPosition: [number, number]) => (deck: ChannelControl) => (modifier: Modifier) => (device: LaunchpadDevice) => {
  return {
    bindings: {
      cue: {
        type: 'button',
        target: gridPosition,
        midi: retainAttackMode(modifier, (mode, { value }) => {
          modes(mode,
            () => {
              if (value) {
                deck.cue_default.setValue(1)
              } else {
                deck.cue_default.setValue(0)
              }
            },
            () => value && deck.cue_set.setValue(1)
          )
        })
      },
      cueIndicator: {
        type: 'control',
        target: deck.cue_indicator,
        update: ({ value }: ControlMessage, { bindings }: Object) => {
          if (value) {
            bindings.cue.button.sendColor(device.colors.hi_red)
          } else if (!value) {
            bindings.cue.button.sendColor(device.colors.black)
          }
        }
      }
    }
  }
}
