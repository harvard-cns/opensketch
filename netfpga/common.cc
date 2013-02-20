#include "common.h"

using namespace std;

// http://en.wikipedia.org/wiki/Universal_hashing is great!
uint32 os_dietz_thorup32(uint32 x, uint32 bins, uint64 a, uint64 b){
  // ((ax mod 2**62) + b) mod 2 ** 62, hi 32, pad 0 on left
  // = (ax + b) mod 2 ** 62, hi 32, pad 0 on the left

  return ((uint32) ((a*x+b) >> 32)) % bins;
  // mod 64 then top 32 bits 
  // (this impl. okay for little endian)
  // wiki says just ax .. is 2-universal
  // why is it strongly universal/ 2-wise indep.?
}

// 2-universal hashing, assumes bins power of 2, very fast
uint8 os_dietz8to3(uint8 x, uint8 a) {
  return (((a*x)  >> 5) & 7);
}
// mod 8 then top 3 bits


uint32 reversible4096(uint32 value, uint32 bins, uint64 hasha) {
  uint32 j, index, tmp; 



  uint8 a, x;  
  int64views vv, vha;
  //cout << "hashing value " << value << "\n";
  //cout << "hashing value " << value << "\n";
  vv.as_int64 = value;
  vha.as_int64 = hasha;
  index = 0;
  
  for (j = 0; j < 4; j++) {
      x = vv.as_int8s[j];
      a = vha.as_int8s[j];
      /*cout << "x: " << (uint32) (x & 0xFF)\
	<< ", a: " << (uint32) (a & 0xFF)\
	<< ", b: " << (uint32) (b & 0xFF) << "\n";*/
      tmp = os_dietz8to3(x, a) & 0xFF;
      //cout << "tmp: " << tmp << "\n";
      tmp = tmp << 3*j;
      index += tmp;
    }
  return index;

}
// ----------- from Reversible Sketch Code at http://www.zhichunli.org/software/download.php?file=RevSketch-1.0.tar.gz 

double os_current_time()
{
  struct timeval tv;
  assert ( gettimeofday(&tv, 0) >= 0 );


  return double(tv.tv_sec) + double(tv.tv_usec) / 1e6;
}

// Generates a 32-bit random string
// Note that rand() is not random enough -- it has a period of 2^32.
uint32 os_rand32bit()
{
  uint32 base = 0;
  uint32 i;

  for (i=1; i<=2; i++)
    base = (base<<16) | rand();
  return(base);
}


// ==========================================================================
// CRC Generation Unit - Linear Feedback Shift Register implementation
// (c) Kay Gorontzi, GHSi.de, distributed under the terms of LGPL
// ==========================================================================
char *MakeCRC(char *BitString)
{
  static char Res[34];                                 // CRC Result
  char CRC[33];
  int  i;
  char DoInvert;
   
  for (i=0; i<33; ++i)  CRC[i] = 0;                    // Init before calculation
   
  for (i=0; i<strlen(BitString); ++i)
    {
      DoInvert = ('1'==BitString[i]) ^ CRC[32];         // XOR required?

      CRC[32] = CRC[31];
      CRC[31] = CRC[30] ^ DoInvert;
      CRC[30] = CRC[29];
      CRC[29] = CRC[28] ^ DoInvert;
      CRC[28] = CRC[27];
      CRC[27] = CRC[26] ^ DoInvert;
      CRC[26] = CRC[25];
      CRC[25] = CRC[24] ^ DoInvert;
      CRC[24] = CRC[23];
      CRC[23] = CRC[22];
      CRC[22] = CRC[21] ^ DoInvert;
      CRC[21] = CRC[20];
      CRC[20] = CRC[19] ^ DoInvert;
      CRC[19] = CRC[18];
      CRC[18] = CRC[17] ^ DoInvert;
      CRC[17] = CRC[16];
      CRC[16] = CRC[15] ^ DoInvert;
      CRC[15] = CRC[14];
      CRC[14] = CRC[13] ^ DoInvert;
      CRC[13] = CRC[12];
      CRC[12] = CRC[11] ^ DoInvert;
      CRC[11] = CRC[10];
      CRC[10] = CRC[9] ^ DoInvert;
      CRC[9] = CRC[8] ^ DoInvert;
      CRC[8] = CRC[7];
      CRC[7] = CRC[6] ^ DoInvert;
      CRC[6] = CRC[5];
      CRC[5] = CRC[4];
      CRC[4] = CRC[3] ^ DoInvert;
      CRC[3] = CRC[2];
      CRC[2] = CRC[1] ^ DoInvert;
      CRC[1] = CRC[0];
      CRC[0] = DoInvert;
    }
      
  for (i=0; i<33; ++i)  Res[32-i] = CRC[i] ? '1' : '0'; // Convert binary to ASCII
  Res[33] = 0;                                         // Set string terminator

  return(Res);
}

uint32 commonHash(uint32 value, uint32 hashFnIndex) {
  return 0;
}
