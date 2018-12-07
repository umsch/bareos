const spawn = require('child_process').spawn
const config = require('config')

const bconsoleAsync = async (command, api = 2) => {
  const bconsole = spawn(config.get('bareos.bconsole_executable'))
  console.log(bconsole)
  bconsole.stdin.write(`.api ${api}\n`)
  bconsole.stdin.write(command + '\n')
  bconsole.stdin.write('exit\n')

  // console.log(command)

  let consoleOutput = []
  for await (const data of bconsole.stdout) {
    let str = data.toString()
    let lines = str.split(/(\r?\n)/g)
    consoleOutput.push(lines.join(''))
    // console.log(data.toString())
  }
  let completeResult = consoleOutput.join('')
  // console.log(completeResult)
  const commandpos = completeResult.indexOf(command) + command.length
  const exitpos = completeResult.indexOf('exit\n{')
  // console.log({ commandpos, exitpos })
  // console.log('and the answer is: ' + completeResult.substr(commandpos, exitpos - commandpos))
  let result = completeResult.substr(commandpos, exitpos - commandpos)
  console.log(JSON.parse(result).result)
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
