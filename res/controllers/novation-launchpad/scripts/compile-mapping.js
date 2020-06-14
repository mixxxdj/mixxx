#!/usr/bin/env node

const { promisify } = require('util')
const ejs = require('ejs')
const path = require('path')
const readFile = promisify(require('fs').readFile)
const writeFile = promisify(require('fs').writeFile)
const mkdirp = promisify(require('mkdirp'))

if (process.argv.length !== 4) {
  throw Error('Usage: target outFile')
}

const tgt = process.argv[2]
const pkg = require(path.resolve('package.json'))
const tgtPkg = require(path.resolve('packages', tgt, 'package.json'))
const buttons = require(path.resolve('packages', tgt, tgtPkg.controller.path, 'buttons'))
const templateFile = path.join('packages', tgt, tgtPkg.controller.path, 'template.xml.ejs')

const leftPad = (str, padString, length) => {
  let buf = str
  while (buf.length < length) {
    buf = padString + buf
  }
  return buf
}

const hexFormat = (n, d) => '0x' + leftPad(n.toString(16).toUpperCase(), '0', d)

Promise.resolve().then(async () => {
  const template = await readFile(templateFile)
  const rendered = ejs.render(template.toString(), {
    author: pkg.author,
    description: pkg.description,
    homepage: pkg.homepage,
    device: tgtPkg.controller.device,
    manufacturer: tgtPkg.controller.manufacturer,
    global: tgtPkg.controller.global,
    buttons: Object.keys(buttons).map((key) => buttons[key]),
    hexFormat: hexFormat
  })
  await mkdirp(path.dirname(path.resolve(process.argv[3])))
  await writeFile(path.resolve(process.argv[3]), rendered)
}).catch((err) => { throw err })
