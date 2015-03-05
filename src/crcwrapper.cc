#include "crcwrapper.h"

#include <node.h>
#include <nan.h>

#include "crc.h"

using namespace v8;

NAN_METHOD(CRC16_t10dif_01) {
  NanScope();
  if (args.Length() != 3) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  UINT16 init_crc = (UINT16)args[0]->Int32Value();
  Local<Object> bufObj = args[1]->ToObject();
  unsigned char *buf = (unsigned char*)(node::Buffer::Data(bufObj));
  int len = node::Buffer::Length(bufObj);
  UINT16 crc_value = crc16_t10dif_01(init_crc, buf, len);
  Local<Number> result = NanNew(crc_value);
  NanReturnValue(result);
}

NAN_METHOD(CRC32_ieee_01) {
  NanScope();
  if (args.Length() != 3) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  UINT16 init_crc = (UINT16)args[0]->Int32Value();
  Local<Object> bufObj = args[1]->ToObject();
  unsigned char *buf = (unsigned char*)(node::Buffer::Data(bufObj));
  int len = node::Buffer::Length(bufObj);
  UINT16 crc_value = crc32_ieee_01(init_crc, buf, len);
  Local<Number> result = NanNew(crc_value);
  NanReturnValue(result);
}

NAN_METHOD(CRC32_iscsi_simple) {
  NanScope();
  if (args.Length() != 3) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  UINT16 init_crc = (UINT16)args[0]->Int32Value();
  Local<Object> bufObj = args[1]->ToObject();
  unsigned char *buf = (unsigned char*)(node::Buffer::Data(bufObj));
  int len = node::Buffer::Length(bufObj);
  UINT16 crc_value = crc32_iscsi_simple(buf, len, init_crc);
  Local<Number> result = NanNew(crc_value);
  NanReturnValue(result);
}

NAN_METHOD(CRC32_iscsi_baseline) {
  NanScope();
  if (args.Length() != 3) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  UINT16 init_crc = (UINT16)args[0]->Int32Value();
  Local<Object> bufObj = args[1]->ToObject();
  unsigned char *buf = (unsigned char*)(node::Buffer::Data(bufObj));
  int len = node::Buffer::Length(bufObj);
  UINT16 crc_value = crc32_iscsi_baseline(buf, len, init_crc);
  Local<Number> result = NanNew(crc_value);
  NanReturnValue(result);
}

NAN_METHOD(CRC32_iscsi_00) {
  NanScope();
  if (args.Length() != 3) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  UINT16 init_crc = (UINT16)args[0]->Int32Value();
  Local<Object> bufObj = args[1]->ToObject();
  unsigned char *buf = (unsigned char*)(node::Buffer::Data(bufObj));
  int len = node::Buffer::Length(bufObj);
  UINT16 crc_value = crc32_iscsi_00(buf, len, init_crc);
  Local<Number> result = NanNew(crc_value);
  NanReturnValue(result);
}

NAN_METHOD(CRC32_iscsi_01) {
  NanScope();
  if (args.Length() != 3) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  UINT16 init_crc = (UINT16)args[0]->Int32Value();
  Local<Object> bufObj = args[1]->ToObject();
  unsigned char *buf = (unsigned char*)(node::Buffer::Data(bufObj));
  int len = node::Buffer::Length(bufObj);
  UINT16 crc_value = crc32_iscsi_01(buf, len, init_crc);
  Local<Number> result = NanNew(crc_value);
  NanReturnValue(result);
}

NAN_METHOD(CRC16_t10dif_by4) {
  NanScope();
  if (args.Length() != 3) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  UINT16 init_crc = (UINT16)args[0]->Int32Value();
  Local<Object> bufObj = args[1]->ToObject();
  unsigned char *buf = (unsigned char*)(node::Buffer::Data(bufObj));
  int len = node::Buffer::Length(bufObj);
  UINT16 crc_value = crc16_t10dif_by4(init_crc, buf, len);
  Local<Number> result = NanNew(crc_value);
  NanReturnValue(result);
}

NAN_METHOD(CRC32_ieee_by4) {
  NanScope();
  if (args.Length() != 3) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  UINT16 init_crc = (UINT16)args[0]->Int32Value();
  Local<Object> bufObj = args[1]->ToObject();
  unsigned char *buf = (unsigned char*)(node::Buffer::Data(bufObj));
  int len = node::Buffer::Length(bufObj);
  UINT16 crc_value = crc32_ieee_by4(init_crc, buf, len);
  Local<Number> result = NanNew(crc_value);
  NanReturnValue(result);
}

void exportCRCComponent(v8::Handle<v8::Object>& exports) {
  printf("Jingfu:exportCRCComponent\n");
  exports->Set(NanNew("crc16_t10dif_01"), NanNew<FunctionTemplate>(CRC16_t10dif_01)->GetFunction());
  exports->Set(NanNew("crc32_ieee_01"), NanNew<FunctionTemplate>(CRC32_ieee_01)->GetFunction());
  exports->Set(NanNew("crc32_iscsi_simple"), NanNew<FunctionTemplate>(CRC32_iscsi_simple)->GetFunction());
  exports->Set(NanNew("crc32_iscsi_baseline"), NanNew<FunctionTemplate>(CRC32_iscsi_baseline)->GetFunction());
  exports->Set(NanNew("crc32_iscsi_00"), NanNew<FunctionTemplate>(CRC32_iscsi_00)->GetFunction());
  exports->Set(NanNew("crc32_iscsi_01"), NanNew<FunctionTemplate>(CRC32_iscsi_01)->GetFunction());
  exports->Set(NanNew("crc16_t10dif_by4"), NanNew<FunctionTemplate>(CRC16_t10dif_by4)->GetFunction());
  exports->Set(NanNew("crc32_ieee_by4"), NanNew<FunctionTemplate>(CRC32_ieee_by4)->GetFunction());
}


