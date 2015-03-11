var fs = require('fs-extra');
var path = require('path');
var tokenizer_class = require('tokenizer');

var headerfiles = [
  'test.h'
];

var HeaderParser = function(filePath) {
  this.filePath = filePath;
  this.tokenizer = null;
  this.funcNames = [];
};

HeaderParser.prototype = {
  doParse: function() {
    var outPath = this.filePath + '.func';
    var outStream = fs.createWriteStream(outPath);

    var parsingFunc = false;
    var funcName = '';
    var self = this;

    this.tokenizer = new tokenizer_class();
    this.tokenizer.on('end', function(){
      console.log('======on token end');  // why never come here?
    });
    this.tokenizer.on('token', function(token, type) {
      //console.log('on token:'+token.type+'=>'+token.content);
      if (token.type !== 'linecomment' &&
          token.type !== 'areacomment' &&
          token.type !== 'areacommentcontinue' &&
          token.type !== 'whitespace') {
        //
        if (parsingFunc) {
          funcName += token.content + ' ';
          if (token.type === 'halfcomma') {
            parsingFunc = false;
            console.log('function: ' + funcName);
            self.funcNames.push(funcName);
            outStream.write(funcName + '\n');
            funcName = '';
          }
        } else {
          if (self.isFunctionStartToken(token)) {
            parsingFunc = true;
            funcName += token.content + ' ';
          }
        }
      }
    });

    this.tokenizer.addRule(/^\/\/[^\n]*$/, 'linecomment');
    this.tokenizer.addRule(/^\/\*([^*]|\*(?!\/))*\*\/$/, 'areacomment');
    this.tokenizer.addRule(/^\/\*([^*]|\*(?!\/))*\*?$/, 'areacommentcontinue');

    this.tokenizer.addRule(/^#(\S*)$/, 'directive');

    this.tokenizer.addRule(/^"([^"\n]|\\")*"?$/, 'quote');
    this.tokenizer.addRule(/^'(\\?[^'\n]|\\')'?$/, 'char');
    this.tokenizer.addRule(/^'[^']*$/, 'charcontinue');
    this.tokenizer.addRule(/^\($/, 'openparen');
    this.tokenizer.addRule(/^\)$/, 'closeparen');
    this.tokenizer.addRule(/^\[$/, 'opensquare');
    this.tokenizer.addRule(/^\]$/, 'closesquare');
    this.tokenizer.addRule(/^\{$/, 'opencurly');
    this.tokenizer.addRule(/^\}$/, 'closecurly');
    this.tokenizer.addRule(/^;$/, 'halfcomma');
    this.tokenizer.addRule(/^,$/, 'comma');
    this.tokenizer.addRule(/^\\\n?$/, 'linecontinue');
    this.tokenizer.addRule(/^([-<>~!%^&*\/+=?|.,:;]|->|<<|>>|\*\*|\|\||&&|--|\+\+|[-+*|&%%\/=]=)$/, 'operator')
    this.tokenizer.addRule(/^[-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?$/, 'number');
    this.tokenizer.addRule(/^([_A-Za-z]\w*)$/, 'identifier');
    this.tokenizer.addRule(tokenizer_class.whitespace);
    /*
    fs.readFile(this.filePath, function(err, data) {
      if (err) return console.log('Failed to read file: ' + self.filePath + ':' + err);
      self.tokenizer.write(data);
      self.tokenizer.end();
    });
    */
    var stream = fs.createReadStream(this.filePath);
    stream.pipe(this.tokenizer);
    stream.on('close', function() {
      outStream.end();
      self.done();
    });
  },

  isFunctionStartToken: function(token) {
    if (token.type === 'identifier' &&
      (token.content === 'void' ||
      token.content === 'UINT16' ||
      token.content === 'UINT32' ||
      token.content === 'unsigned' ||
      token.content === 'int' ||
      token.content === 'JOB_MD5' ||
      token.content === 'JOB_SHA1' ||
      token.content === 'JOB_SHA256' ||
      token.content === 'JOB_SHA512' ||
      token.content === 'MD5_HASH_CTX' ||
      token.content === 'MD5_JOB' ||
      token.content === 'SHA1_HASH_CTX' ||
      token.content === 'SHA1_JOB' ||
      token.content === 'SHA256_HASH_CTX' ||
      token.content === 'SHA256_JOB' ||
      token.content === 'SHA512_HASH_CTX' ||
      token.content === 'SHA512_JOB'
      ))
      return true;
    return false;
  },

  done: function() {
    console.log('done');
  }
};

function parseHeaderFiles(headerfiles) {
  var fullpath;
  var parsers = [];
  for (var i = 0, len = headerfiles.length; i < len; i++) {
    fullpath = path.join('include', headerfiles[i]);
    var parser = new HeaderParser(fullpath);
    parsers.push(parser);
    parser.doParse();
  }
}

parseHeaderFiles(headerfiles);
