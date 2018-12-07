const spawn = require('child_process').spawn
const config = require('config')

const bconsoleAsync = async (command, api = 2) => {
  const bconsole = spawn(config.get('bareos.bconsole_executable'))
  bconsole.stdin.write(`.api ${api}\n`)
  bconsole.stdin.write(command + '\n')
  bconsole.stdin.write('exit\n')

  let consoleOutput = ''
  for await (const data of bconsole.stdout) {
    consoleOutput += data.toString()
  }

  const commandPos = consoleOutput.indexOf(command) + command.length
  const exitPos = consoleOutput.indexOf('exit\n{', commandPos)
  let result = consoleOutput.substr(commandPos, exitPos - commandPos)
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
