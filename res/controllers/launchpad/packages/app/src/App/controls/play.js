/* @flow */
import { Colors } from '../../Launchpad'

import type { ControlMessage, ChannelControl } from '../../Mixxx'

import { modes } from '../ModifierSidebar'
import type { Modifier } from '../ModifierSidebar'

export default (gridPosition: [number, number]) => (deck: ChannelControl) => (modifier: Modifier) => {
  return {
    bindings: {
      playIndicator: {
        type: 'control',
        target: deck.play_indicator,
        update: ({ value }: ControlMessage, { bindings }: Object) => {
          if (value) {
            bindings.play.button.sendColor(Colors.hi_red)
          } else if (!value) {
            bindings.play.button.sendColor(Colors.black)
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
