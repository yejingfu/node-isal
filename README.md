# node-isal
Node.js addon wrapping Intel ISA-L

## Install
```bash
  $ISAL_HOME=/path/to/isal-l npm install node-isal
```

## Build from source code
```bash
  $node-gyp configure
  $node-gyp build
```
## Test
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
Then run the file by node.js.
```bash
$ node test.js
```

