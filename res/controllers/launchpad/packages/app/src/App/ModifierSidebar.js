/* @flow */
import { Buttons, Colors } from '../Launchpad'

import type { MidiMessage } from '../Launchpad'

import type MidiComponent, { MidiComponentBuilder } from '../Controls/MidiComponent'

export type ModifierState = {
  ctrl: boolean,
  shift: boolean
}

export interface Modifier {
  getState (): ModifierState
}

const Component = require('../Component').default // TODO: I don't even ...

export default class ModifierSidebar extends Component implements Modifier {
  shift: MidiComponent
  ctrl: MidiComponent
  state: {| shift: boolean, ctrl: boolean |}
  listener: (MidiMessage) => void

  constructor (midiComponentBuilder: MidiComponentBuilder) {
    super()
    this.shift = midiComponentBuilder(Buttons.solo)
    this.ctrl = midiComponentBuilder(Buttons.arm)

    this.state = {
      shift: false,
      ctrl: false
    }

    this.listener = ({ value, button }) => {
      if (value) {
        button.sendColor(Colors.hi_red)
      } else {
        button.sendColor(Colors.black)
      }
      if (button.def.name === Buttons.solo.def.name) {
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

export const makeModifierSidebar =
  (midiComponentBuilder: MidiComponentBuilder) => new ModifierSidebar(midiComponentBuilder)

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
