#include "taskHeavyHitters.h"


void TaskHeavyHitters::setUserPreferencesDirectly(int field, int numRows, int countersPerRow) {
  countMin.setup(field, numRows, countersPerRow);
}

void TaskHeavyHitters::configureDataPlane(DataPlane &dataPlane) {
  
  cm_id = dataPlane.addHashFunctions(countMin.getHashInfo());

  int taskSize = 1;
  taskSize *= countMin.getSize();
  task_id = dataPlane.setSRAMSize(taskSize);

  vector<tCounterInfo> counterInfos;
  tCounterInfo countMinCounterInfo = countMin.getCounterInfo();
  countMinCounterInfo.counterSize = 1;
  countMinCounterInfo.sketch_id = cm_id;
  countMinCounterInfo.nextOffset = 0; // okay to access, won't make any difference
  counterInfos.push_back(countMinCounterInfo);

  dataPlane.setPacketProcessing(task_id, counterInfos);
}


int TaskHeavyHitters::queryGivenKey(int key) {
  vector<int> counts;
  queue<int> addresses;

  addresses.push(0);
  vector<int> hashValues;

  getHashValues(key, countMin.getHashInfo(), hashValues);
  //for (int i = 0; i < hashValues.size(); i++) {printf("%d, ", hashValues[i]);}
  //printf(" are the hash values for this key %d.\n", key);

  tCounterInfo countMinCounterInfo = countMin.getCounterInfo();
  countMinCounterInfo.counterSize = 1;
  countMinCounterInfo.nextOffset = 0;

  updateAddresses(addresses, countMinCounterInfo, hashValues);
  int addr;
  while(!addresses.empty()) {
    //printf("getting counter at %d\n", addr);
    addr = addresses.front();      
    addresses.pop();
    counts.push_back(sramCounters[addr]);
  }

  int ret = countMin.query(counts); 
  return ret;
}

int TaskHeavyHitters::queryGivenPacket(const Packet& p){
  int key = getField(p, field);
  return queryGivenKey(key);
}
