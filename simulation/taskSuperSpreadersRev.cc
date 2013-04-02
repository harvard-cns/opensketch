#include "taskSuperSpreadersRev.h"


void TaskSuperSpreadersRev::setUserPreferencesDirectly1(int k, int b, int delta, int field1, int field2, int numRows, int countersPerRow, int numBits, int threshold) {
  TaskSuperSpreaders::setUserPreferencesDirectly1(k, b, delta, field1, field2, numRows, countersPerRow, numBits);
  reversible.setup(field1);
  this->threshold = threshold; 
}

void TaskSuperSpreadersRev::setUserPreferencesDirectly(int field1, int numRows, int countersPerRow, int field2, int numBits, double ratio, int threshold) {
  TaskSuperSpreaders::setUserPreferencesDirectly(field1, numRows, countersPerRow, field2, numBits, threshold, ratio);
  reversible.setup(field1);
  this->threshold = threshold;
}

void TaskSuperSpreadersRev::configureDataPlane(DataPlane &dataPlane) {
  dataPlane.addSampling(countMin.getHashInfo().field*10\
			+bitmap.getHashInfo().field,   \
			MAXUINT32 * samplingRatio);

  cm_id = dataPlane.addHashFunctions(countMin.getHashInfo());
  bm_id = dataPlane.addHashFunctions(bitmap.getHashInfo());
  rev_id = dataPlane.addHashFunctions(reversible.getHashInfo());
  

  int taskSize = (countMin.getSize() + reversible.getSize()) * bitmap.getSize();
  printf("taskSize is %d\n", taskSize);
  task_id = dataPlane.setSRAMSize(taskSize);

  vector<tCounterInfo> counterInfos;
  tCounterInfo countMinCounterInfo = countMin.getCounterInfo();
  countMinCounterInfo.counterSize = bitmap.getSize();
  countMinCounterInfo.sketch_id = cm_id;
  countMinCounterInfo.updateType = UPDATETYPE_NEXT;
 // shouldn't check nextOffset if type is UPDATETYPE_NEXT
  counterInfos.push_back(countMinCounterInfo);

  tCounterInfo bitmapCounterInfo = bitmap.getCounterInfo();
  bitmapCounterInfo.counterSize = 1;
  bitmapCounterInfo.sketch_id = bm_id;
  bitmapCounterInfo.nextOffset = countMin.getSize() * bitmap.getSize();
  counterInfos.push_back(bitmapCounterInfo);

  tCounterInfo reversibleCounterInfo = reversible.getCounterInfo();
  reversibleCounterInfo.counterSize = bitmap.getSize();
  reversibleCounterInfo.sketch_id = rev_id;
  reversibleCounterInfo.updateType = UPDATETYPE_NEXT;
 // shouldn't check nextOffset if type is UPDATETYPE_NEXT
  counterInfos.push_back(reversibleCounterInfo);

  bitmapCounterInfo.nextOffset = 0; // okay to access, won't make any difference
  counterInfos.push_back(bitmapCounterInfo);

  dataPlane.setPacketProcessing(task_id, counterInfos);
}

void TaskSuperSpreadersRev::getManglerFromDataPlane(const DataPlane &dataPlane) {
  mangler = new Mangler(dataPlane.getMangleSeed1(), dataPlane.getMangleSeed2());
}

void TaskSuperSpreadersRev::getSuperSpreaders(vector<int>&superSpreaders) {

  superSpreaders.clear();

  vector<uint32> counts;
  queue<int> addresses;

  int reversibleOffset = countMin.getSize() * bitmap.getSize();
  addresses.push(reversibleOffset);
  vector<int> hashValues;

  //  getHashValues(key, countMin.getHashInfo(), hashValues);
  //for (int i = 0; i < hashValues.size(); i++) {printf("%d, ", hashValues[i]);}
  //printf(" are the hash values for this key %d.\n", key);

  tCounterInfo reversibleCounterInfo = reversible.getCounterInfo();
  reversibleCounterInfo.counterSize = 1;
  reversibleCounterInfo.nextOffset = 0;

  updateAddresses(addresses, reversibleCounterInfo, hashValues);
  int addr;

  //  printf("reversible counter addresses\n");
  //  printf("total number of counters %d\n", sramCounters.size());
  printf("\n");
  while(!addresses.empty()) {
    
    addr = addresses.front();      

    addresses.pop();

    vector<int> countsForBitmap;
    int tmp;
    int bitsSet = 0;
    int totalBits = bitmap.getSize();
    for(int i = 0; i < totalBits; i++) {
      tmp = sramCounters[addr+i];
      bitsSet += tmp;
      countsForBitmap.push_back(tmp);
    }

    uint32 bitmapEstimate = bitmap.query(countsForBitmap);
    if (bitmapEstimate > threshold)
      printf("%u, ", bitmapEstimate);
    counts.push_back(bitmapEstimate);


    //    printf("getting counter at %d: %d\n", addr, sramCounters[addr]);
  }
  printf("\n\n\n\n");
  vector<uint32> heavyKeys;
  assert(threshold > 0);
  reversible.get_heavy_keys(counts, heavyKeys, threshold, hashA);
  // still mangled

  cout << "threshold: " << threshold << endl;
  cout << heavyKeys.size() << " heavy keys." <<  endl;
  cout << "heavy keys:\n"; 

  cout << "checking heavy keys against countercm.\n";
  uint32 tmp;
  int count;
  for(int n = 0; n < heavyKeys.size(); n++) {
    tmp = mangler->ReverseShortTable((uint32) heavyKeys[n]);
    count = queryGivenKey(tmp);
    if (count > threshold) {
      char str[20];
      os_ipint2string(tmp, str);
      superSpreaders.push_back(tmp);
      cout << "!!! -------        ----------    is heavy .." << str << ": " << count << "\n";
    }
    //        else cout << "not heavy .. " <<   tmp << ": " << count << "\n";
  }

}

