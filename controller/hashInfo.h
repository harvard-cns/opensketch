#ifndef _HASHINFO_H
#define _HASHINFO_H
#include "common.h"

using namespace std;

typedef struct HashInfo {
  int field; // 1: srcip, 2:dstip, .. in common.h 
  int numHashValues;
  int range; // output range
  vector<int> hashFnIndexes; // not used by dataPlane
} tHashInfo;

#endif
