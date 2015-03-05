var isal = require('../build/Release/isal');
//var isal = require('node-isal');

console.log('Test begin');

var buf = isal.generateBuffer(40);
if (!Buffer.isBuffer(buf)) {
  console.error('Invalid buffer');
}
buf.write('abcdefghij', 0, 'utf8');
isal.printBuffer(buf);

console.log('Test end');

