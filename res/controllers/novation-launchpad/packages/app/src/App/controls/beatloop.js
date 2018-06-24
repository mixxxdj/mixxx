/* @flow */
import { Colors } from '../../Launchpad'

import { modes } from '../ModifierSidebar'
import type { Modifier } from '../ModifierSidebar'
import type { ChannelControl } from '../../Mixxx'

export default (loops: number[], d: number) => (gridPosition: [number, number]) => (deck: ChannelControl) => (modifier: Modifier) => {
  const bindings = { }
  const onAttack = (l) => (modifier) => () => {
    modes(modifier.getState(),
      () => deck.beatloops[l].toggle.setValue(1)
    )
  }

  const onUpdate = (i) => ({ value }, { bindings }) => {
    if (value) {
      bindings[i].button.sendColor(Colors.hi_red)
    } else {
      bindings[i].button.sendColor(Colors.lo_red)
    }
  }

  loops.forEach((loop, i) => {
    const dx = i % d
    const dy = ~~(i / d)
    bindings[i] = {
      type: 'button',
      target: [gridPosition[0] + dx, gridPosition[1] + dy],
      attack: onAttack(loop)(modifier)
    }
    bindings[`${loop}.enabled`] = {
      type: 'control',
      target: deck.beatloops[loop].enabled,
      update: onUpdate(i)
    }
  })
  return {
    bindings
  }
}
