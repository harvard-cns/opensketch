#ifndef __SKETCHCOUNTMIN_H
#define __SKETCHCOUNTMIN_H
#include "common.h"

class SketchCountMin {
  tHashInfo hashInfo;
  tCounterInfo counterInfo;

 public:
  SketchCountMin();
  ~SketchCountMin();

  void setup(int field, int numRows, int countersPerRow);
  tHashInfo getHashInfo();
  tCounterInfo getCounterInfo();
  int getSize();
  int query(const vector<int>& counts);
};
#endif  
