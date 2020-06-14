/* @flow */
import play from '../controls/play'
import sync from '../controls/sync'
import nudge from '../controls/nudge'
import cue from '../controls/cue'
import tap from '../controls/tap'
import grid from '../controls/grid'
import pfl from '../controls/pfl'
import quantize from '../controls/quantize'
import load from '../controls/load'
import key from '../controls/key'
import reloop from '../controls/reloop'
import loopIo from '../controls/loopIo'
import slip from '../controls/slip'

export default {
  play: play([0, 0]),
  sync: sync([1, 0]),
  nudge: nudge([2, 0]),
  cue: cue([0, 1]),
  tap: tap([1, 1]),
  grid: grid([2, 1]),
  pfl: pfl([0, 2]),
  quantize: quantize([1, 2]),
  loopIo: loopIo([2, 2]),
  load: load([0, 3]),
  key: key([1, 3]),
  reloop: reloop([2, 3]),
  slip: slip([3, 3])
}
