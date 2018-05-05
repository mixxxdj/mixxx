/* @flow */
import type { MidiMessage } from '../'
import type { MidiBus } from '../MidiBus'

import MidiComponent from '../Controls/MidiComponent'

import MidiButtonComponent from '../Controls/MidiButtonComponent'

export type ModifierState = {
  ctrl: boolean,
  shift: boolean
}

export interface Modifier {
  getState (): ModifierState
}

export default class ModifierSidebar extends MidiComponent implements Modifier {
  shift: MidiButtonComponent
  ctrl: MidiButtonComponent
  state: {| shift: boolean, ctrl: boolean |}
  listener: (MidiMessage) => void

  constructor (midibus: MidiBus) {
    super(midibus)
    this.shift = new MidiButtonComponent(this.midibus, this.device.buttons.solo)
    this.ctrl = new MidiButtonComponent(this.midibus, this.device.buttons.arm)

    this.state = {
      shift: false,
      ctrl: false
    }

    this.listener = ({ value, button, device }) => {
      if (value) {
        button.sendColor(device.colors.hi_red)
      } else {
        button.sendColor(device.colors.black)
      }
      if (button.def.name === this.device.buttons.solo.def.name) {
        this.state.shift = !!value
        this.emit('shift', value)
      } else {
        this.state.ctrl = !!value
        this.emit('ctrl', value)
      }
    }
  }

  onMount () {
    this.shift.mount()
    this.ctrl.mount()

    this.shift.on('midi', this.listener)
    this.ctrl.on('midi', this.listener)
  }

  onUnmount () {
    this.shift.removeListener('midi', this.listener)
    this.ctrl.removeListener('midi', this.listener)

    this.shift.unmount()
    this.ctrl.unmount()
  }

  getState () {
    return this.state
  }
}

export const modes = (ctx: ModifierState, n?: () => void, c?: () => void, s?: () => void, cs?: () => void) => {
  if (ctx.shift && ctx.ctrl) {
    cs && cs()
  } else if (ctx.shift) {
    s && s()
  } else if (ctx.ctrl) {
    c && c()
  } else {
    n && n()
  }
}

export const retainAttackMode = <Rest: $ReadOnlyArray<mixed>, R>(modifier: Modifier, cb: (ModifierState, MidiMessage, ...Rest) => R) => {
  let state = {
    shift: false,
    ctrl: false
  }

  return function (data: MidiMessage, ...rest: Rest) {
    if (data.value) {
      state = modifier.getState()
    }
    return cb(state, data, ...rest)
  }
}
