/* @flow */
import { flatMap } from 'lodash-es'

import { Colors } from '../../Launchpad'

import { modes, retainAttackMode } from '../ModifierSidebar'
import type { Modifier } from '../ModifierSidebar'
import type { ChannelControl } from '../../Mixxx'

export default (jumps: [number, number][], vertical?: boolean) => (gridPosition: [number, number]) => (deck: ChannelControl) => (modifier: Modifier) => {
  const bindings = { }
  const onMidi = (k, j, d) => (modifier) => retainAttackMode(modifier, (mode, { value }, { bindings, state }) => {
    modes(mode,
      () => {
        if (!state.mode) {
          if (value) {
            deck.beatjump.setValue(j[state.set] * d)
          }
        } else {
          if (value) {
            const currentJump = j[state.set] * d
            deck.beatjump.setValue(currentJump)
            if (state.pressing != null) {
              bindings[state.pressing].button.sendColor(Colors[`lo_${state.color[state.set]}`])
            }
            bindings[k].button.sendColor(Colors[`hi_${state.color[state.set]}`])
            state.pressing = k
            state.diff = state.diff + currentJump
          } else {
            if (state.pressing === k) {
              bindings[k].button.sendColor(Colors[`lo_${state.color[state.set]}`])
              state.pressing = null
              deck.beatjump.setValue(-state.diff)
              state.diff = 0
            }
          }
        }
      },
      () => {
        if (value) {
          if (state.set === 1) {
            state.set = 0
            const prefix = state.mode ? 'lo' : 'hi'
            for (let b = 0; b < spec.length; ++b) {
              bindings[b].button.sendColor(Colors[`${prefix}_${state.color[state.set]}`])
            }
          }
        }
      },
      () => {
        if (value) {
          if (state.set === 0) {
            state.set = 1
            const prefix = state.mode ? 'lo' : 'hi'
            for (let b = 0; b < spec.length; ++b) {
              bindings[b].button.sendColor(Colors[`${prefix}_${state.color[state.set]}`])
            }
          }
        }
      },
      () => {
        if (value) {
          state.mode = !state.mode
          const prefix = state.mode ? 'lo' : 'hi'
          for (let b = 0; b < spec.length; ++b) {
            bindings[b].button.sendColor(Colors[`${prefix}_${state.color[state.set]}`])
          }
        }
      }
    )
  })
  const onMount = (k) => (dontKnow, { bindings, state }) => {
    const prefix = state.mode ? 'lo' : 'hi'
    bindings[k].button.sendColor(Colors[`${prefix}_${state.color[state.set]}`])
  }
  const spec = flatMap(jumps, (j, i) => [[j, -1], [j, 1]])

  spec.forEach(([jump, dir], i) => {
    bindings[i] = {
      type: 'button',
      target: vertical ? [gridPosition[0] + i % 2, gridPosition[1] + ~~(i / 2)] : [gridPosition[0] + ~~(i / 2), gridPosition[1] + i % 2],
      midi: onMidi(i, jump, dir)(modifier),
      mount: onMount(i)
    }
  })
  return {
    bindings,
    state: {
      mode: false,
      pressing: 0,
      diff: 0,
      set: 0,
      color: [
        'green',
        'red'
      ]
    }
  }
}
