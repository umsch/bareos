const app = require('./server')

if (!process.env.BCONSOLE) {
  console.error('environment variable BCONSOLE not set...')
} else {
  console.log(`using bconsole: ${process.env.BCONSOLE}`)
  console.log('listening on port 3000 for api calls')
  app.listen(3000)
}
