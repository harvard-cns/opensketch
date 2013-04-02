#ifndef __SKETCHBITMAP_H
#define __SKETCHBITMAP_H
#include "sketch.h"

class SketchBitmap : public Sketch {
  int max;

 public:
  void setup(int field, int numBits, int max); // directly
  int query(const vector<int>& counts);
};
#endif  
