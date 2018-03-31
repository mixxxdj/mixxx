/* @flow */

import { Colors } from '../../Launchpad'
import type { MidiMessage } from '../../Launchpad'

import { modes } from '../ModifierSidebar'
import type { Modifier } from '../ModifierSidebar'
import type { ChannelControl } from '../../Mixxx'

export default (gridPosition: [number, number]) => (deck: ChannelControl) => (modifier: Modifier) => {
  const onGrid = (dir) => ({ value }: MidiMessage, { bindings, state }: Object) => {
    if (!value) {
      bindings[dir].button.sendColor(Colors.black)
    } else {
      modes(modifier.getState(),
        () => {
          bindings[dir].button.sendColor(Colors.hi_yellow)
          state[dir].normal.setValue(1)
        },
        () => {
          bindings[dir].button.sendColor(Colors.hi_amber)
          state[dir].ctrl.setValue(1)
        })
    }
  }
  return {
    bindings: {
      back: {
        type: 'button',
        target: gridPosition,
        midi: onGrid('back')
      },
      forth: {
        type: 'button',
        target: [gridPosition[0] + 1, gridPosition[1]],
        midi: onGrid('forth')
      }
    },
    state: {
      back: {
        normal: deck.beats_translate_earlier,
        ctrl: deck.beats_adjust_slower
      },
      forth: {
        normal: deck.beats_translate_later,
        ctrl: deck.beats_adjust_faster
      }
    }
  }
}
