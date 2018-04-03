/* @flow */

import type { ChannelControl, ControlMessage } from '../../Mixxx'
import { Colors } from '../../Launchpad'
import type { MidiMessage } from '../../Launchpad'

import { modes } from '../ModifierSidebar'
import type { Modifier } from '../ModifierSidebar'

export default (gridPosition: [number, number]) => (deck: ChannelControl) => (modifier: Modifier) => {
  const onStateChanged = (loaded, playing, bindings) => {
    if (loaded && playing) {
      bindings.button.button.sendColor(Colors.lo_red)
    } else if (loaded) {
      bindings.button.button.sendColor(Colors.lo_yellow)
    } else {
      bindings.button.button.sendColor(Colors.lo_green)
    }
  }
  return {
    bindings: {
      samples: {
        type: 'control',
        target: deck.track_samples,
        update: ({ value }: ControlMessage, { bindings }: Object) =>
          onStateChanged(value, bindings.play.getValue(), bindings)
      },
      play: {
        type: 'control',
        target: deck.play,
        update: ({ value }: ControlMessage, { bindings }: Object) =>
          onStateChanged(bindings.samples.getValue(), value, bindings)
      },
      button: {
        type: 'button',
        target: gridPosition,
        attack: (message: MidiMessage, { bindings }: Object) => {
          modes(modifier.getState(),
            () => {
              if (!bindings.samples.getValue()) {
                deck.LoadSelectedTrack.setValue(1)
              }
            },
            () => deck.LoadSelectedTrack.setValue(1),
            () => deck.eject.setValue(1)
          )
        }
      }
    }
  }
}
