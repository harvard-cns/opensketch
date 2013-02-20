#include "taskHeavyHitters.h"

TaskHeavyHitters::TaskHeavyHitters(){
}

TaskHeavyHitters::~TaskHeavyHitters(){
}

void TaskHeavyHitters::setUserPreferencesDirectly(int field, int numRows, int countersPerRow) {
  countMin.setup(field, numRows, countersPerRow);
}

void TaskHeavyHitters::configureDataPlane(DataPlane &dataPlane) {
  
  //cm_id = dataPlane.addHashFunctions(countMin.getHashInfo());
  tHashInfo hashInfo =countMin.getHashInfo(); 
  int hashFnIndex =  dataPlane.getNumAddHashFunctions();
  for (int i = 0; i < hashInfo.numHashValues; i++) {
    hashInfo.hashFnIndexes.push_back(hashFnIndex++);
  }
  dataPlane.addHashFunction(hashInfo);
  

  int taskSize = 1;
  taskSize *= countMin.getSize();
  task_id = dataPlane.setSRAMSize(taskSize);

  // vector<tCounterInfo> counterInfos;
  tCounterInfo countMinCounterInfo = countMin.getCounterInfo();
  countMinCounterInfo.counterSize = 1 ; 
  
  //counterInfos.push_back(countMinCounterInfo);

  //  dataPlane.setPacketProcessing(task_id, counterInfos);
  for (int i = 0; i < countMinCounterInfo.numRows; i++) {
    dataPlane.updateSRAM(ONELEVEL_FUNCTION, hashInfo.hashFnIndexes[i],\
			 i, countMinCounterInfo.countersPerRow, \
			 countMinCounterInfo.counterSize,  \
			 UPDATETYPE_INCREMENT);
  }
}

/*void TaskHeavyHitters::getHashSeedsFromDataPlane(const DataPlane &dataPlane) {
  hashA = dataPlane.getHashA();
  hashB = dataPlane.getHashB();
  }*/

void TaskHeavyHitters::updateCountersFromDataPlane(DataPlane &dataPlane) {
  sramCounters = dataPlane.getSRAM();
}

int TaskHeavyHitters::queryGivenKey(int key) {
  vector<int> counts;
  queue<int> addresses;

  addresses.push(0);
  vector<int> hashValues;

  vector<int> hashFnIndexes = countMin.getHashInfo().hashFnIndexes;

  getHashValues(key, countMin.getHashInfo(), hashValues);
  //for (int i = 0; i < hashValues.size(); i++) {printf("%d, ", hashValues[i]);}
  //printf(" are the hash values for this key %d.\n", key);

  tCounterInfo counterInfo = countMin.getCounterInfo();
  counterInfo.counterSize = 1;

  // use ONELEVEL_FUNCTION, hashValues to calculate addresses
  int offset = 0;
  int index;
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
      counts.push_back(sramCounters[index]);
      //printf(".. pushing %d\n", index);
    }

  int ret = countMin.query(counts); 
  return ret;
}

int TaskHeavyHitters::queryGivenPacket(const Packet& p){
  int key = getField(p, field);
  return queryGivenKey(key);
}

int TaskHeavyHitters::getField(const Packet& p, int field){
  if (field == FIELD_SRCIP) return p.srcip;
  else if (field == FIELD_DSTIP) return p.dstip;

}

// to change, which hash function (index) to use is in counter info
void TaskHeavyHitters::getHashValues(int x,  const tHashInfo& hashInfo, \
				     vector<int>& hashValues) {
  hashValues.clear();
  uint32 hashvalue;
  int numHashFns = hashInfo.numHashValues;
  for(int i = 0; i < numHashFns; i++) {
    hashvalue = commonHash(hashInfo.hashFnIndexes[i], x)%hashInfo.range;
    hashValues.push_back(hashvalue);
    //os_dietz_thorup32(x, hashInfo.range, hashA[i], hashB[i]));
  }
}

int TaskHeavyHitters::getTaskId() {
  return task_id;
}



