/* @flow */

import Component from '../Component'
import { Control } from '@mixxx-launchpad/mixxx'

import type { ControlBus } from '@mixxx-launchpad/mixxx'

export const makeControlComponent = (controlBus: ControlBus) =>
  (id: string) =>
    (control: Control) =>
      new ControlComponent(controlBus, id, control)

export default class ControlComponent extends Component {
  value: ?number
  id: string
  controlBus: ControlBus
  control: Control
  _handle: ?any

  constructor (controlBus: ControlBus, id: string, control: Control) {
    super()
    this.value = null
    this.id = id
    this.controlBus = controlBus
    this.control = control
    this._handle = null
  }

  onMount () {
    if (!this._handle) {
      this._handle = this.controlBus.connect(this.id, this.control.def, (data) => {
        this.value = data.value
        this.emit('update', data)
      })
      this.value = this.control.getValue()
      this.emit('update', this)
    }
  }

  onUnmount () {
    if (this._handle) {
      this.controlBus.disconnect(this._handle)
      this._handle = null
    }
  }

  setValue (value: number) {
    this.control.setValue(value)
    this.value = this.control.getValue()
  }

  toggleValue () {
    this.setValue(Number(!this.getValue()))
  }

  getValue () {
    if (!this._handle) {
      this.value = this.control.getValue()
    }
    return this.value
  }
}

export type ControlComponentBuilder = (string) => (Control) => ControlComponent
