#include "sse2neon.h"
#define XOR(a, b)     _mm_xor_si128(a, b)
#define AND(a, b)     _mm_and_si128(a, b)
#define ADD(a, b)     _mm_add_epi32(a, b)
#define SET(v)        _mm_set1_epi16((short)v)
#define SLL(a)        _mm_slli_epi16(a, 1)
#define SRA(a)        _mm_srai_epi16(a, 15)
#define TAP7(Hi, Lo)  _mm_alignr_epi8(Hi, Lo, 7 * 2)
#define SIGMA(a)      \
  _mm_shuffle_epi8(a, _mm_set_epi64x(\
  0x0f0b07030e0a0602ULL, 0x0d0905010c080400ULL));
#define AESR(a, k)    _mm_aesenc_si128(a, k)
#define ZERO()        _mm_setzero_si128()
#define LOAD(src)     \
  _mm_loadu_si128((const __m128i*)(src))
#define STORE(dst, x) \
  _mm_storeu_si128((__m128i*)(dst), x)
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

inline void SnowVi_encdec(int length, u8 * out,
  u8 * in, u8 * key, u8 * iv)
{ __m128i A0, A1, B0, B1, R1, R2, R3, T1, T2;
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
int main()
{
  unsigned char key[32] = {0};
  unsigned char iv[16] = {0};
  unsigned char in[128] = {0};
  unsigned char out[128] = {0};
  SnowVi_encdec(128, out, in, key, iv);
  for (int i = 0; i < 128; ++i)
  {
    printf("%02x ", out[i]);
    if (i % 16 == 15) printf("\n");
  }
  return 0;
}