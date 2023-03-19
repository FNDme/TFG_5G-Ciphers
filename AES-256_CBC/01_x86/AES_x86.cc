#define cpuid(func, ax, bx, cx, dx)                             \
  __asm__ __volatile__("cpuid"                                  \
                       : "=a"(ax), "=b"(bx), "=c"(cx), "=d"(dx) \
                       : "a"(func));

int Check_CPU_support_AES()
{
  unsigned int a, b, c, d;
  cpuid(1, a, b, c, d);
  return (c & 0x2000000);
}

#include <wmmintrin.h>

#if !defined(ALIGN16)
#if defined(__GNUC__)
#define ALIGN16 __attribute__((aligned(16)))
#else
#define ALIGN16 __declspec(align(16))
#endif
#endif

typedef struct KEY_SCHEDULE
{
  ALIGN16 unsigned char KEY[16 * 15];
  unsigned int nr;
} AES_KEY;

inline void KEY_256_ASSIST_1(__m128i *temp1, __m128i *temp2)
{
  __m128i temp4;
  *temp2 = _mm_shuffle_epi32(*temp2, 0xff);
  temp4 = _mm_slli_si128(*temp1, 0x4);
  *temp1 = _mm_xor_si128(*temp1, temp4);
  temp4 = _mm_slli_si128(temp4, 0x4);
  *temp1 = _mm_xor_si128(*temp1, temp4);
  temp4 = _mm_slli_si128(temp4, 0x4);
  *temp1 = _mm_xor_si128(*temp1, temp4);
  *temp1 = _mm_xor_si128(*temp1, *temp2);
}

inline void KEY_256_ASSIST_2(__m128i *temp1, __m128i *temp3)
{
  __m128i temp2, temp4;
  temp4 = _mm_aeskeygenassist_si128(*temp1, 0x0);
  temp2 = _mm_shuffle_epi32(temp4, 0xaa);
  temp4 = _mm_slli_si128(*temp3, 0x4);
  *temp3 = _mm_xor_si128(*temp3, temp4);
  temp4 = _mm_slli_si128(temp4, 0x4);
  *temp3 = _mm_xor_si128(*temp3, temp4);
  temp4 = _mm_slli_si128(temp4, 0x4);
  *temp3 = _mm_xor_si128(*temp3, temp4);
  *temp3 = _mm_xor_si128(*temp3, temp2);
}

void AES_256_Key_Expansion(const unsigned char *userkey,
                           unsigned char *key)
{
  __m128i temp1, temp2, temp3;
  __m128i *Key_Schedule = (__m128i *)key;
  temp1 = _mm_loadu_si128((__m128i *)userkey);
  temp3 = _mm_loadu_si128((__m128i *)(userkey + 16));
  Key_Schedule[0] = temp1;
  Key_Schedule[1] = temp3;
  temp2 = _mm_aeskeygenassist_si128(temp3, 0x01);
  KEY_256_ASSIST_1(&temp1, &temp2);
  Key_Schedule[2] = temp1;
  KEY_256_ASSIST_2(&temp1, &temp3);
  Key_Schedule[3] = temp3;
  temp2 = _mm_aeskeygenassist_si128(temp3, 0x02);
  KEY_256_ASSIST_1(&temp1, &temp2);
  Key_Schedule[4] = temp1;
  KEY_256_ASSIST_2(&temp1, &temp3);
  Key_Schedule[5] = temp3;
  temp2 = _mm_aeskeygenassist_si128(temp3, 0x04);
  KEY_256_ASSIST_1(&temp1, &temp2);
  Key_Schedule[6] = temp1;
  KEY_256_ASSIST_2(&temp1, &temp3);
  Key_Schedule[7] = temp3;
  temp2 = _mm_aeskeygenassist_si128(temp3, 0x08);
  KEY_256_ASSIST_1(&temp1, &temp2);
  Key_Schedule[8] = temp1;
  KEY_256_ASSIST_2(&temp1, &temp3);
  Key_Schedule[9] = temp3;
  temp2 = _mm_aeskeygenassist_si128(temp3, 0x10);
  KEY_256_ASSIST_1(&temp1, &temp2);
  Key_Schedule[10] = temp1;
  KEY_256_ASSIST_2(&temp1, &temp3);
  Key_Schedule[11] = temp3;
  temp2 = _mm_aeskeygenassist_si128(temp3, 0x20);
  KEY_256_ASSIST_1(&temp1, &temp2);
  Key_Schedule[12] = temp1;
  KEY_256_ASSIST_2(&temp1, &temp3);
  Key_Schedule[13] = temp3;
  temp2 = _mm_aeskeygenassist_si128(temp3, 0x40);
  KEY_256_ASSIST_1(&temp1, &temp2);
  Key_Schedule[14] = temp1;
}

void AES_CBC_encrypt(const unsigned char *in,
                     unsigned char *out,
                     unsigned char ivec[16],
                     unsigned long length,
                     unsigned char *key,
                     int number_of_rounds)
{
  __m128i feedback, data;
  int i, j;

  if (length % 16)
    length = length / 16 + 1;
  else
    length /= 16;

  feedback = _mm_loadu_si128((__m128i *)ivec);
  for (i = 0; i < length; i++)
  {
    data = _mm_loadu_si128(&((__m128i *)in)[i]);
    feedback = _mm_xor_si128(data, feedback);
    feedback = _mm_xor_si128(feedback, ((__m128i *)key)[0]);
    for (j = 1; j < number_of_rounds; j++)
      feedback = _mm_aesenc_si128(feedback, ((__m128i *)key)[j]);
    feedback = _mm_aesenclast_si128(feedback, ((__m128i *)key)[j]);
    _mm_storeu_si128(&((__m128i *)out)[i], feedback);
  }
}

/* -------------------------------------------------------------------------------- */

#ifndef LENGTH
#define LENGTH 64
#endif

#include <stdint.h>
#include <stdio.h>

void AES_256_set_encrypt_key(const unsigned char *userKey, AES_KEY *key)
{
  AES_256_Key_Expansion(userKey, key->KEY);
  key->nr = 14;
}

ALIGN16 uint8_t AES256_TEST_KEY[] = {0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
                                     0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4};

ALIGN16 uint8_t AES_TEST_VECTOR[] = {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
                                     0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c, 0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
                                     0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11, 0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
                                     0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17, 0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10};

ALIGN16 uint8_t CBC_IV[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

ALIGN16 uint8_t CBC256_EXPECTED[] = {0xf5, 0x8c, 0x4c, 0x04, 0xd6, 0xe5, 0xf1, 0xba, 0x77, 0x9e, 0xab, 0xfb, 0x5f, 0x7b, 0xfb, 0xd6,
                                     0x9c, 0xfc, 0x4e, 0x96, 0x7e, 0xdb, 0x80, 0x8d, 0x67, 0x9f, 0x77, 0x7b, 0xc6, 0x70, 0x2c, 0x7d,
                                     0x39, 0xf2, 0x33, 0x69, 0xa9, 0xd9, 0xba, 0xcf, 0xa5, 0x30, 0xe2, 0x63, 0x04, 0x23, 0x14, 0x61,
                                     0xb2, 0xeb, 0x05, 0xe2, 0xc3, 0x9b, 0xe9, 0xfc, 0xda, 0x6c, 0x19, 0x07, 0x8c, 0x6a, 0x9d, 0x1b};

void print_m128i_with_string(__m128i data)
{
  unsigned char *pointer = (unsigned char *)&data;
  int i;
  printf("%-40s[0x", "");
  for (i = 0; i < 16; i++)
    printf("%02x", pointer[i]);
  printf("]\n");
}
void print_m128i_with_string_short(__m128i data, int length)
{
  unsigned char *pointer = (unsigned char *)&data;
  int i;
  printf("%-40s[0x", "");
  for (i = 0; i < length; i++)
    printf("%02x", pointer[i]);
  printf("]\n");
}

int test()
{
  AES_KEY key;
  uint8_t *PLAINTEXT;
  uint8_t *CIPHERTEXT;
  uint8_t *EXPECTED_CIPHERTEXT;
  uint8_t *CIPHER_KEY;
  int i, j;
  int key_length;

  if (!Check_CPU_support_AES())
  {
    printf("Cpu does not support AES instruction set. Bailing out.\n");
    return 1;
  }
  printf("CPU support AES instruction set.\n\n");

  CIPHER_KEY = AES256_TEST_KEY;
  EXPECTED_CIPHERTEXT = CBC256_EXPECTED;
  key_length = 256;

  PLAINTEXT = (uint8_t *)malloc(LENGTH);
  CIPHERTEXT = (uint8_t *)malloc(LENGTH);
  for (i = 0; i < LENGTH / 16 / 4; i++)
  {
    for (j = 0; j < 4; j++)
    {
      _mm_storeu_si128(&((__m128i *)PLAINTEXT)[i * 4 + j],
                       ((__m128i *)AES_TEST_VECTOR)[j]);
    }
  }
  for (j = i * 4; j < LENGTH / 16; j++)
  {
    _mm_storeu_si128(&((__m128i *)PLAINTEXT)[j],
                     ((__m128i *)AES_TEST_VECTOR)[j % 4]);
  }
  if (LENGTH % 16)
  {
    _mm_storeu_si128(&((__m128i *)PLAINTEXT)[j],
                     ((__m128i *)AES_TEST_VECTOR)[j % 4]);
  }

  AES_256_set_encrypt_key(CIPHER_KEY, &key);

  AES_CBC_encrypt(PLAINTEXT,
                  CIPHERTEXT,
                  CBC_IV,
                  LENGTH,
                  key.KEY,
                  key.nr);

  printf("The Cipher Key:\n");
  print_m128i_with_string(((__m128i *)CIPHER_KEY)[0]);
  if (key_length > 128)
    print_m128i_with_string_short(((__m128i *)CIPHER_KEY)[1], (key_length / 8) - 16);

  printf("The Key Schedule:\n");
  for (i = 0; i < key.nr; i++)
    print_m128i_with_string(((__m128i *)key.KEY)[i]);

  printf("The PLAINTEXT:\n");
  for (i = 0; i < LENGTH / 16; i++)
    print_m128i_with_string(((__m128i *)PLAINTEXT)[i]);
  if (LENGTH % 16)
    print_m128i_with_string_short(((__m128i *)PLAINTEXT)[i], LENGTH % 16);

  printf("\n\nThe CIPHERTEXT:\n");
  for (i = 0; i < LENGTH / 16; i++)
    print_m128i_with_string(((__m128i *)CIPHERTEXT)[i]);
  if (LENGTH % 16)
    print_m128i_with_string_short(((__m128i *)CIPHERTEXT)[i], LENGTH % 16);

  for (i = 0; i < ((64 < LENGTH) ? 64 : LENGTH); i++)
  {
    if (CIPHERTEXT[i] != EXPECTED_CIPHERTEXT[i % 64])
    {
      printf("The ciphertext is not equal to the expected ciphertext.\n\n");
      return 1;
    }
  }

  printf("\n\nThe CIPHERTEXT equals to the EXPECTED CIHERTEXT"
         " for bytes where expected text was entered.\n\n");

  return 0;
}

int main(){
  // testing 256 key expansion
  const unsigned char key[32] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
                                  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                                  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f};
  ALIGN16 unsigned char KEY[16 * 15];
  printf("Key expansion:\n");

  AES_256_Key_Expansion(key, KEY);

  for (int i = 0; i < 15; i++) {
    for (int j = 0; j < 16; j++) {
      printf("%02x ", KEY[i * 16 + j]);
    }
    printf("\n");
  }
}