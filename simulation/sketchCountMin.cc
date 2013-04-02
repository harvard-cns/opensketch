#include "sketchCountMin.h"


void SketchCountMin::setup(int field, int numRows, int countersPerRow) {
  Sketch::setup(field, numRows, countersPerRow);
  counterInfo.updateType = UPDATETYPE_INCREMENT;
  // sketch_id, nextOffset, counterSize is set by Calling Function
}

int SketchCountMin::query(const vector<int>& counts) {
  return *(min_element(counts.begin(), counts.end()));
}
 
