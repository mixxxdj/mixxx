/* @flow */

import type { ChannelControl, ControlMessage } from '../../Mixxx'
import { Colors } from '../../Launchpad'
import type { MidiMessage } from '../../Launchpad'

import { modes } from '../ModifierSidebar'
import type { Modifier } from '../ModifierSidebar'

export default (gridPosition: [number, number]) => (deck: ChannelControl) => (modifier: Modifier) => {
  return {
    bindings: {
      button: {
        type: 'button',
        target: gridPosition,
        attack: (message: MidiMessage, { bindings }: Object) => {
          modes(modifier.getState(),
            () => {
              bindings.keylock.setValue(Number(!bindings.keylock.getValue()))
            },
            () => {
              deck.key.setValue(deck.key.getValue() - 1)
            },
            () => {
              deck.key.setValue(deck.key.getValue() + 1)
            },
            () => {
              deck.reset_key.setValue(1)
            }
          )
        }
      },
      keylock: {
        type: 'control',
        target: deck.keylock,
        update: ({ value }: ControlMessage, { bindings }: Object) => {
          if (value) {
            bindings.button.button.sendColor(Colors.hi_red)
          } else {
            bindings.button.button.sendColor(Colors.black)
          }
        }
      }
    }
  }
}
