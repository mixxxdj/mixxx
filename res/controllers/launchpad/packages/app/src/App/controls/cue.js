/* @flow */

import type { ControlMessage, ChannelControl } from '../../Mixxx'
import { Colors } from '../../Launchpad'

import { modes, retainAttackMode } from '../ModifierSidebar'
import type { Modifier } from '../ModifierSidebar'

export default (gridPosition: [number, number]) => (deck: ChannelControl) => (modifier: Modifier) => {
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
            () => value && deck.cue_set.setValue(1),
          )
        })
      },
      cueIndicator: {
        type: 'control',
        target: deck.cue_indicator,
        update: ({ value }: ControlMessage, { bindings }: Object) => {
          if (value) {
            bindings.cue.button.sendColor(Colors.hi_red)
          } else if (!value) {
            bindings.cue.button.sendColor(Colors.black)
          }
        }
      }
    }
  }
}
