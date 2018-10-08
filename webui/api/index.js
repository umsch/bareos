const Koa = require('koa')
const cors = require('koa2-cors')

const app = new Koa()

const _ = require('lodash')

const spawn = require('child_process').spawn

const sendCommand = async (command) => {
  const bconsole = spawn(process.env.BCONSOLE)
  bconsole.stdin.write('.api 2\n')
  bconsole.stdin.write(command + '\n')
  bconsole.stdin.write('exit\n')

  let consoleOutput = []
  for await (const data of bconsole.stdout) {
    let str = data.toString()
    let lines = str.split(/(\r?\n)/g)
    consoleOutput.push(lines.join(''))
    console.log(data.toString())
  }
  let commandOutput = _.find(consoleOutput, o => o.endsWith('exit\n'))
  let result = commandOutput.substring(0, commandOutput.length - 'exit\n'.length)
  return JSON.parse(result).result
}

const uniqueConsoles = new Map()

//
// router.get('/status/:res', async (ctx, next) => {
//   ctx.body = await sendCommand(`status ${ctx.params.res}`)
// })
//
// router.get('/list/:res', async (ctx, next) => {
//   ctx.body = await sendCommand(`llist ${ctx.params.res}`)
// })
//
// router.get('/unique/open/:id', async (ctx, next) => {
//   console.log(uniqueConsoles.get(ctx.params.id))
//   if (uniqueConsoles.get(ctx.params.id)) {
//     ctx.body = { id: ctx.params.id, isopen: true, created: false }
//   } else {
//     uniqueConsoles.set(ctx.params.id, Math.random())
//     ctx.body = { id: ctx.params.id, isopen: true, created: true }
//   }
// })
//
// router.get('/unique/close/:id', async (ctx, next) => {
//
// })

app.use(cors())

const router = require('./router')()
app.use(router.routes())

app.use(router.allowedMethods())

if (!process.env.BCONSOLE) {
  console.error('environment variable BCONSOLE not set...')
} else {
  console.log(`using bconsole: ${process.env.BCONSOLE}`)
  console.log('listening on port 3000 for api calls')
  app.listen(3000)
}
