/* @flow */

import EventEmitter from 'eventemitter3'

export default class Component extends EventEmitter {
  mount () {
    this.onMount()
    this.emit(`mount`, this)
  }

  unmount () {
    this.onUnmount()
    this.emit(`unmount`, this)
  }

  onMount () { }

  onUnmount () { }
}
