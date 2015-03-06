#include "aeswrapper.h"

#include <node.h>
#include <nan.h>

#include "aes_xts.h"

using namespace v8;

NAN_METHOD(XTS_AES_128_enc) {
  NanScope();
  if (args.Length() != 4) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }

  Local<Object> bufObj = args[0]->ToObject();
  Local<Object> key1Obj = args[1]->ToObject();
  Local<Object> key2Obj = args[2]->ToObject();
  Local<Object> twObj = args[3]->ToObject();
  unsigned char *buf_data = (unsigned char*)(node::Buffer::Data(bufObj));
  int buf_len = node::Buffer::Length(bufObj);
  unsigned char *key1_data = (unsigned char*)(node::Buffer::Data(key1Obj));
  int key1_len = node::Buffer::Length(key1Obj);
  unsigned char *key2_data = (unsigned char*)(node::Buffer::Data(key2Obj));
  int key2_len = node::Buffer::Length(key2Obj);
  unsigned char *tw_data = (unsigned char*)(node::Buffer::Data(twObj));
  int tw_len = node::Buffer::Length(twObj);

  if (buf_len < 16 || key1_len != 16 || key2_len != 16 || tw_len != 16) {
    NanThrowTypeError("Invalid arguments: ensure the key /tw vector length is 16 and the buffer len is larger than 16.");
    NanReturnUndefined();
  }

  node::Buffer *cipherBuf = node::Buffer::New(buf_len);
  unsigned char *pOutput = (unsigned char*)(node::Buffer::Data(cipherBuf));
  XTS_AES_128_enc(key2_data, key1_data, tw_data, buf_len, buf_data, pOutput);

  // create JS Buffer and return
  Local<Object> globalObj = Context::GetCurrent()->Global();
  Local<Function> funcTmpl = Local<Function>::Cast(globalObj->Get(NanNew("Buffer")));
  Handle<Value> ctrargs[3] = {cipherBuf->handle_, Integer::New(buf_len), Integer::New(0)};  // offset : 0
  Local<Object> cipherBufObj = funcTmpl->NewInstance(3, ctrargs);
  NanReturnValue(cipherBufObj);
}

NAN_METHOD(XTS_AES_128_dec) {
  NanScope();
  if (args.Length() != 4) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }

  Local<Object> bufObj = args[0]->ToObject();
  Local<Object> key1Obj = args[1]->ToObject();
  Local<Object> key2Obj = args[2]->ToObject();
  Local<Object> twObj = args[3]->ToObject();
  unsigned char *buf_data = (unsigned char*)(node::Buffer::Data(bufObj));
  int buf_len = node::Buffer::Length(bufObj);
  unsigned char *key1_data = (unsigned char*)(node::Buffer::Data(key1Obj));
  int key1_len = node::Buffer::Length(key1Obj);
  unsigned char *key2_data = (unsigned char*)(node::Buffer::Data(key2Obj));
  int key2_len = node::Buffer::Length(key2Obj);
  unsigned char *tw_data = (unsigned char*)(node::Buffer::Data(twObj));
  int tw_len = node::Buffer::Length(twObj);

  if (buf_len < 16 || key1_len != 16 || key2_len != 16 || tw_len != 16) {
    NanThrowTypeError("Invalid arguments: ensure the key /tw vector length is 16 and the buffer len is larger than 16.");
    NanReturnUndefined();
  }

  node::Buffer *plainBuf = node::Buffer::New(buf_len);
  unsigned char *pOutput = (unsigned char*)(node::Buffer::Data(plainBuf));
  XTS_AES_128_dec(key2_data, key1_data, tw_data, buf_len, buf_data, pOutput);

  // create JS Buffer and return
  Local<Object> globalObj = Context::GetCurrent()->Global();
  Local<Function> funcTmpl = Local<Function>::Cast(globalObj->Get(NanNew("Buffer")));
  Handle<Value> ctrargs[3] = {plainBuf->handle_, Integer::New(buf_len), Integer::New(0)};  // offset : 0
  Local<Object> plainBufObj = funcTmpl->NewInstance(3, ctrargs);
  NanReturnValue(plainBufObj);
}

void exportAESComponent(v8::Handle<v8::Object>& exports) {
  printf("Jingfu:exportAESComponent\n");
  exports->Set(NanNew("XTS_AES_128_enc"), NanNew<FunctionTemplate>(XTS_AES_128_enc)->GetFunction());
  exports->Set(NanNew("XTS_AES_128_dec"), NanNew<FunctionTemplate>(XTS_AES_128_dec)->GetFunction());
}

