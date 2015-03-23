# node-isal
Node.js addon wrapping Intel ISA-L

## Install
```bash
  $ISAL_HOME=/path/to/isa-l npm install node-isal
```

## Build from source code

- Build ISA-L library

```bash
$cd /path/to/isa-l/src
$make
```
Then the static library is generated at: `/path/to/isa-l/bin/libisa-l.a`. Please modify build target in its make file to `lib_name := bin/libisa-l.a`.

- Generate API wrapper using tools.

```bash
$cd /path/to/node-isal/tools
$cp -r /path/to/isa-l/src/include ./
$node parsecpp.js
$node genwrapper.js
```

The batch would generate XXX_wrapper.h files under `/path/node-isal/src/`.

- Build Node addon with isa-l library

The node-isal addon depends on Intel ISA-L library, so please set the environment variable `ISAL_HOME` to right path of ISA-L before building the addon.
```bash
  $export ISAL_HOME=/path/to/isa-l
  $node-gyp configure
  $node-gyp build
```
After the build is completed, a node addon is generated at: `/path/to/node-isal/build/Release/isal.node`. Then you can include it within your Node.js project.

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

## Issues

- Link error, some APIs failed to build into Node.js addon, the following link error may occur:

    relocation R_X86_64_32S against crc32_table_iscsi_base can not be used when making a shared object; recompile with -fPIC
    libisa-l.a: error adding symbols: Bad value

- UINT64 value may overflow when passing from Javascript to C++(addon).

- Need to write more test cases.

