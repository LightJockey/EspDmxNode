'use strict'
require('./check-versions')()

process.env.NODE_ENV = 'production'

const ora = require('ora')
const rm = require('rimraf')
const path = require('path')
const chalk = require('chalk')
const webpack = require('webpack')
const config = require('../config')
const webpackConfig = require('./webpack.prod.conf')

const spinner = ora('building for production...')
spinner.start()

rm(path.join(config.build.assetsRoot, config.build.assetsSubDirectory), err => {
  if (err) throw err
  webpack(webpackConfig, (err, stats) => {
    spinner.stop()
    if (err) throw err
    process.stdout.write(stats.toString({
      colors: true,
      modules: false,
      children: false, // If you are using ts-loader, setting this to true will make TypeScript errors show up during build.
      chunks: false,
      chunkModules: false
    }) + '\n\n')

    if (stats.hasErrors()) {
      console.log(chalk.red('  Build failed with errors.\n'))
      process.exit(1)
    }

    console.log(chalk.cyan('  Build complete.\n'))
    console.log(chalk.yellow(
      '  Tip: built files are meant to be served over an HTTP server.\n' +
      '  Opening index.html over file:// won\'t work.\n'
    ))
  })
})


// Build header file for the final index.html.gz
const fs = require('fs')

const dataFolder = 'dist/'

var source = dataFolder + 'index.html.gz'
var destination = dataFolder + 'index.html.gz.h'

var wstream = fs.createWriteStream(destination)
wstream.on('error', function (err) {
    console.log(err)
});

var data = fs.readFileSync(source)

const moment = require('moment')
wstream.write('const char *WEBAPP_LASTMODIFIED = "' + moment.utc().format('ddd, DD MMM YYYY HH:mm:ss') + ' GMT";\n')
wstream.write('#define WEBAPP_COMPRESSED_LEN ' + data.length + '\n')
wstream.write('const uint8_t WEBAPP_COMPRESSED[] PROGMEM = {')

for (let i = 0; i < data.length; i++) {
    if (i % 1000 == 0) wstream.write("\n")
    wstream.write('0x' + ('00' + data[i].toString(16)).slice(-2))
    if (i < data.length - 1) wstream.write(',')
}

wstream.write('\n};')
wstream.end()