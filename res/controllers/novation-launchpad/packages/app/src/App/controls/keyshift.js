/* @flow */

import { Colors } from '../../Launchpad'

import { modes, retainAttackMode } from '../ModifierSidebar'
import type { Modifier } from '../ModifierSidebar'
import type { ChannelControl } from '../../Mixxx'

export default (shifts: [number, number][], d: number) => (gridPosition: [number, number]) => (deck: ChannelControl) => (modifier: Modifier) => {
  const bindings = { }

  const temporaryChange = (i, value, bindings, state) => {
    if (value) {
      const base = state.on === -1 ? deck.key.getValue() : state.base
      if (state.on !== -1) {
        bindings[state.on].button.sendColor(Colors[`lo_${state.color[state.set]}`])
      }
      bindings[i].button.sendColor(Colors[`hi_${state.color[state.set]}`])
      deck.key.setValue(((base + shifts[i][state.set]) % 12) + 12)
      state.on = i
      state.base = base
    } else {
      if (state.on === i) {
        bindings[i].button.sendColor(Colors[`lo_${state.color[state.set]}`])
        deck.key.setValue(state.base)
        state.on = -1
      }
    }
  }

  const onMidi = (i) => (modifier) => retainAttackMode(modifier, (mode, { value }, { bindings, state }) => {
    modes(mode,
      () => temporaryChange(i, value, bindings, state),
      () => {
        if (value) {
          if (state.set === 1) {
            state.set = 0
            for (let i = 0; i < shifts.length; ++i) {
              bindings[i].button.sendColor(Colors[`lo_${state.color[state.set]}`])
            }
          }
        }
      },
      () => {
        if (value) {
          if (state.set === 0) {
            state.set = 1
            for (let i = 0; i < shifts.length; ++i) {
              bindings[i].button.sendColor(Colors[`lo_${state.color[state.set]}`])
            }
          }
        }
      }
    )
  })

  shifts.forEach((s, i) => {
    const dx = i % d
    const dy = ~~(i / d)
    const position = [gridPosition[0] + dx, gridPosition[1] + dy]
    bindings[i] = {
      type: 'button',
      target: position,
      midi: onMidi(i)(modifier),
      mount: function (dontKnow, { bindings, state }) {
        bindings[i].button.sendColor(Colors[`lo_${state.color[state.set]}`])
      }
    }
  })
  return {
    bindings,
    state: {
      on: -1,
      base: null,
      set: 0,
      color: [
        'green',
        'red'
      ]
    }
  }
}
