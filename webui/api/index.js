console.log('here we go')


var spawn = require('child_process').spawn;
// var prc = spawn('bcoonsole',  ['-jar', '-Xmx512M', '-Dfile.encoding=utf8', 'script/importlistings.jar']);
var prc = spawn('bcoonsole',  ['-jar', '-Xmx512M', '-Dfile.encoding=utf8', 'script/importlistings.jar']);

//noinspection JSUnresolvedFunction
prc.stdout.setEncoding('utf8');
prc.stdout.on('data', function (data) {
  var str = data.toString()
  var lines = str.split(/(\r?\n)/g);
  console.log(lines.join(""));
});

prc.on('close', function (code) {
  console.log('process exit code ' + code);
});
