#include "x86intrin.h"
#define XOR(a, b)     _mm_xor_si128(a, b)
#define AND(a, b)     _mm_and_si128(a, b)
#define ADD(a, b)     _mm_add_epi32(a, b)
#define SET(v)        _mm_set1_epi16((short)v)
#define SLL(a)        _mm_slli_epi16(a, 1)
#define SRA(a)        _mm_srai_epi16(a, 15)
#define TAP7(Hi, Lo)                              \
  _mm_alignr_epi8(Hi, Lo, 7 * 2)
#define SIGMA(a)                                  \
  _mm_shuffle_epi8(a, _mm_set_epi64x(             \
  0x0f0b07030e0a0602ULL, 0x0d0905010c080400ULL));
#define AESR(a, k)    _mm_aesenc_si128(a, k)
#define ZERO()        _mm_setzero_si128()
#define LOAD(src)                                 \
  _mm_loadu_si128((const __m128i*)(src))
#define STORE(dst, x)                             \
  _mm_storeu_si128((__m128i*)(dst), x)
#define u8            unsigned char

#define SIZE 1024 * 512

#define SnowVi_3ROUNDS_INIT                       \
A2 = XOR(XOR(XOR(TAP7(A1, A0), B0),               \
     AND(SRA(A0),SET(0x4a6d))), SLL(A0));         \
B2 = XOR(XOR(B1, AND(SRA(B0), SET(0xcc87))),      \
     XOR(A0, SLL(B0)));                           \
A2 = XOR(A2, XOR(ADD(B1, R1), R2));               \
A0 = ADD(R2, R3);                                 \
R3 = AESR(R2, A2);                                \
R2 = AESR(R1, ZERO());                            \
R1 = SIGMA(A0);                                   \
A0 = XOR(XOR(XOR(TAP7(A2, A1), B1),               \
     AND(SRA(A1),SET(0x4a6d))), SLL(A1));         \
B0 = XOR(XOR(B2, AND(SRA(B1), SET(0xcc87))),      \
     XOR(A1, SLL(B1)));                           \
A0 = XOR(A0, XOR(ADD(B2, R1), R2));               \
A1 = ADD(R2, R3);                                 \
R3 = AESR(R2, A0);                                \
R2 = AESR(R1, ZERO());                            \
R1 = SIGMA(A1);                                   \
A1 = XOR(XOR(XOR(TAP7(A0, A2), B2),               \
     AND(SRA(A2),SET(0x4a6d))), SLL(A2));         \
B1 = XOR(XOR(B0, AND(SRA(B2), SET(0xcc87))),      \
     XOR(A2, SLL(B2)));                           \
A1 = XOR(A1, XOR(ADD(B0, R1), R2));               \
A2 = ADD(R2, R3);                                 \
R3 = AESR(R2, A1);                                \
R2 = AESR(R1, ZERO());                            \
R1 = SIGMA(A2);

#define SnowVi_4ROUNDS_INIT                       \
A2 = XOR(XOR(XOR(TAP7(A1, A0), B0),               \
     AND(SRA(A0),SET(0x4a6d))), SLL(A0));         \
B2 = XOR(XOR(B1, AND(SRA(B0), SET(0xcc87))),      \
     XOR(A0, SLL(B0)));                           \
A2 = XOR(A2, XOR(ADD(B1, R1), R2));               \
A0 = ADD(R2, R3);                                 \
R3 = AESR(R2, A2);                                \
R2 = AESR(R1, ZERO());                            \
R1 = SIGMA(A0);                                   \
A0 = XOR(XOR(XOR(TAP7(A2, A1), B1),               \
     AND(SRA(A1),SET(0x4a6d))), SLL(A1));         \
B0 = XOR(XOR(B2, AND(SRA(B1), SET(0xcc87))),      \
     XOR(A1, SLL(B1)));                           \
A0 = XOR(A0, XOR(ADD(B2, R1), R2));               \
A1 = ADD(R2, R3);                                 \
R3 = AESR(R2, A0);                                \
R2 = AESR(R1, ZERO());                            \
R1 = SIGMA(A1);                                   \
A1 = XOR(XOR(XOR(TAP7(A0, A2), B2),               \
     AND(SRA(A2),SET(0x4a6d))), SLL(A2));         \
B1 = XOR(XOR(B0, AND(SRA(B2), SET(0xcc87))),      \
     XOR(A2, SLL(B2)));                           \
A1 = XOR(A1, XOR(ADD(B0, R1), R2));               \
A2 = ADD(R2, R3);                                 \
R3 = AESR(R2, A1);                                \
R2 = AESR(R1, ZERO());                            \
R1 = SIGMA(A2);                                   \
R1 = XOR(R1, LOAD(key));                          \
A2 = XOR(XOR(XOR(TAP7(A1, A0), B0),               \
     AND(SRA(A0),SET(0x4a6d))), SLL(A0));         \
B2 = XOR(XOR(B1, AND(SRA(B0), SET(0xcc87))),      \
     XOR(A0, SLL(B0)));                           \
A2 = XOR(A2, XOR(ADD(B1, R1), R2));               \
A0 = ADD(R2, R3);                                 \
R3 = AESR(R2, A2);                                \
R2 = AESR(R1, ZERO());                            \
R1 = SIGMA(A0);                                   \
R1 = XOR(R1, LOAD(key + 16));                     \

#define SnowVi_3ROUNDS(offset)                    \
A0 = XOR(XOR(XOR(TAP7(A2, A1), B1),               \
     AND(SRA(A1),SET(0x4a6d))), SLL(A1));         \
B0 = XOR(XOR(B2, AND(SRA(B1),                     \
     SET(0xcc87))), XOR(A1, SLL(B1)));            \
STORE(out + offset, XOR(ADD(B2, R1),              \
     XOR(LOAD(in + offset), R2)));                \
A1 = ADD(R2, R3);                                 \
R3 = AESR(R2, A0);                                \
R2 = AESR(R1, ZERO());                            \
R1 = SIGMA(A1);                                   \
A1 = XOR(XOR(XOR(TAP7(A0, A2), B2),               \
     AND(SRA(A2),SET(0x4a6d))), SLL(A2));         \
B1 = XOR(XOR(B0, AND(SRA(B2),                     \
     SET(0xcc87))), XOR(A2, SLL(B2)));            \
STORE(out + offset + 16, XOR(ADD(B0, R1),         \
     XOR(LOAD(in + offset + 16), R2)));           \
A2 = ADD(R2, R3);                                 \
R3 = AESR(R2, A1);                                \
R2 = AESR(R1, ZERO());                            \
R1 = SIGMA(A2);                                   \
A2 = XOR(XOR(XOR(TAP7(A1, A0), B0),               \
     AND(SRA(A0),SET(0x4a6d))), SLL(A0));         \
B2 = XOR(XOR(B1, AND(SRA(B0),                     \
     SET(0xcc87))), XOR(A0, SLL(B0)));            \
STORE(out + offset + 32, XOR(ADD(B1, R1),         \
     XOR(LOAD(in + offset + 32), R2)));           \
A0 = ADD(R2, R3);                                 \
R3 = AESR(R2, A2);                                \
R2 = AESR(R1, ZERO());                            \
R1 = SIGMA(A0);                                   \

#define SnowVi_2ROUNDS(offset)                    \
A0 = XOR(XOR(XOR(TAP7(A2, A1), B1),               \
     AND(SRA(A1),SET(0x4a6d))), SLL(A1));         \
B0 = XOR(XOR(B2, AND(SRA(B1),                     \
     SET(0xcc87))), XOR(A1, SLL(B1)));            \
STORE(out + offset, XOR(ADD(B2, R1),              \
     XOR(LOAD(in + offset), R2)));                \
A1 = ADD(R2, R3);                                 \
R2 = AESR(R1, ZERO());                            \
R1 = SIGMA(A1);                                   \
STORE(out + offset + 16, XOR(ADD(B0, R1),         \
     XOR(LOAD(in + offset + 16), R2)));           \



// Objetivo: Reducir el numero de copias del estado haciendo uso de un pivoteo
// de registros en lugar de hacer uso de variables temporales.
static inline void SnowVi_improved
  (int length, u8 * out, u8 * in, u8 * key, u8 * iv)
{ __m128i A0, A1, A2, B0, B1, B2, R1, R2, R3;
  B0 = R1 = R2 = ZERO();
  A0 = LOAD(iv);
  R3 = A1 = LOAD(key);
  B1 = LOAD(key + 16);

  // Initial rounds
  SnowVi_3ROUNDS_INIT;
  SnowVi_3ROUNDS_INIT;
  SnowVi_3ROUNDS_INIT;
  SnowVi_3ROUNDS_INIT;
  SnowVi_4ROUNDS_INIT;

  // Main loop
  int i = 0;
  for (; i <= length - 48; i += 48)
  {
    SnowVi_3ROUNDS(i);
  }
  if (i <= length - 32)
  {
    SnowVi_2ROUNDS(i);
    return;
  }
  if (i <= length - 16)
  {
    STORE(out + i, XOR(ADD(B2, R1),               \
        XOR(LOAD(in + i), R2)));
  }
}

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
// Time testings
#include <time.h>

int main(int argc, char *argv[])
{
  unsigned char key[32] = {0};
  unsigned char iv[16] = {0};
  unsigned char in[SIZE] = { 0 };
  unsigned char out[SIZE] = { 0 };
  unsigned char out2[SIZE] = { 0 };

  printf("              SnowVi tests\n");
  printf("========================================\n");

  // Time comparison
  clock_t time_original, time_improved;

  // Vector de tamaños a probar
  int sizes[] = { 64, 256, 1024, 4096, 16384, 65536, 262144 };

  int repetitions = 1000000;
  if (argc > 1)
  {
    repetitions = atoi(argv[1]);
  }

  // Test de velocidad
  for (int i = 0; i < 7; ++i)
  {
    // Original
    time_original = clock();
    for (int j = 0; j < repetitions; ++j)
    {
      SnowVi_encdec(sizes[i], out, in, key, iv);
    }
    time_original = clock() - time_original;

    // Improved
    time_improved = clock();
    for (int j = 0; j < repetitions; ++j)
    {
      SnowVi_improved(sizes[i], out2, in, key, iv);
    }
    time_improved = clock() - time_improved;

    printf("Size: %*d,           Speedup: %.2f %%\n", 6, sizes[i], (((double)time_original / (double)time_improved) - 1.0) * 100.0);
  }
  return 0;
}
