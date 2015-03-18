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
  'sha512_sb_mgr_flush_sse4',

  'mem_cpy_sse',
  'mem_cpy_avx'
];

function filterFunction(func) {
  for (var i = 0; i < excluded.length; i++) {
    if (excluded[i] === func) return true;
  }
  return false;
}

function main() {
  var fullpath;
  var generators = [];
  for (var i = 0, len = headerfiles.length; i < len; i++) {
    //if (i !== 1) continue;
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
    console.log('====Handling ' + self.filePath);
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
    var sourcePath = path.join(self.outDir, self.headerName) + '_wrapper.cc';
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
      if (line.length < 5) continue;
      //console.log('line ' + i + ': ' + line);
      tokens = line.split(' ');
      if (tokens.length < 5) {
        badlines.push(line);
        continue;
      }
      funcRet = [];
      funcArgs = [];
      funcName = undefined;
      pos = 0;
      while (pos < tokens.length - 1) {
        if (!funcName && tokens[pos+1] === '(') {
          funcName = tokens[pos++];
          pos++;
          continue;
        }
        if (!funcName) {
          funcRet.push(tokens[pos++]);
          continue;
        }
        if (tokens[pos] !== ')' && tokens[pos] !== ';') {
          funcArgs.push(tokens[pos++]);
        } else {
          pos++;
        }
      }

      if (funcName) {
        if (!filterFunction(funcName)) {
          APINames.push(funcName);
          self.writeToSourceFile(sourceStream, funcRet, funcName, funcArgs);
        } else {
          console.log('Exclude the API: ' + funcName);
        }
      } else {
        badlines.push(line);
      }
    }
    if (badlines.length > 0) {
      console.log('Bad functions detected: total ' + badlines.length);
      for (i = 0, len = badlines.length; i < len; i++) {
        console.log('' + i +': ' + badlines[i]);
      }
    }
    var sourceEnd = ['void export_'+self.headerName+'_component(v8::Handle<v8::Object>& exports) {'];
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
    //console.log('function: ' + funcRet.join(' ') + ' ' + funcName + '(' + funcArgs.join(' ') + ');');
    var getNumOfArgs = function(argList) {
      if (argList.length === 0 || (argList.length === 1 && argList[0] === 'void')) return 0;
      var num = 1, i, len;
      for (i = 0, len = argList.length; i < len; i++) {
        if (argList[i] === ',') num++;
      }
      return num;
    };

    var iterateArgs = function(argList, cb) {
      // assume each argument is consist of <argType, argName>. It must not miss argName.
      var pos = 0;
      var argTypes = [];
      var argName;
      var index = 0;
      while (pos < argList.length) {
        if (argList[pos] !== ',') {
          argTypes.push(argList[pos]);
        } else {
          argName = argTypes.pop();
          cb(argTypes.join(' '), argName, index++);
          argTypes = [];
        }
        pos++;
      }
      if (argTypes.length > 1) {
        argName = argTypes.pop();
        cb(argTypes.join(' '), argName, index);
      }
    };


    var numOfArgs = getNumOfArgs(funcArgs);

    stream.write([
'',
'NAN_METHOD('+funcName+') {',
'  NanScope();',
'  if (args.Length() != '+numOfArgs+') {',
'    NanThrowTypeError("Invalid arguments");',
'    NanReturnUndefined();',
'  }',
''].join('\n'));
  
    var argList = [];
    var comment = '';
    iterateArgs(funcArgs, function(argType, argName, index) {
      //console.log('Iterate argument: ' + argType + '--' + argName);
      if (argType === 'UINT16' || argType === 'UINT32' || argType === 'UINT64' ||
          argType === 'int' || argType === 'unsigned int' || argType === 'const int' ||
          argType === 'uint16_t' || argType === 'uint32_t' || argType === 'size_t') {
        stream.write([
'  '+argType+' arg_'+index+' = ('+argType+')args['+index+']->Int32Value();',
''
          ].join('\n'));
      } else if (argType === 'const unsigned char *' || argType === 'unsigned char *' ||
          argType === 'UINT8 *' || argType === 'const UINT8 *' || argType === 'void *' ||
          argType === 'const void *' || argType === 'unsigned int *') {
        stream.write([
'  Local<Object> arg_obj_'+index+' = args['+index+']->ToObject();',
'  '+argType+' arg_'+index+' = ('+argType+')(node::Buffer::Data(arg_obj_'+index+'));',
''
          ].join('\n'));
      } else if (argType === 'const unsigned char **' || argType === 'unsigned char **' ||
          argType === 'void **') {
        stream.write([
'  Local<Object> arg_obj_'+index+' = args['+index+']->ToObject();',
'  '+argType+' arg_'+index+' = ('+argType+')(&((unsigned char*)node::Buffer::Data(arg_obj_'+index+')));',
''
          ].join('\n'));
      } else if (argType === 'unsigned char' || argType === 'char') {
        stream.write([
'  '+argType+' arg_'+index+' = ('+argType+')((args['+index+']->Int32Value())&0xFF);',
''
          ].join('\n'));
      } else if (argType === 'LZ_Stream1 *' ||
       argType === 'MD5_MB_MGR *' || argType === 'MD5_MB_MGR_X8X2 *' || argType === 'JOB_MD5 *' ||
       argType === 'SHA1_MB_MGR *' || argType === 'SHA1_MB_MGR_X8 *' || argType === 'JOB_SHA1 *' ||
       argType === 'SHA256_MB_MGR *' || argType === 'SHA256_MB_MGR_X8 *' || argType === 'JOB_SHA256 *' ||
       argType === 'SHA512_MB_MGR *' || argType === 'SHA512_MB_MGR_X4 *' || argType === 'JOB_SHA512 *') {
        //comment += '  // TODO: support LZ_Stream1 * as argument type.\n';
        stream.write([
'  Local<Object> arg_obj_' + index + ' = args['+index+']->ToObject();',
'  '+argType+' arg_'+index+' = ('+argType+')NanGetInternalFieldPointer(arg_obj_'+index+', 0);',
''
          ].join('\n'));
      } else if (argType === 'MD5_HASH_CTX_MGR *') {
        comment += '  // TODO: support MD5_HASH_CTX_MGR * as argument type.\n';
      } else if (argType === 'MD5_HASH_CTX *') {
        comment += '  // TODO: support MD5_HASH_CTX * as argument type.\n';
      } else if (argType === 'HASH_CTX_FLAG') {
        comment += '  // TODO: support HASH_CTX_FLAG as argument type.\n';
      } else if (argType === 'MD5_MB_JOB_MGR *') {
        comment += '  // TODO: support MD5_MB_JOB_MGR * as argument type.\n';
      } else if (argType === 'MD5_JOB *') {
        comment += '  // TODO: support MD5_JOB * as argument type.\n';
      } else if (argType === 'SHA1_HASH_CTX_MGR *') {
        comment += '  // TODO: support SHA1_HASH_CTX_MGR * as argument type.\n';
      } else if (argType === 'SHA1_HASH_CTX *') {
        comment += '  // TODO: support SHA1_HASH_CTX * as argument type.\n';
      } else if (argType === 'SHA1_MB_JOB_MGR *') {
        comment += '  // TODO: support SHA1_MB_JOB_MGR * as argument type.\n';
      } else if (argType === 'SHA1_JOB *') {
        comment += '  // TODO: support SHA1_JOB * as argument type.\n';
      } else if (argType === 'SHA256_HASH_CTX_MGR *') {
        comment += '  // TODO: support SHA256_HASH_CTX_MGR * as argument type.\n';
      } else if (argType === 'SHA256_HASH_CTX *') {
        comment += '  // TODO: support SHA256_HASH_CTX * as argument type.\n';
      } else if (argType === 'SHA256_MB_JOB_MGR *') {
        comment += '  // TODO: support SHA256_MB_JOB_MGR * as argument type.\n';
      } else if (argType === 'SHA256_JOB *') {
        comment += '  // TODO: support SHA256_JOB * as argument type.\n';
      } else if (argType === 'SHA512_HASH_CTX_MGR *') {
        comment += '  // TODO: support SHA512_HASH_CTX_MGR * as argument type.\n';
      } else if (argType === 'SHA512_HASH_CTX *') {
        comment += '  // TODO: support SHA512_HASH_CTX * as argument type.\n';
      } else if (argType === 'SHA512_MB_JOB_MGR *') {
        comment += '  // TODO: support SHA512_MB_JOB_MGR * as argument type.\n';
      } else if (argType === 'SHA512_JOB *') {
        comment += '  // TODO: support SHA512_JOB * as argument type.\n';
      } else {
        console.log('Failed to parse the argument: ' + argType);
      }
      argList.push('arg_' + index);
    });

    stream.write(comment);

    var funcRetStr = funcRet.join(' ');

    // var argstr = '';
    // if (argList.length > 1) {
    //   var last = argList.pop();
    //   argstr = argList.join(', ') + last;
    // } else if (argList.length === 1) {
    //   argstr = argList[0];
    // }
    if (funcRetStr === 'void') {
      stream.write([
'',
'  ' + funcName + '('+argList.join(', ')+');',
''
        ].join('\n'));
    } else {
      stream.write([
'',
'  ' + funcRetStr + ' result = ' + funcName + '('+argList.join(', ')+');',
''
        ].join('\n'));
    }
    comment = '';

    if (funcRetStr === 'UINT16' || funcRetStr === 'UINT32' || 
        funcRetStr === 'unsigned int' || funcRetStr === 'int') {
      stream.write([
'',
'  Local<Number> result_wrapper = NanNew(result);',
'  NanReturnValue(result_wrapper);',
''
        ].join('\n'));
    } else if (funcRetStr === 'void') {
      stream.write([
'',
'  NanReturnUndefined();',
''
        ].join('\n'));
    }/* else if (funcRetStr === 'void *') {

    } */else if (funcRetStr === 'JOB_MD5 *' || funcRetStr === 'JOB_SHA1 *' ||
        funcRetStr === 'JOB_SHA256 *' || funcRetStr === 'JOB_SHA512 *') {
      //comment = '  //TODO: return object of JOB_MD5 *';
      stream.write([
'',
'  Local<ObjectTemplate> tpl = ObjectTemplate::New();',
'  tpl->SetInternalFieldCount(1);',
'  Local<Object> retObj = NanNew(tpl)->NewInstance();',
'  NanSetInternalFieldPointer(retObj, 0, result);',
'  NanReturnValue(retObj);',
''
        ].join('\n'));
    } else if (funcRetStr === 'MD5_HASH_CTX *') {
      comment = '  //TODO: return object of MD5_HASH_CTX *';
    } else if (funcRetStr === 'MD5_JOB *') {
      comment = '  //TODO: return object of MD5_JOB *';
    } else if (funcRetStr === 'SHA1_HASH_CTX *') {
      comment = '  //TODO: return object of SHA1_HASH_CTX *';
    } else if (funcRetStr === 'SHA1_JOB *') {
      comment = '  //TODO: return object of SHA1_JOB *';
    } else if (funcRetStr === 'SHA256_HASH_CTX *') {
      comment = '  //TODO: return object of SHA256_HASH_CTX *';
    } else if (funcRetStr === 'SHA256_JOB *') {
      comment = '  //TODO: return object of SHA256_JOB *';
    } else if (funcRetStr === 'SHA512_HASH_CTX *') {
      comment = '  //TODO: return object of SHA512_HASH_CTX *';
    } else if (funcRetStr === 'SHA512_JOB *') {
      comment = '  //TODO: return object of SHA512_JOB *';
    } else {
      console.log('Failed to recognize the return type: ' + funcRetStr);
    }
    if (comment.length > 0) {
      console.log(comment);
    }

    stream.write(comment + '\n}\n');

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
'void export_'+compName+'_component(v8::Handle<v8::Object>& exports);',
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
