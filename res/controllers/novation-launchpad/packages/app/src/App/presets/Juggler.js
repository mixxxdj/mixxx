/* @flow */
import play from '../controls/play'
import hotcue from '../controls/hotcue'
import load from '../controls/load'
import beatjump from '../controls/beatjump'
import beatloop from '../controls/beatloop'
import { loopjump } from '../controls/loopjump'
import loopMultiply from '../controls/loopMultiply'
import reloop from '../controls/reloop'

export default {
  play: play([0, 0]),
  load: load([1, 0]),
  beatjump: beatjump([[0.5, 4], [1, 16], [2, 32], [4, 64]], true)([2, 0]),
  loopjump: loopjump([[1, 16], [4, 64]])([0, 1]),
  reloop: reloop([0, 3]),
  loopMultiply: loopMultiply([0, 4]),
  hotcue: hotcue(8, 2)([2, 4]),
  beatloop: beatloop([0.5, 1, 2, 4, 8, 16], 2)([0, 5])
}
