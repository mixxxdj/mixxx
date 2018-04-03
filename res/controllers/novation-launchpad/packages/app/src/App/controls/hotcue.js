/* @flow */
import { range } from 'lodash-es'

import { Colors } from '../../Launchpad'

import { modes } from '../ModifierSidebar'
import type { Modifier } from '../ModifierSidebar'
import type { ChannelControl } from '../../Mixxx'

export default (n: number, d: number, s: number = 0) => (gridPosition: [number, number]) => (deck: ChannelControl) => (modifier: Modifier) => {
  const onHotcueMidi = (i) => ({ value }, { bindings }) => {
    modes(modifier.getState(),
      () => {
        if (value) {
          deck.hotcues[1 + i + s].activate.setValue(1)
        } else {
          deck.hotcues[1 + i + s].activate.setValue(0)
        }
      },
      () => {
        if (value) {
          if (bindings[`${i}.enabled`].getValue()) {
            deck.hotcues[1 + i + s].clear.setValue(1)
          } else {
            deck.hotcues[1 + i + s].set.setValue(1)
          }
        }
      })
  }
  const onHotcueEnabled = (i) => ({ value }, { bindings }) => {
    if (value) {
      bindings[`${i}.btn`].button.sendColor(Colors.lo_yellow)
    } else {
      bindings[`${i}.btn`].button.sendColor(Colors.black)
    }
  }
  const bindings = { }
  range(n).map((i) => {
    const dx = i % d
    const dy = ~~(i / d)
    bindings[`${i}.btn`] = {
      type: 'button',
      target: [gridPosition[0] + dx, gridPosition[1] + dy],
      midi: onHotcueMidi(i)
    }
    bindings[`${i}.enabled`] = {
      type: 'control',
      target: deck.hotcues[1 + i + s].enabled,
      update: onHotcueEnabled(i)
    }
  })
  return {
    bindings
  }
}
