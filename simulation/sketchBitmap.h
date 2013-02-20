#ifndef __SKETCHBITMAP_H
#define __SKETCHBITMAP_H
#include "common.h"

class SketchBitmap {
  tHashInfo hashInfo;
  tCounterInfo counterInfo;

 public:
  SketchBitmap();
  ~SketchBitmap();

  void setup(int field, int numBits); // directly
  tHashInfo getHashInfo();
  tCounterInfo getCounterInfo();
  int getSize();
  int query(const vector<int>& counts);
};
#endif  
