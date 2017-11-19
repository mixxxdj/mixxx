/* @flow */

import type { ChannelControl, ControlMessage } from '../../Mixxx'
import { Colors } from '../../Launchpad'
import type { MidiMessage } from '../../Launchpad'

import { modes } from '../ModifierSidebar'
import type { Modifier } from '../ModifierSidebar'

export default (gridPosition: [number, number]) => (deck: ChannelControl) => (modifier: Modifier) => {
  return {
    bindings: {
      sync: {
        type: 'button',
        target: gridPosition,
        attack: (message: MidiMessage, { bindings }: Object) => {
          modes(modifier.getState(),
            () => {
              if (bindings.syncMode.getValue()) {
                deck.sync_enabled.setValue(0)
              } else {
                deck.sync_enabled.setValue(1)
              }
            },
            () => {
              if (bindings.syncMode.getValue() === 2) {
                deck.sync_master.setValue(0)
              } else {
                deck.sync_master.setValue(1)
              }
            }
          )
        }
      },
      syncMode: {
        type: 'control',
        target: deck.sync_mode,
        update: ({ value }: ControlMessage, { bindings }: Object) => {
          if (value === 0) {
            bindings.sync.button.sendColor(Colors.black)
          } else if (value === 1) {
            bindings.sync.button.sendColor(Colors.hi_orange)
          } else if (value === 2) {
            bindings.sync.button.sendColor(Colors.hi_red)
          }
        }
      }
    }
  }
}
