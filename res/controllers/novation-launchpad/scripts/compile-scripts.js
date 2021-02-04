#!/usr/bin/env node

var prify = require('es6-promisify')
var path = require('path')
var mkdirp = prify(require('mkdirp'))

var rollup = require('rollup')
var nodeResolve = require('rollup-plugin-node-resolve')
var babel = require('rollup-plugin-babel')
var commonjs = require('rollup-plugin-commonjs')
var json = require('rollup-plugin-json')

if (process.argv.length !== 4) {
  throw Error('Usage: target outFile')
}
var tgt = process.argv[2]
var tgtPkg = require(path.resolve('packages', tgt, 'package.json'))
var entry = path.resolve('packages', tgt, tgtPkg.main)

var global = tgtPkg.controller.global

mkdirp(path.dirname(path.resolve(process.argv[3])))
  .then(function () {
    return rollup.rollup({
      entry,
      plugins: [
        nodeResolve({
          extensions: ['.js', '.json'],
          main: true,
          modulesOnly: false, // required for rollup-plugin-commonjs
          // for valid values see https://github.com/substack/node-resolve
          customResolveOptions: {
            paths: [ path.resolve('packages', tgt, 'node_modules') ]
          }
        }),
        json(),
        babel({
          exclude: 'node_modules/**'
        }),
        commonjs()
      ]
    })
  })
  .then(function (bundle) {
    return bundle.write({
      format: 'iife',
      moduleName: global,
      dest: path.resolve(process.argv[3])
    })
  })
  .catch(function (err) {
    console.error(err)
    process.exit(1)
  })
