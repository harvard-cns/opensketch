#include "sketchBitmap.h"


SketchBitmap::SketchBitmap() {
}

void SketchBitmap::setup(int field, int numBits) {

  hashInfo.field = field;
  hashInfo.numHashValues = 1;
  hashInfo.range = numBits;

  counterInfo.numRows = 1;
  counterInfo.countersPerRow = numBits;
  counterInfo.updateType = UPDATETYPE_SET;
  // sketch_id, nextOffset, counterSize is set by Calling Function
}

SketchBitmap::~SketchBitmap() {
}

tHashInfo SketchBitmap::getHashInfo() {
  return hashInfo;
}

tCounterInfo SketchBitmap::getCounterInfo() {
  return counterInfo;
}

int SketchBitmap::getSize(){
  return counterInfo.numRows * counterInfo.countersPerRow;
}

int SketchBitmap::query(const vector<int>& counts) {
  int setBits = accumulate(counts.begin(), counts.end(), 0);
  int numBits = counts.size();
  int zeroBits = numBits - setBits;

  double estimate = zeroBits * log(((double) numBits)/zeroBits);
  return (int) estimate;
}
 
