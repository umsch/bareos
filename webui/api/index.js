const Koa = require('koa')
const app = new Koa()
const Router = require('koa-router')
const _ = require('lodash')

const spawn = require('child_process').spawn

const sendCommand = async (command) => {
  const bconsole = spawn('/home/torsten/git/bareos-umsch/regress/bin/bconsole')
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

const router = new Router()
router.get('/status/:res', async (ctx, next) => {
  ctx.body = await sendCommand(`status ${ctx.params.res}`)
})

router.get('/list/:res', async (ctx, next) => {
  ctx.body = await sendCommand(`list ${ctx.params.res}`)
})

app.use(router.routes())
app.use(router.allowedMethods())
app.listen(3000)
