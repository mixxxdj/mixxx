/* @flow */
import assign from 'lodash-es/assign'
import findIndex from 'lodash-es/findIndex'

import { channelControls } from '@mixxx-launchpad/mixxx'

import Grande from './presets/Grande'
import Juggler from './presets/Juggler'
import Sampler from './presets/Sampler'
import Short from './presets/Short'
import Tall from './presets/Tall'
import MidiComponent from '../Controls/MidiComponent'
import { modes, retainAttackMode } from './ModifierSidebar'

import type { MidiMessage } from '../'
import type { Preset } from './Preset'
import type { Modifier } from './ModifierSidebar'
import type { ControlComponentBuilder } from '../Controls/ControlComponent'
import type { MidiBus } from '../MidiBus'
import MidiButtonComponent from '../Controls/MidiButtonComponent'
import { makePresetFromPartialTemplate } from './Preset'

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

class SelectorBar extends MidiComponent {
  id: string
  bindings: [MidiButtonComponent, Function][]
  controlComponentBuilder: ControlComponentBuilder
  modifier: Modifier
  chord: number[]
  layout: { [key: string]: Block }
  mountedPresets: { [key: number]: Preset }

  static buttons = ['up', 'down', 'left', 'right', 'session', 'user1', 'user2', 'mixer']

  static channels = [0, 1, 2, 3, 4, 5, 6, 7]

  constructor (midibus: MidiBus, controlComponentBuilder: ControlComponentBuilder, modifier: Modifier, id: string) {
    super(midibus)
    this.id = id
    this.bindings = SelectorBar.buttons
      .map((v, i) => {
        const binding = new MidiButtonComponent(this.midibus, this.device.buttons[v])
        return [binding, onMidi(this, i, modifier)]
      })
    this.controlComponentBuilder = controlComponentBuilder
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
      this.bindings[ch][0].button.sendColor(this.device.colors.black)
      this.mountedPresets[ch].unmount()
    })
    const addedBlocks = diff[1]
    addedBlocks.forEach((block) => {
      this.layout[String(block.channel)] = block
      if (block.index) {
        this.bindings[block.channel][0].button.sendColor(this.device.colors.hi_orange)
      } else {
        this.bindings[block.channel][0].button.sendColor(this.device.colors.hi_green)
      }
      this.mountedPresets[block.channel] =
        makePresetFromPartialTemplate(`${this.id}.deck.${block.channel}`, cycled[block.size][block.index], block.offset)(channelControls[block.channel])(this.controlComponentBuilder)(this.midibus)(this.modifier)
      this.mountedPresets[block.channel].mount()
    })
  }

  removeChord () {
    const layout = this.getLayout()
    this.chord.forEach((ch) => {
      const found = findIndex(layout, (b) => b.channel === ch)
      if (found === -1) {
        this.bindings[ch][0].button.sendColor(this.device.colors.black)
      } else {
        const block = layout[found]
        if (block.index) {
          this.bindings[ch][0].button.sendColor(this.device.colors.hi_orange)
        } else {
          this.bindings[ch][0].button.sendColor(this.device.colors.hi_green)
        }
      }
      this.chord = []
    })
  }

  addToChord (channel: number) {
    if (this.chord.length === 4) {
      const rem = this.chord.shift()
      const found = findIndex((this.layout: any), (b) => b.channel === rem) // FIXME: badly typed
      if (found === -1) {
        this.bindings[rem][0].button.sendColor(this.device.colors.black)
      } else {
        const layout = this.layout[String(found)]
        if (layout.index) {
          this.bindings[rem][0].button.sendColor(this.device.colors.hi_orange)
        } else {
          this.bindings[rem][0].button.sendColor(this.device.colors.hi_green)
        }
      }
    }
    this.chord.push(channel)
    this.bindings[channel][0].button.sendColor(this.device.colors.hi_red)
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

export default class Layout extends MidiComponent {
  selectorBar: SelectorBar

  constructor (midibus: MidiBus, controlComponentBuilder: ControlComponentBuilder, modifier: Modifier, id: string) {
    super(midibus)
    this.selectorBar = new SelectorBar(midibus, controlComponentBuilder, modifier, `${id}.selectorBar`)
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

const offsets = [
  [0, 0],
  [4, 0],
  [0, 4],
  [4, 4]
]

const presets = {
  grande: [Grande],
  tall: [Tall, Juggler],
  short: [Short, Sampler]
}

const cycled = {
  grande: [...presets.grande, ...presets.tall, ...presets.short],
  tall: [...presets.tall, ...presets.short],
  short: presets.short
}

const blockEquals = (a: Block, b: Block): boolean => {
  return a.offset === b.offset && a.size === b.size &&
    a.channel === b.channel && a.index === b.index
}

const reorganize = (current: Block[], selectedChannels: number[]): Diff => {
  const next = ((chs) => {
    switch (chs.length) {
      case 0: return []
      case 1: return [
        { offset: offsets[0], size: 'grande', channel: chs[0], index: 0 }
      ]
      case 2: return [
        { offset: offsets[0], size: 'tall', channel: chs[0], index: 0 },
        { offset: offsets[1], size: 'tall', channel: chs[1], index: 0 }
      ]
      case 3: return [
        { offset: offsets[0], size: 'tall', channel: chs[0], index: 0 },
        { offset: offsets[1], size: 'short', channel: chs[1], index: 0 },
        { offset: offsets[3], size: 'short', channel: chs[2], index: 0 }
      ]
      default: return [
        { offset: offsets[2], size: 'short', channel: chs[0], index: 0 },
        { offset: offsets[3], size: 'short', channel: chs[1], index: 0 },
        { offset: offsets[2], size: 'short', channel: chs[2], index: 0 },
        { offset: offsets[3], size: 'short', channel: chs[3], index: 0 }
      ]
    }
  })(selectedChannels)
  return current.reduce((diff, block) => {
    const [neg, pos] = diff
    const matched = findIndex(pos, (b) => blockEquals(block, b))
    return matched === -1
      ? [neg.concat([block]), pos]
      : [neg, pos.slice(0, matched).concat(pos.slice(matched + 1, pos.length))]
  }, [[], next])
}

const posMod = (x, n) => ((x % n) + n) % n

const cycle = (channel: number, current: Block[], dir: 1 | -1): Diff => {
  const matched = findIndex(current, (block) => block.channel === channel)
  if (matched === -1) {
    return [[], []]
  }
  const nextIndex = posMod((current[matched].index + dir), cycled[current[matched].size].length)
  if (nextIndex === current[matched].index) {
    return [[], []]
  }
  return [[current[matched]], [assign({}, current[matched], { index: nextIndex })]]
}
