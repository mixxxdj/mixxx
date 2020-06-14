/* @flow */
import PlaylistSidebar from './PlaylistSidebar'
import ModifierSidebar from './ModifierSidebar'
import Layout from './Layout'
import MidiComponent from '../Controls/MidiComponent'

import type { TimerBuilder } from '@mixxx-launchpad/mixxx'
import type { ControlComponentBuilder } from '../Controls/ControlComponent'
import type { MidiBus } from '../MidiBus'

export default class Screen extends MidiComponent {
  modifier: ModifierSidebar
  playListSidebar: PlaylistSidebar
  layout: Layout

  constructor (midibus: MidiBus, timerBuilder: TimerBuilder, controlComponentBuilder: ControlComponentBuilder, id: string) {
    super(midibus)
    this.modifier = new ModifierSidebar(midibus)
    this.playListSidebar = new PlaylistSidebar(midibus, timerBuilder)
    this.layout = new Layout(midibus, controlComponentBuilder, this.modifier, `${id}.layout`)
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
