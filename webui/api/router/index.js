const fs = require('fs')
const path = require('path')
const routeBasePath = path.dirname(module.filename)
console.log(routeBasePath)
console.log(path.join(routeBasePath, 'routes'))

module.exports = () => {
  const Router = require('koa-router')
  const router = new Router({
    prefix: '/api'
  })

  // fs.readdirSync(path.join([routeBasePath, 'routes'])).forEach(file => {
  fs.readdirSync(path.join(routeBasePath, 'routes')).forEach(file => {
    console.log(`register route module '${file}'`)
    require(path.join(routeBasePath, 'routes', file))(router)
  })

  return router
}
