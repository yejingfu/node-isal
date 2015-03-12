var fs = require('fs-extra');
var path = require('path');

var headerfiles = [
  'aes_xts',
  'crc',
  'erasure_code',
  'gf_vect_mul',
  'igzip_lib',
  'mb_md5',
  'mb_sha1',
  'mb_sha256',
  'mb_sha512',
  'md5_mb',
  'sha',
  'sha1_mb',
  'sha256_mb',
  'sha512_mb',
  'mem_routines',
  'raid'
];

var excluded = [
  'ec_encode_data_avx',
  'ec_encode_data_avx2',
  'ec_encode_data_update_sse',
  'ec_encode_data_update_avx',
  'ec_encode_data_update_avx2',
  'ec_encode_data_update_base',
  'gf_vect_mad_base',
  'gf_mul',
  'gf_inv',
  'gf_gen_rs_matrix',
  'gf_gen_cauchy1_matrix',
  'gf_invert_matrix',

  'md5_submit_job_avx2',

  'sha1_mb_mgr_init_sse',
  'sha1_mb_mgr_submit_sse',
  'sha1_mb_mgr_flush_sse',
  'sha1_mb_mgr_submit_avx',
  'sha1_mb_mgr_flush_avx',
  'sha1_mb_mgr_init_avx2',
  'sha1_mb_mgr_submit_avx2',
  'sha1_mb_mgr_flush_avx2',

  'sha256_mb_mgr_init_sse',
  'sha256_mb_mgr_submit_sse',
  'sha256_mb_mgr_flush_sse',
  'sha256_mb_mgr_submit_avx',
  'sha256_mb_mgr_flush_avx',
  'sha256_mb_mgr_init_avx2',
  'sha256_mb_mgr_submit_avx2',
  'sha256_mb_mgr_flush_avx2',

  'sha512_mb_mgr_init_sse',
  'sha512_mb_mgr_submit_sse',
  'sha512_mb_mgr_flush_sse',
  'sha512_mb_mgr_submit_avx',
  'sha512_mb_mgr_flush_avx',
  'sha512_mb_mgr_init_avx2',
  'sha512_mb_mgr_submit_avx2',
  'sha512_mb_mgr_flush_avx2',
  'sha512_sse4',
  'sha512_sb_mgr_init_sse4',
  'sha512_sb_mgr_submit_sse4',
  'sha512_sb_mgr_flush_sse4'
];

function main() {
  var fullpath;
  var generators = [];
  for (var i = 0, len = headerfiles.length; i < 1; i++) {
    var headername = headerfiles[i];
    fullpath = path.join('include', headername) + '.h.func';
    var gen = new Generator(headername, fullpath);
    generators.push(gen);
    gen.generate();
  }
}

var Generator = function(headername, filepath) {
  this.headerName = headername;
  this.filePath = filepath;
  this.outDir = path.join('..', 'src');
};

Generator.prototype = {
  generate: function() {
    var self = this;
    console.log('Handling ' + self.filePath);
    fs.exists(self.filePath, function(exists) {
      if (!exists) return console.log('The file does not exists: ' + self.filePath);
      fs.readFile(self.filePath, function(err, data) {
        if (err) return console.log('Failed to read file: ' + self.filePath + ' (' + err + ')');
        self.parse(data.toString());
      });
    });
  },

  parse: function(data) {
    var self = this;
    var headerPath = path.join(self.outDir, self.headerName) + '_wrapper.h';
    var sourcePath = path.join(self.outDir, self.headerName) + '_wrapper.cpp';
    self.writeToHeaderFile(headerPath, self.headerName);
    var sourceStream = fs.createWriteStream(sourcePath);
    var sourceBegin = [
'//This is automatically generated, please DO NOT modify it manually.',
'',
'#include "'+self.headerName+'_wrapper.h"',
'',
'#include <node.h>',
'#include <nan.h>',
'#include "util.h"',
'',
'#include "'+self.headerName+'.h"',
'',
'using namespace v8;',
'',
''
    ].join('\n');
    sourceStream.write(sourceBegin);

    var APINames = [];
    //console.log('=====generating: ' + headerPath + '|.cpp');
    var lines = data.split('\n'), badlines = [], line, tokens, token, funcName, funcRet = [], funcArgs = [];
    var pos, i, len;
    for (i = 0, len = lines.length; i < len; i++) {
      line = lines[i];
      //console.log('line ' + i + ': ' + line);
      tokens = line.split(' ');
      if (tokens.length < 5) {
        badlines.push(line);
        continue;
      }
      funcRet = [];
      funcArgs = [];
      pos = 0;
      funcRet.push(tokens[pos++]);
      if (tokens[pos] === '*') {
        funcRet.push(tokens[pos++]);
      }
      funcName = tokens[pos++];
      if (tokens[pos] !== '(') {
        badlines.push(line);
        continue;
      }
      pos++;
      while (pos < tokens.length && tokens[pos] !== ')') {
        funcArgs.push(tokens[pos++]);
      }
      APINames.push(funcName);
      self.writeToSourceFile(sourceStream, funcRet, funcName, funcArgs);
    }
    var sourceEnd = ['void export_'+self.headerName+'_Component(v8::Handle<v8::Object>& exports) {'];
    for (i = 0, len = APINames.length; i < len; i++) {
      sourceEnd.push('  exports->Set(NanNew("'+APINames[i]+'"), NanNew<FunctionTemplate>('+APINames[i]+')->GetFunction());');
    }
    sourceEnd.push('}');
    sourceEnd.push('');
    sourceStream.write(sourceEnd.join('\n'));
    sourceStream.end();
    console.log(sourcePath + ' is generated.');
  },

  writeToSourceFile: function(stream, funcRet, funcName, funcArgs) {
    console.log('function: ' + funcRet.join(' ') + ' ' + funcName + '(' + funcArgs.join(' ') + ');');
  },

  writeToHeaderFile: function(filePath, compName) {
    var compNameUpper = compName.toUpperCase();
    var content = [
'//This is automatically generated, please DO NOT modify it manually.',
'',
'#ifndef ISAL_'+compNameUpper+'_WRAPPER_H_',
'#define ISAL_'+compNameUpper+'_WRAPPER_H_',
'',
'#include <v8.h>',
'',
'void export_'+compName+'_omponent(v8::Handle<v8::Object>& exports);',
'',
'#endif  // ISAL_'+compNameUpper+'_WRAPPER_H_',
''].join('\n');

    fs.writeFile(filePath, content, function(err) {
      if (err) throw err;
      console.log(filePath + ' is generated.');
    });
  }

};

main();
