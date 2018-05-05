/* @flow */
import type { LaunchpadDevice } from '../../'
import type { Modifier } from '../ModifierSidebar'
import type { ChannelControl } from '@mixxx-launchpad/mixxx'

export default (gridPosition: [number, number]) => (deck: ChannelControl) => (_: Modifier) => (device: LaunchpadDevice) => {
  const onMount = (k) => (dk: null, { bindings }: Object) => {
    bindings[k].button.sendColor(device.colors.lo_yellow)
  }
  const onAttack = (k: 'double' | 'halve') => () => {
    // TODO: remove unsafe cast once flow supports https://github.com/facebook/flow/issues/3637
    deck[(`loop_${k}`: any)].setValue(1)
  }
  return {
    bindings: {
      halve: {
        type: 'button',
        target: gridPosition,
        mount: onMount('halve'),
        attack: onAttack('halve')
      },
      double: {
        type: 'button',
        target: [gridPosition[0] + 1, gridPosition[1]],
        mount: onMount('double'),
        attack: onAttack('double')
      }
    }
  }
}
