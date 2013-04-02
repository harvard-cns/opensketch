#include "sketch.h"



void Sketch::setup(int field, int numRows, int countersPerRow) {

  hashInfo.field = field;
  hashInfo.numHashValues = numRows;
  hashInfo.range = countersPerRow;
  hashInfo.rev = HASHTYPE_DIETZTHORUP32;

  counterInfo.numRows = numRows;
  counterInfo.countersPerRow = countersPerRow;
  // sketch_id, nextOffset, counterSize is set by Calling Function
}

tHashInfo Sketch::getHashInfo() {
  return hashInfo;
}

tCounterInfo Sketch::getCounterInfo() {
  return counterInfo;
}

int Sketch::getSize(){
  return counterInfo.numRows * counterInfo.countersPerRow;
}
 
