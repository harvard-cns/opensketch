#include "sketchBitmap.h"

void SketchBitmap::setup(int field, int numBits, int max) {
  Sketch::setup(field, 1, numBits);
  counterInfo.updateType = UPDATETYPE_SET;
  this->max = max;
  // sketch_id, nextOffset, counterSize is set by Calling Function
}

int SketchBitmap::query(const vector<int>& counts) {
  int setBits = accumulate(counts.begin(), counts.end(), 0);
  int numBits = counts.size();
  int zeroBits = numBits - setBits;

  double load = ((double)max)/numBits;
  double logEmpty =  log(((double) numBits)/zeroBits);
  double estimate = numBits * logEmpty;

  if (zeroBits < 0.3 * numBits) {return (load * numBits * log(1.0/0.3)) + 1;}
  //  if (numBits == zeroBits) return MAXUINT32;

  return (int) estimate;
}
 
