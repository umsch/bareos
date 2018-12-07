/* eslint-disable new-cap */
const Koa = require('koa')
const app = new Koa()
const config = require('config')
const bconsole = config.get('bareos.bconsole_executable')
const fs = require('fs')

const checkConfig = async (ctx, next) => {
  if (!bconsole) {
    console.log('env BCONSOLE not set')
    ctx.status = 500
    ctx.body = 'env BSONSOLE not set'
  } else if (!fs.existsSync(bconsole)) {
    console.log(`BCONSOLE not found: ${bconsole}`)
    ctx.status = 500
    ctx.body = `BCONSOLE not found: ${bconsole}`
  } else {
    await next()
  }
}

const cors = require('@koa/cors')
app.use(cors())
app.use(checkConfig)

const router = require('./router')()
app.use(router.routes())

const Boom = require('boom')
app.use(router.allowedMethods({
  throw: true,
  notImplemented: () => new Boom.notImplemented(),
  methodNotAllowed: () => new Boom.methodNotAllowed()
}))

console.log(router.stack.map(i => `[${i.methods.join()}]: ${i.path}`))

module.exports = app
