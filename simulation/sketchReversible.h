#ifndef __SKETCHREVERSIBLE_H
#define __SKETCHREVERSIBLE_H

#include "sketch.h"
#include "os_mangler.h"
#include "os_counterrevanalyze.h"

class SketchReversible : public Sketch {
  Mangler* mangler;
  OS_CounterRevAnalyze* counterreva;

 public:
  SketchReversible();
  ~SketchReversible();
  void setup(int field);//, int numRows, int countersPerRow);
  //int query(const vector<int>& counts);
  void get_heavy_keys(vector<uint32>& counts, vector<uint32>& heavy_keys, uint32 threshold, vector<uint64>& hash);
};
#endif  
