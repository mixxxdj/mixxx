/* @flow */
import { makePresetFromPartialTemplate } from '../Preset'
import { channelControls } from '../../Mixxx/Control'

import play from '../controls/play'
import sync from '../controls/sync'
import nudge from '../controls/nudge'
import cue from '../controls/cue'
import tap from '../controls/tap'
import grid from '../controls/grid'
import pfl from '../controls/pfl'
import quantize from '../controls/quantize'
import hotcue from '../controls/hotcue'
import load from '../controls/load'
import key from '../controls/key'
import beatjump from '../controls/beatjump'
import beatloop from '../controls/beatloop'
import loopMultiply from '../controls/loopMultiply'
import reloop from '../controls/reloop'
import loopIo from '../controls/loopIo'
import slip from '../controls/slip'

import type { Modifier } from '../ModifierSidebar'
import type { ControlComponentBuilder } from '../../Controls/ControlComponent'
import type { MidiComponentBuilder } from '../../Controls/MidiComponent'

export default (controlComponentBuilder: ControlComponentBuilder) =>
  (midiComponentBuilder: MidiComponentBuilder) =>
    (modifier: Modifier) => (id: string) => (i: number) => (offset: [number, number]) => {
      const deck = channelControls[i]

      const partial = {
        play: play([0, 0]),
        sync: sync([1, 0]),
        nudge: nudge([2, 0]),
        cue: cue([0, 1]),
        tap: tap([1, 1]),
        grid: grid([2, 1]),
        pfl: pfl([0, 2]),
        quantize: quantize([1, 2]),
        loopIo: loopIo([2, 3]),
        load: load([0, 3]),
        key: key([1, 3]),
        reloop: reloop([2, 3]),
        slip: slip([2, 4]),
        hotcue: hotcue(4, 2)([0, 4]),
        loopMultiply: loopMultiply([2, 4]),
        beatloop: beatloop([0.5, 1, 2, 4, 8, 16], 2)([2, 5]),
        beatjump: beatjump([[1, 16], [2, 32]])([0, 6])
      }
      const preset = makePresetFromPartialTemplate(id, partial, offset)(deck)(controlComponentBuilder)(midiComponentBuilder)(modifier)
      return preset
    }
