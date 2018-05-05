/* @flow */

import play from '../controls/play'
import sync from '../controls/sync'
import nudge from '../controls/nudge'
import cue from '../controls/cue'
import tap from '../controls/tap'
import grid from '../controls/grid'
import pfl from '../controls/pfl'
import quantize from '../controls/quantize'
import keyshift from '../controls/keyshift'
import hotcue from '../controls/hotcue'
import load from '../controls/load'
import key from '../controls/key'
import beatjump from '../controls/beatjump'
import beatloop from '../controls/beatloop'
import { loopjump, loopjumpSmall } from '../controls/loopjump'
import loopMultiply from '../controls/loopMultiply'
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
  keyshift: keyshift([[1, 1], [2, 2], [3, 3], [5, 4], [7, 5], [8, 6], [10, 7], [12, 8]], 2)([2, 2]),
  load: load([0, 3]),
  key: key([1, 3]),
  hotcue: hotcue(8, 2)([0, 4]),
  beatjump: beatjump([[0.25, 1], [0.33, 2], [0.5, 4], [0.75, 8], [1, 16], [2, 32]])([2, 6]),
  beatloop: beatloop([0.5, 1, 2, 4, 8, 16, 32, 64], 2)([4, 2]),
  loopjump: loopjump([[0.5, 8], [1, 16], [2, 32], [4, 64]])([6, 2]),
  loopjumpSmall: loopjumpSmall(0.03125)([6, 1]),
  loopMultiply: loopMultiply([4, 1]),
  reloop: reloop([4, 0]),
  loopIo: loopIo([5, 0]),
  slip: slip([7, 0])
}
