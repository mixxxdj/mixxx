/* @flow */
import { Colors } from '../../Launchpad'
import type { MidiMessage } from '../../Launchpad'

import type { ControlMessage, ChannelControl } from '../../Mixxx'

import { modes } from '../ModifierSidebar'
import type { Modifier } from '../ModifierSidebar'

export default (gridPosition: [number, number]) => (deck: ChannelControl) => (modifier: Modifier) => {
  return {
    bindings: {
      quantize: {
        type: 'control',
        target: deck.quantize,
        update: ({ value }: ControlMessage, { bindings }: Object) => value
          ? bindings.button.button.sendColor(Colors.hi_orange)
          : bindings.button.button.sendColor(Colors.black)
      },
      button: {
        type: 'button',
        target: gridPosition,
        attack: (message: MidiMessage, { bindings }: Object) => modes(modifier.getState(),
          () => bindings.quantize.setValue(Number(!bindings.quantize.getValue())))
      }
    }
  }
}
