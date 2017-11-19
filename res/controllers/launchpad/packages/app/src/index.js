/* @flow */
import './Mixxx/console-polyfill'

import { MidiBus } from './Launchpad'
import { ControlBus, makeTimer } from './Mixxx'
import type Screen from './App/Screen'
import { makeScreen } from './App/Screen'
import Component from './Component'
import { makeControlComponent } from './Controls/ControlComponent'
import { makeMidiComponent } from './Controls/MidiComponent'

class Global extends Component {
  screen: Screen
  constructor (globalName, globalObj = {}) {
    const timerBuilder = makeTimer(globalName, globalObj)
    const controlComponentBuilder = makeControlComponent(ControlBus.create(globalName, globalObj))
    const midiComponentBuilder = makeMidiComponent(MidiBus.create(globalObj))
    super()
    this.screen = makeScreen(timerBuilder)(controlComponentBuilder)(midiComponentBuilder)('main')
  }

  onMount () {
    this.screen.mount()
  }

  onUnmount () {
    this.screen.unmount()
  }
}

export function create (globalName: string, globalObj?: Object = {}) {
  const globalComponent = new Global(globalName, globalObj)
  globalObj.init = () => {
    globalComponent.mount()
  }
  globalObj.shutdown = () => { globalComponent.unmount() }
  return globalObj
}
