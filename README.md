# node-isal
Node.js addon wrapping Intel ISA-L

## Install
```bash
  $ISAL_HOME=/path/to/isa-l npm install node-isal
```

## Build from source code
The node-isal addon depends on Intel ISA-L library, so please set the environment variable `ISAL_HOME` to right path of ISA-L before building the addon.
```bash
  $export ISAL_HOME=/path/to/isa-l
  $node-gyp configure
  $node-gyp build
```
## Test
- CRC API testing
```js
  // test.js
  var isal = require('node-isal');
  var SEED = 0x1234;
  var BUF_SIZE = 512;
  var buf = new Buffer(BUF_SIZE);
  for (var i = 0; i < BUF_SIZE; i++) {
    buf[i] = 0;
  }
  var crc1 = isal.crc16_t10dif_by4(SEED, buf, BUF_SIZE);
  console.log('crc16_t10dif_by4(0): ' + crc1);
  var crc2 = isal.crc32_ieee_by4(SEED, buf, BUF_SIZE);
  console.log('crc32_ieee_by4(0): ' + crc2);
```

- Crypto API testing
```js
var KEY_BUF_SIZE = 16;
var key1 = new Buffer(KEY_BUF_SIZE);
var key2 = new Buffer(KEY_BUF_SIZE);
var tw = new Buffer(KEY_BUF_SIZE);

var plainData = 'Example raw data for crypto testing';
var plainBuf = new Buffer(plainData, 'utf8');

// encrypt
var cipherBuf = isal.XTS_AES_128_enc(plainBuf, key1, key2, tw);
console.log('encrypted data:' + cipherBuf.toString());

// decrypt
plainBuf = isal.XTS_AES_128_dec(cipherBuf, key1, key2, tw);
console.log('decrypted plain data' + plainBuf.toString());
```

- Run test in Node.js
```bash
$ node test.js
```

