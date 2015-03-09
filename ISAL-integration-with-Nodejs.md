ISA-L integration with Node.js
=============================
## About This Document
This document descibes ISA-L integration with Node.js. It's aiming to empower Node.js users with Intel ISA-L algrithoms to accelerate storage functionalities on Intel architectures.

This document also describes the strategies about integrating ISA-L into Node.js and provides an example demontrating how efficient ISA-L is working with Node.js.

## Overview
With web & HTML5 technologies development, Node.js is adopted by more and more companies to build fast scable web applications. Node.js is perfect for data-intensive real-time applications that run across distributed devices because it uses an event-driven, non-blocking I/O programming model.

The Intel Intelligent Storage Acceleration Library(Intel ISA-L) is a collection of functions used in storage applications highly optimized for Intel architecture. The ISA-L functions are designed to deliver performance beyond what optimized compilers alone can deliver. Its core functions are assembly language.

Node.js can largely improve I/O latency and would be very suitable for [IO bound](http://en.wikipedia.org/wiki/I/O_bound) senarioes. While working with ISA-L, Node.js can also get benefits in storage applications.

## Build Node-ISAL
This section describes how to build ISA-L into Node.js and expose its API through Node.js addon.

The ISA-L is developped by pure C language. The library has hundreds of functions exposed as API, covering the data storage related areas. They are grouped into serveral categories such as CRC, RAID, Erase Code, Hashing functions and so on.

The Node.js uses addon technology to wrap C/C++ libraries. Please reference to Node.js [Addons](https://nodejs.org/api/addons.html) documentation about developping Node.js addon.

Before building the Node.js addon, please make sure ISA-L library (including module file and header files) is already prepared. Set its path to the environment variable "ISAL_HOME".

Then create a Node.js addon for ISA-L. Name it as "node-isal". When the project is created, the file structure is like below:

    |- package.json
    |- binding.gyp
    |- index.js
    |- src
        |- main.cc
    |- test
        |- test.js

The "package.json" is the manifest of the project. The "binding.gyp" is parsed by node-gyp and used to generate the make file i for the project. Node-gyp would also invoke make tools to build the project. The "index.js" exposes functions to outside. The "main.cc" bridges Javascript world to C world. It could convert Javascript calls into C calls. Under it, the real ISA-L is located.

When the project source files are ready, call the following command to build the "node-isal" component.
```bash
  $export ISAL_HOME=/path/to/isal
  $node-gyp configure
  $node-gyp build
```

If the "node-isal" component is built successfully, a Node.js addon module is generated at "build/Release/isal.node".

It's easy to write test cases for the "node-isal" component. All the test cases are written on Javascript and placed under "test" folder. Here is an example to test crypto functions which are calling into ISA-L within the C codes.
```js
//aes_test.js
var isal = require('../build/Release/isal');
var KEY_BUF_SIZE = 16;
var key1 = new Buffer(KEY_BUF_SIZE);
var key2 = new Buffer(KEY_BUF_SIZE);
var tw = new Buffer(KEY_BUF_SIZE);

var plainData = 'Sample raw data for encryption testing.....';
var plainBuf = new Buffer(plainData, 'utf8');

// encrypt
var begin = (new Date()).getTime();
var cipherBuf = isal.XTS_AES_128_enc(plainBuf, key1, key2, tw);
var duration = (new Date()).getTime() - begin;
console.log('cipher Data(' + duration + '): ' + cipherBuf.toString());

// decrypt
begin = (new Date()).getTime();
plainBuf = isal.XTS_AES_128_dec(cipherBuf, key1, key2, tw);
duration = (new Date()).getTime() - begin;
console.log('decrypted plain data('+duration+'): ' + plainBuf.toString());
```

In the example, the "XTS_AES_128_enc" and "XTS_AES_128_dec" are called to do encryption and decryption. The ISA-L corresponding APIs would be called finally.

To run the test, just execute from Node.js shell like below.
```bash
  $node test/aes_test.js
```

This project is pushed on github, please get it from [https://github.com/yejingfu/node-isal](https://github.com/yejingfu/node-isal). As above note, it depends on Intel ISA-L to build this component, please contact me(jingfu.ye@intel.com) to get the ISA-L source code or binary.

## Demonstration
This section gives a cool demonstration about how to invoke the "node-isal" in a real senario. It aslo compares the performance between ISA-L and non-ISA-L on data encryption, showing you how ISA-L accelerating your real work.

This demonstration is a typical web application allowing to select any kind of file for data encryption. At background it would invoke Node.js build-in [crypto functions](https://nodejs.org/api/crypto.html) and the corresponding ISA-L functions individually and compare their efficiency. The comparison result is as blow:

![snapshot](isal_crypto.png)

From the result we can see the ISA-L has around 4.5 times speed increased comparing than Node.js build-in crypto functions. For encryption on huge batch of files, the performance benefit would be more visible.

The demonstration is aslo located on the github and you can get it from here: [https://github.com/yejingfu/samples/tree/master/crypto/web](https://github.com/yejingfu/samples/tree/master/crypto/web). By the way it's a Node.js Express application. And you need to manually copy the "node-isal" component into the "node_moudules" before launching it.

## Future
At present, only a small set of functions are exposed from the "node-isal" component. The target would be exposing all the functions in the ISA-L. The work is in progress now.

Meanwhile we are gathering requirements from our customers and meet their real requirements with high priority. And on the other hand their feedbacks would improve the quality of the "node-isal".



