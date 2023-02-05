#include <stdint.h>
#include <arm_neon.h>

typedef struct Context {
  int64x2_t state[8]; // state
  unsigned int  sizeM;    // byte length of input data
  unsigned int  sizeAD;   // byte length of associated data
} context;

#define S_NUM              8
#define M_NUM              2
#define BLKSIZE            32
#define NUM_LOOP_FOR_INIT  20

// Z0 = 428a2f98d728ae227137449123ef65cd
#define Z0_3      0x428a2f98
#define Z0_2      0xd728ae22
#define Z0_1      0x71374491
#define Z0_0      0x23ef65cd

// Z1 = b5c0fbcfec4d3b2fe9b5dba58189dbbc
#define Z1_3      0xb5c0fbcf
#define Z1_2      0xec4d3b2f
#define Z1_1      0xe9b5dba5
#define Z1_0      0x8189dbbc

#define enc(m, k) vreinterpretq_s64_u8(vaesmcq_u8(vaeseq_u8(vreinterpretq_u8_s64(m), vdupq_n_u8(0))) ^ vreinterpretq_u8_s64(k))
#define xor(a, b) veorq_s64(a, b)

#define UPDATE_STATE(X) \
  tmp7  =    S[7]; \
  tmp6  =    S[6]; \
  S[7] = xor(S[6], S[0]); \
  S[6] = enc(S[5], S[4]); \
  S[5] = enc(S[4], S[3]); \
  S[4] = xor(S[3], X[1]); \
  S[3] = enc(S[2], S[1]); \
  S[2] = xor(S[1], tmp6); \
  S[1] = enc(S[0], tmp7); \
  S[0] = xor(tmp7, X[0]);

#define LOAD(src, dst) \
  dst[0] = vld1q_s64((const int64_t*)((src)   )); \
  dst[1] = vld1q_s64((const int64_t*)((src)+16));

#define XOR_STRM(src, dst) \
  dst[0] = xor(src[0], enc(S[1]          , S[5])); \
  dst[1] = xor(src[1], enc(xor(S[0],S[4]), S[2]));

#define STORE(src, dst) \
  vst1q_s64((int64_t*)((dst)), (src[0])); \
  vst1q_s64((int64_t*)((dst)+16), (src[1]));

#define CAST_U64_TO_M128(v) \
  vreinterpretq_s64_s32(vld1q_s32((const int32_t[]){ \
    (((uint64_t)(v))>>0 )&0xFFFFFFFF, \
    (((uint64_t)(v))>>32)&0xFFFFFFFF, \
    0, 0}))
  


void stream_init(context * ctx, const uint8_t * key, \
const uint8_t * nonce) {
  int64x2_t S[S_NUM], M[M_NUM], tmp7, tmp6;

  // Initialize internal state
  S[0] = vld1q_s64((const int64_t*)(key+16));
  S[1] = vld1q_s64((const int64_t*)(nonce ));
  int32_t data[4] = {Z0_0, Z0_1, Z0_2, Z0_3};
  S[2] = vreinterpretq_s64_s32(vld1q_s32(data));
  int32_t data[4] = {Z1_0, Z1_1, Z1_2, Z1_3};
  S[3] = vreinterpretq_s64_s32(vld1q_s32(data));
  S[4] = veorq_s64((S[1]), (S[0]));
  S[5] = vdupq_n_s64(0);
  S[6] = vld1q_s64((const int64_t*)(key   ));
  S[7] = vdupq_n_s64(0);
  M[0] = S[2];
  M[1] = S[3];

  // Update local state
  for (unsigned int i = 0; i < NUM_LOOP_FOR_INIT; ++i) {
    UPDATE_STATE(M);
  }

  // Update context
  for (unsigned int i = 0; i < S_NUM; ++i) {
    ctx->state[i] = S[i];
  }
  ctx->sizeM = 0;
  ctx->sizeAD = 0;
}

unsigned int stream_proc_ad(context * ctx, const uint8_t *ad, \
unsigned int size) {
  int64x2_t S[S_NUM], M[M_NUM], tmp7, tmp6;

  // Copy state from context
  for (unsigned int i = 0; i < S_NUM; ++i) {
    S[i] = ctx->state[i];
  }

  // Update local state with associated data
  unsigned int proc_size = 0;
  for (unsigned int size2 = size / BLKSIZE * BLKSIZE;
  proc_size < size2; proc_size += BLKSIZE) {
    LOAD(ad + proc_size, M);
    UPDATE_STATE(M);
  }

  // Update context
  for (unsigned int i = 0; i < S_NUM; ++i) {
    ctx->state[i] = S[i];
  }
  ctx->sizeAD += proc_size;

  return proc_size;
}

unsigned int stream_enc(context * ctx, uint8_t *dst, const uint8_t *src, \
unsigned int size) {
  int64x2_t S[S_NUM], M[M_NUM], C[M_NUM], tmp7, tmp6;

  // Copy state from context
  for (unsigned int i = 0; i < S_NUM; ++i) {
    S[i] = ctx->state[i];
  }

  // Generate and output ciphertext
  // Update internal state with plaintext
  unsigned int proc_size = 0;
  for (unsigned int size2 = size / BLKSIZE * BLKSIZE; \
  proc_size < size2; proc_size += BLKSIZE) {
    LOAD(src + proc_size, M);
    XOR_STRM(M, C);
    STORE(C, dst + proc_size);
    UPDATE_STATE(M);
  }

  // Update context
  for (unsigned int i = 0; i < S_NUM; ++i) {
    ctx->state[i] = S[i];
  }
  ctx->sizeM += proc_size;

  return proc_size;
}

unsigned int stream_dec(context * ctx, uint8_t *dst, const uint8_t *src, \
unsigned int size) {
  int64x2_t S[S_NUM], M[M_NUM], C[M_NUM], tmp7, tmp6;

  // Copy state from context
  for (unsigned int i = 0; i < S_NUM; ++i) {
    S[i] = ctx->state[i];
  }

  // Generate and output plaintext
  // Update internal state with plaintext
  unsigned int proc_size = 0;
  for (unsigned int size2 = size / BLKSIZE * BLKSIZE; \
  proc_size < size2; proc_size += BLKSIZE) {
    LOAD(src + proc_size, C);
    XOR_STRM(C, M);
    STORE(M, dst + proc_size);
    UPDATE_STATE(M);
  }

  // Update context
  for (unsigned int i = 0; i < S_NUM; ++i) {
    ctx->state[i] = S[i];
  }
  ctx->sizeM += proc_size;

  return proc_size;
}

void stream_finalize(context * ctx, uint8_t *tag) {
  int64x2_t S[S_NUM], M[M_NUM], tmp7, tmp6;

  // Copy state from context
  for (unsigned int i = 0; i < S_NUM; ++i) {
    S[i] = ctx->state[i];
  }

  // set M[0] to bit length of processed AD
  // set M[1] to bit length of processed M
  M[0] = CAST_U64_TO_M128((uint64_t)ctx->sizeAD << 3);
  M[1] = CAST_U64_TO_M128((uint64_t)ctx->sizeM  << 3);

  // Update internal state
  for (unsigned int i = 0; i < NUM_LOOP_FOR_INIT; ++i) {
    UPDATE_STATE(M);
  }

  // Generate tag by XORing all S[i]s
  for (unsigned int i = 1; i < S_NUM; ++i) {
    S[0] = veorq_s64(S[0], S[i]);
  }

  // Output tag
  vst1q_s64((int64_t*)tag, S[0]);
}

#include <stdio.h>
// testing
int main() {
  uint8_t key[32] = {0};
  uint8_t nonce[16] = {0};
  uint8_t ad[32] = {0};
  uint8_t pt[64] = {0};
  context ctx;
  uint8_t out[64];
  uint8_t outTag[16];

  uint8_t ct[64] =
    { 0x15, 0x89, 0x2f, 0x85, 0x55, 0xad, 0x2d, 0xb4, 0x74, 0x9b, 0x90, 0x92, 0x65, 0x71, 0xc4, 0xb8,
      0xc2, 0x8b, 0x43, 0x4f, 0x27, 0x77, 0x93, 0xc5, 0x38, 0x33, 0xcb, 0x6e, 0x41, 0xa8, 0x55, 0x29,
      0x17, 0x84, 0xa2, 0xc7, 0xfe, 0x37, 0x4b, 0x34, 0xd8, 0x75, 0xfd, 0xcb, 0xe8, 0x4f, 0x5b, 0x88,
      0xbf, 0x3f, 0x38, 0x6f, 0x22, 0x18, 0xf0, 0x46, 0xa8, 0x43, 0x18, 0x56, 0x50, 0x26, 0xd7, 0x55 };
  uint8_t tag[16] =
    { 0xcc, 0x72, 0x8c, 0x8b, 0xae, 0xdd, 0x36, 0xf1, 0x4c, 0xf8, 0x93, 0x8e, 0x9e, 0x07, 0x19, 0xbf };

  stream_init(&ctx, key, nonce);
  stream_proc_ad(&ctx, ad, 32);
  stream_enc(&ctx, out, pt, 64);
  stream_finalize(&ctx, outTag);

  printf("output:\n");
  for (int i = 0; i < 64; ++i) {
    printf("%02x ", out[i]);
    if (i % 16 == 15) {
      printf("\n");
    }
  }
  printf("\n");
  for (int i = 0; i < 16; ++i) {
    printf("%02x ", outTag[i]);
  }

  for (int i = 0; i < 64; ++i) {
    if (out[i] != ct[i]) {
      printf("\nCT error at %d", i);
      return 1;
    }
  }

  for (int i = 0; i < 16; ++i) {
    if (outTag[i] != tag[i]) {
      printf("\nTag error at %d", i);
      return 1;
    }
  }
  printf("\nEverything is correct\n");
  return 0;
}