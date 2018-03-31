/* @flow */
import { midi } from '../Mixxx/globals'

import { assign } from 'lodash-es'

import type { LaunchpadMidiButtonDef } from '@mixxx-launchpad/mk-specs/buttons'
import type { LaunchpadMidiButtonColor } from '@mixxx-launchpad/mk-specs/colors'

import buttons from '@mixxx-launchpad/mk-specs/buttons'

export { default as Colors } from '@mixxx-launchpad/mk-specs/colors'

export class LaunchpadMidiButton {
  def: LaunchpadMidiButtonDef

  constructor (def: LaunchpadMidiButtonDef) {
    this.def = def
  }

  sendColor (value: LaunchpadMidiButtonColor) {
    midi.sendShortMsg(this.def.status, this.def.midino, value)
  }
}

export const Buttons: { [key: string]: LaunchpadMidiButton } = Object.keys(buttons).reduce(
  (obj, name) => assign(obj, { [name]: new LaunchpadMidiButton(buttons[name]) }),
  {})
