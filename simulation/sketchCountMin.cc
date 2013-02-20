#include "sketchCountMin.h"


SketchCountMin::SketchCountMin() {
}

void SketchCountMin::setup(int field, int numRows, int countersPerRow) {

  hashInfo.field = field;
  hashInfo.numHashValues = numRows;
  hashInfo.range = countersPerRow;

  counterInfo.numRows = numRows;
  counterInfo.countersPerRow = countersPerRow;
  counterInfo.updateType = UPDATETYPE_INCREMENT;
  // sketch_id, nextOffset, counterSize is set by Calling Function
}

SketchCountMin::~SketchCountMin() {
}

tHashInfo SketchCountMin::getHashInfo() {
  return hashInfo;
}

tCounterInfo SketchCountMin::getCounterInfo() {
  return counterInfo;
}

int SketchCountMin::getSize(){
  return counterInfo.numRows * counterInfo.countersPerRow;
}

int SketchCountMin::query(const vector<int>& counts) {
  return *(min_element(counts.begin(), counts.end()));
}
 
