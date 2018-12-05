const _ = require('lodash')
const spawn = require('child_process').spawn

const bconsoleAsync = async (command, api = 2) => {
  const bconsole = spawn(process.env.BCONSOLE)
  bconsole.stdin.write(`.api ${api}\n`)
  bconsole.stdin.write(command + '\n')
  bconsole.stdin.write('exit\n')

  console.log(command)

  let consoleOutput = []
  for await (const data of bconsole.stdout) {
    let str = data.toString()
    let lines = str.split(/(\r?\n)/g)
    consoleOutput.push(lines.join(''))
    // console.log(data.toString())
  }
  let commandOutput = _.find(consoleOutput, o => o.endsWith('exit\n'))
  let result = commandOutput.substring(0, commandOutput.length - 'exit\n'.length)
  return JSON.parse(result).result
}

const consoles = new Map()

const dedicatedConsole = (consoleId, api = 0) => {
  let bconsole = consoles.get(consoleId)
  if (!bconsole) {
    bconsole = spawn(process.env.BCONSOLE)
    bconsole.stdin.write(`.api ${api}\n`)
    consoles.set(consoleId, bconsole)
  }

  return bconsole
}

module.exports = {
  bconsoleAsync,
  dedicatedConsole
}
