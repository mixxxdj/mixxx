/* @flow */
import type { LaunchpadDevice } from '../../'

import type { ChannelControl, ControlMessage } from '@mixxx-launchpad/mixxx'

import { modes, retainAttackMode } from '../ModifierSidebar'
import type { Modifier } from '../ModifierSidebar'

export default (gridPosition: [number, number]) => (deck: ChannelControl) => (modifier: Modifier) => (device: LaunchpadDevice) => {
  const onMidi = (modifier) => retainAttackMode(modifier, (mode, { value }, { bindings, state }) => {
    modes(mode,
      () => {
        if (value) {
          bindings.control.setValue(Number(!bindings.control.getValue()))
        } else {
          if (state.mode) {
            bindings.control.setValue(Number(!bindings.control.getValue()))
          }
        }
      },
      () => {
        if (value) {
          state.mode = !state.mode
          const color = state.mode ? 'orange' : 'red'
          bindings.button.button.sendColor(device.colors[`lo_${color}`])
        }
      }
    )
  })
  return {
    bindings: {
      control: {
        type: 'control',
        target: deck.slip_enabled,
        update: ({ value }: ControlMessage, { bindings, state }: Object) => {
          const color = state.mode ? 'orange' : 'red'
          if (value) {
            bindings.button.button.sendColor(device.colors[`hi_${color}`])
          } else {
            bindings.button.button.sendColor(device.colors[`lo_${color}`])
          }
        }
      },
      button: {
        type: 'button',
        target: gridPosition,
        midi: onMidi(modifier),
        mount: (dk: null, { bindings, state }: Object) => {
          const color = state.mode ? 'orange' : 'red'
          bindings.button.button.sendColor(device.colors[`lo_${color}`])
        }
      }
    },
    state: {
      mode: 1
    }
  }
}
