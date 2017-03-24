/* @flow */
import flatMap from 'lodash.flatmap'
import assign from 'lodash.assign'
import isEqual from 'lodash.isequal'
import pick from 'lodash.pick'
import findIndex from 'lodash.findindex'

import Grande from './presets/Grande'
import Juggler from './presets/Juggler'
import Sampler from './presets/Sampler'
import Short from './presets/Short'
import Tall from './presets/Tall'

import Component from '../Component'
import type { MidiMessage } from '../Launchpad'
import { Buttons, Colors } from '../Launchpad'

import { retainAttackMode, modes } from './ModifierSidebar'
import type { Preset } from './Preset'
import type { Modifier } from './ModifierSidebar'
import type { ControlComponentBuilder } from '../Controls/ControlComponent'
import type MidiComponent, { MidiComponentBuilder } from '../Controls/MidiComponent'

type Size = 'short' | 'tall' | 'grande'
type Block = {|
  offset: [number, number],
  size: Size,
  channel: number,
  index: number
|}

type Diff = [Block[], Block[]]

const initialChannels = [0, 1]

const onMidi = (selectorBar, channel, modifier: Modifier) => retainAttackMode(modifier, (mode, { value }: MidiMessage) => {
  const selected = selectorBar.getChord()
  modes(mode,
    () => {
      if (!value && selected.length) {
        const diff = reorganize(selectorBar.getLayout(), selected)
        selectorBar.updateLayout(diff)
        selectorBar.removeChord()
      } else if (value) {
        selectorBar.addToChord(channel)
      }
    },
    () => {
      if (value) {
        if (selected.length) selectorBar.removeChord()
        const diff = cycle(channel, selectorBar.getLayout(), 1)
        selectorBar.updateLayout(diff)
      }
    },
    () => {
      if (value) {
        if (selected.length) selectorBar.removeChord()
        const diff = cycle(channel, selectorBar.getLayout(), -1)
        selectorBar.updateLayout(diff)
      }
    }
  )
})

class SelectorBar extends Component {
  id: string
  bindings: [MidiComponent, Function][]
  controlComponentBuilder: ControlComponentBuilder
  midiComponentBuilder: MidiComponentBuilder
  modifier: Modifier
  chord: number[]
  layout: { [key: string]: Block }
  mountedPresets: { [key: number]: Preset }

  static buttons = [ 'up', 'down', 'left', 'right', 'session', 'user1', 'user2', 'mixer' ]

  static channels = [0, 1, 2, 3, 4, 5, 6, 7]

  constructor (controlComponentBuilder: ControlComponentBuilder, midiComponentBuilder: MidiComponentBuilder, modifier: Modifier, id: string) {
    super()
    this.id = id
    this.bindings = SelectorBar.buttons
      .map((v, i) => {
        const binding = midiComponentBuilder(Buttons[v])
        return [binding, onMidi(this, i, modifier)]
      })
    this.controlComponentBuilder = controlComponentBuilder
    this.midiComponentBuilder = midiComponentBuilder
    this.modifier = modifier
    this.chord = []
    this.layout = { }
    this.mountedPresets = { }
  }

  getLayout () {
    const res = []
    for (const k in this.layout) {
      res.push(this.layout[k])
    }
    return res
  }

  updateLayout (diff: Diff) {
    const removedChannels = diff[0].map((block) => block.channel)
    removedChannels.forEach((ch) => {
      delete this.layout[String(ch)]
      this.bindings[ch][0].button.sendColor(Colors.black)
      this.mountedPresets[ch].unmount()
    })
    const addedBlocks = diff[1]
    addedBlocks.forEach((block) => {
      this.layout[String(block.channel)] = block
      if (block.index) {
        this.bindings[block.channel][0].button.sendColor(Colors.hi_orange)
      } else {
        this.bindings[block.channel][0].button.sendColor(Colors.hi_green)
      }
      this.mountedPresets[block.channel] = cycled[block.size][block.index](this.controlComponentBuilder)(this.midiComponentBuilder)(this.modifier)(`${this.id}.deck.${block.channel}`)(block.channel)(block.offset)
      this.mountedPresets[block.channel].mount()
    })
  }

  removeChord () {
    const layout = this.getLayout()
    this.chord.forEach((ch) => {
      const found = findIndex(layout, (b) => b.channel === ch)
      if (found === -1) {
        this.bindings[ch][0].button.sendColor(Colors.black)
      } else {
        const block = layout[found]
        if (block.index) {
          this.bindings[ch][0].button.sendColor(Colors.hi_orange)
        } else {
          this.bindings[ch][0].button.sendColor(Colors.hi_green)
        }
      }
      this.chord = []
    })
  }

  addToChord (channel: number) {
    if (this.chord.length === 4) {
      const rem = this.chord.shift()
      const found = findIndex(this.layout, (b) => b.channel === rem)
      if (found === -1) {
        this.bindings[rem][0].button.sendColor(Colors.black)
      } else {
        const layout = this.layout[found]
        if (layout.index) {
          this.bindings[rem][0].button.sendColor(Colors.hi_orange)
        } else {
          this.bindings[rem][0].button.sendColor(Colors.hi_green)
        }
      }
    }
    this.chord.push(channel)
    this.bindings[channel][0].button.sendColor(Colors.hi_red)
  }

  getChord () {
    return this.chord
  }

  onMount () {
    this.bindings.forEach(([binding, midi]) => {
      binding.mount()
      binding.on('midi', midi)
    })
  }

  onUnmount () {
    this.bindings.forEach(([binding, midi]) => {
      binding.removeListener('midi', midi)
      binding.unmount()
    })
  }
}

export default class Layout extends Component {
  selectorBar: SelectorBar

  constructor (controlComponentBuilder: ControlComponentBuilder, midiComponentBuilder: MidiComponentBuilder, modifier: Modifier, id: string) {
    super()
    this.selectorBar = new SelectorBar(controlComponentBuilder, midiComponentBuilder, modifier, `${id}.selectorBar`)
  }
  onMount () {
    this.selectorBar.mount()
    const diff = reorganize([], initialChannels)
    this.selectorBar.updateLayout(diff)
  }
  onUnmount () {
    const diff = reorganize(this.selectorBar.getLayout(), [])
    this.selectorBar.updateLayout(diff)
    this.selectorBar.unmount()
  }
}

export const makeLayout =
  (controlComponentBuilder: ControlComponentBuilder) =>
    (midiComponentBuilder: MidiComponentBuilder) =>
      (modifier: Modifier) =>
        (id: string) => {
          return new Layout(controlComponentBuilder, midiComponentBuilder, modifier, `${id}.selectorBar`)
        }

const offsets = [
  [0, 0],
  [4, 0],
  [0, 4],
  [4, 4]
]

const presets = {
  grande: [ Grande ],
  tall: [ Tall, Juggler ],
  short: [ Short, Sampler ]
}

const cycled = {
  'grande': flatMap(pick(presets, ['grande', 'tall', 'short']), (x) => x),
  'tall': flatMap(pick(presets, ['tall', 'short']), (x) => x),
  'short': flatMap(pick(presets, ['short']), (x) => x)
}

const reorganize = (current: Block[], selectedChannels: number[]): Diff => {
  const next = selectedChannels.length <= 1
    ? [{
      offset: offsets[0],
      size: 'grande',
      channel: selectedChannels[0],
      index: 0
    }]
    : selectedChannels.length <= 2
      ? [{
        offset: offsets[0],
        size: 'tall',
        channel: selectedChannels[0],
        index: 0
      }, {
        offset: offsets[1],
        size: 'tall',
        channel: selectedChannels[1],
        index: 0
      }]
      : selectedChannels.length <= 3
        ? [{
          offset: offsets[0],
          size: 'tall',
          channel: selectedChannels[0],
          index: 0
        }, {
          offset: offsets[1],
          size: 'short',
          channel: selectedChannels[1],
          index: 0
        }, {
          offset: offsets[3],
          size: 'short',
          channel: selectedChannels[2],
          index: 0
        }]
        : [{
          offset: offsets[0],
          size: 'short',
          channel: selectedChannels[0],
          index: 0
        }, {
          offset: offsets[1],
          size: 'short',
          channel: selectedChannels[1],
          index: 0
        }, {
          offset: offsets[2],
          size: 'short',
          channel: selectedChannels[2],
          index: 0
        }, {
          offset: offsets[3],
          size: 'short',
          channel: selectedChannels[3],
          index: 0
        }]
  return current.reduce((diff, block) => {
    const [neg, pos] = diff
    const matched = findIndex(pos, (b) => isEqual(block, b))
    return matched === -1
    ? [neg.concat([block]), pos]
    : [neg, pos.slice(0, matched).concat(pos.slice(matched + 1, pos.length))]
  }, [[], next])
}

const cycle = (channel: number, current: Block[], dir: 1 | -1): Diff => {
  const matched = findIndex(current, (block) => block.channel === channel)
  if (matched === -1) {
    return [[], []]
  }
  const nextIndex = (current[matched].index + dir) % cycled[current[matched].size].length
  if (nextIndex === current[matched].index) {
    return [[], []]
  }
  return [[current[matched]], [assign({}, current[matched], { index: nextIndex })]]
}
