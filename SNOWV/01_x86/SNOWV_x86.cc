// SNOW-V Reference Implementation utilizing AES-NI, SSE2, SSSE3, AVX, AVX2 (Little endian)
#include <immintrin.h>
#include <stdint.h>

#define vpset16(value) _mm256_set1_epi16(value)
const __m256i _snowv_mul  = _mm256_blend_epi32(vpset16( 0x990f), vpset16( 0xc963), 0xf0);
const __m256i _snowv_inv  = _mm256_blend_epi32(vpset16(-0xcc87), vpset16(-0xe4b1), 0xf0);
const __m128i _snowv_aead = _mm_lddqu_si128((__m128i*)"AlexEkd JingThom");
const __m128i _snowv_sigma= _mm_set_epi8(15,11,7,3,14,10,6,2,13,9,5,1,12,8,4,0);
const __m128i _snowv_zero = _mm_setzero_si128();

struct SnowV256{
  __m256i hi, lo;     // LFSR
  __m128i R1, R2, R3; // FSM

  inline __m128i keystream(void){
    // Extract the tags T1 and T2
    __m128i T1 = _mm256_extracti128_si256(hi, 1);
    __m128i T2 = _mm256_castsi256_si128(lo);

    // LFSR Update
    __m256i mulx =  _mm256_xor_si256(_mm256_slli_epi16(lo, 1),
                    _mm256_and_si256(_snowv_mul, _mm256_srai_epi16(lo, 15)));
    __m256i invx =  _mm256_xor_si256(_mm256_srli_epi16(hi, 1),
                    _mm256_sign_epi16(_snowv_inv, _mm256_slli_epi16(hi, 15)));
    __m256i hi_old = hi;
    hi =  _mm256_xor_si256(
            _mm256_xor_si256(
              _mm256_blend_epi32(
                _mm256_alignr_epi8(hi, lo, 1 * 2),
                _mm256_alignr_epi8(hi, lo, 3 * 2),
                0xf0),
              _mm256_permute4x64_epi64(lo, 0x4e)),
            _mm256_xor_si256(invx, mulx));
    lo = hi_old;

    // Keystream word
    __m128i z = _mm_xor_si128(R2, _mm_add_epi32(R1, T1));

    // FSM Update
    __m128i R3new = _mm_aesenc_si128(R2, _snowv_zero);
    __m128i R2new = _mm_aesenc_si128(R1, _snowv_zero);
    R1 = _mm_shuffle_epi8(_mm_add_epi32(R2, _mm_xor_si128(R3, T2)), _snowv_sigma);
    R3 = R3new;
    R2 = R2new;
    return z;
  }

  inline void keyiv_setup(const unsigned char * key, const unsigned char * iv)
  {
    R1 = R2 = R3 = _mm_setzero_si128();
    hi = _mm256_lddqu_si256((const __m256i*)key);
    lo = _mm256_zextsi128_si256(_mm_lddqu_si128((__m128i*)iv));

    for (int i = 0; i < 15; ++i)
      hi = _mm256_xor_si256(hi, _mm256_zextsi128_si256( keystream() ));

    R1 = _mm_xor_si128(R1, _mm_lddqu_si128((__m128i*)(key + 0)));
    hi = _mm256_xor_si256(hi, _mm256_zextsi128_si256( keystream() ));
    R1 = _mm_xor_si128(R1, _mm_lddqu_si128((__m128i*)(key + 16)));
  }
};

// Testing
#include <stdio.h>
int main()
{
  uint8_t key[32] = {0};
  uint8_t iv[16] = {0};
  uint8_t plaintext[64] = {0};

  // Thevectors are written with theleast significant byteof the 128-bit word appearing to theleft in the row.

  SnowV256 snowv;
  snowv.keyiv_setup(key, iv);

  for (int i = 0; i < 64; ++i) {
    __m128i aux = snowv.keystream();
    for (int j = 0; j < 16; ++j)
      printf("%02x ", ((uint8_t*)&aux)[j]);
    printf("\n");
  }

  return 0;
}