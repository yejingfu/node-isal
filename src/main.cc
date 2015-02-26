#include <v8.h>
#include <node.h>
#include <nan.h>

using namespace v8;

NAN_METHOD(GenBuffer) {
  NanScope();
  if (args.Length() != 1 || !args[0]->IsNumber()) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  uint32_t len = args[0]->Uint32Value();
  if (len == 0) {
    NanReturnUndefined();
  }
  node::Buffer *slowBuf = node::Buffer::New(len);
  if (len > 12) {
    // test
    memcpy(node::Buffer::Data(slowBuf), "Hello Buffer", 12);
  }
  Local<Object> globalObj = Context::GetCurrent()->Global();
  Local<Function> ctr = Local<Function>::Cast(globalObj->Get(NanNew("Buffer")));
  Handle<Value> ctrArgs[3] = {slowBuf->handle_, Integer::New(len), Integer::New(0)}; // last is offset
  Local<Object> actualBuf = ctr->NewInstance(3, ctrArgs);
  NanReturnValue(actualBuf);
  //Local<Value> val = NanNew("Hello Buffer");
  //NanReturnValue(val);
}

NAN_METHOD(PrintBuffer) {
  NanScope();
  if (args.Length() != 1 || !args[0]->IsObject()) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  Local<Object> bufObj = args[0]->ToObject();
  char *buf = node::Buffer::Data(bufObj);
  int len = node::Buffer::Length(bufObj);
  printf("Jingfu: buffer---\n");
  buf[len - 1] = 0;
  printf("%d: %s\n", len, buf);
  NanReturnUndefined();
}

void Initialize(Handle<Object> exports) {
  //Isolate *isolate = Isolate::GetCurrent();
  //exports->Set(String::NewFromUtf8(isolate, "generateBuffer"), 
  //    FunctionTemplate::New(isolate, GenBuffer)->GetFunction());
  exports->Set(NanNew("generateBuffer"), NanNew<FunctionTemplate>(GenBuffer)->GetFunction());
  exports->Set(NanNew("printBuffer"), NanNew<FunctionTemplate>(PrintBuffer)->GetFunction());
}

NODE_MODULE(isal, Initialize)

