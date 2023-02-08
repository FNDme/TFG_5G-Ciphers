#include <immintrin.h>
#define vpset16(value) _mm256_set1_epi16(value)

__m256i _mm256_zextsi128_si256 (__m128i x)
{
  return __extension__ (__m256i) { 0, 0, x[0], x[1] };
}

struct SnowV256
{
  // Constants
  __m256i _mul, _inv;
  __m128i zero128;    // AES RoundKeys
  // State
  __m256i hi, lo;     // LFSR
  __m128i R1, R2, R3; // FSM

  SnowV256()
  {
    _mul = _mm256_blend_epi32(vpset16( 0x990f), vpset16( 0xc963), 0xf0);
    _inv = _mm256_blend_epi32(vpset16( 0xcc87), vpset16( 0xe4b1), 0xf0);
    zero128 = _mm_setzero_si128();
  }

  inline __m256i mul_x(__m256i s)
  {
    return  _mm256_xor_si256(
              _mm256_and_si256(_mul,
                _mm256_srai_epi16(s, 15)),
              _mm256_slli_epi16(s, 1));
  }
  inline __m256i mul_x_inv(__m256i s)
  {
    return  _mm256_xor_si256(
              _mm256_sign_epi16(_inv,
                _mm256_slli_epi16(s, 15)),
              _mm256_srli_epi16(s, 1));
  }
  inline void lfsr_update(void)
  {
    __m256i hi_old = hi;
    hi =  _mm256_xor_si256(
            _mm256_xor_si256(
              _mm256_blend_epi32(
                _mm256_alignr_epi8(hi, lo, 1 * 2),
                _mm256_alignr_epi8(hi, lo, 3 * 2), 0xf0),
              _mm256_permute4x64_epi64(lo, 0x4e)),
            _mm256_xor_si256(mul_x_inv(hi), mul_x(lo)));
    lo = hi_old;
  }
  inline void fsm_update(void)
  {
    __m128i T2 = _mm256_castsi256_si128(lo);
    __m128i newR1 = _mm_add_epi32(R2, _mm_xor_si128(R3, T2));
    R3 = _mm_aesenc_si128(R2, zero128);
    R2 = _mm_aesenc_si128(R1, zero128);
    R1 = newR1;
  }
  inline __m128i keystream(void)
  {
    __m128i T1 = _mm256_extracti128_si256(hi, 1);
    __m128i z = _mm_xor_si128(R2, _mm_add_epi32(R1, T1));
    fsm_update();
    lfsr_update();
    return z;
  }
  inline void keyiv_setup(const unsigned char * key, const unsigned char * iv)
  {
    hi = _mm256_lddqu_si256((const __m256i*)key);
    lo = _mm256_zextsi128_si256(_mm_lddqu_si128((const __m128i*)iv));
    R1 = R2 = R3 = _mm_setzero_si128();
    for (int i = 0; i < 15; ++i)
      hi = _mm256_xor_si256(hi, _mm256_zextsi128_si256( keystream() ));
    R1 = _mm_xor_si128(R1, _mm_lddqu_si128((const __m128i*)(key + 0)));
    hi = _mm256_xor_si256(hi, _mm256_zextsi128_si256( keystream() ));
    R1 = _mm_xor_si128(R1, _mm_lddqu_si128((const __m128i*)(key + 16)));
  }
};

#include <stdio.h>
int main() {
  SnowV256 snowv;
  unsigned char key[32] = {0};
  unsigned char iv[16] = {0};
  snowv.keyiv_setup(key, iv);
  for (int i = 0; i < 16; ++i) {
    __m128i stream = snowv.keystream();
    for (int j = 0; j < 16; ++j) {
      printf("%02x ", ((unsigned char*)&stream)[j]);
    }
    printf("\n");
  }
  return 0;
}