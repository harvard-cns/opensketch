#include "taskHeavyHitters.h"

TaskHeavyHitters::TaskHeavyHitters(){
}

TaskHeavyHitters::~TaskHeavyHitters(){
}

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

void TaskHeavyHitters::getHashSeedsFromDataPlane(const DataPlane &dataPlane) {
  hashA = dataPlane.getHashA();
  hashB = dataPlane.getHashB();
}

void TaskHeavyHitters::updateCountersFromDataPlane(DataPlane &dataPlane) {
  sramCounters = dataPlane.getSRAM(task_id);
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

void TaskHeavyHitters::updateAddresses(queue<int>& addresses, const tCounterInfo& counterInfo, const vector<int>& hashValues) {
  int startingAddresses = addresses.size();
  int offset;
  int index;
  while(startingAddresses > 0) {
    offset = addresses.front(); 
    addresses.pop();
    startingAddresses--;
    for (int i = 0; i < counterInfo.numRows; i++) {
      //printf("address in row %d\n", i);
      index = offset;
      //printf(".. index = offset %d\n", index);
      index += i * counterInfo.countersPerRow;
      //printf(".. index += i * counterInfo.countersPerRow %d\n", index);
      index += hashValues[i] % counterInfo.countersPerRow;
      //printf(".. hashValues[i] mod counterInfo.countersPerRow %d\n", index);
      index *= counterInfo.counterSize;
      //printf(".. index *= counterInfo.counterSize %d\n", index);
      addresses.push(index);
      //printf(".. pushing %d\n", index);
    }
  }

}

int TaskHeavyHitters::getField(const Packet& p, int field){
  if (field == FIELD_SRCIP) return p.srcip;
  else if (field == FIELD_DSTIP) return p.dstip;

}


void TaskHeavyHitters::getHashValues(int x, const tHashInfo& hashInfo, vector<int>& hashValues) {
  hashValues.clear();
  for(int i = 0; i < hashInfo.numHashValues; i++) {
    hashValues.push_back(os_dietz_thorup32(x, hashInfo.range, hashA[i], hashB[i]));
  }
}

int TaskHeavyHitters::getTaskId() {
  return task_id;
}



