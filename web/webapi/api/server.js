/* eslint-disable new-cap */
const Koa = require('koa')
const app = new Koa()

const server = require('http').createServer(app.callback())
const io = require('socket.io')(server)

const cors = require('@koa/cors')
app.use(cors())

const router = require('./router')()
app.use(router.routes())

const Boom = require('boom')
app.use(router.allowedMethods({
  throw: true,
  notImplemented: () => new Boom.notImplemented(),
  methodNotAllowed: () => new Boom.methodNotAllowed()
}))

console.log(router.stack.map(i => `[${i.methods.join()}]: ${i.path}`))

io.on('connection', client => {
  client.on('event', data => {
    console.log(data)
  })
})

module.exports = app
