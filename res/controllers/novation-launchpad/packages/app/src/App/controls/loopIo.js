/* @flow */
import { Colors } from '../../Launchpad'
import type { MidiMessage } from '../../Launchpad'

import { modes } from '../ModifierSidebar'
import type { Modifier } from '../ModifierSidebar'
import type { ChannelControl } from '../../Mixxx'

export default (gridPosition: [number, number]) => (deck: ChannelControl) => (modifier: Modifier) => {
  const onMidi = (dir: 'in' | 'out') => ({ value }: MidiMessage, { bindings }: Object) => {
    modes(modifier.getState(), () => {
      if (value) {
        // TODO: remove unsafe cast once flow supports https://github.com/facebook/flow/issues/3637
        deck[(`loop_${dir}`: any)].setValue(1)
        bindings[dir].button.sendColor(Colors.hi_green)
      } else {
        // TODO: remove unsafe cast once flow supports https://github.com/facebook/flow/issues/3637
        deck[(`loop_${dir}`: any)].setValue(0)
        bindings[dir].button.sendColor(Colors.black)
      }
    })
  }
  return {
    bindings: {
      in: {
        type: 'button',
        target: gridPosition,
        midi: onMidi('in')
      },
      out: {
        type: 'button',
        target: [gridPosition[0] + 1, gridPosition[1]],
        midi: onMidi('out')
      }
    }
  }
}
