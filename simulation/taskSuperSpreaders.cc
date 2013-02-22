#include "taskSuperSpreaders.h"

TaskSuperSpreaders::TaskSuperSpreaders(){
}

TaskSuperSpreaders::~TaskSuperSpreaders(){
}

void TaskSuperSpreaders::setUserPreferencesDirectly(int field1, int numRows, int countersPerRow,\
						    int field2, int numBits) {
  countMin.setup(field1, numRows, countersPerRow);
  bitmap.setup(field2, numBits);
}

void TaskSuperSpreaders::configureDataPlane(DataPlane &dataPlane) {
  
  cm_id = dataPlane.addHashFunctions(countMin.getHashInfo());
  bm_id = dataPlane.addHashFunctions(bitmap.getHashInfo());

  int taskSize = bitmap.getSize();
  taskSize *= countMin.getSize();
  task_id = dataPlane.setSRAMSize(taskSize);

  vector<tCounterInfo> counterInfos;
  tCounterInfo countMinCounterInfo = countMin.getCounterInfo();
  countMinCounterInfo.counterSize = bitmap.getSize();
  countMinCounterInfo.sketch_id = cm_id;
  countMinCounterInfo.updateType = UPDATETYPE_NEXT;
  countMinCounterInfo.nextOffset = 0; // shouldn't check nextOffset if type is UPDATETYPE_NEXT
  counterInfos.push_back(countMinCounterInfo);

  tCounterInfo bitmapCounterInfo = bitmap.getCounterInfo();
  bitmapCounterInfo.counterSize = 1;
  bitmapCounterInfo.sketch_id = bm_id;
  bitmapCounterInfo.nextOffset = 0; // okay to access, won't make any difference
  counterInfos.push_back(bitmapCounterInfo);


  dataPlane.setPacketProcessing(task_id, counterInfos);
}

void TaskSuperSpreaders::getHashSeedsFromDataPlane(const DataPlane &dataPlane) {
  hashA = dataPlane.getHashA();
  hashB = dataPlane.getHashB();
}

void TaskSuperSpreaders::updateCountersFromDataPlane(DataPlane &dataPlane) {
  sramCounters = dataPlane.getSRAM(task_id);
}

int TaskSuperSpreaders::queryGivenKey(int key) {
  vector<int> counts;
  queue<int> addresses;

  addresses.push(0);
  vector<int> hashValues;

  getHashValues(key, countMin.getHashInfo(), hashValues);
  
  if (key == -873408477) {
    for (int i = 0; i < hashValues.size(); i++) {printf("%d, ", hashValues[i]);}
    printf(" are the hash values for this key %d.\n", key);
  }

  tCounterInfo countMinCounterInfo = countMin.getCounterInfo();
  countMinCounterInfo.counterSize = bitmap.getSize();
  countMinCounterInfo.nextOffset = 0;

  updateAddresses(addresses, countMinCounterInfo, hashValues);


  int addr;
  while(!addresses.empty()) {
    if (key == -873408477) {
	printf("getting counter at %d\n", addr);
      }
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
    printf("%d set out of %d\n", bitsSet, bitmap.getSize());

    int bitmapEstimate = bitmap.query(countsForBitmap);
    counts.push_back(bitmapEstimate);
  }

  int ret = countMin.query(counts); 
  return ret;
}

int TaskSuperSpreaders::queryGivenPacket(const Packet& p){
  int key = getField(p, countMin.getHashInfo().field);
  return queryGivenKey(key);
}

void TaskSuperSpreaders::updateAddresses(queue<int>& addresses, const tCounterInfo& counterInfo, const vector<int>& hashValues) {
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

int TaskSuperSpreaders::getField(const Packet& p, int field){
  if (field == FIELD_SRCIP) return p.srcip;
  else if (field == FIELD_DSTIP) return p.dstip;

}


void TaskSuperSpreaders::getHashValues(int x, const tHashInfo& hashInfo, vector<int>& hashValues) {
  hashValues.clear();
  for(int i = 0; i < hashInfo.numHashValues; i++) {
    hashValues.push_back(os_dietz_thorup32(x, hashInfo.range, hashA[i], hashB[i]));
  }
}

int TaskSuperSpreaders::getTaskId() {
  return task_id;
}



