#include "sketchReversible.h"


SketchReversible::SketchReversible() {
  counterreva = new OS_CounterRevAnalyze();
  //mangler = new Mangler();

}

void SketchReversible::setup(int field){//, int numRows, int countersPerRow) {
  Sketch::setup(field, REVERSIBLE_NUMROWS, REVERSIBLE_COUNTERSPERROW);
  hashInfo.rev = HASHTYPE_REVERSIBLE8TO3;
  counterInfo.updateType = UPDATETYPE_INCREMENT;
  // sketch_id, nextOffset, counterSize is set by Calling Function
}

SketchReversible::~SketchReversible() {
  delete counterreva;
  //delete mangler;
}

void SketchReversible::get_heavy_keys(vector<uint32>& counts, vector<uint32>& heavy_keys, uint32 maxThres, vector<uint64>& hash) {
  uint32 i;
  for (i = 0; i < REVERSIBLE_NUMROWS; i++)
    counterreva->populateLookupTable(i, hash[i]);

  printf("populated lookup tables.\n");

  counterreva->update_counts(counts);

  printf("updated counts.\n");

  list<uint32> mangled_keys;

  counterreva->getKeys(REVERSIBLE_ITERSIZE, &maxThres, mangled_keys);
  printf("got %zu heavy(> %d) keys.\n", mangled_keys.size(), maxThres);
  //" << mangled_keys.size() << " heavy (>" << maxThres << ") keys.\n";

  list<uint32>::iterator mkIt;
  uint32 tmp;
  for (mkIt = mangled_keys.begin(), i=0;\
       mkIt != mangled_keys.end();
       mkIt++, i++){
    //    tmp = mangler.ReverseShortTable(*mkIt);
    heavy_keys.push_back(*mkIt);
  }


}

 
