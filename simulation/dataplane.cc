#include "common.h"
#include "dataplane.h"

using namespace std;

DataPlane::DataPlane() {
  int n = sizeof(hash::A)/sizeof(hash::A[0]);
  for (int i = 0; i < n; i++) {
    hashA.push_back(hash::A[i]);
    hashB.push_back(hash::B[i]);
  }
  mangler = new Mangler();
}

DataPlane::~DataPlane(){
}

int DataPlane::getField(const Packet& p, int field) {
    if (field == FIELD_SRCIP) return p.srcip;
    else if (field == FIELD_DSTIP) return p.dstip;
  }
  
void DataPlane::getHashValues(int sketch_id, vector<int>& hashValues) {
    // assert(it's not .end())
    hashValues.clear();
    tHashInfo hashInfo;
    vector<int> allHashValues;

    if (sketch_id < 100) {
      hashInfo = perSketchHashInfo[sketch_id];
      allHashValues = perFieldHashValues[hashInfo.field];
    }
    else {
      hashInfo = perSketchRevHashInfo[sketch_id%100]; // rev. range is 2^12 btw
      allHashValues = perFieldRevHashValues[hashInfo.field];

    }
    for (int i = 0; i < hashInfo.numHashValues; i++) {
      hashValues.push_back(allHashValues[i]%hashInfo.range);
    }
  }

void DataPlane::hashSeeds(const vector<uint64>& A, const vector<uint64>& B) {
    hashA = A;
    hashB = B;
  }
  
int DataPlane::setPacketProcessing(int task_id, const vector<tCounterInfo>& counterInfos) {
    int meas_id = perTaskCounterInfo.size()+1;
    vector<tCounterInfo> myCounterInfos = counterInfos;
    perTaskCounterInfo.insert(pair<int, vector<tCounterInfo> > (meas_id, myCounterInfos));
    return meas_id;
}
 
int DataPlane::setSRAMSize(int sramSize){
    int meas_id = perTaskSRAM.size()+1;
    vector<int> emptySram(sramSize);

    perTaskSRAM.insert(pair<int, vector<int> > (meas_id, emptySram));
    return meas_id;
  }

int DataPlane::addHashFunctions(const tHashInfo& hashInfo) {
  tHashInfo myHashInfo = hashInfo;
  int task_id;
  if (hashInfo.rev == HASHTYPE_DIETZTHORUP32) {
    task_id = perSketchHashInfo.size()+1;
    perSketchHashInfo.insert(pair<int, tHashInfo>(task_id, myHashInfo));
  }
  else if (hashInfo.rev == HASHTYPE_REVERSIBLE8TO3) {
    task_id = perSketchRevHashInfo.size()+1+100;
    perSketchRevHashInfo.insert(pair<int, tHashInfo> (task_id, myHashInfo));
  } 
  
  return task_id;
}    

void DataPlane::processPacket(const Packet& p, int task_id) {
  //printf("calling doHash.\n");
  doHash(p, perFieldHashInfo, perFieldHashValues);

  //printf("calling doSRAM.\n");
  doSRAM(p, task_id);

  clearHash(perFieldHashInfo, perFieldHashValues);
}

void DataPlane::getHashByField() {
  getHashByField(perSketchHashInfo, perFieldHashInfo);
  getHashByField(perSketchRevHashInfo, perFieldRevHashInfo);
}

void DataPlane::getHashByField(map<int, tHashInfo>& perSketch, map<int, tHashInfo>& perField) {

    map<int, tHashInfo>::iterator it;
    map<int, tHashInfo>::iterator found;
    int field;
    int numHashValues;
    int range;
    tHashInfo best, hashInfo;
    tHashInfo tmp, *existing;

    // also store maximum range and choose between os_dietz_thorup32
    // and os_dietz64to32
    for(it = perSketch.begin(); it != perSketch.end(); ++it) {
      field = it->second.field;
      hashInfo = it->second;
      numHashValues = hashInfo.numHashValues;
      range = hashInfo.range;
      found = perField.find(field); 
      
      if (found == perField.end()){
	best.numHashValues = numHashValues;
	best.range = range;
	perField.insert(pair<int, tHashInfo>(field, best));
	vector<int> emptyHashValues;
	perFieldHashValues.insert(pair<int, vector<int> >(field, emptyHashValues));
      }
      else {
	existing = &(found->second);
	if (numHashValues > existing->numHashValues) {
	  existing->numHashValues = numHashValues;
	}
	if (range > existing->range) {
	  existing->range = range;
	}
      }
    }

}


void DataPlane::doHash(const Packet& p, map<int, tHashInfo>& perField, map<int, vector<int> >& perFieldValues ) {


    map<int, tHashInfo>::iterator it2;
    tHashInfo tmp;
    int field;

    for(it2 = perField.begin(); it2 != perField.end(); ++it2) {
      field = it2->first;
      tmp = it2->second;
      int x = getField(p, field);

      for(int i = 0; i < tmp.numHashValues; i++) {
	int hashvalue;
	if (tmp.rev == HASHTYPE_DIETZTHORUP32) {
	  hashvalue = os_dietz_thorup32(x, tmp.range, hashA[i], hashB[i]);}
	else if (tmp.rev == HASHTYPE_REVERSIBLE8TO3) {
	  uint32 value = mangler->MangleShortTable((uint32) x); 
	  hashvalue = reversible4096(value, tmp.range, hashA[i]);
	}
	 perFieldValues[field].push_back(hashvalue);
	 //printf("hashing field %d, with hash function # %d to get %d\n", field, i, hashvalue);
      }

    }

}

void DataPlane::clearHash(map<int, tHashInfo >& perField, map<int, vector<int> >& perFieldValues) {

    map<int, tHashInfo>::iterator it2;
    vector<int> hashValues;
    tHashInfo tmp;
    int field;

    for(it2 = perField.begin(); it2 != perField.end(); ++it2) {
      field = it2->first;
      perFieldValues[field].clear();
	 //printf("hashing field %d, with hash function # %d to get %d\n", field, i, hashvalue);OB
    }

}

	  
  

// sketches will need to return counterInfo object, for now say vec<int>
// doesn't use packet here 
void DataPlane::doSRAM(const Packet& p, int task_id){    
  vector<tCounterInfo> counterInfos = perTaskCounterInfo[task_id];   
  // offsets are relative to start of task's SRAM
    queue<int> addresses;
    addresses.push(0);


    vector<tCounterInfo>::iterator it;
    for (it = counterInfos.begin(); it != counterInfos.end(); ++it) {
      doSRAM_updateAddresses(addresses, *it);
      if (it->updateType == UPDATETYPE_SET || it->updateType == UPDATETYPE_INCREMENT) {
	int addr;
	while(!addresses.empty()) {
	  addr = addresses.front();
	  addresses.pop();
	  if (it->updateType == UPDATETYPE_SET) perTaskSRAM[task_id][addr] = 1;
	  else if (it->updateType == UPDATETYPE_INCREMENT) {
	    perTaskSRAM[task_id][addr]++;
	    if (p.srcip == 2056072923) {
	      //printf("perTaskSRAM[%d][%d]++ is now %d, ", task_id, addr, perTaskSRAM[task_id][addr]);
	    }

	  }
	}
	//if (p.srcip == 2056072923) {printf("\n");}
	addresses.push(it->nextOffset);
      }
    }
}


void DataPlane::doSRAM_updateAddresses(queue<int>& addresses, const tCounterInfo& counterInfo){
  int startingAddresses = addresses.size();
  int offset;
  vector<int> hashValues;
  int index;
  while(startingAddresses > 0) {
    offset = addresses.front();
    addresses.pop(); 
    startingAddresses--;
  
    getHashValues(counterInfo.sketch_id, hashValues);
    for (int i = 0; i < counterInfo.numRows; i++) {
      index = offset;
      index += i * counterInfo.countersPerRow;
      index += hashValues[i] % counterInfo.countersPerRow;
      index *= counterInfo.counterSize;
      addresses.push(index);
    }
  }
}

