const app = require('./server')
const config = require('config')
const bconsole = config.get('bareos.bconsole_executable')

if (!process.env.BCONSOLE) {
  console.error('environment variable BCONSOLE not set...')
} else {
  console.log(`using bconsole: ${bconsole}`)
  console.log('listening on port 3000 for api calls')
}

app.listen(3000)
