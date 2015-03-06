var isal = require('../build/Release/isal');

console.log('=====Test AES encryption & decryption begin');

var KEY_BUF_SIZE = 16;
var key1 = new Buffer(KEY_BUF_SIZE);
var key2 = new Buffer(KEY_BUF_SIZE);
var tw = new Buffer(KEY_BUF_SIZE);

var plainData = '中国 上海 陆家嘴';
var plainBuf = new Buffer(plainData, 'utf8');

// encrypt
var begin = (new Date()).getTime();
var cipherBuf = isal.XTS_AES_128_enc(plainBuf, key1, key2, tw);
var duration = (new Date()).getTime() - begin;
console.log('cipher Data(' + duration + '): ' + cipherBuf.toString('base64'));

// decrypt
begin = (new Date()).getTime();
plainBuf = isal.XTS_AES_128_dec(cipherBuf, key1, key2, tw);
duration = (new Date()).getTime() - begin;
console.log('decrypted plain data('+duration+'): ' + plainBuf.toString('utf8'));


console.log('======Test AES encryption & decryption end');

