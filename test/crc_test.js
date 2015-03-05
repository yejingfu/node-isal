var isal = require('../build/Release/isal');

console.log('=====Test CRC begin');

var SEED = 0x1234;
var BUF_SIZE = 512;

var buf = new Buffer(BUF_SIZE);
//console.log('buffer size: ' + buf.length);
for (var i = 0; i < BUF_SIZE; i++) {
  buf[i] = 0;
}
var crc0 = isal.crc16_t10dif_01(SEED, buf, BUF_SIZE);
console.log('crc16_t10dif_01(0): ' + crc0);

var crc1 = isal.crc16_t10dif_by4(SEED, buf, BUF_SIZE);
console.log('crc16_t10dif_by4(0): ' + crc1);

var crc2 = isal.crc32_ieee_by4(SEED, buf, BUF_SIZE);
console.log('crc32_ieee_by4(0): ' + crc2);

var crc3 = isal.crc32_ieee_01(SEED, buf, BUF_SIZE);
console.log('crc32_ieee_01(0): ' + crc3);

var crc4 = isal.crc32_iscsi_simple(SEED, buf, BUF_SIZE);
console.log('crc32_iscsi_simple(0): ' + crc4);

var crc5 = isal.crc32_iscsi_baseline(SEED, buf, BUF_SIZE);
console.log('crc32_iscsi_baseline(0): ' + crc5);

var crc6 = isal.crc32_iscsi_00(SEED, buf, BUF_SIZE);
console.log('crc32_iscsi_00(0): ' + crc6);

var crc7 = isal.crc32_iscsi_01(SEED, buf, BUF_SIZE);
console.log('crc32_iscsi_01(0): ' + crc7);


console.log('======Test CRC end');

