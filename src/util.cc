#include "util.h"
#include <node.h>
#include <nan.h>

#include "igzip_lib.h"

using namespace v8;

namespace {

  enum Array_TYPE {
    AT_UINT32,
    AT_UINT16,
    AT_UINT8
  }

  bool cast_array(Local<Value>& src, uint32_t *dst, uint32_t len, Array_TYPE at) {
    if (!dst || !src->IsArray()) return false;
    Local<Array> arr = Array::Cast(src);
    if (!arr && arr->Length() < len) return false;
    for (uint32_t i = 0; i < len; i++) {
      if (at == AT_UINT32) {
        dst[i] = (uint32_t)(arr->Get(i)->Int32Value());
      } else if (at == AT_UINT16) {
        dst[i] = (uint16_t)(arr->Get(i)->Int32Value());
      } else if (at == AT_UINT8) {
        dst[i] = (uint8_t)(arr->Get(i)->Int32Value());
      }
    }
    return true;
  }

}

NAN_METHOD(create_LZ_State1) {
  NanScope();
  if (args.Length() != 36) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  int idx = 0;
  uint8_t *next_in = (uint8_t*)(node::Buffer::Data(args[idx++]->ToObject()));
  uint32_t avail_in = (uint32_t)args[idx++]->Int32Value();
  uint32_t total_in = (uint32_t)args[idx++]->Int32Value();
  uint8_t *next_out = (uint8_t*)(node::Buffer::Data(args[idx++]->ToObject()));
  uint32_t avail_out = (uint32_t)args[idx++]->Int32Value();
  uint32_t total_out = (uint32_t)args[idx++]->Int32Value();
  uint32_t end_of_stream = (uint32_t)args[idx++]->Int32Value();
  uint32_t flush = (uint32_t)args[idx++]->Int32Value();
  uint32_t bytes_consumed = (uint32_t)args[idx++]->Int32Value();


  uint32_t b_bytes_valid = (uint32_t)args[idx++]->Int32Value();
  uint32_t b_bytes_processed = (uint32_t)args[idx++]->Int32Value();
  uint8_t *file_start = (uint8_t*)(node::Buffer::Data(args[idx++]->ToObject()));
  uint32_t crc[16];
  if (cast_array(args[idx++], crc, 16, AT_UINT32)) {
    NanThrowTypeError("Failed to parse array crc[16]");
    NanReturnUndefined();
  }
  // BitBuf2
  uint64_t m_bits = (uint64_t)args[idx++]->Int32Value();
  uint32_t m_bit_count = (uint32_t)args[idx++]->Int32Value();
  uint8_t *m_out_buff = (uint8_t*)(node::Buffer::Data(args[idx++]->ToObject()));
  uint8_t *m_out_end = (uint8_t*)(node::Buffer::Data(args[idx++]->ToObject()));
  uint8_t *m_out_start = (uint8_t*)(node::Buffer::Data(args[idx++]->ToObject()));

  uint32_t state = (uint32_t)args[idx++]->Int32Value();
  uint32_t count = (uint32_t)args[idx++]->Int32Value();

  uint8_t tmp_out_buff[16];
  if (cast_array(args[idx++], tmp_out_buff, 16, AT_UINT8)) {
    NanThrowTypeError("Failed to parse array tmp_out_buff[16]");
    NanReturnUndefined();
  }
  uint32_t tmp_out_start = (uint32_t)args[idx++]->Int32Value();
  uint32_t tmp_out_end = (uint32_t)args[idx++]->Int32Value();
  uint32_t last_flush = (uint32_t)args[idx++]->Int32Value();
  uint32_t submitted = (uint32_t)args[idx++]->Int32Value();
  uint8_t *last_next_in = (uint8_t*)(node::Buffer::Data(args[idx++]->ToObject()));
  uint32_t has_eob = (uint32_t)args[idx++]->Int32Value();
  uint32_t has_eob_hdr = (uint32_t)args[idx++]->Int32Value();
  uint32_t stored_blk_len = (uint32_t)args[idx++]->Int32Value();
  uint32_t no_comp = (uint32_t)args[idx++]->Int32Value();
  uint32_t left_over = (uint32_t)args[idx++]->Int32Value();
  uint32_t overflow_submitted = (uint32_t)args[idx++]->Int32Value();
  uint32_t overflow = (uint32_t)args[idx++]->Int32Value();
  uint32_t had_overflow = (uint32_t)args[idx++]->Int32Value();

  uint8_t buffer[BSIZE + 16];
  if (cast_array(args[idx++], buffer, BSIZE + 16, AT_UINT8)) {
    NanThrowTypeError("Failed to parse array buffer[BSIZE + 16]");
    NanReturnUndefined();
  }
  uint16_t head[HASH_SIZE];
  if (cast_array(args[idx++], head, HASH_SIZE, AT_UINT16)) {
    NanThrowTypeError("Failed to parse array head[HASH_SIZE]");
    NanReturnUndefined();
  }

  if (idx != 36) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }

  LZ_Stream1 * lz_stream1 = malloc(sizeof(LZ_Stream1));
  lz_stream1->next_in = next_in;
  lz_stream1->avail_in = avail_in;
  lz_stream1->total_in = total_in;
  lz_stream1->next_out = next_out;
  lz_stream1->avail_out = avail_out;
  lz_stream1->total_out = total_out;
  lz_stream1->end_of_stream = end_of_stream;
  lz_stream1->flush = flush;
  lz_stream1->bytes_consumed = bytes_consumed;
  lz_stream1->internal_state.b_bytes_valid = b_bytes_valid;
  lz_stream1->internal_state.b_bytes_processed = b_bytes_processed;
  lz_stream1->internal_state.crc = crc;
  lz_stream1->internal_state.bitbuf.m_bits = m_bits;
  lz_stream1->internal_state.bitbuf.m_bit_count = m_bit_count;
  lz_stream1->internal_state.bitbuf.m_out_buf = m_out_buf;
  lz_stream1->internal_state.bitbuf.m_out_end = m_out_end;
  lz_stream1->internal_state.bitbuf.m_out_start = m_out_start;
  lz_stream1->internal_state.state = state;
  lz_stream1->internal_state.count = count;
  lz_stream1->internal_state.tmp_out_buff = tmp_out_buff;
  lz_stream1->internal_state.tmp_out_start = tmp_out_start;
  lz_stream1->internal_state.tmp_out_end = tmp_out_end;
  lz_stream1->internal_state.last_flush = last_flush;
  lz_stream1->internal_state.submitted = submitted;
  lz_stream1->internal_state.last_next_in = last_next_in;
  lz_stream1->internal_state.has_eob = has_eob;
  lz_stream1->internal_state.has_eob_hdr = has_eob_hdr;
  lz_stream1->internal_state.stored_blk_len = stored_blk_len;
  lz_stream1->internal_state.no_comp = no_comp;
  lz_stream1->internal_state.left_over = left_over;
  lz_stream1->internal_state.overflow_submitted = overflow_submitted;
  lz_stream1->internal_state.overflow = overflow;
  lz_stream1->internal_state.had_overflow = had_overflow;
  lz_stream1->internal_state.buffer = buffer;
  lz_stream1->internal_state.head = head;

  // create JS object
  Local<ObjectTemplate> tpl = ObjectTemplate::New();
  tpl->SetInternalFieldCount(1);
  Local<Oject> jsObj = NanNew(tpl)->NewInstance();
  //jsObj->Set(NanNew<String>("id"), NanNew<Integer>(1));
  NanSetInternalFieldPointer(jsObj, 0, lz_stream1);
  NanReturnValue(jsObj);
}

NAN_METHOD(release_LZ_State1) {
  NanScope();
  if (args.Length() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  Local<Object> jsObj = args[0]->ToObject();
  if (!jsObj || jsObj->InternalFieldCount() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  LZ_Stream1 *lz_stream1 = (LZ_Stream1*)NanGetInternalFieldPointer(jsObj, 0);
  free(lz_stream1);
  NanSetInternalFieldPointer(jsObj, 0, NULL);
  NanReturnValue(NanTrue());
}

void export_util_component(v8::Handle<v8::Object>& exports) {
  exports->Set(NanNew("create_LZ_State1"), NanNew<FunctionTemplate>(create_LZ_State1)->GetFunction());
  exports->Set(NanNew("release_LZ_State1"), NanNew<FunctionTemplate>(release_LZ_State1)->GetFunction());
}
