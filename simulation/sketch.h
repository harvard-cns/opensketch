#ifndef __SKETCH_H
#define __SKETCH_H
#include "common.h"

class Sketch {

 protected:
  tHashInfo hashInfo;
  tCounterInfo counterInfo;

 public:
  void setup(int field, int numRows, int countersPerRow);
  //  int query(const vector<int>& counts);

  tHashInfo getHashInfo();
  tCounterInfo getCounterInfo();
  int getSize();

};
#endif  
