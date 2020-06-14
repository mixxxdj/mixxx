/* @flow */
import type { LaunchpadDevice } from '../../'
import flatMap from 'lodash-es/flatMap'

import { modes, retainAttackMode } from '../ModifierSidebar'
import type { Modifier } from '../ModifierSidebar'
import type { ChannelControl } from '@mixxx-launchpad/mixxx'

export const loopjump = (jumps: [number, number][]) => (gridPosition: [number, number]) => (deck: ChannelControl) => (modifier: Modifier) => (device: LaunchpadDevice) => {
  const bindings = { }
  const onMidi = (k, j, d) => (modifier) => retainAttackMode(modifier, (mode, { value }, { bindings, state }) => {
    modes(mode,
      () => {
        if (!state.mode) {
          if (value) {
            deck.loop_move.setValue(j[state.set] * d)
          }
        } else {
          if (value) {
            const currentJump = j[state.set] * d
            deck.loop_move.setValue(currentJump)
            if (state.pressing != null) {
              bindings[state.pressing].button.sendColor(device.colors[`lo_${state.color[state.set]}`])
            }
            bindings[k].button.sendColor(device.colors[`hi_${state.color[state.set]}`])
            state.pressing = k
            state.diff = state.diff + currentJump
          } else {
            if (state.pressing === k) {
              bindings[k].button.sendColor(device.colors[`lo_${state.color[state.set]}`])
              state.pressing = null
              deck.loop_move.setValue(-state.diff)
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
              bindings[b].button.sendColor(device.colors[`${prefix}_${state.color[state.set]}`])
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
              bindings[b].button.sendColor(device.colors[`${prefix}_${state.color[state.set]}`])
            }
          }
        }
      },
      () => {
        if (value) {
          state.mode = !state.mode
          const prefix = state.mode ? 'lo' : 'hi'
          for (let b = 0; b < spec.length; ++b) {
            bindings[b].button.sendColor(device.colors[`${prefix}_${state.color[state.set]}`])
          }
        }
      }
    )
  })
  const onMount = (k) => (_, { bindings, state }) => {
    const prefix = state.mode ? 'lo' : 'hi'
    bindings[k].button.sendColor(device.colors[`${prefix}_${state.color[state.set]}`])
  }
  const spec = flatMap((jumps: any), (j, i) => [[j, 1], [j, -1]]) // FIXME: flatMap is incorrectly typed see https://github.com/flow-typed/flow-typed/issues/2463

  spec.forEach(([jump, dir], i) => {
    bindings[i] = {
      type: 'button',
      target: [gridPosition[0] + i % 2, gridPosition[1] + ~~(i / 2)],
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

export const loopjumpSmall = (amount: number) => (button: [number, number]) => (deck: ChannelControl) => (modifier: Modifier) => (device: LaunchpadDevice) => {
  const onAttack = (dir) => () => {
    modes(modifier.getState(),
      () => deck.loop_move.setValue(dir * amount)
    )
  }
  return {
    bindings: {
      back: {
        type: 'button',
        target: button,
        attack: onAttack(-1),
        mount: (dk: null, { bindings }: Object) => {
          bindings.back.button.sendColor(device.colors.hi_yellow)
        }
      },
      forth: {
        type: 'button',
        target: [button[0] + 1, button[1]],
        attack: onAttack(1),
        mount: (dk: null, { bindings }: Object) => {
          bindings.forth.button.sendColor(device.colors.hi_yellow)
        }
      }
    }
  }
}
