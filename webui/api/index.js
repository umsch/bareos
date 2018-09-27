const Koa = require('koa')
const app = new Koa()
const Router = require('koa-router')

const _ = require('lodash')

const spawn = require('child_process').spawn
const prc = spawn('/home/torsten/git/bareos-umsch/regress/bin/bconsole')
prc.stdin.write('.api 2\n')

let result = []

prc.stdout.on('data', function (data) {
  var str = data.toString()
  var lines = str.split(/(\r?\n)/g)

  for (let line of lines) {
    if (line === '}') {
      console.log('schluss')
    }
  }

  var cleanJson = _.dropWhile(lines, o => !o.startsWith('{'))
  _.dropRightWhile(cleanJson, o => !o.startsWith('}'))

  console.log(cleanJson.join(''))
})

prc.on('close', function (code) {
  console.log('process exit code ' + code)
})

const router = new Router()

router.get('/status', async (ctx, next) => {
  buffer = ''

  prc.stdin.write('status dir\n')

  ctx.body = 'ok'
})

router.get('/stop', (ctx, next) => {
  prc.stdin.write('exit\n')
  ctx.body = 'ok'
})

router.get('/list/jobs/:jobid', (ctx, next) => {
  const jobid = ctx.params.jobid
  prc.stdin.write(`list joblog jobid=${jobid}\n`)
  ctx.body = 'ok'
})

router.get('/list/clients', (ctx, next) => {
  // const clientid = ctx.params.clientid
  // prc.stdin.write(`list clients jobid=${clientid}\n`)
  prc.stdin.write(`list clients\n`)
  ctx.body = 'ok client'
})

app.use(router.routes())
app.use(router.allowedMethods())

app.listen(3000)



