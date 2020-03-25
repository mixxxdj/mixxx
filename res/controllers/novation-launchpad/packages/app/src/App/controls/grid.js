/* @flow */

import type { LaunchpadDevice, MidiMessage } from '../../'

import { modes } from '../ModifierSidebar'
import type { Modifier } from '../ModifierSidebar'
import type { ChannelControl } from '@mixxx-launchpad/mixxx'

export default (gridPosition: [number, number]) => (deck: ChannelControl) => (modifier: Modifier) => (device: LaunchpadDevice) => {
  const onGrid = (dir) => ({ value }: MidiMessage, { bindings, state }: Object) => {
    if (!value) {
      bindings[dir].button.sendColor(device.colors.black)
    } else {
      modes(modifier.getState(),
        () => {
          bindings[dir].button.sendColor(device.colors.hi_yellow)
          state[dir].normal.setValue(1)
        },
        () => {
          bindings[dir].button.sendColor(device.colors.hi_amber)
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
