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
  
  tHashInfo hashInfo =countMin.getHashInfo(); 
  int hashFnIndex =  dataPlane.getNumAddHashFunctions();
  for (int i = 0; i < hashInfo.numHashValues; i++) {
    hashInfo.hashFnIndexes.push_back(hashFnIndex++);
  }
  dataPlane.addHashFunction(hashInfo);
 
  hashInfo = bitmap.getHashInfo(); 
  hashFnIndex =  dataPlane.getNumAddHashFunctions();
  for (int i = 0; i < hashInfo.numHashValues; i++) {
    hashInfo.hashFnIndexes.push_back(hashFnIndex++);
  }
  dataPlane.addHashFunction(hashInfo);

  int taskSize = bitmap.getSize();
  taskSize *= countMin.getSize();
  task_id = dataPlane.setSRAMSize(taskSize);

  vector<tCounterInfo> counterInfos;
  tCounterInfo countMinCounterInfo = countMin.getCounterInfo();
  countMinCounterInfo.counterSize = bitmap.getSize();
  countMinCounterInfo.updateType = UPDATETYPE_NEXT;

  tCounterInfo bitmapCounterInfo = bitmap.getCounterInfo();
  bitmapCounterInfo.counterSize = 1;
  bitmapCounterInfo.updateType = UPDATETYPE_SET;

  tHashInfo countMinHashInfo = countMin.getHashInfo();
  tHashInfo bitmapHashInfo = bitmap.getHashInfo();

  for (int i = 0; i < countMinCounterInfo.numRows; i++) {
    dataPlane.updateSRAM(TWOLEVEL_FUNCTION, countMinHashInfo.hashFnIndexes[i],\
			 i, countMinCounterInfo.countersPerRow, \
			 countMinCounterInfo.counterSize,  \
			 bitmapHashInfo.hashFnIndexes[0],\
			 0, bitmapCounterInfo.countersPerRow,\
			 bitmapCounterInfo.counterSize,\
			 UPDATETYPE_SET);
  }


}


void TaskSuperSpreaders::updateCountersFromDataPlane(DataPlane &dataPlane) {
  sramCounters = dataPlane.getSRAM();
}

int TaskSuperSpreaders::queryGivenKey(int key) {
  vector<int> counts;
  vector<int> hashValues;

  getHashValues(key, countMin.getHashInfo(), hashValues);
  
  int offset = 0;
  int index;

  tCounterInfo counterInfo = countMin.getCounterInfo();

  vector<int> countsForBitmap;
  for (int i = 0; i < counterInfo.numRows; i++) {
    index = offset;
    index += i * counterInfo.countersPerRow;
    index += hashValues[i] % counterInfo.countersPerRow;
    index *= counterInfo.counterSize;
    countsForBitmap.clear();
    for(int j = 0; j < bitmap.getSize(); j++) {
      countsForBitmap.push_back(sramCounters[index+j]);
    }
    int bitmapEstimate = bitmap.query(counts);
    counts.push_back(bitmapEstimate);
  }


  int ret = countMin.query(counts); 
  return ret;
}

int TaskSuperSpreaders::queryGivenPacket(const Packet& p){
  int key = getField(p, countMin.getHashInfo().field);
  return queryGivenKey(key);
}


int TaskSuperSpreaders::getField(const Packet& p, int field){
  if (field == FIELD_SRCIP) return p.srcip;
  else if (field == FIELD_DSTIP) return p.dstip;

}

void TaskSuperSpreaders::getHashValues(int x,  const tHashInfo& hashInfo, \
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

int TaskSuperSpreaders::getTaskId() {
  return task_id;
}



