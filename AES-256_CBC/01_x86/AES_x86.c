#include <wmmintrin.h>

// TODO: Arreglar fallo en linea 39

// AES-256 Key Expansion
void KEY_256_ASSIST_1(__m128i* temp1, __m128i * temp2)
{
  __m128i temp4;
  *temp2 = _mm_shuffle_epi32(*temp2, 0xff);
  temp4 = _mm_slli_si128 (*temp1, 0x4);
  *temp1 = _mm_xor_si128 (*temp1, temp4);
  temp4 = _mm_slli_si128 (temp4, 0x4);
  *temp1 = _mm_xor_si128 (*temp1, temp4);
  temp4 = _mm_slli_si128 (temp4, 0x4);
  *temp1 = _mm_xor_si128 (*temp1, temp4);
  *temp1 = _mm_xor_si128 (*temp1, *temp2);
}

void KEY_256_ASSIST_2(__m128i* temp1, __m128i * temp3)
{
  __m128i temp2,temp4;
  temp4 = _mm_aeskeygenassist_si128 (*temp1, 0x0);
  temp2 = _mm_shuffle_epi32(temp4, 0xaa);
  temp4 = _mm_slli_si128 (*temp3, 0x4);
  *temp3 = _mm_xor_si128 (*temp3, temp4);
  temp4 = _mm_slli_si128 (temp4, 0x4);
  *temp3 = _mm_xor_si128 (*temp3, temp4);
  temp4 = _mm_slli_si128 (temp4, 0x4);
  *temp3 = _mm_xor_si128 (*temp3, temp4);
  *temp3 = _mm_xor_si128 (*temp3, temp2);
}

void AES_256_Key_Expansion (const unsigned char *userkey,
  unsigned char *key)
{
  __m128i temp1, temp2, temp3;
  __m128i *Key_Schedule = (__m128i*)key;

  temp1 = _mm_loadu_si128((__m128i*)userkey);
  temp3 = _mm_loadu_si128((__m128i*)(userkey+16));
  Key_Schedule[0] = temp1;
  Key_Schedule[1] = temp3;
  temp2 = _mm_aeskeygenassist_si128 (temp3,0x01);
  KEY_256_ASSIST_1(&temp1, &temp2);
  Key_Schedule[2]=temp1;
  KEY_256_ASSIST_2(&temp1, &temp3);
  Key_Schedule[3]=temp3;
  temp2 = _mm_aeskeygenassist_si128 (temp3,0x02);
  KEY_256_ASSIST_1(&temp1, &temp2);
  Key_Schedule[4]=temp1;
  KEY_256_ASSIST_2(&temp1, &temp3);
  Key_Schedule[5]=temp3;
  temp2 = _mm_aeskeygenassist_si128 (temp3,0x04);
  KEY_256_ASSIST_1(&temp1, &temp2);
  Key_Schedule[6]=temp1;
  KEY_256_ASSIST_2(&temp1, &temp3);
  Key_Schedule[7]=temp3;
  temp2 = _mm_aeskeygenassist_si128 (temp3,0x08);
  KEY_256_ASSIST_1(&temp1, &temp2);
  Key_Schedule[8]=temp1;
  KEY_256_ASSIST_2(&temp1, &temp3);
  Key_Schedule[9]=temp3;
  temp2 = _mm_aeskeygenassist_si128 (temp3,0x10);
  KEY_256_ASSIST_1(&temp1, &temp2);
  Key_Schedule[10]=temp1;
  KEY_256_ASSIST_2(&temp1, &temp3);
  Key_Schedule[11]=temp3;
  temp2 = _mm_aeskeygenassist_si128 (temp3,0x20);
  KEY_256_ASSIST_1(&temp1, &temp2);
  Key_Schedule[12]=temp1;
  KEY_256_ASSIST_2(&temp1, &temp3);
  Key_Schedule[13]=temp3;
  temp2 = _mm_aeskeygenassist_si128 (temp3,0x40);
  KEY_256_ASSIST_1(&temp1, &temp2);
  Key_Schedule[14]=temp1;
}

// AES-256 Encryption in CBC mode
void AES_CBC_encrypt( const unsigned char *in,
                      unsigned char *out,
                      unsigned char ivec[16],
                      unsigned long length,
                      unsigned char *key,
                      int number_of_rounds)
  {
    __m128i feedback,data;
    int i,j;

    if (length%16)
      length = length/16+1;
    else length /=16;

    feedback=_mm_loadu_si128 ((__m128i*)ivec);
    for(i=0; i < length; i++){
      data = _mm_loadu_si128 (&((__m128i*)in)[i]);
      feedback = _mm_xor_si128 (data,feedback);
      feedback = _mm_xor_si128 (feedback,((__m128i*)key)[0]);
      for(j=1; j <number_of_rounds; j++)
        feedback = _mm_aesenc_si128 (feedback,((__m128i*)key)[j]);
      feedback = _mm_aesenclast_si128 (feedback,((__m128i*)key)[j]);
      _mm_storeu_si128 (&((__m128i*)out)[i],feedback);
    }
  }

// AES-256 Decryption in CBC mode
void AES_CBC_decrypt( const unsigned char *in,
                      unsigned char *out,
                      unsigned char ivec[16],
                      unsigned long length,
                      unsigned char *key,
                      int number_of_rounds)
  {
  __m128i data,feedback,last_in;
  int i,j;

  if (length%16)
    length = length/16+1;
  else length /=16;

  feedback=_mm_loadu_si128 ((__m128i*)ivec);
  for(i=0; i < length; i++){
    last_in=_mm_loadu_si128 (&((__m128i*)in)[i]);
    data = _mm_xor_si128 (last_in,((__m128i*)key)[0]);
    for(j=1; j <number_of_rounds; j++){
      data = _mm_aesdec_si128 (data,((__m128i*)key)[j]);
    }
    data = _mm_aesdeclast_si128 (data,((__m128i*)key)[j]);
    data = _mm_xor_si128 (data,feedback);
    _mm_storeu_si128 (&((__m128i*)out)[i],data);
    feedback=last_in;
    }
  }

#include <stdio.h>
int main() {
  unsigned char key[32];
  unsigned char in[16];
  unsigned char out[16];
  unsigned char ivec[16];
  int i;

  for (i=0; i<16; i++) {
    in[i] = i;
    ivec[i] = i;
  }

  AES_256_Key_Expansion(in, key);

  AES_CBC_encrypt(in, out, ivec, 16, key, 14);
  for (i=0; i<16; i++) printf("%02x", out[i]);
  printf("\n");

  AES_CBC_decrypt(out, in, ivec, 16, key, 14);
  for (i=0; i<16; i++) printf("%02x", in[i]);
  printf("\n");

  return 0;
}