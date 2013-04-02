#ifndef __SKETCHCOUNTMIN_H
#define __SKETCHCOUNTMIN_H
#include "sketch.h"

class SketchCountMin : public Sketch{

 public:
  void setup(int field, int numRows, int countersPerRow);
  int query(const vector<int>& counts);
};
#endif  
