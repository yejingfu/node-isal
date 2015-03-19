#include "util.h"
#include <node.h>
#include <nan.h>

#include "igzip_lib.h"
#include "mb_md5.h"
#include "mb_sha1.h"
#include "mb_sha256.h"
#include "mb_sha512.h"
#include "md5_mb.h"
#include "sha1_mb.h"
#include "sha256_mb.h"
#include "sha512_mb.h"

using namespace v8;

namespace {

  enum Array_TYPE {
    AT_UINT64,
    AT_UINT32,
    AT_UINT16,
    AT_UINT8
  };

  bool cast_array(Local<Value> src, void *dst, uint32_t len, Array_TYPE at) {
    if (!dst || !src->IsArray()) return false;
    //Local<Array> arr = Array::Cast(src);
    Local<Array> arr = Local<Array>::Cast(src);
    if (arr.IsEmpty() && arr->Length() < len) return false;
    for (uint32_t i = 0; i < len; i++) {
      if (at == AT_UINT64) {
        uint64_t* p = (uint64_t*)dst;
        p[i] = (uint64_t)(arr->Get(i)->Int32Value());
      } else if (at == AT_UINT32) {
        uint32_t* p = (uint32_t*)dst;
        p[i] = (uint32_t)(arr->Get(i)->Int32Value());
      } else if (at == AT_UINT16) {
        uint16_t* p = (uint16_t*)dst;
        p[i] = (uint16_t)(arr->Get(i)->Int32Value());
      } else if (at == AT_UINT8) {
        uint8_t* p = (uint8_t*)dst;
        p[i] = (uint8_t)(arr->Get(i)->Int32Value());
      }
    }
    return true;
  }

}

/**
*  LZ_State1
*/

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
  if (!cast_array(args[idx++], crc, 16, AT_UINT32)) {
    NanThrowTypeError("Failed to parse array crc[16]");
    NanReturnUndefined();
  }
  // BitBuf2
  uint64_t m_bits = (uint64_t)args[idx++]->Int32Value();
  uint32_t m_bit_count = (uint32_t)args[idx++]->Int32Value();
  uint8_t *m_out_buf = (uint8_t*)(node::Buffer::Data(args[idx++]->ToObject()));
  uint8_t *m_out_end = (uint8_t*)(node::Buffer::Data(args[idx++]->ToObject()));
  uint8_t *m_out_start = (uint8_t*)(node::Buffer::Data(args[idx++]->ToObject()));

  uint32_t state = (uint32_t)args[idx++]->Int32Value();
  uint32_t count = (uint32_t)args[idx++]->Int32Value();

  uint8_t tmp_out_buff[16];
  if (!cast_array(args[idx++], tmp_out_buff, 16, AT_UINT8)) {
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
  if (!cast_array(args[idx++], buffer, BSIZE + 16, AT_UINT8)) {
    NanThrowTypeError("Failed to parse array buffer[BSIZE + 16]");
    NanReturnUndefined();
  }
  uint16_t head[HASH_SIZE];
  if (!cast_array(args[idx++], head, HASH_SIZE, AT_UINT16)) {
    NanThrowTypeError("Failed to parse array head[HASH_SIZE]");
    NanReturnUndefined();
  }

  if (idx != 36) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }

  LZ_Stream1 * lz_stream1 = (LZ_Stream1*)malloc(sizeof(LZ_Stream1));
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
  lz_stream1->internal_state.file_start = file_start;
  memcpy(lz_stream1->internal_state.crc, crc, 64);
  lz_stream1->internal_state.bitbuf.m_bits = m_bits;
  lz_stream1->internal_state.bitbuf.m_bit_count = m_bit_count;
  lz_stream1->internal_state.bitbuf.m_out_buf = m_out_buf;
  lz_stream1->internal_state.bitbuf.m_out_end = m_out_end;
  lz_stream1->internal_state.bitbuf.m_out_start = m_out_start;
  lz_stream1->internal_state.state = (LZ_State1_state)state;
  lz_stream1->internal_state.count = count;
  memcpy(lz_stream1->internal_state.tmp_out_buff, tmp_out_buff, 16);
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
  memcpy(lz_stream1->internal_state.buffer, buffer, BSIZE + 16);
  memcpy(lz_stream1->internal_state.head, head, HASH_SIZE);


  // create JS object
  Local<ObjectTemplate> tpl = ObjectTemplate::New();
  tpl->SetInternalFieldCount(1);
  Local<Object> jsObj = NanNew(tpl)->NewInstance();
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
  if (jsObj.IsEmpty() || jsObj->InternalFieldCount() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  LZ_Stream1 *lz_stream1 = (LZ_Stream1*)NanGetInternalFieldPointer(jsObj, 0);
  free(lz_stream1);
  NanSetInternalFieldPointer(jsObj, 0, NULL);
  NanReturnValue(NanTrue());
}

/**
* JOB_MD5
*/
NAN_METHOD(create_JOB_MD5) {
  NanScope();
  if (args.Length() != 7) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  int idx = 0;
  UINT8 *buffer = (UINT8*)(node::Buffer::Data(args[idx++]->ToObject()));
  UINT32 len = (UINT32)args[idx++]->Int32Value();
  UINT32 len_total = (UINT32)args[idx++]->Int32Value();
  UINT32 result_digest[NUM_MD5_DIGEST_WORDS];
  if (!cast_array(args[idx++], result_digest, NUM_MD5_DIGEST_WORDS, AT_UINT32)) {
    NanThrowTypeError("Failed to parse array result_digest[NUM_MD5_DIGEST_WORDS]");
    NanReturnUndefined();
  }
  UINT32 status = (UINT32)args[idx++]->Int32Value();
  UINT32 flags = (UINT32)args[idx++]->Int32Value();
  void *user_data = (void*)(node::Buffer::Data(args[idx++]->ToObject()));
  JOB_MD5 * job_md5 = (JOB_MD5*)malloc(sizeof(JOB_MD5));
  job_md5->buffer = buffer;
  job_md5->len = len;
  job_md5->len_total = len_total;
  memcpy(job_md5->result_digest, result_digest, NUM_MD5_DIGEST_WORDS * 4);
  job_md5->status = (JOB_STS)status;
  job_md5->flags = flags;
  job_md5->user_data = user_data;

  Local<ObjectTemplate> tpl = ObjectTemplate::New();
  tpl->SetInternalFieldCount(1);
  Local<Object> retObj = NanNew(tpl)->NewInstance();
  NanSetInternalFieldPointer(retObj, 0, job_md5);
  NanReturnValue(retObj);
}

NAN_METHOD(release_JOB_MD5) {
  NanScope();
  if (args.Length() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  Local<Object> jsObj = args[0]->ToObject();
  if (jsObj.IsEmpty() || jsObj->InternalFieldCount() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  JOB_MD5 *job_md5 = (JOB_MD5*)NanGetInternalFieldPointer(jsObj, 0);
  free(job_md5);
  NanSetInternalFieldPointer(jsObj, 0, NULL);
  NanReturnValue(NanTrue());
}

NAN_METHOD(create_MD5_HMAC_LANE_DATA) {
  NanScope();
  if (args.Length() != 5) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  int idx = 0;
  UINT8 *extra_block = (UINT8*)(node::Buffer::Data(args[idx++]->ToObject()));
  JOB_MD5 *job_in_lane = (JOB_MD5*)NanGetInternalFieldPointer(args[idx++]->ToObject(), 0);
  UINT32 extra_blocks = (UINT32)args[idx++]->Int32Value();
  UINT32 size_offset = (UINT32)args[idx++]->Int32Value();
  UINT32 start_offset = (UINT32)args[idx++]->Int32Value();
  MD5_HMAC_LANE_DATA *data = (MD5_HMAC_LANE_DATA*)malloc(sizeof(MD5_HMAC_LANE_DATA));
  memcpy(data->extra_block, extra_block, NUM_MD5_DIGEST_WORDS);
  data->job_in_lane = job_in_lane;
  data->extra_blocks = extra_blocks;
  data->size_offset = size_offset;
  data->start_offset = start_offset;

  Local<ObjectTemplate> tpl = ObjectTemplate::New();
  tpl->SetInternalFieldCount(1);
  Local<Object> retObj = NanNew(tpl)->NewInstance();
  NanSetInternalFieldPointer(retObj, 0, data);
  NanReturnValue(retObj);
}

NAN_METHOD(release_MD5_HMAC_LANE_DATA) {
  NanScope();
  if (args.Length() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  MD5_HMAC_LANE_DATA* data = (MD5_HMAC_LANE_DATA*)NanGetInternalFieldPointer(args[0]->ToObject(), 0);
  free(data);
  NanSetInternalFieldPointer(args[0]->ToObject(), 0, NULL);
  NanReturnValue(NanTrue());
}

/**
* MD5_MB_MGR
*/
NAN_METHOD(create_MD5_MB_MGR) {
  NanScope();
  if (args.Length() != 5) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  MD5_MB_MGR * mgr = (MD5_MB_MGR*)malloc(sizeof(MD5_MB_MGR));
  int idx = 0;
  if (!cast_array(args[idx++], mgr->args.digest, NUM_MD5_DIGEST_WORDS*NUM_MD5_LANES, AT_UINT32)) {
    NanThrowTypeError("Failed to parse array digest[NUM_MD5_DIGEST_WORDS*NUM_MD5_LANES]");
    free(mgr);
    NanReturnUndefined();
  }
  Local<Array> arr = Local<Array>::Cast(args[idx++]);
  if (arr.IsEmpty() && arr->Length() < NUM_MD5_LANES) {
    free(mgr);
    NanReturnUndefined();
  }
  for (int i = 0; i < NUM_MD5_LANES; i++) {
    mgr->args.data_ptr[i] = (UINT8*)(node::Buffer::Data(arr->Get(i)->ToObject()));
  }
  if (!cast_array(args[idx++], mgr->lens, NUM_MD5_LANES, AT_UINT32)) {
    NanThrowTypeError("Failed to parse array len[NUM_MD5_LANES]");
    free(mgr);
    NanReturnUndefined();
  }
  mgr->unused_lanes = (UINT64)args[idx++]->Int32Value();
  Local<Array> arr2 = Local<Array>::Cast(args[idx++]);
  if (arr2.IsEmpty() || arr2->Length() < NUM_MD5_LANES) {
    NanThrowTypeError("Failed to parse array ldata[NUM_MD5_LANES]");
    free(mgr);
    NanReturnUndefined();
  }
  for (int i = 0; i < NUM_MD5_LANES; i++) {
    MD5_HMAC_LANE_DATA* pData = (MD5_HMAC_LANE_DATA*)NanGetInternalFieldPointer(arr2->Get(i)->ToObject(), 0);
    if (!pData) {
      NanThrowTypeError("Failed to parse array ldata[NUM_MD5_LANES] because of invalid MD5_HMAC_LANE_DATA");
      free(mgr);
      NanReturnUndefined();
    }
    memcpy(mgr->ldata[i].extra_block, pData->extra_block, 2*64+8);
    mgr->ldata[i].job_in_lane = pData->job_in_lane;
    mgr->ldata[i].extra_blocks = pData->extra_blocks;
    mgr->ldata[i].size_offset = pData->size_offset;
    mgr->ldata[i].start_offset = pData->start_offset;
  }

  Local<ObjectTemplate> tpl = ObjectTemplate::New();
  tpl->SetInternalFieldCount(1);
  Local<Object> retObj = NanNew(tpl)->NewInstance();
  NanSetInternalFieldPointer(retObj, 0, mgr);
  NanReturnValue(retObj);
}

NAN_METHOD(release_MD5_MB_MGR) {
  NanScope();
  if (args.Length() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  MD5_MB_MGR* mgr = (MD5_MB_MGR*)NanGetInternalFieldPointer(args[0]->ToObject(), 0);
  free(mgr);
  NanSetInternalFieldPointer(args[0]->ToObject(), 0, NULL);
  NanReturnValue(NanTrue());
}

/**
* MD5_MB_MGR_X8X2
*/
NAN_METHOD(create_MD5_MB_MGR_X8X2) {
  NanScope();
  if (args.Length() != 6) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  MD5_MB_MGR_X8X2 * mgr = (MD5_MB_MGR_X8X2*)malloc(sizeof(MD5_MB_MGR_X8X2));
  int idx = 0;
  if (!cast_array(args[idx++], mgr->args.digest, NUM_MD5_DIGEST_WORDS*NUM_MD5_LANES_X8X2, AT_UINT32)) {
    NanThrowTypeError("Failed to parse array digest[NUM_MD5_DIGEST_WORDS*NUM_MD5_LANES]");
    free(mgr);
    NanReturnUndefined();
  }
  Local<Array> arr = Local<Array>::Cast(args[idx++]);
  if (arr.IsEmpty() && arr->Length() < NUM_MD5_LANES_X8X2) {
    free(mgr);
    NanReturnUndefined();
  }
  for (int i = 0; i < NUM_MD5_LANES_X8X2; i++) {
    mgr->args.data_ptr[i] = (UINT8*)(node::Buffer::Data(arr->Get(i)->ToObject()));
  }
  if (!cast_array(args[idx++], mgr->lens, NUM_MD5_LANES_X8X2, AT_UINT32)) {
    NanThrowTypeError("Failed to parse array len[NUM_MD5_LANES]");
    free(mgr);
    NanReturnUndefined();
  }
  mgr->unused_lanes = (UINT64)args[idx++]->Int32Value();
  Local<Array> arr2 = Local<Array>::Cast(args[idx++]);
  if (arr2.IsEmpty() || arr2->Length() < NUM_MD5_LANES) {
    NanThrowTypeError("Failed to parse array ldata[NUM_MD5_LANES]");
    free(mgr);
    NanReturnUndefined();
  }
  for (int i = 0; i < NUM_MD5_LANES; i++) {
    MD5_HMAC_LANE_DATA* pData = (MD5_HMAC_LANE_DATA*)NanGetInternalFieldPointer(arr2->Get(i)->ToObject(), 0);
    if (!pData) {
      NanThrowTypeError("Failed to parse array ldata[NUM_MD5_LANES] because of invalid MD5_HMAC_LANE_DATA");
      free(mgr);
      NanReturnUndefined();
    }
    memcpy(mgr->ldata[i].extra_block, pData->extra_block, 2*64+8);
    mgr->ldata[i].job_in_lane = pData->job_in_lane;
    mgr->ldata[i].extra_blocks = pData->extra_blocks;
    mgr->ldata[i].size_offset = pData->size_offset;
    mgr->ldata[i].start_offset = pData->start_offset;
  }
  mgr->num_lanes_inuse = (UINT32)args[idx++]->Int32Value();

  Local<ObjectTemplate> tpl = ObjectTemplate::New();
  tpl->SetInternalFieldCount(1);
  Local<Object> retObj = NanNew(tpl)->NewInstance();
  NanSetInternalFieldPointer(retObj, 0, mgr);
  NanReturnValue(retObj);
}

NAN_METHOD(release_MD5_MB_MGR_X8X2) {
  NanScope();
  if (args.Length() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  MD5_MB_MGR_X8X2* mgr = (MD5_MB_MGR_X8X2*)NanGetInternalFieldPointer(args[0]->ToObject(), 0);
  free(mgr);
  NanSetInternalFieldPointer(args[0]->ToObject(), 0, NULL);
  NanReturnValue(NanTrue());
}


/***
* mb_sh1.h
*/

NAN_METHOD(create_JOB_SHA1) {
  NanScope();
  if (args.Length() != 7) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  int idx = 0;
  UINT8 *buffer = (UINT8*)(node::Buffer::Data(args[idx++]->ToObject()));
  UINT32 len = (UINT32)args[idx++]->Int32Value();
  UINT32 len_total = (UINT32)args[idx++]->Int32Value();
  UINT32 result_digest[NUM_SHA1_DIGEST_WORDS];
  if (!cast_array(args[idx++], result_digest, NUM_SHA1_DIGEST_WORDS, AT_UINT32)) {
    NanThrowTypeError("Failed to parse array result_digest[NUM_SHA1_DIGEST_WORDS]");
    NanReturnUndefined();
  }
  UINT32 status = (UINT32)args[idx++]->Int32Value();
  UINT32 flags = (UINT32)args[idx++]->Int32Value();
  void *user_data = (void*)(node::Buffer::Data(args[idx++]->ToObject()));
  JOB_SHA1 * job_sha1 = (JOB_SHA1*)malloc(sizeof(JOB_SHA1));
  job_sha1->buffer = buffer;
  job_sha1->len = len;
  job_sha1->len_total = len_total;
  memcpy(job_sha1->result_digest, result_digest, NUM_SHA1_DIGEST_WORDS * 4);
  job_sha1->status = (JOB_STS)status;
  job_sha1->flags = flags;
  job_sha1->user_data = user_data;

  Local<ObjectTemplate> tpl = ObjectTemplate::New();
  tpl->SetInternalFieldCount(1);
  Local<Object> retObj = NanNew(tpl)->NewInstance();
  NanSetInternalFieldPointer(retObj, 0, job_sha1);
  NanReturnValue(retObj);
}

NAN_METHOD(release_JOB_SHA1) {
  NanScope();
  if (args.Length() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  Local<Object> jsObj = args[0]->ToObject();
  if (jsObj.IsEmpty() || jsObj->InternalFieldCount() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  JOB_SHA1 *job_sha1 = (JOB_SHA1*)NanGetInternalFieldPointer(jsObj, 0);
  free(job_sha1);
  NanSetInternalFieldPointer(jsObj, 0, NULL);
  NanReturnValue(NanTrue());
}

NAN_METHOD(create_SHA1_HMAC_LANE_DATA) {
  NanScope();
  if (args.Length() != 5) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  int idx = 0;
  UINT8 *extra_block = (UINT8*)(node::Buffer::Data(args[idx++]->ToObject()));
  JOB_SHA1 *job_in_lane = (JOB_SHA1*)NanGetInternalFieldPointer(args[idx++]->ToObject(), 0);
  UINT32 extra_blocks = (UINT32)args[idx++]->Int32Value();
  UINT32 size_offset = (UINT32)args[idx++]->Int32Value();
  UINT32 start_offset = (UINT32)args[idx++]->Int32Value();
  SHA1_HMAC_LANE_DATA *data = (SHA1_HMAC_LANE_DATA*)malloc(sizeof(SHA1_HMAC_LANE_DATA));
  memcpy(data->extra_block, extra_block, 2*64+8);
  data->job_in_lane = job_in_lane;
  data->extra_blocks = extra_blocks;
  data->size_offset = size_offset;
  data->start_offset = start_offset;

  Local<ObjectTemplate> tpl = ObjectTemplate::New();
  tpl->SetInternalFieldCount(1);
  Local<Object> retObj = NanNew(tpl)->NewInstance();
  NanSetInternalFieldPointer(retObj, 0, data);
  NanReturnValue(retObj);
}

NAN_METHOD(release_SHA1_HMAC_LANE_DATA) {
  NanScope();
  if (args.Length() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  SHA1_HMAC_LANE_DATA* data = (SHA1_HMAC_LANE_DATA*)NanGetInternalFieldPointer(args[0]->ToObject(), 0);
  free(data);
  NanSetInternalFieldPointer(args[0]->ToObject(), 0, NULL);
  NanReturnValue(NanTrue());
}

NAN_METHOD(create_SHA1_MB_MGR) {
  NanScope();
  if (args.Length() != 5) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  SHA1_MB_MGR * mgr = (SHA1_MB_MGR*)malloc(sizeof(SHA1_MB_MGR));
  int idx = 0;
  if (!cast_array(args[idx++], mgr->args.digest, NUM_SHA1_DIGEST_WORDS*NUM_SHA1_LANES_X8, AT_UINT32)) {
    NanThrowTypeError("Failed to parse array digest[NUM_SHA1_DIGEST_WORDS*NUM_SHA1_LANES_X8]");
    free(mgr);
    NanReturnUndefined();
  }
  Local<Array> arr = Local<Array>::Cast(args[idx++]);
  if (arr.IsEmpty() && arr->Length() < NUM_SHA1_LANES_X8) {
    free(mgr);
    NanReturnUndefined();
  }
  for (int i = 0; i < NUM_SHA1_LANES_X8; i++) {
    mgr->args.data_ptr[i] = (UINT8*)(node::Buffer::Data(arr->Get(i)->ToObject()));
  }
  if (!cast_array(args[idx++], mgr->lens, NUM_SHA1_LANES, AT_UINT64)) {
    NanThrowTypeError("Failed to parse array len[NUM_SHA1_LANES]");
    free(mgr);
    NanReturnUndefined();
  }
  mgr->unused_lanes = (UINT64)args[idx++]->Int32Value();
  Local<Array> arr2 = Local<Array>::Cast(args[idx++]);
  if (arr2.IsEmpty() || arr2->Length() < NUM_SHA1_LANES) {
    NanThrowTypeError("Failed to parse array ldata[NUM_SHA1_LANES]");
    free(mgr);
    NanReturnUndefined();
  }
  for (int i = 0; i < NUM_SHA1_LANES; i++) {
    SHA1_HMAC_LANE_DATA* pData = (SHA1_HMAC_LANE_DATA*)NanGetInternalFieldPointer(arr2->Get(i)->ToObject(), 0);
    if (!pData) {
      NanThrowTypeError("Failed to parse array ldata[NUM_SHA1_LANES] because of invalid SHA1_HMAC_LANE_DATA");
      free(mgr);
      NanReturnUndefined();
    }
    memcpy(mgr->ldata[i].extra_block, pData->extra_block, 2*64+8);
    mgr->ldata[i].job_in_lane = pData->job_in_lane;
    mgr->ldata[i].extra_blocks = pData->extra_blocks;
    mgr->ldata[i].size_offset = pData->size_offset;
    mgr->ldata[i].start_offset = pData->start_offset;
  }

  Local<ObjectTemplate> tpl = ObjectTemplate::New();
  tpl->SetInternalFieldCount(1);
  Local<Object> retObj = NanNew(tpl)->NewInstance();
  NanSetInternalFieldPointer(retObj, 0, mgr);
  NanReturnValue(retObj);
}

NAN_METHOD(release_SHA1_MB_MGR) {
  NanScope();
  if (args.Length() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  SHA1_MB_MGR* mgr = (SHA1_MB_MGR*)NanGetInternalFieldPointer(args[0]->ToObject(), 0);
  free(mgr);
  NanSetInternalFieldPointer(args[0]->ToObject(), 0, NULL);
  NanReturnValue(NanTrue());
}

NAN_METHOD(create_SHA1_MB_MGR_X8) {
  NanScope();
  if (args.Length() != 5) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  SHA1_MB_MGR_X8 * mgr = (SHA1_MB_MGR_X8*)malloc(sizeof(SHA1_MB_MGR_X8));
  int idx = 0;
  if (!cast_array(args[idx++], mgr->args.digest, NUM_SHA1_DIGEST_WORDS*NUM_SHA1_LANES_X8, AT_UINT32)) {
    NanThrowTypeError("Failed to parse array digest[NUM_SHA1_DIGEST_WORDS*NUM_SHA1_LANES_X8]");
    free(mgr);
    NanReturnUndefined();
  }
  Local<Array> arr = Local<Array>::Cast(args[idx++]);
  if (arr.IsEmpty() && arr->Length() < NUM_SHA1_LANES_X8) {
    free(mgr);
    NanReturnUndefined();
  }
  for (int i = 0; i < NUM_SHA1_LANES_X8; i++) {
    mgr->args.data_ptr[i] = (UINT8*)(node::Buffer::Data(arr->Get(i)->ToObject()));
  }
  if (!cast_array(args[idx++], mgr->lens, NUM_SHA1_LANES_X8, AT_UINT32)) {
    NanThrowTypeError("Failed to parse array len[NUM_SHA1_LANES_X8]");
    free(mgr);
    NanReturnUndefined();
  }
  mgr->unused_lanes = (UINT64)args[idx++]->Int32Value();
  Local<Array> arr2 = Local<Array>::Cast(args[idx++]);
  if (arr2.IsEmpty() || arr2->Length() < NUM_SHA1_LANES_X8) {
    NanThrowTypeError("Failed to parse array ldata[NUM_SHA1_LANES_X8]");
    free(mgr);
    NanReturnUndefined();
  }
  for (int i = 0; i < NUM_SHA1_LANES_X8; i++) {
    SHA1_HMAC_LANE_DATA* pData = (SHA1_HMAC_LANE_DATA*)NanGetInternalFieldPointer(arr2->Get(i)->ToObject(), 0);
    if (!pData) {
      NanThrowTypeError("Failed to parse array ldata[NUM_SHA1_LANES_X8] because of invalid SHA1_HMAC_LANE_DATA");
      free(mgr);
      NanReturnUndefined();
    }
    memcpy(mgr->ldata[i].extra_block, pData->extra_block, 2*64+8);
    mgr->ldata[i].job_in_lane = pData->job_in_lane;
    mgr->ldata[i].extra_blocks = pData->extra_blocks;
    mgr->ldata[i].size_offset = pData->size_offset;
    mgr->ldata[i].start_offset = pData->start_offset;
  }

  Local<ObjectTemplate> tpl = ObjectTemplate::New();
  tpl->SetInternalFieldCount(1);
  Local<Object> retObj = NanNew(tpl)->NewInstance();
  NanSetInternalFieldPointer(retObj, 0, mgr);
  NanReturnValue(retObj);
}

NAN_METHOD(release_SHA1_MB_MGR_X8) {
  NanScope();
  if (args.Length() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  SHA1_MB_MGR_X8* mgr = (SHA1_MB_MGR_X8*)NanGetInternalFieldPointer(args[0]->ToObject(), 0);
  free(mgr);
  NanSetInternalFieldPointer(args[0]->ToObject(), 0, NULL);
  NanReturnValue(NanTrue());
}

/**
* mb_sha256.h
*/

NAN_METHOD(create_JOB_SHA256) {
  NanScope();
  if (args.Length() != 7) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  int idx = 0;
  UINT8 *buffer = (UINT8*)(node::Buffer::Data(args[idx++]->ToObject()));
  UINT32 len = (UINT32)args[idx++]->Int32Value();
  UINT32 len_total = (UINT32)args[idx++]->Int32Value();
  UINT32 result_digest[NUM_SHA256_DIGEST_WORDS];
  if (!cast_array(args[idx++], result_digest, NUM_SHA256_DIGEST_WORDS, AT_UINT32)) {
    NanThrowTypeError("Failed to parse array result_digest[NUM_SHA256_DIGEST_WORDS]");
    NanReturnUndefined();
  }
  UINT32 status = (UINT32)args[idx++]->Int32Value();
  UINT32 flags = (UINT32)args[idx++]->Int32Value();
  void *user_data = (void*)(node::Buffer::Data(args[idx++]->ToObject()));
  JOB_SHA256 * job_sha256 = (JOB_SHA256*)malloc(sizeof(JOB_SHA256));
  job_sha256->buffer = buffer;
  job_sha256->len = len;
  job_sha256->len_total = len_total;
  memcpy(job_sha256->result_digest, result_digest, NUM_SHA256_DIGEST_WORDS * 4);
  job_sha256->status = (JOB_STS)status;
  job_sha256->flags = flags;
  job_sha256->user_data = user_data;

  Local<ObjectTemplate> tpl = ObjectTemplate::New();
  tpl->SetInternalFieldCount(1);
  Local<Object> retObj = NanNew(tpl)->NewInstance();
  NanSetInternalFieldPointer(retObj, 0, job_sha256);
  NanReturnValue(retObj);
}

NAN_METHOD(release_JOB_SHA256) {
  NanScope();
  if (args.Length() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  Local<Object> jsObj = args[0]->ToObject();
  if (jsObj.IsEmpty() || jsObj->InternalFieldCount() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  JOB_SHA256 *job_sha256 = (JOB_SHA256*)NanGetInternalFieldPointer(jsObj, 0);
  free(job_sha256);
  NanSetInternalFieldPointer(jsObj, 0, NULL);
  NanReturnValue(NanTrue());
}

NAN_METHOD(create_SHA256_HMAC_LANE_DATA) {
  NanScope();
  if (args.Length() != 6) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  int idx = 0;
  UINT8 *extra_block = (UINT8*)(node::Buffer::Data(args[idx++]->ToObject()));
  JOB_SHA256 *job_in_lane = (JOB_SHA256*)NanGetInternalFieldPointer(args[idx++]->ToObject(), 0);
  UINT32 extra_blocks = (UINT32)args[idx++]->Int32Value();
  UINT32 size_offset = (UINT32)args[idx++]->Int32Value();
  UINT32 start_offset = (UINT32)args[idx++]->Int32Value();
  UINT32 padding = (UINT32)args[idx++]->Int32Value();
  SHA256_HMAC_LANE_DATA *data = (SHA256_HMAC_LANE_DATA*)malloc(sizeof(SHA256_HMAC_LANE_DATA));
  memcpy(data->extra_block, extra_block, 2*64+8);
  data->job_in_lane = job_in_lane;
  data->extra_blocks = extra_blocks;
  data->size_offset = size_offset;
  data->start_offset = start_offset;
  data->padding = padding;

  Local<ObjectTemplate> tpl = ObjectTemplate::New();
  tpl->SetInternalFieldCount(1);
  Local<Object> retObj = NanNew(tpl)->NewInstance();
  NanSetInternalFieldPointer(retObj, 0, data);
  NanReturnValue(retObj);
}

NAN_METHOD(release_SHA256_HMAC_LANE_DATA) {
  NanScope();
  if (args.Length() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  SHA256_HMAC_LANE_DATA* data = (SHA256_HMAC_LANE_DATA*)NanGetInternalFieldPointer(args[0]->ToObject(), 0);
  free(data);
  NanSetInternalFieldPointer(args[0]->ToObject(), 0, NULL);
  NanReturnValue(NanTrue());
}

NAN_METHOD(create_SHA256_MB_MGR) {
  NanScope();
  if (args.Length() != 5) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  SHA256_MB_MGR * mgr = (SHA256_MB_MGR*)malloc(sizeof(SHA256_MB_MGR));
  int idx = 0;
  if (!cast_array(args[idx++], mgr->args.digest, NUM_SHA256_DIGEST_WORDS*NUM_SHA256_LANES, AT_UINT32)) {
    NanThrowTypeError("Failed to parse array digest[NUM_SHA256_DIGEST_WORDS*NUM_SHA256_LANES]");
    free(mgr);
    NanReturnUndefined();
  }
  Local<Array> arr = Local<Array>::Cast(args[idx++]);
  if (arr.IsEmpty() && arr->Length() < NUM_SHA256_LANES) {
    free(mgr);
    NanReturnUndefined();
  }
  for (int i = 0; i < NUM_SHA256_LANES; i++) {
    mgr->args.data_ptr[i] = (UINT8*)(node::Buffer::Data(arr->Get(i)->ToObject()));
  }
  if (!cast_array(args[idx++], mgr->lens, NUM_SHA256_LANES, AT_UINT64)) {
    NanThrowTypeError("Failed to parse array len[NUM_SHA256_LANES]");
    free(mgr);
    NanReturnUndefined();
  }
  mgr->unused_lanes = (UINT64)args[idx++]->Int32Value();
  Local<Array> arr2 = Local<Array>::Cast(args[idx++]);
  if (arr2.IsEmpty() || arr2->Length() < NUM_SHA256_LANES) {
    NanThrowTypeError("Failed to parse array ldata[NUM_SHA256_LANES]");
    free(mgr);
    NanReturnUndefined();
  }
  for (int i = 0; i < NUM_SHA256_LANES; i++) {
    SHA256_HMAC_LANE_DATA* pData = (SHA256_HMAC_LANE_DATA*)NanGetInternalFieldPointer(arr2->Get(i)->ToObject(), 0);
    if (!pData) {
      NanThrowTypeError("Failed to parse array ldata[NUM_SHA256_LANES] because of invalid SHA256_HMAC_LANE_DATA");
      free(mgr);
      NanReturnUndefined();
    }
    memcpy(mgr->ldata[i].extra_block, pData->extra_block, 2*64+8);
    mgr->ldata[i].job_in_lane = pData->job_in_lane;
    mgr->ldata[i].extra_blocks = pData->extra_blocks;
    mgr->ldata[i].size_offset = pData->size_offset;
    mgr->ldata[i].start_offset = pData->start_offset;
  }

  Local<ObjectTemplate> tpl = ObjectTemplate::New();
  tpl->SetInternalFieldCount(1);
  Local<Object> retObj = NanNew(tpl)->NewInstance();
  NanSetInternalFieldPointer(retObj, 0, mgr);
  NanReturnValue(retObj);
}

NAN_METHOD(release_SHA256_MB_MGR) {
  NanScope();
  if (args.Length() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  SHA256_MB_MGR* mgr = (SHA256_MB_MGR*)NanGetInternalFieldPointer(args[0]->ToObject(), 0);
  free(mgr);
  NanSetInternalFieldPointer(args[0]->ToObject(), 0, NULL);
  NanReturnValue(NanTrue());
}

NAN_METHOD(create_SHA256_MB_MGR_X8) {
  NanScope();
  if (args.Length() != 5) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  SHA256_MB_MGR_X8 * mgr = (SHA256_MB_MGR_X8*)malloc(sizeof(SHA256_MB_MGR_X8));
  int idx = 0;
  if (!cast_array(args[idx++], mgr->args.digest, NUM_SHA256_DIGEST_WORDS*NUM_SHA256_LANES_X8, AT_UINT32)) {
    NanThrowTypeError("Failed to parse array digest[NUM_SHA256_DIGEST_WORDS*NUM_SHA256_LANES_X8]");
    free(mgr);
    NanReturnUndefined();
  }
  Local<Array> arr = Local<Array>::Cast(args[idx++]);
  if (arr.IsEmpty() && arr->Length() < NUM_SHA256_LANES_X8) {
    free(mgr);
    NanReturnUndefined();
  }
  for (int i = 0; i < NUM_SHA256_LANES_X8; i++) {
    mgr->args.data_ptr[i] = (UINT8*)(node::Buffer::Data(arr->Get(i)->ToObject()));
  }
  if (!cast_array(args[idx++], mgr->lens, NUM_SHA256_LANES_X8, AT_UINT32)) {
    NanThrowTypeError("Failed to parse array len[NUM_SHA256_LANES_X8]");
    free(mgr);
    NanReturnUndefined();
  }
  mgr->unused_lanes = (UINT64)args[idx++]->Int32Value();
  Local<Array> arr2 = Local<Array>::Cast(args[idx++]);
  if (arr2.IsEmpty() || arr2->Length() < NUM_SHA256_LANES_X8) {
    NanThrowTypeError("Failed to parse array ldata[NUM_SHA256_LANES_X8]");
    free(mgr);
    NanReturnUndefined();
  }
  for (int i = 0; i < NUM_SHA256_LANES_X8; i++) {
    SHA256_HMAC_LANE_DATA* pData = (SHA256_HMAC_LANE_DATA*)NanGetInternalFieldPointer(arr2->Get(i)->ToObject(), 0);
    if (!pData) {
      NanThrowTypeError("Failed to parse array ldata[NUM_SHA256_LANES_X8] because of invalid SHA256_HMAC_LANE_DATA");
      free(mgr);
      NanReturnUndefined();
    }
    memcpy(mgr->ldata[i].extra_block, pData->extra_block, 2*64+8);
    mgr->ldata[i].job_in_lane = pData->job_in_lane;
    mgr->ldata[i].extra_blocks = pData->extra_blocks;
    mgr->ldata[i].size_offset = pData->size_offset;
    mgr->ldata[i].start_offset = pData->start_offset;
  }

  Local<ObjectTemplate> tpl = ObjectTemplate::New();
  tpl->SetInternalFieldCount(1);
  Local<Object> retObj = NanNew(tpl)->NewInstance();
  NanSetInternalFieldPointer(retObj, 0, mgr);
  NanReturnValue(retObj);
}

NAN_METHOD(release_SHA256_MB_MGR_X8) {
  NanScope();
  if (args.Length() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  SHA256_MB_MGR_X8* mgr = (SHA256_MB_MGR_X8*)NanGetInternalFieldPointer(args[0]->ToObject(), 0);
  free(mgr);
  NanSetInternalFieldPointer(args[0]->ToObject(), 0, NULL);
  NanReturnValue(NanTrue());
}

/**
* mb_sha512.h
*/

NAN_METHOD(create_JOB_SHA512) {
  NanScope();
  if (args.Length() != 7) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  int idx = 0;
  UINT8 *buffer = (UINT8*)(node::Buffer::Data(args[idx++]->ToObject()));
  UINT32 len = (UINT32)args[idx++]->Int32Value();
  UINT32 len_total = (UINT32)args[idx++]->Int32Value();
  UINT32 result_digest[NUM_SHA512_DIGEST_WORDS];
  if (!cast_array(args[idx++], result_digest, NUM_SHA512_DIGEST_WORDS, AT_UINT64)) {
    NanThrowTypeError("Failed to parse array result_digest[NUM_SHA512_DIGEST_WORDS]");
    NanReturnUndefined();
  }
  UINT32 status = (UINT32)args[idx++]->Int32Value();
  UINT32 flags = (UINT32)args[idx++]->Int32Value();
  void *user_data = (void*)(node::Buffer::Data(args[idx++]->ToObject()));
  JOB_SHA512 * job_sha512 = (JOB_SHA512*)malloc(sizeof(JOB_SHA512));
  job_sha512->buffer = buffer;
  job_sha512->len = len;
  job_sha512->len_total = len_total;
  memcpy(job_sha512->result_digest, result_digest, NUM_SHA512_DIGEST_WORDS * 8);
  job_sha512->status = (JOB_STS)status;
  job_sha512->flags = flags;
  job_sha512->user_data = user_data;

  Local<ObjectTemplate> tpl = ObjectTemplate::New();
  tpl->SetInternalFieldCount(1);
  Local<Object> retObj = NanNew(tpl)->NewInstance();
  NanSetInternalFieldPointer(retObj, 0, job_sha512);
  NanReturnValue(retObj);
}

NAN_METHOD(release_JOB_SHA512) {
  NanScope();
  if (args.Length() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  Local<Object> jsObj = args[0]->ToObject();
  if (jsObj.IsEmpty() || jsObj->InternalFieldCount() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  JOB_SHA512 *job_sha512 = (JOB_SHA512*)NanGetInternalFieldPointer(jsObj, 0);
  free(job_sha512);
  NanSetInternalFieldPointer(jsObj, 0, NULL);
  NanReturnValue(NanTrue());
}

NAN_METHOD(create_SHA512_HMAC_LANE_DATA) {
  NanScope();
  if (args.Length() != 6) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  int idx = 0;
  UINT8 *extra_block = (UINT8*)(node::Buffer::Data(args[idx++]->ToObject()));
  JOB_SHA512 *job_in_lane = (JOB_SHA512*)NanGetInternalFieldPointer(args[idx++]->ToObject(), 0);
  UINT32 extra_blocks = (UINT32)args[idx++]->Int32Value();
  UINT32 size_offset = (UINT32)args[idx++]->Int32Value();
  UINT32 start_offset = (UINT32)args[idx++]->Int32Value();
  UINT32 padding = (UINT32)args[idx++]->Int32Value();
  SHA512_HMAC_LANE_DATA *data = (SHA512_HMAC_LANE_DATA*)malloc(sizeof(SHA512_HMAC_LANE_DATA));
  memcpy(data->extra_block, extra_block, 2*SHA512_BLOCK_SIZE+16);
  data->job_in_lane = job_in_lane;
  data->extra_blocks = extra_blocks;
  data->size_offset = size_offset;
  data->start_offset = start_offset;
  data->padding = padding;

  Local<ObjectTemplate> tpl = ObjectTemplate::New();
  tpl->SetInternalFieldCount(1);
  Local<Object> retObj = NanNew(tpl)->NewInstance();
  NanSetInternalFieldPointer(retObj, 0, data);
  NanReturnValue(retObj);
}

NAN_METHOD(release_SHA512_HMAC_LANE_DATA) {
  NanScope();
  if (args.Length() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  SHA512_HMAC_LANE_DATA* data = (SHA512_HMAC_LANE_DATA*)NanGetInternalFieldPointer(args[0]->ToObject(), 0);
  free(data);
  NanSetInternalFieldPointer(args[0]->ToObject(), 0, NULL);
  NanReturnValue(NanTrue());
}

NAN_METHOD(create_SHA512_MB_MGR) {
  NanScope();
  if (args.Length() != 5) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  SHA512_MB_MGR * mgr = (SHA512_MB_MGR*)malloc(sizeof(SHA512_MB_MGR));
  int idx = 0;
  if (!cast_array(args[idx++], mgr->args.digest, NUM_SHA512_DIGEST_WORDS*NUM_SHA512_LANES, AT_UINT64)) {
    NanThrowTypeError("Failed to parse array digest[NUM_SHA512_DIGEST_WORDS*NUM_SHA512_LANES]");
    free(mgr);
    NanReturnUndefined();
  }
  Local<Array> arr = Local<Array>::Cast(args[idx++]);
  if (arr.IsEmpty() && arr->Length() < NUM_SHA512_LANES) {
    free(mgr);
    NanReturnUndefined();
  }
  for (int i = 0; i < NUM_SHA512_LANES; i++) {
    mgr->args.data_ptr[i] = (UINT8*)(node::Buffer::Data(arr->Get(i)->ToObject()));
  }
  if (!cast_array(args[idx++], mgr->lens, NUM_SHA512_LANES, AT_UINT64)) {
    NanThrowTypeError("Failed to parse array len[NUM_SHA512_LANES]");
    free(mgr);
    NanReturnUndefined();
  }
  mgr->unused_lanes = (UINT64)args[idx++]->Int32Value();
  Local<Array> arr2 = Local<Array>::Cast(args[idx++]);
  if (arr2.IsEmpty() || arr2->Length() < NUM_SHA512_LANES) {
    NanThrowTypeError("Failed to parse array ldata[NUM_SHA512_LANES]");
    free(mgr);
    NanReturnUndefined();
  }
  for (int i = 0; i < NUM_SHA512_LANES; i++) {
    SHA512_HMAC_LANE_DATA* pData = (SHA512_HMAC_LANE_DATA*)NanGetInternalFieldPointer(arr2->Get(i)->ToObject(), 0);
    if (!pData) {
      NanThrowTypeError("Failed to parse array ldata[NUM_SHA512_LANES] because of invalid SHA512_HMAC_LANE_DATA");
      free(mgr);
      NanReturnUndefined();
    }
    memcpy(mgr->ldata[i].extra_block, pData->extra_block, 2*SHA512_BLOCK_SIZE+16);
    mgr->ldata[i].job_in_lane = pData->job_in_lane;
    mgr->ldata[i].extra_blocks = pData->extra_blocks;
    mgr->ldata[i].size_offset = pData->size_offset;
    mgr->ldata[i].start_offset = pData->start_offset;
  }

  Local<ObjectTemplate> tpl = ObjectTemplate::New();
  tpl->SetInternalFieldCount(1);
  Local<Object> retObj = NanNew(tpl)->NewInstance();
  NanSetInternalFieldPointer(retObj, 0, mgr);
  NanReturnValue(retObj);
}

NAN_METHOD(release_SHA512_MB_MGR) {
  NanScope();
  if (args.Length() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  SHA512_MB_MGR* mgr = (SHA512_MB_MGR*)NanGetInternalFieldPointer(args[0]->ToObject(), 0);
  free(mgr);
  NanSetInternalFieldPointer(args[0]->ToObject(), 0, NULL);
  NanReturnValue(NanTrue());
}

NAN_METHOD(create_SHA512_MB_MGR_X4) {
  NanScope();
  if (args.Length() != 5) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  SHA512_MB_MGR_X4 * mgr = (SHA512_MB_MGR_X4*)malloc(sizeof(SHA512_MB_MGR_X4));
  int idx = 0;
  if (!cast_array(args[idx++], mgr->args.digest, NUM_SHA512_DIGEST_WORDS*NUM_SHA512_LANES_X4, AT_UINT64)) {
    NanThrowTypeError("Failed to parse array digest[NUM_SHA512_DIGEST_WORDS*NUM_SHA512_LANES_X4]");
    free(mgr);
    NanReturnUndefined();
  }
  Local<Array> arr = Local<Array>::Cast(args[idx++]);
  if (arr.IsEmpty() && arr->Length() < NUM_SHA512_LANES_X4) {
    free(mgr);
    NanReturnUndefined();
  }
  for (int i = 0; i < NUM_SHA512_LANES_X4; i++) {
    mgr->args.data_ptr[i] = (UINT8*)(node::Buffer::Data(arr->Get(i)->ToObject()));
  }
  if (!cast_array(args[idx++], mgr->lens, NUM_SHA512_LANES_X4, AT_UINT32)) {
    NanThrowTypeError("Failed to parse array len[NUM_SHA512_LANES_X4]");
    free(mgr);
    NanReturnUndefined();
  }
  mgr->unused_lanes = (UINT64)args[idx++]->Int32Value();
  Local<Array> arr2 = Local<Array>::Cast(args[idx++]);
  if (arr2.IsEmpty() || arr2->Length() < NUM_SHA512_LANES_X4) {
    NanThrowTypeError("Failed to parse array ldata[NUM_SHA512_LANES_X4]");
    free(mgr);
    NanReturnUndefined();
  }
  for (int i = 0; i < NUM_SHA512_LANES_X4; i++) {
    SHA512_HMAC_LANE_DATA* pData = (SHA512_HMAC_LANE_DATA*)NanGetInternalFieldPointer(arr2->Get(i)->ToObject(), 0);
    if (!pData) {
      NanThrowTypeError("Failed to parse array ldata[NUM_SHA512_LANES_X4] because of invalid SHA512_HMAC_LANE_DATA");
      free(mgr);
      NanReturnUndefined();
    }
    memcpy(mgr->ldata[i].extra_block, pData->extra_block, 2*64+8);
    mgr->ldata[i].job_in_lane = pData->job_in_lane;
    mgr->ldata[i].extra_blocks = pData->extra_blocks;
    mgr->ldata[i].size_offset = pData->size_offset;
    mgr->ldata[i].start_offset = pData->start_offset;
  }

  Local<ObjectTemplate> tpl = ObjectTemplate::New();
  tpl->SetInternalFieldCount(1);
  Local<Object> retObj = NanNew(tpl)->NewInstance();
  NanSetInternalFieldPointer(retObj, 0, mgr);
  NanReturnValue(retObj);
}

NAN_METHOD(release_SHA512_MB_MGR_X4) {
  NanScope();
  if (args.Length() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  SHA512_MB_MGR_X4* mgr = (SHA512_MB_MGR_X4*)NanGetInternalFieldPointer(args[0]->ToObject(), 0);
  free(mgr);
  NanSetInternalFieldPointer(args[0]->ToObject(), 0, NULL);
  NanReturnValue(NanTrue());
}


/**
* md5_mb.h
*/

NAN_METHOD(create_MD5_JOB) {
  NanScope();
  if (args.Length() != 5) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  int idx = 0;
  UINT8 *buffer = (UINT8*)(node::Buffer::Data(args[idx++]->ToObject()));
  UINT32 len = (UINT32)args[idx++]->Int32Value();
  UINT32 result_digest[MD5_DIGEST_NWORDS];
  if (!cast_array(args[idx++], result_digest, MD5_DIGEST_NWORDS, AT_UINT32)) {
    NanThrowTypeError("Failed to parse array result_digest[MD5_DIGEST_NWORDS]");
    NanReturnUndefined();
  }
  UINT32 status = (UINT32)args[idx++]->Int32Value();
  void *user_data = (void*)(node::Buffer::Data(args[idx++]->ToObject()));
  MD5_JOB * md5_job = (MD5_JOB*)malloc(sizeof(MD5_JOB));
  md5_job->buffer = buffer;
  md5_job->len = len;
  memcpy(md5_job->result_digest, result_digest, MD5_DIGEST_NWORDS * 4);
  md5_job->status = (JOB_STS)status;
  md5_job->user_data = user_data;

  Local<ObjectTemplate> tpl = ObjectTemplate::New();
  tpl->SetInternalFieldCount(1);
  Local<Object> retObj = NanNew(tpl)->NewInstance();
  NanSetInternalFieldPointer(retObj, 0, md5_job);
  NanReturnValue(retObj);
}

NAN_METHOD(release_MD5_JOB) {
  NanScope();
  if (args.Length() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  Local<Object> jsObj = args[0]->ToObject();
  if (jsObj.IsEmpty() || jsObj->InternalFieldCount() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  MD5_JOB *md5_job = (MD5_JOB*)NanGetInternalFieldPointer(jsObj, 0);
  free(md5_job);
  NanSetInternalFieldPointer(jsObj, 0, NULL);
  NanReturnValue(NanTrue());
}

NAN_METHOD(create_MD5_LANE_DATA) {
  NanScope();
  if (args.Length() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  int idx = 0;
  MD5_JOB *job_in_lane = (MD5_JOB*)NanGetInternalFieldPointer(args[idx++]->ToObject(), 0);
  MD5_LANE_DATA *data = (MD5_LANE_DATA*)malloc(sizeof(MD5_LANE_DATA));
  data->job_in_lane = job_in_lane;
  Local<ObjectTemplate> tpl = ObjectTemplate::New();
  tpl->SetInternalFieldCount(1);
  Local<Object> retObj = NanNew(tpl)->NewInstance();
  NanSetInternalFieldPointer(retObj, 0, data);
  NanReturnValue(retObj);
}

NAN_METHOD(release_MD5_LANE_DATA) {
  NanScope();
  if (args.Length() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  MD5_LANE_DATA* data = (MD5_LANE_DATA*)NanGetInternalFieldPointer(args[0]->ToObject(), 0);
  free(data);
  NanSetInternalFieldPointer(args[0]->ToObject(), 0, NULL);
  NanReturnValue(NanTrue());
}

NAN_METHOD(create_MD5_MB_JOB_MGR) {
  NanScope();
  if (args.Length() != 6) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  MD5_MB_JOB_MGR * mgr = (MD5_MB_JOB_MGR*)malloc(sizeof(MD5_MB_JOB_MGR));
  int idx = 0;
  if (!cast_array(args[idx++], mgr->args.digest, MD5_DIGEST_NWORDS*MD5_MAX_LANES, AT_UINT32)) {
    NanThrowTypeError("Failed to parse array digest[MD5_DIGEST_NWORDS*MD5_MAX_LANES]");
    free(mgr);
    NanReturnUndefined();
  }
  Local<Array> arr = Local<Array>::Cast(args[idx++]);
  if (arr.IsEmpty() && arr->Length() < MD5_MAX_LANES) {
    free(mgr);
    NanReturnUndefined();
  }
  for (int i = 0; i < MD5_MAX_LANES; i++) {
    mgr->args.data_ptr[i] = (UINT8*)(node::Buffer::Data(arr->Get(i)->ToObject()));
  }
  if (!cast_array(args[idx++], mgr->lens, MD5_MAX_LANES, AT_UINT64)) {
    NanThrowTypeError("Failed to parse array len[MD5_MAX_LANES]");
    free(mgr);
    NanReturnUndefined();
  }
  mgr->unused_lanes = (UINT64)args[idx++]->Int32Value();
  Local<Array> arr2 = Local<Array>::Cast(args[idx++]);
  if (arr2.IsEmpty() || arr2->Length() < MD5_MAX_LANES) {
    NanThrowTypeError("Failed to parse array ldata[MD5_MAX_LANES]");
    free(mgr);
    NanReturnUndefined();
  }
  for (int i = 0; i < MD5_MAX_LANES; i++) {
    MD5_LANE_DATA* pData = (MD5_LANE_DATA*)NanGetInternalFieldPointer(arr2->Get(i)->ToObject(), 0);
    if (!pData) {
      NanThrowTypeError("Failed to parse array ldata[MD5_MAX_LANES] because of invalid MD5_LANE_DATA");
      free(mgr);
      NanReturnUndefined();
    }
    mgr->ldata[i].job_in_lane = pData->job_in_lane;
  }
  mgr->num_lanes_inuse = (uint32_t)args[idx++]->Int32Value();

  Local<ObjectTemplate> tpl = ObjectTemplate::New();
  tpl->SetInternalFieldCount(1);
  Local<Object> retObj = NanNew(tpl)->NewInstance();
  NanSetInternalFieldPointer(retObj, 0, mgr);
  NanReturnValue(retObj);
}

NAN_METHOD(release_MD5_MB_JOB_MGR) {
  NanScope();
  if (args.Length() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  MD5_MB_JOB_MGR* mgr = (MD5_MB_JOB_MGR*)NanGetInternalFieldPointer(args[0]->ToObject(), 0);
  free(mgr);
  NanSetInternalFieldPointer(args[0]->ToObject(), 0, NULL);
  NanReturnValue(NanTrue());
}

NAN_METHOD(create_MD5_HASH_CTX_MGR) {
  NanScope();
  if (args.Length() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  MD5_HASH_CTX_MGR * mgr = (MD5_HASH_CTX_MGR*)malloc(sizeof(MD5_HASH_CTX_MGR));
  int idx = 0;
  MD5_MB_JOB_MGR* pMgr = (MD5_MB_JOB_MGR*)NanGetInternalFieldPointer(args[idx++]->ToObject(), 0);
  mgr->mgr = *pMgr;

  Local<ObjectTemplate> tpl = ObjectTemplate::New();
  tpl->SetInternalFieldCount(1);
  Local<Object> retObj = NanNew(tpl)->NewInstance();
  NanSetInternalFieldPointer(retObj, 0, mgr);
  NanReturnValue(retObj);
}

NAN_METHOD(release_MD5_HASH_CTX_MGR) {
  NanScope();
  if (args.Length() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  MD5_HASH_CTX_MGR* mgr = (MD5_HASH_CTX_MGR*)NanGetInternalFieldPointer(args[0]->ToObject(), 0);
  free(mgr);
  NanSetInternalFieldPointer(args[0]->ToObject(), 0, NULL);
  NanReturnValue(NanTrue());
}

NAN_METHOD(create_MD5_HASH_CTX) {
  NanScope();
  if (args.Length() != 9) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  MD5_HASH_CTX * mgr = (MD5_HASH_CTX*)malloc(sizeof(MD5_HASH_CTX));
  int idx = 0;
  MD5_JOB* pJob = (MD5_JOB*)NanGetInternalFieldPointer(args[idx++]->ToObject(), 0);
  mgr->job = *pJob;
  mgr->status = (HASH_CTX_STS)args[idx++]->Int32Value();
  mgr->error = (HASH_CTX_ERROR)args[idx++]->Int32Value();
  mgr->total_length = (uint32_t)args[idx++]->Int32Value();
  mgr->incoming_buffer = (const void*)(node::Buffer::Data(args[idx++]->ToObject()));
  mgr->incoming_buffer_length = (uint32_t)args[idx++]->Int32Value();
  if (!cast_array(args[idx++], mgr->partial_block_buffer, MD5_BLOCK_SIZE*2, AT_UINT8)) {
    NanThrowTypeError("Failed to parse array partial_block_buffer[MD5_BLOCK_SIZE*2]");
    free(mgr);
    NanReturnUndefined();
  }
  mgr->partial_block_buffer_length = (uint32_t)args[idx++]->Int32Value();
  mgr->user_data = (void*)(node::Buffer::Data(args[idx++]->ToObject()));


  Local<ObjectTemplate> tpl = ObjectTemplate::New();
  tpl->SetInternalFieldCount(1);
  Local<Object> retObj = NanNew(tpl)->NewInstance();
  NanSetInternalFieldPointer(retObj, 0, mgr);
  NanReturnValue(retObj);
}

NAN_METHOD(release_MD5_HASH_CTX) {
  NanScope();
  if (args.Length() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  MD5_HASH_CTX* mgr = (MD5_HASH_CTX*)NanGetInternalFieldPointer(args[0]->ToObject(), 0);
  free(mgr);
  NanSetInternalFieldPointer(args[0]->ToObject(), 0, NULL);
  NanReturnValue(NanTrue());
}


/***
* sh1_mb.h
*/

NAN_METHOD(create_SHA1_JOB) {
  NanScope();
  if (args.Length() != 5) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  int idx = 0;
  UINT8 *buffer = (UINT8*)(node::Buffer::Data(args[idx++]->ToObject()));
  UINT32 len = (UINT32)args[idx++]->Int32Value();
  UINT32 result_digest[SHA1_DIGEST_NWORDS];
  if (!cast_array(args[idx++], result_digest, SHA1_DIGEST_NWORDS, AT_UINT32)) {
    NanThrowTypeError("Failed to parse array result_digest[SHA1_DIGEST_NWORDS]");
    NanReturnUndefined();
  }
  UINT32 status = (UINT32)args[idx++]->Int32Value();
  void *user_data = (void*)(node::Buffer::Data(args[idx++]->ToObject()));
  SHA1_JOB * sha1_job = (SHA1_JOB*)malloc(sizeof(SHA1_JOB));
  sha1_job->buffer = buffer;
  sha1_job->len = len;
  memcpy(sha1_job->result_digest, result_digest, SHA1_DIGEST_NWORDS * 4);
  sha1_job->status = (JOB_STS)status;
  sha1_job->user_data = user_data;

  Local<ObjectTemplate> tpl = ObjectTemplate::New();
  tpl->SetInternalFieldCount(1);
  Local<Object> retObj = NanNew(tpl)->NewInstance();
  NanSetInternalFieldPointer(retObj, 0, sha1_job);
  NanReturnValue(retObj);
}

NAN_METHOD(release_SHA1_JOB) {
  NanScope();
  if (args.Length() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  Local<Object> jsObj = args[0]->ToObject();
  if (jsObj.IsEmpty() || jsObj->InternalFieldCount() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  SHA1_JOB *sha1_job = (SHA1_JOB*)NanGetInternalFieldPointer(jsObj, 0);
  free(sha1_job);
  NanSetInternalFieldPointer(jsObj, 0, NULL);
  NanReturnValue(NanTrue());
}

NAN_METHOD(create_SHA1_LANE_DATA) {
  NanScope();
  if (args.Length() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  int idx = 0;
  SHA1_JOB *job_in_lane = (SHA1_JOB*)NanGetInternalFieldPointer(args[idx++]->ToObject(), 0);
  SHA1_LANE_DATA *data = (SHA1_LANE_DATA*)malloc(sizeof(SHA1_LANE_DATA));
  data->job_in_lane = job_in_lane;

  Local<ObjectTemplate> tpl = ObjectTemplate::New();
  tpl->SetInternalFieldCount(1);
  Local<Object> retObj = NanNew(tpl)->NewInstance();
  NanSetInternalFieldPointer(retObj, 0, data);
  NanReturnValue(retObj);
}

NAN_METHOD(release_SHA1_LANE_DATA) {
  NanScope();
  if (args.Length() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  SHA1_LANE_DATA* data = (SHA1_LANE_DATA*)NanGetInternalFieldPointer(args[0]->ToObject(), 0);
  free(data);
  NanSetInternalFieldPointer(args[0]->ToObject(), 0, NULL);
  NanReturnValue(NanTrue());
}

NAN_METHOD(create_SHA1_MB_JOB_MGR) {
  NanScope();
  if (args.Length() != 5) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  SHA1_MB_JOB_MGR * mgr = (SHA1_MB_JOB_MGR*)malloc(sizeof(SHA1_MB_JOB_MGR));
  int idx = 0;
  if (!cast_array(args[idx++], mgr->args.digest, SHA1_DIGEST_NWORDS*SHA1_MAX_LANES, AT_UINT32)) {
    NanThrowTypeError("Failed to parse array digest[SHA1_DIGEST_NWORDS*SHA1_MAX_LANES]");
    free(mgr);
    NanReturnUndefined();
  }
  Local<Array> arr = Local<Array>::Cast(args[idx++]);
  if (arr.IsEmpty() && arr->Length() < SHA1_MAX_LANES) {
    free(mgr);
    NanReturnUndefined();
  }
  for (int i = 0; i < SHA1_MAX_LANES; i++) {
    mgr->args.data_ptr[i] = (UINT8*)(node::Buffer::Data(arr->Get(i)->ToObject()));
  }
  if (!cast_array(args[idx++], mgr->lens, SHA1_MAX_LANES, AT_UINT32)) {
    NanThrowTypeError("Failed to parse array len[SHA1_MAX_LANES]");
    free(mgr);
    NanReturnUndefined();
  }
  mgr->unused_lanes = (UINT64)args[idx++]->Int32Value();
  Local<Array> arr2 = Local<Array>::Cast(args[idx++]);
  if (arr2.IsEmpty() || arr2->Length() < SHA1_MAX_LANES) {
    NanThrowTypeError("Failed to parse array ldata[SHA1_MAX_LANES]");
    free(mgr);
    NanReturnUndefined();
  }
  for (int i = 0; i < SHA1_MAX_LANES; i++) {
    SHA1_LANE_DATA* pData = (SHA1_LANE_DATA*)NanGetInternalFieldPointer(arr2->Get(i)->ToObject(), 0);
    if (!pData) {
      NanThrowTypeError("Failed to parse array ldata[SHA1_MAX_LANES] because of invalid SHA1_LANE_DATA");
      free(mgr);
      NanReturnUndefined();
    }
    mgr->ldata[i].job_in_lane = pData->job_in_lane;
  }

  Local<ObjectTemplate> tpl = ObjectTemplate::New();
  tpl->SetInternalFieldCount(1);
  Local<Object> retObj = NanNew(tpl)->NewInstance();
  NanSetInternalFieldPointer(retObj, 0, mgr);
  NanReturnValue(retObj);
}

NAN_METHOD(release_SHA1_MB_JOB_MGR) {
  NanScope();
  if (args.Length() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  SHA1_MB_JOB_MGR* mgr = (SHA1_MB_JOB_MGR*)NanGetInternalFieldPointer(args[0]->ToObject(), 0);
  free(mgr);
  NanSetInternalFieldPointer(args[0]->ToObject(), 0, NULL);
  NanReturnValue(NanTrue());
}


NAN_METHOD(create_SHA1_HASH_CTX_MGR) {
  NanScope();
  if (args.Length() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  SHA1_HASH_CTX_MGR * mgr = (SHA1_HASH_CTX_MGR*)malloc(sizeof(SHA1_HASH_CTX_MGR));
  int idx = 0;
  SHA1_MB_JOB_MGR* pMgr = (SHA1_MB_JOB_MGR*)NanGetInternalFieldPointer(args[idx++]->ToObject(), 0);
  mgr->mgr = *pMgr;

  Local<ObjectTemplate> tpl = ObjectTemplate::New();
  tpl->SetInternalFieldCount(1);
  Local<Object> retObj = NanNew(tpl)->NewInstance();
  NanSetInternalFieldPointer(retObj, 0, mgr);
  NanReturnValue(retObj);
}

NAN_METHOD(release_SHA1_HASH_CTX_MGR) {
  NanScope();
  if (args.Length() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  SHA1_HASH_CTX_MGR* mgr = (SHA1_HASH_CTX_MGR*)NanGetInternalFieldPointer(args[0]->ToObject(), 0);
  free(mgr);
  NanSetInternalFieldPointer(args[0]->ToObject(), 0, NULL);
  NanReturnValue(NanTrue());
}

NAN_METHOD(create_SHA1_HASH_CTX) {
  NanScope();
  if (args.Length() != 9) {
    NanThrowTypeError("Invalid arguments");
    NanReturnUndefined();
  }
  SHA1_HASH_CTX * mgr = (SHA1_HASH_CTX*)malloc(sizeof(SHA1_HASH_CTX));
  int idx = 0;
  SHA1_JOB* pJob = (SHA1_JOB*)NanGetInternalFieldPointer(args[idx++]->ToObject(), 0);
  mgr->job = *pJob;
  mgr->status = (HASH_CTX_STS)args[idx++]->Int32Value();
  mgr->error = (HASH_CTX_ERROR)args[idx++]->Int32Value();
  mgr->total_length = (uint32_t)args[idx++]->Int32Value();
  mgr->incoming_buffer = (const void*)(node::Buffer::Data(args[idx++]->ToObject()));
  mgr->incoming_buffer_length = (uint32_t)args[idx++]->Int32Value();
  if (!cast_array(args[idx++], mgr->partial_block_buffer, SHA1_BLOCK_SIZE*2, AT_UINT8)) {
    NanThrowTypeError("Failed to parse array partial_block_buffer[SHA1_BLOCK_SIZE*2]");
    free(mgr);
    NanReturnUndefined();
  }
  mgr->partial_block_buffer_length = (uint32_t)args[idx++]->Int32Value();
  mgr->user_data = (void*)(node::Buffer::Data(args[idx++]->ToObject()));


  Local<ObjectTemplate> tpl = ObjectTemplate::New();
  tpl->SetInternalFieldCount(1);
  Local<Object> retObj = NanNew(tpl)->NewInstance();
  NanSetInternalFieldPointer(retObj, 0, mgr);
  NanReturnValue(retObj);
}

NAN_METHOD(release_SHA1_HASH_CTX) {
  NanScope();
  if (args.Length() != 1) {
    NanThrowTypeError("Invalid arguments");
    NanReturnValue(NanFalse());
  }
  SHA1_HASH_CTX* mgr = (SHA1_HASH_CTX*)NanGetInternalFieldPointer(args[0]->ToObject(), 0);
  free(mgr);
  NanSetInternalFieldPointer(args[0]->ToObject(), 0, NULL);
  NanReturnValue(NanTrue());
}


void export_util_component(v8::Handle<v8::Object>& exports) {
  exports->Set(NanNew("create_LZ_State1"), NanNew<FunctionTemplate>(create_LZ_State1)->GetFunction());
  exports->Set(NanNew("release_LZ_State1"), NanNew<FunctionTemplate>(release_LZ_State1)->GetFunction());

  exports->Set(NanNew("create_MD5_HMAC_LANE_DATA"), NanNew<FunctionTemplate>(create_MD5_HMAC_LANE_DATA)->GetFunction());
  exports->Set(NanNew("release_MD5_HMAC_LANE_DATA"), NanNew<FunctionTemplate>(release_MD5_HMAC_LANE_DATA)->GetFunction());
  exports->Set(NanNew("create_JOB_MD5"), NanNew<FunctionTemplate>(create_JOB_MD5)->GetFunction());
  exports->Set(NanNew("release_JOB_MD5"), NanNew<FunctionTemplate>(release_JOB_MD5)->GetFunction());
  exports->Set(NanNew("create_MD5_MB_MGR"), NanNew<FunctionTemplate>(create_MD5_MB_MGR)->GetFunction());
  exports->Set(NanNew("release_MD5_MB_MGR"), NanNew<FunctionTemplate>(release_MD5_MB_MGR)->GetFunction());
  exports->Set(NanNew("create_MD5_MB_MGR_X8X2"), NanNew<FunctionTemplate>(create_MD5_MB_MGR_X8X2)->GetFunction());
  exports->Set(NanNew("release_MD5_MB_MGR_X8X2"), NanNew<FunctionTemplate>(release_MD5_MB_MGR_X8X2)->GetFunction());

  exports->Set(NanNew("create_SHA1_HMAC_LANE_DATA"), NanNew<FunctionTemplate>(create_SHA1_HMAC_LANE_DATA)->GetFunction());
  exports->Set(NanNew("release_SHA1_HMAC_LANE_DATA"), NanNew<FunctionTemplate>(release_SHA1_HMAC_LANE_DATA)->GetFunction());
  exports->Set(NanNew("create_JOB_SHA1"), NanNew<FunctionTemplate>(create_JOB_SHA1)->GetFunction());
  exports->Set(NanNew("release_JOB_SHA1"), NanNew<FunctionTemplate>(release_JOB_SHA1)->GetFunction());
  exports->Set(NanNew("create_SHA1_MB_MGR"), NanNew<FunctionTemplate>(create_SHA1_MB_MGR)->GetFunction());
  exports->Set(NanNew("release_SHA1_MB_MGR"), NanNew<FunctionTemplate>(release_SHA1_MB_MGR)->GetFunction());
  exports->Set(NanNew("create_SHA1_MB_MGR_X8"), NanNew<FunctionTemplate>(create_SHA1_MB_MGR_X8)->GetFunction());
  exports->Set(NanNew("release_SHA1_MB_MGR_X8"), NanNew<FunctionTemplate>(release_SHA1_MB_MGR_X8)->GetFunction());

  exports->Set(NanNew("create_SHA256_HMAC_LANE_DATA"), NanNew<FunctionTemplate>(create_SHA256_HMAC_LANE_DATA)->GetFunction());
  exports->Set(NanNew("release_SHA256_HMAC_LANE_DATA"), NanNew<FunctionTemplate>(release_SHA256_HMAC_LANE_DATA)->GetFunction());
  exports->Set(NanNew("create_JOB_SHA256"), NanNew<FunctionTemplate>(create_JOB_SHA256)->GetFunction());
  exports->Set(NanNew("release_JOB_SHA256"), NanNew<FunctionTemplate>(release_JOB_SHA256)->GetFunction());
  exports->Set(NanNew("create_SHA256_MB_MGR"), NanNew<FunctionTemplate>(create_SHA256_MB_MGR)->GetFunction());
  exports->Set(NanNew("release_SHA256_MB_MGR"), NanNew<FunctionTemplate>(release_SHA256_MB_MGR)->GetFunction());
  exports->Set(NanNew("create_SHA256_MB_MGR_X8"), NanNew<FunctionTemplate>(create_SHA256_MB_MGR_X8)->GetFunction());
  exports->Set(NanNew("release_SHA256_MB_MGR_X8"), NanNew<FunctionTemplate>(release_SHA256_MB_MGR_X8)->GetFunction());

  exports->Set(NanNew("create_SHA512_HMAC_LANE_DATA"), NanNew<FunctionTemplate>(create_SHA512_HMAC_LANE_DATA)->GetFunction());
  exports->Set(NanNew("release_SHA512_HMAC_LANE_DATA"), NanNew<FunctionTemplate>(release_SHA512_HMAC_LANE_DATA)->GetFunction());
  exports->Set(NanNew("create_JOB_SHA512"), NanNew<FunctionTemplate>(create_JOB_SHA512)->GetFunction());
  exports->Set(NanNew("release_JOB_SHA512"), NanNew<FunctionTemplate>(release_JOB_SHA512)->GetFunction());
  exports->Set(NanNew("create_SHA512_MB_MGR"), NanNew<FunctionTemplate>(create_SHA512_MB_MGR)->GetFunction());
  exports->Set(NanNew("release_SHA512_MB_MGR"), NanNew<FunctionTemplate>(release_SHA512_MB_MGR)->GetFunction());
  exports->Set(NanNew("create_SHA512_MB_MGR_X4"), NanNew<FunctionTemplate>(create_SHA512_MB_MGR_X4)->GetFunction());
  exports->Set(NanNew("release_SHA512_MB_MGR_X4"), NanNew<FunctionTemplate>(release_SHA512_MB_MGR_X4)->GetFunction());

  exports->Set(NanNew("create_MD5_JOB"), NanNew<FunctionTemplate>(create_MD5_JOB)->GetFunction());
  exports->Set(NanNew("release_MD5_JOB"), NanNew<FunctionTemplate>(release_MD5_JOB)->GetFunction());
  exports->Set(NanNew("create_MD5_LANE_DATA"), NanNew<FunctionTemplate>(create_MD5_LANE_DATA)->GetFunction());
  exports->Set(NanNew("release_MD5_LANE_DATA"), NanNew<FunctionTemplate>(release_MD5_LANE_DATA)->GetFunction());
  exports->Set(NanNew("create_MD5_MB_JOB_MGR"), NanNew<FunctionTemplate>(create_MD5_MB_JOB_MGR)->GetFunction());
  exports->Set(NanNew("release_MD5_MB_JOB_MGR"), NanNew<FunctionTemplate>(release_MD5_MB_JOB_MGR)->GetFunction());
  exports->Set(NanNew("create_MD5_HASH_CTX_MGR"), NanNew<FunctionTemplate>(create_MD5_HASH_CTX_MGR)->GetFunction());
  exports->Set(NanNew("release_MD5_HASH_CTX_MGR"), NanNew<FunctionTemplate>(release_MD5_HASH_CTX_MGR)->GetFunction());
  exports->Set(NanNew("create_MD5_HASH_CTX"), NanNew<FunctionTemplate>(create_MD5_HASH_CTX)->GetFunction());
  exports->Set(NanNew("release_MD5_HASH_CTX"), NanNew<FunctionTemplate>(release_MD5_HASH_CTX)->GetFunction());

  exports->Set(NanNew("create_SHA1_JOB"), NanNew<FunctionTemplate>(create_SHA1_JOB)->GetFunction());
  exports->Set(NanNew("release_SHA1_JOB"), NanNew<FunctionTemplate>(release_SHA1_JOB)->GetFunction());
  exports->Set(NanNew("create_SHA1_LANE_DATA"), NanNew<FunctionTemplate>(create_SHA1_LANE_DATA)->GetFunction());
  exports->Set(NanNew("release_SHA1_LANE_DATA"), NanNew<FunctionTemplate>(release_SHA1_LANE_DATA)->GetFunction());
  exports->Set(NanNew("create_SHA1_MB_JOB_MGR"), NanNew<FunctionTemplate>(create_SHA1_MB_JOB_MGR)->GetFunction());
  exports->Set(NanNew("release_SHA1_MB_JOB_MGR"), NanNew<FunctionTemplate>(release_SHA1_MB_JOB_MGR)->GetFunction());
  exports->Set(NanNew("create_SHA1_HASH_CTX_MGR"), NanNew<FunctionTemplate>(create_SHA1_HASH_CTX_MGR)->GetFunction());
  exports->Set(NanNew("release_SHA1_HASH_CTX_MGR"), NanNew<FunctionTemplate>(release_SHA1_HASH_CTX_MGR)->GetFunction());
  exports->Set(NanNew("create_SHA1_HASH_CTX"), NanNew<FunctionTemplate>(create_SHA1_HASH_CTX)->GetFunction());
  exports->Set(NanNew("release_SHA1_HASH_CTX"), NanNew<FunctionTemplate>(release_SHA1_HASH_CTX)->GetFunction());
}
