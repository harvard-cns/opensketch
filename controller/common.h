#ifndef _COMMON_H_
#define _COMMON_H_


#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "packet.h"
#include "counterInfo.h"
#include "hashInfo.h"

#include <stdio.h>
#include <string.h>
#include <algorithm>

#include <vector>
#include <map>
#include <queue>
#include <fstream>
#include <numeric>
#include <cmath>

typedef unsigned long long  uint64;
typedef unsigned int        uint32;
typedef unsigned short      uint16;
typedef unsigned char       uint8;

const uint64 MAXUINT64 = (uint64) (-1);
const uint32 MAXUINT32 = (uint32) (-1);
const uint16 MAXUINT16 = (uint16) (-1);
const uint8  MAXUINT8  = (uint8)  (-1);
const double E = (double) (2.71828182846);

const int FIELD_SRCIP = 1;
const int FIELD_DSTIP = 2;

const int UPDATETYPE_SET = 1;
const int UPDATETYPE_INCREMENT = 2;
const int UPDATETYPE_NEXT = 3;

const int ONELEVEL_FUNCTION = 1;
const int TWOLEVEL_FUNCTION = 2;


const int HASHTYPE_DIETZTHORUP32 = 1;
const int HASHTYPE_REVERSIBLE8TO3 = 2;

typedef union {
  uint64 as_int64; 
  uint32 as_int32s[2];
  uint16 as_int16s[4];
  uint8  as_int8s[8];
} int64views;

typedef union {
  uint32 as_int32; 
  uint16 as_int16s[2];
  uint8  as_int8s[4];
} int32views;

namespace hash{                                                                            
  const uint64 A[] = {59032440799460394LL,\
		      1380096083914250750LL,\
		      9216393848249138261LL,\
		      1829347879307711444LL,\
		      9218705108064111365LL};

  // later change to regular uint64 A[] and use randint64()
  const uint64 B[] = {832108633134565846LL,\
		      9207888196126356626LL,\
		      1106582827276932161LL,\
		      7850759173320174309LL,\
		      8297516128533878091LL};
};

uint32 commonHash(uint32 value, uint32 hashFnIndex);

char *MakeCRC(char *BitString);

uint32 os_dietz_thorup32(uint32 x, uint32 bins, uint64 a, uint64 b);
uint8 os_dietz8to3(uint8 x, uint8 a);
uint32 reversible4096(uint32 value, uint32 bins, uint64 a);

// ----------- from Reversible Sketch Code at http://www.zhichunli.org/software/download.php?file=RevSketch-1.0.tar.gz 

double os_current_time();
// Generates a 32-bit random string
// Note that rand() is not random enough -- it has a period of 2^32.
uint32 os_rand32bit();
#endif
