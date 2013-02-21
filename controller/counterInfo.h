#ifndef _COUNTERINFO_H
#define _COUNTERINFO_H
#include "common.h"

#include <vector>
using namespace std;

typedef struct CounterInfo {
  int numRows;
  int countersPerRow;
  int counterSize;
  int updateType; // this is not used by dataplane
} tCounterInfo;
#endif
