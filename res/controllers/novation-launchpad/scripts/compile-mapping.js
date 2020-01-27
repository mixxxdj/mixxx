#!/usr/bin/env node

var prify = require('es6-promisify')
var ejs = require('ejs')
var path = require('path')
var readFile = prify(require('fs').readFile)
var writeFile = prify(require('fs').writeFile)
var mkdirp = prify(require('mkdirp'))

if (process.argv.length !== 4) {
  throw Error('Usage: target outFile')
}

var tgt = process.argv[2]
var pkg = require(path.resolve('package.json'))
var tgtPkg = require(path.resolve('packages', tgt, 'package.json'))
var buttons = require(path.resolve('packages', tgt, 'buttons'))
var templateFile = path.join('packages', tgt, 'template.xml.ejs')

readFile(templateFile)
  .then(function (template) {
    var rendered = ejs.render(template.toString(), {
      author: pkg.author,
      description: pkg.description,
      homepage: pkg.homepage,
      device: tgtPkg.controller.device,
      manufacturer: tgtPkg.controller.manufacturer,
      global: tgtPkg.controller.global,
      buttons: Object.keys(buttons).map(function (key) { return buttons[key] })
    })
    return mkdirp(path.dirname(path.resolve(process.argv[3])))
      .then(function () {
        return rendered
      })
  })
  .then(function (rendered) {
    return writeFile(path.resolve(process.argv[3]), rendered)
  })
  .catch(function (err) {
    throw err
  })
