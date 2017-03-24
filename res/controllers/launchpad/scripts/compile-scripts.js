#!/usr/bin/env node

var prify = require('es6-promisify')
var path = require('path')
var browserify = require('browserify')
var createWriteStream = require('fs').createWriteStream
var mkdirp = prify(require('mkdirp'))

if (process.argv.length !== 4) {
  throw Error('Usage: target outFile')
}
var tgt = process.argv[2]
var tgtPkg = require(path.resolve('packages', tgt, 'package.json'))
var entry = path.resolve('packages', tgt, tgtPkg.main)

var global = tgtPkg.controller.global

mkdirp(path.dirname(path.resolve(process.argv[3])))
  .then(function () {
    var output = createWriteStream(path.resolve(process.argv[3]))

    browserify(entry, {
      standalone: global,
      paths: [ path.resolve('packages', tgt, 'node_modules') ]
    })
      .transform('babelify', { global: true })
      .bundle().pipe(output)
  })
