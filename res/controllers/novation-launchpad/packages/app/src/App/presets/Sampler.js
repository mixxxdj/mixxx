/* @flow */
import { makePresetFromPartialTemplate } from '../Preset'
import { channelControls } from '../../Mixxx/Control'

import hotcue from '../controls/hotcue'

import type { Modifier } from '../ModifierSidebar'
import type { ControlComponentBuilder } from '../../Controls/ControlComponent'
import type { MidiComponentBuilder } from '../../Controls/MidiComponent'

export default (controlComponentBuilder: ControlComponentBuilder) =>
  (midiComponentBuilder: MidiComponentBuilder) =>
    (modifier: Modifier) => (id: string) => (i: number) => (offset: [number, number]) => {
      const deck = channelControls[i]

      const partial = {
        hotcue: hotcue(16, 4)([0, 0])
      }
      return makePresetFromPartialTemplate(id, partial, offset)(deck)(controlComponentBuilder)(midiComponentBuilder)(modifier)
    }
