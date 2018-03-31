/* @flow */
import { makePlaylistSidebar } from './PlaylistSidebar'
import { makeModifierSidebar } from './ModifierSidebar'
import Layout, { makeLayout } from './Layout'
import Component from '../Component'

import type { TimerBuilder } from '../Mixxx/Timer'
import type { ControlComponentBuilder } from '../Controls/ControlComponent'
import type { MidiComponentBuilder } from '../Controls/MidiComponent'
import type PlaylistSidebar from './PlaylistSidebar'
import type ModifierSidebar from './ModifierSidebar'

export default class Screen extends Component {
  modifier: ModifierSidebar
  playListSidebar: PlaylistSidebar
  layout: Layout

  constructor (timerBuilder: TimerBuilder, controlComponentBuilder: ControlComponentBuilder, midiComponentBuilder: MidiComponentBuilder, id: string) {
    super()
    this.modifier = makeModifierSidebar(midiComponentBuilder)
    this.playListSidebar = makePlaylistSidebar(timerBuilder)(midiComponentBuilder)
    this.layout = makeLayout(controlComponentBuilder)(midiComponentBuilder)(this.modifier)(`${id}.layout`)
  }
  onMount () {
    this.modifier.mount()
    this.playListSidebar.mount()
    this.layout.mount()
  }
  onUnmount () {
    this.layout.unmount()
    this.playListSidebar.unmount()
    this.modifier.unmount()
  }
}

export const makeScreen = (timerBuilder: TimerBuilder) =>
  (controlComponentBuilder: ControlComponentBuilder) =>
    (midiComponentBuilder: MidiComponentBuilder) =>
      (id: string) => new Screen(timerBuilder, controlComponentBuilder, midiComponentBuilder, id)
