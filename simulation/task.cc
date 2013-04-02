#include "task.h"

Task::Task(){
}

Task::~Task(){
}

void Task::setUserPreferencesDirectly(int field, int numRows, int countersPerRow) {
  //  reversible.setup(field);
  //  countMin.setup(field, numRows, countersPerRow);
}


void Task::configureDataPlane(DataPlane &dataPlane) {
  // update hashInfos, SRAM size, counterInfos based on sketches
}

void Task::getHashSeedsFromDataPlane(const DataPlane &dataPlane) {
  hashA = dataPlane.getHashA();
  hashB = dataPlane.getHashB();
}

void Task::updateCountersFromDataPlane(DataPlane &dataPlane) {
  sramCounters = dataPlane.getSRAM(task_id);
}


void Task::updateAddresses(queue<int>& addresses, const tCounterInfo& counterInfo, const vector<int>& hashValues) {
  int startingAddresses = addresses.size();
  int offset;
  int index;
  while(startingAddresses > 0) {
    offset = addresses.front(); 
    addresses.pop();
    startingAddresses--;
    for (int i = 0; i < counterInfo.numRows; i++) {
      //printf("address in row %d\n", i);
      
      //printf(".. index = offset %d\n", index);
      index = i * counterInfo.countersPerRow;
      //printf(".. index += i * counterInfo.countersPerRow %d\n", index);
      
      // if hashValues is empty, want all counters
      if (hashValues.size() > 0) {
	index += hashValues[i] % counterInfo.countersPerRow;
	//printf(".. hashValues[i] mod counterInfo.countersPerRow %d\n", index);
	index *= counterInfo.counterSize;
	//printf(".. index *= counterInfo.counterSize %d\n", index);
	addresses.push(offset+index);
      //printf(".. pushing %d\n", index);
      }
      else {
	for (int j = 0; j < counterInfo.countersPerRow; j++) {
	  addresses.push(offset + ((index + j) * counterInfo.counterSize)); 
	  //printf(".. pushing %d\n", index);
	}
      
      }
    }

  }
}

int Task::getField(const Packet& p, int field){
  if (field == FIELD_SRCIP) return p.srcip;
  else if (field == FIELD_DSTIP) return p.dstip;

}

// only simple hahs function, don't need this function for reversible
void Task::getHashValues(int x, const tHashInfo& hashInfo, vector<int>& hashValues) {
  hashValues.clear();
  for(int i = 0; i < hashInfo.numHashValues; i++) {
    hashValues.push_back(os_dietz_thorup32(x, hashInfo.range, hashA[i], hashB[i]));
  }
}

int Task::getTaskId() {
  return task_id;
}



