/* @flow */
import { playListControl } from '@mixxx-launchpad/mixxx'
import MidiComponent from '../Controls/MidiComponent'
import MidiButtonComponent from '../Controls/MidiButtonComponent'

import type { MidiBus } from '../MidiBus'
import type { TimerBuilder } from '@mixxx-launchpad/mixxx'

const autoscrolled = (binding) => (timerBuilder: TimerBuilder) => {
  let started
  let minInterval = 32
  let interval
  let timer

  binding.on('midi', (data) => {
    if (data.value) {
      interval = 250
      started = timer.start(interval)
    } else {
      timer.end()
    }
  })

  binding.on('mount', () => {
    timer = timerBuilder(() => {
      binding.emit('scroll')
      if (interval > minInterval) {
        const current = Date.now()
        // silence Flow with unsafe casts
        if (interval === 250 && current - (started: any) > 1500) {
          interval = 125
          timer.restart(interval)
        } else if (interval === 125 && current - (started: any) > 3000) {
          interval = 63
          timer.restart(interval)
        } else if (interval === 63 && current - (started: any) > 6000) {
          interval = minInterval
          timer.restart(interval)
        }
      }
    })
  })

  binding.on('unmount', () => {
    timer.end()
  })

  return binding
}

const onScroll = (control) => () => { control.setValue(1) }

const onMidi = (control) => ({ value, button, device }) => {
  if (value) {
    control.setValue(1)
    button.sendColor(device.colors.hi_red)
  } else {
    button.sendColor(device.colors.hi_yellow)
  }
}

const onMount = ({ button, device }) => {
  button.sendColor(device.colors.hi_yellow)
}

const onUnmount = ({ button, device }) => {
  button.sendColor(device.colors.black)
}

export default class PlaylistSidebar extends MidiComponent {
  buttons: MidiButtonComponent[]

  constructor (midibus: MidiBus, timerBuilder: TimerBuilder) {
    super(midibus)

    const btns = [
      new MidiButtonComponent(midibus, this.device.buttons.vol),
      new MidiButtonComponent(midibus, this.device.buttons.pan),
      new MidiButtonComponent(midibus, this.device.buttons.snda),
      new MidiButtonComponent(midibus, this.device.buttons.sndb),
      new MidiButtonComponent(midibus, this.device.buttons.stop)
    ]

    const prevPlaylist = autoscrolled(btns[0])(timerBuilder)
    const nextPlaylist = autoscrolled(btns[1])(timerBuilder)
    const toggleItem = btns[2]
    const prevTrack = autoscrolled(btns[3])(timerBuilder)
    const nextTrack = autoscrolled(btns[4])(timerBuilder)

    prevPlaylist.on('scroll', onScroll(playListControl.SelectPrevPlaylist))
    prevPlaylist.on('midi', onMidi(playListControl.SelectPrevPlaylist))
    prevPlaylist.on('mount', onMount)
    prevPlaylist.on('unmount', onUnmount)

    nextPlaylist.on('scroll', onScroll(playListControl.SelectNextPlaylist))
    nextPlaylist.on('midi', onMidi(playListControl.SelectNextPlaylist))
    nextPlaylist.on('mount', onMount)
    nextPlaylist.on('unmount', onUnmount)

    prevTrack.on('scroll', onScroll(playListControl.SelectPrevTrack))
    prevTrack.on('midi', onMidi(playListControl.SelectPrevTrack))
    prevTrack.on('mount', onMount)
    prevTrack.on('unmount', onUnmount)

    nextTrack.on('scroll', onScroll(playListControl.SelectNextTrack))
    nextTrack.on('midi', onMidi(playListControl.SelectNextTrack))
    nextTrack.on('mount', onMount)
    nextTrack.on('unmount', onUnmount)

    toggleItem.on('midi', onMidi(playListControl.ToggleSelectedSidebarItem))
    toggleItem.on('mount', onMount)
    toggleItem.on('unmount', onUnmount)

    this.buttons = btns
  }
  onMount () {
    this.buttons.forEach((button) => button.mount())
  }
  onUnmount () {
    this.buttons.forEach((button) => button.unmount())
  }
}
