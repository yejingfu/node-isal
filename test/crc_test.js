var isal = require('../build/Release/isal');

console.log('=====Test CRC begin');

var SEED = 0x1234;
var BUF_SIZE = 512;

var buf = new Buffer(BUF_SIZE);
//console.log('buffer size: ' + buf.length);
for (var i = 0; i < BUF_SIZE; i++) {
  buf[i] = 0;
}
var crc1 = isal.crc16_t10dif_by4(SEED, buf, BUF_SIZE);
console.log('crc16_t10dif_by4(0): ' + crc1);
var crc2 = isal.crc32_ieee_by4(SEED, buf, BUF_SIZE);
console.log('crc32_ieee_by4(0): ' + crc2);

console.log('======Test CRC end');

