#include <arm_neon.h>
#define XOR(a, b) veorq_s64(a, b)
#define AND(a, b) vandq_s64(a, b)
#define ADD(a, b) vreinterpretq_s64_s32(vaddq_s32(vreinterpretq_s32_s64(a), \
                                                  vreinterpretq_s32_s64(b)))
#define SET(v) vreinterpretq_s64_s16(vdupq_n_s16((short)v))
#define SLL(a) vreinterpretq_s64_s16(vshlq_n_s16(vreinterpretq_s16_s64(a), 1))
#define SRA(a) vreinterpretq_s64_s16(vshrq_n_s16(vreinterpretq_s16_s64(a), 15))
#define TAP7(Hi, Lo) vreinterpretq_s64_u8(vextq_u8(vreinterpretq_u8_s64(Lo), \
                                                   vreinterpretq_u8_s64(Hi), 14))
#define SIGMA(a)                                                                                  \
  vreinterpretq_s64_s8(vqtbl1q_s8(vreinterpretq_s8_s64(a),                                        \
                                  vandq_u8(vreinterpretq_u8_s64(                                  \
                                               vcombine_s64(vcreate_s64(0x0d0905010c080400ULL),   \
                                                            vcreate_s64(0x0f0b07030e0a0602ULL))), \
                                           vdupq_n_u8(0x8F))))
#define AESR(a, k) vreinterpretq_s64_u8(vaesmcq_u8(vaeseq_u8(vreinterpretq_u8_s64(a), \
                                                             (uint8x16_t){})) ^       \
                                        vreinterpretq_u8_s64(k))
#define ZERO() vdupq_n_s64(0)
#define LOAD(src) vld1q_s64((const int64_t *)(src))
#define STORE(dst, x) vst1q_s64((int64_t *)(dst), (x))
#define u8 unsigned char

#define SIZE 1024 * 1024 * 3


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


#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main()
{
  unsigned char key[32] = {0};
  unsigned char iv[16] = {0};
  unsigned char in[SIZE] = { 0 };
  unsigned char out[SIZE] = { 0 };

  clock_t time_original;

  time_original = clock();
  for (int i = 0; i < 1000; ++i)
  {
    SnowVi_encdec(SIZE, out, in, key, iv);
  }
  time_original = clock() - time_original;

  printf("Original: %f\n", (double)time_original);
  return 0;
}