/* @flow */
import type { LaunchpadDevice } from '../../'

import type { ChannelControl, ControlMessage } from '@mixxx-launchpad/mixxx'
import Bpm from '../../App/Bpm'

import { modes } from '../ModifierSidebar'
import type { Modifier } from '../ModifierSidebar'

export default (gridPosition: [number, number]) => (deck: ChannelControl) => (modifier: Modifier) => (device: LaunchpadDevice) => {
  const tempoBpm = new Bpm()
  tempoBpm.on('tap', (avg) => {
    deck.bpm.setValue(avg)
  })
  return {
    bindings: {
      tap: {
        type: 'button',
        target: gridPosition,
        attack: () => {
          modes(modifier.getState(),
            () => {
              tempoBpm.tap()
            },
            undefined,
            () => {
              deck.beats_translate_curpos.setValue(1)
            },
            () => {
              deck.beats_translate_match_alignment.setValue(1)
            }
          )
        }
      },
      beat: {
        type: 'control',
        target: deck.beat_active,
        update: ({ value }: ControlMessage, { bindings }: Object) => {
          if (value) {
            bindings.tap.button.sendColor(device.colors.hi_red)
          } else {
            bindings.tap.button.sendColor(device.colors.black)
          }
        }
      }
    }
  }
}
