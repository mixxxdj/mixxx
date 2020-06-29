/* @flow */

// This monstrous dynamic giant needs some serious refactor

import assign from 'lodash-es/assign'

import MidiComponent from '../Controls/MidiComponent'
import MidiButtonComponent from '../Controls/MidiButtonComponent'

import type { MidiBus } from '../MidiBus'
import type { Modifier } from './ModifierSidebar'
import type { ControlComponentBuilder } from '../Controls/ControlComponent'
import type { LaunchpadDevice } from '../'

import type { ChannelControl } from '@mixxx-launchpad/mixxx'

export type PresetType = {
  controlBindings: Object,
  controlListeners: Object,
  buttonBindings: Object,
  buttonListeners: Object
}

export type Template = Object

export type PartialTemplate = { [key: string]: (ChannelControl) => (Modifier) => (LaunchpadDevice) => Template }

export const makePresetFromPartialTemplate = (id: string, partialTemplate: PartialTemplate, offset: [number, number]) =>
  (deck: ChannelControl) => (controlComponentBuilder: ControlComponentBuilder) =>
    (midibus: MidiBus) =>
      (modifier: Modifier) => {
        const template = {}
        Object.keys(partialTemplate).forEach((k) => {
          assign(template, { [k]: partialTemplate[k](deck)(modifier)(midibus.device) })
        })
        return new Preset(midibus, controlComponentBuilder, modifier, id, template, offset)
      }

export class Preset extends MidiComponent {
  preset: PresetType

  constructor (midibus: MidiBus, controlComponentBuilder: ControlComponentBuilder, modifier: Modifier, id: string, template: Template, offset: [number, number]) {
    super(midibus)

    const controlBindings = {}
    const controlListeners = {}
    const buttonBindings = {}
    const buttonListeners = {}

    Object.keys(template).forEach((tk) => {
      if (template[tk] && template[tk].bindings) {
        const bindings = template[tk].bindings
        const instance = {
          state: template[tk].state,
          bindings: {}
        }
        Object.keys(bindings).forEach((bk) => {
          if (bindings[bk]) {
            const binding = bindings[bk]
            if (binding.type === 'control') {
              const name = `${binding.target.def.group}${binding.target.def.name}`
              if (!controlBindings[name]) {
                controlBindings[name] = controlComponentBuilder(`${id}.${tk}.${bk}`)(binding.target)
              }
              instance.bindings[bk] = controlBindings[name]
              controlListeners[name] = controlListeners[name] || { }
              ;['update', 'mount', 'unmount'].forEach((action) => {
                if (typeof binding[action] === 'function') {
                  appendListener(action, controlListeners[name], function (data) {
                    return binding[action](data, instance, modifier)
                  })
                }
              })
            } else if (binding.type === 'button') {
              const position = tr(binding.target, offset)
              const name = nameOf(position[0], position[1])
              if (!buttonBindings[name]) {
                buttonBindings[name] = new MidiButtonComponent(this.midibus, this.device.buttons[name])
              }
              instance.bindings[bk] = buttonBindings[name]
              buttonListeners[name] = buttonListeners[name] || { }
              ;['attack', 'release', 'midi', 'mount', 'unmount'].forEach((action) => {
                if (typeof binding[action] === 'function') {
                  appendListener(action, buttonListeners[name], function (data) {
                    return binding[action](data, instance)
                  })
                }
              })
              if (typeof binding['unmount'] !== 'function') {
                appendListener('unmount', buttonListeners[name], function (data) {
                  instance.bindings[bk].button.sendColor(this.device.colors.black)
                })
              }
            }
          }
        })
      }
    })
    this.preset = { controlBindings, controlListeners, buttonBindings, buttonListeners }
  }

  onMount () {
    const { controlBindings, buttonBindings, controlListeners, buttonListeners } = this.preset
    addListeners(controlBindings, controlListeners)
    addListeners(buttonBindings, buttonListeners)
    Object.keys(controlBindings).forEach((k) => controlBindings[k].mount())
    Object.keys(buttonBindings).forEach((k) => buttonBindings[k].mount())
  }

  onUnmount () {
    const { controlBindings, buttonBindings, controlListeners, buttonListeners } = this.preset
    Object.keys(controlBindings).forEach((k) => controlBindings[k].unmount())
    Object.keys(buttonBindings).forEach((k) => buttonBindings[k].unmount())
    removeListeners(controlBindings, controlListeners)
    removeListeners(buttonBindings, buttonListeners)
  }
}

const tr = (a, b) => [a[0] + b[0], a[1] + b[1]]

const nameOf = (x, y) => `${7 - y},${x}`

const appendListener = (type, bindings, binding) => {
  if (bindings[type] && Array.isArray(bindings[type])) {
    bindings[type].push(binding)
  } else if (bindings[type]) {
    const first = bindings[type]
    bindings[type] = [first, binding]
  } else {
    bindings[type] = binding
  }
}

const addListeners = (tgt, bindings) => {
  Object.keys(bindings).forEach((binding) => {
    if (tgt[binding]) {
      Object.keys(bindings[binding]).forEach((k) => {
        if (Array.isArray(bindings[binding][k])) {
          bindings[binding][k].forEach((f) => {
            tgt[binding].on(k, f)
          })
        } else {
          tgt[binding].on(k, bindings[binding][k])
        }
      })
    }
  })
}

const removeListeners = (tgt, bindings) => {
  Object.keys(bindings).forEach((binding) => {
    if (tgt[binding]) {
      Object.keys(bindings[binding]).forEach((k) => {
        if (Array.isArray(bindings[binding][k])) {
          bindings[binding][k].forEach((f) => {
            tgt[binding].removeListener(k, f)
          })
        } else {
          tgt[binding].removeListener(k, bindings[binding][k])
        }
      })
    }
  })
}
