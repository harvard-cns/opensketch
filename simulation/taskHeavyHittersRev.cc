#include "taskHeavyHittersRev.h"

void TaskHeavyHittersRev::setUserPreferencesDirectly(int field, int numRows, int countersPerRow){
  TaskHeavyHitters::setUserPreferencesDirectly(field, numRows, countersPerRow);
  reversible.setup(field);
}

void TaskHeavyHittersRev::setUserPreferencesDirectly(int field, int numRows, int countersPerRow, int threshold){
  setUserPreferencesDirectly(field, numRows, countersPerRow);
  this->threshold = threshold;
}


void TaskHeavyHittersRev::configureDataPlane(DataPlane &dataPlane) {
  
  cm_id = dataPlane.addHashFunctions(countMin.getHashInfo());
  rev_id = dataPlane.addHashFunctions(reversible.getHashInfo());

  int taskSize = 0;
  taskSize += countMin.getSize();
  taskSize += reversible.getSize();
  task_id = dataPlane.setSRAMSize(taskSize);

  vector<tCounterInfo> counterInfos;
  tCounterInfo countMinCounterInfo = countMin.getCounterInfo();
  countMinCounterInfo.counterSize = 1;
  countMinCounterInfo.sketch_id = cm_id;
  countMinCounterInfo.nextOffset = countMin.getSize(); // okay to access, won't make any difference
  counterInfos.push_back(countMinCounterInfo);

  tCounterInfo reversibleCounterInfo = reversible.getCounterInfo();
  reversibleCounterInfo.counterSize = 1;
  reversibleCounterInfo.sketch_id = rev_id;
  reversibleCounterInfo.nextOffset = 0; // okay to access, won't make any difference
  counterInfos.push_back(reversibleCounterInfo);

  dataPlane.setPacketProcessing(task_id, counterInfos);
}


void TaskHeavyHittersRev::getManglerFromDataPlane(const DataPlane &dataPlane) {
  mangler = new Mangler(dataPlane.getMangleSeed1(), dataPlane.getMangleSeed2());
}


void TaskHeavyHittersRev::getHeavyHitters(vector<int>&heavyHitters) {
  heavyHitters.clear();

  vector<uint32> counts;
  queue<int> addresses;

  int reversibleOffset = countMin.getSize();
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
  while(!addresses.empty()) {
    
    addr = addresses.front();      

    addresses.pop();
    counts.push_back(sramCounters[addr]);

    //    printf("getting counter at %d: %d\n", addr, sramCounters[addr]);
  }

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
      heavyHitters.push_back(tmp);
      cout << "is heavy .." << str << ": " << count << "\n";
    }
    //    else cout << "not heavy .. " <<   tmp << ": " << count << "\n";
  }

}



