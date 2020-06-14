/* @flow */

import type { LaunchpadDevice, MidiMessage } from '../../'

import { modes, retainAttackMode } from '../ModifierSidebar'
import type { Modifier } from '../ModifierSidebar'
import type { ChannelControl, ControlMessage } from '@mixxx-launchpad/mixxx'

export default (gridPosition: [number, number]) => (deck: ChannelControl) => (modifier: Modifier) => (device: LaunchpadDevice) => {
  const rateEpsilon = 1e-3

  const getDirection = (rate) => {
    if (rate < -rateEpsilon) {
      return 'up'
    } else if (rate > rateEpsilon) {
      return 'down'
    } else {
      return ''
    }
  }

  const onNudgeMidi = (dir: 'up' | 'down') => (modifier) => retainAttackMode(modifier, (mode, { value }: MidiMessage, { bindings, state }: Object) => {
    if (value) {
      state[dir] = true
      if (state.down && state.up) {
        deck.rate.setValue(0)
      } else {
        modes(mode,
          () => {
            bindings[dir].button.sendColor(device.colors.hi_yellow)
            // TODO: remove unsafe cast once flow supports https://github.com/facebook/flow/issues/3637
            deck[(`rate_temp_${dir}`: any)].setValue(1)
          },
          () => {
            bindings[dir].button.sendColor(device.colors.hi_red)
            // TODO: remove unsafe cast once flow supports https://github.com/facebook/flow/issues/3637
            deck[(`rate_perm_${dir}`: any)].setValue(1)
          },
          () => {
            bindings[dir].button.sendColor(device.colors.lo_yellow)
            // TODO: remove unsafe cast once flow supports https://github.com/facebook/flow/issues/3637
            deck[(`rate_temp_${dir}_small`: any)].setValue(1)
          },
          () => {
            bindings[dir].button.sendColor(device.colors.lo_red)
            // TODO: remove unsafe cast once flow supports https://github.com/facebook/flow/issues/3637
            deck[(`rate_perm_${dir}_small`: any)].setValue(1)
          }
        )
      }
    } else {
      state[dir] = false
      if (getDirection(bindings.rate.getValue()) === dir) {
        bindings[dir].button.sendColor(device.colors.lo_orange)
      } else {
        bindings[dir].button.sendColor(device.colors.black)
      }
      modes(mode,
        // TODO: remove unsafe cast once flow supports https://github.com/facebook/flow/issues/3637
        () => deck[(`rate_temp_${dir}`: any)].setValue(0),
        undefined,
        // TODO: remove unsafe cast once flow supports https://github.com/facebook/flow/issues/3637
        () => deck[(`rate_temp_${dir}_small`: any)].setValue(0)
      )
    }
  })

  const onRate = ({ value }: ControlMessage, { state, bindings }: Object) => {
    let up = device.colors.black
    let down = device.colors.black
    let rate = getDirection(value)
    if (rate === 'down') {
      down = device.colors.lo_orange
    } else if (rate === 'up') {
      up = device.colors.lo_orange
    }

    if (!state.down) {
      bindings.down.button.sendColor(down)
    }

    if (!state.up) {
      bindings.up.button.sendColor(up)
    }
  }

  return {
    bindings: {
      down: {
        type: 'button',
        target: gridPosition,
        midi: onNudgeMidi('down')(modifier)
      },
      up: {
        type: 'button',
        target: [gridPosition[0] + 1, gridPosition[1]],
        midi: onNudgeMidi('up')(modifier)
      },
      rate: {
        type: 'control',
        target: deck.rate,
        update: onRate
      }
    },
    state: {
      up: false,
      down: false
    }
  }
}
