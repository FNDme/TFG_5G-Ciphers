// TODO: Reformatear estas instrucciones para aumentar la eficiencia
#include <arm_neon.h>
// DONE
#define XOR(a, b)     veorq_s64(a, b)
// DONE
#define AND(a, b)     vandq_s64(a, b)
// TODO
#define ADD(a, b)     \
  vreinterpretq_s64_s32(vaddq_s32(vreinterpretq_s32_s64(a),\
  vreinterpretq_s32_s64(b)))
// TODO
#define SET(v)        vreinterpretq_s64_s16(vdupq_n_s16(v))
// TODO
#define SLL(a)        \
  vreinterpretq_s64_s16(vshlq_n_s16(vreinterpretq_s16_s64(a), 1))
// TODO
#define SRA(a)        \
  vreinterpretq_s64_s16(vshrq_n_s16(vreinterpretq_s16_s64(a), 15))
// TODO
#define TAP7(Hi, Lo)  vreinterpretq_s64_u8(vextq_u8(vreinterpretq_u8_s64(Lo),\
  vreinterpretq_u8_s64(Hi), 14))
// TODO
#define SIGMA(a)      \
  vreinterpretq_s64_s8(vqtbl1q_s8(vreinterpretq_s8_s64(a),\
  vandq_u8(vreinterpretq_u8_s64(vcombine_s64(vcreate_s64(0x0d0905010c080400ULL),\
  vcreate_s64(0x0f0b07030e0a0602ULL))), vdupq_n_u8(0x8F))));
// TODO: Traducir y formatear
#define AESR(a, k)    vreinterpretq_s64_u8(vaesmcq_u8(vaeseq_u8(\
  vreinterpretq_u8_s64(a), vdupq_n_u8(0))) ^ vreinterpretq_u8_s64(k))
// DONE
#define ZERO()        vdupq_n_s64(0)
// DONE
#define LOAD(src)     vld1q_s64((const int64_t *)(src))
// DONE
#define STORE(dst, x) vst1q_s64((int64_t *) (dst), x)
#define u8            unsigned char

#define SnowVi_XMM_ROUND(mode, offset)\
T1 = B1; T2 = A1;\
A1 = XOR(XOR(XOR(TAP7(A1, A0), B0), AND(SRA(A0),\
      SET(0x4a6d))), SLL(A0));\
B1 = XOR(XOR(B1, AND(SRA(B0), SET(0xcc87))),\
      XOR(A0, SLL(B0)));\
A0 = T2; B0 = T1;\
if (mode == 0) A1 = XOR(A1, XOR(ADD(T1, R1), R2));\
else STORE(out + offset, XOR(ADD(T1, R1),\
      XOR(LOAD(in + offset), R2)));\
T2 = ADD(R2, R3);\
R3 = AESR(R2, A1);\
R2 = AESR(R1, ZERO());\
R1 = SIGMA(T2);

static inline void SnowVi_encdec(int length, u8 * out,
  u8 * in, u8 * key, u8 * iv)
{ int64x2_t A0, A1, B0, B1, R1, R2, R3, T1, T2;
  B0 = R1 = R2 = ZERO();
  A0 = LOAD(iv);
  R3 = A1 = LOAD(key);
  B1 = LOAD(key + 16);

  for (int i = -14; i < 2; ++i)
  { SnowVi_XMM_ROUND(0, 0);
    if (i < 0) continue;
    R1 = XOR(R1, LOAD(key + i * 16));
  }

  for (int i = 0; i <= length - 16; i += 16)
  { SnowVi_XMM_ROUND(1, i); }
}

int value = 1;

#include <stdio.h>
int main()
{
  unsigned char key[32] = {0};
  unsigned char iv[16] = {0};
  unsigned char in[128] = {0};
  unsigned char out[128] = {0};
  SnowVi_encdec(128, out, in, key, iv);
  unsigned char test[128] = {
    0x50, 0x17, 0x19, 0xe1, 0x75, 0xe4, 0x9f, 0xb7, 0x41, 0xba, 0xbf, 0x6b, 0xa5, 0xde, 0x60, 0xfe,
    0xcd, 0xa8, 0xb3, 0x4d, 0x7e, 0xc4, 0xc6, 0x42, 0x97, 0x55, 0xc1, 0x9d, 0x2f, 0x67, 0x18, 0x71,
    0x89, 0x57, 0xd3, 0x26, 0xcb, 0x46, 0x50, 0x2c, 0xeb, 0x81, 0x4c, 0xcd, 0x6e, 0xa5, 0x3a, 0xae,
    0xdd, 0x6c, 0x92, 0xfb, 0xf3, 0x92, 0x1e, 0x8b, 0xd7, 0x31, 0x7b, 0xe2, 0x20, 0x15, 0x31, 0xbb,
    0x09, 0x3e, 0xe8, 0x72, 0xe9, 0xeb, 0x40, 0x34, 0xe9, 0xb7, 0x1a, 0x4a, 0xc2, 0xb5, 0x4b, 0xd9,
    0xf0, 0x0f, 0x5a, 0xdc, 0x06, 0xd2, 0xe6, 0xb5, 0x9f, 0xb7, 0x5a, 0x01, 0xbe, 0xf6, 0x13, 0x14,
    0x1c, 0x8a, 0xb2, 0x02, 0xee, 0x38, 0xe2, 0x85, 0x0c, 0xca, 0x60, 0x6a, 0xb8, 0x75, 0xcd, 0x12,
    0x41, 0x03, 0xb3, 0x2f, 0xa5, 0x14, 0x5d, 0xdf, 0x54, 0xe7, 0xa0, 0x7b, 0x0f, 0x3e, 0xb7, 0x7a
  };
  for (int i = 0; i < 128; ++i)
    if (out[i] != test[i])
    {
      printf("Error at %d: %02x != %02x\n", i, out[i], test[i]);
      return 1;
    }
  printf("Everything is OK\n");
  return 0;
}