var isal = require('../build/Release/isal');

console.log('=====Test CRC begin');

var SEED = 0x1234;
var BUF_SIZE = 512;

var buf = new Buffer(BUF_SIZE);
//console.log('buffer size: ' + buf.length);
for (var i = 0; i < BUF_SIZE; i++) {
  buf[i] = 0;
}
var crc = isal.crc16_t10dif(SEED, buf, BUF_SIZE);
console.log('result: ' + crc);

console.log('======Test CRC end');

