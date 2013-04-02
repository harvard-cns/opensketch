#ifndef _DATAPLANE_H
#define _DATAPLANE_H
#include "common.h"
#include "os_mangler.h"

using namespace std;

class DataPlane {

  // all data structures are owned by this object
  // HashInfo, CounterInfo

  vector<uint64> hashA, hashB; // hash seeds
  Mangler* mangler; // table for mangling before rev. hash
  
  int sampleField;
  int sampleThreshold;



  map<int, tHashInfo> perSketchHashInfo; // sketchid - which field, how many, what range
  // per packet, hash per field max(how many) times and store in diff. map<int, vector<int>>
  map<int, tHashInfo> perSketchRevHashInfo;

  map<int, tHashInfo> perFieldHashInfo;
  map<int, vector<uint32> > perFieldHashValues;
  // when you want hashvalues for sketchid, lookup that field, get reqd. #values, mod by range
  
  map<int, tHashInfo> perFieldRevHashInfo;
  map<int, vector<uint32> > perFieldRevHashValues;

  map<int, vector<int> > perTaskSRAM; // taskid - raw counters in SRAM

  map<int, vector<tCounterInfo> > perTaskCounterInfo; // taskid - how to process a packet>
  //- id, #rows, counters_per_row, counter_size, update_type, next_offset 
  //- id and update_type and next_offset (e.g., for reversible) set by task

  void doSRAM_updateAddresses(queue<uint32>& addresses, const tCounterInfo& counterInfo);
  // during packet processing, if sketch A is made of sketch B
  // final counter address is like A's counter 1 + B's counter, A's counter 2 + B's counter ..
  // so to get first addresses call with a vector containing starting offset (e.g., 0)
  // e.g., updateAddresses(startingOffset, counterInfoOfA)
  // then updateAddresses(addressesOfA, counterInfoOfB) to get final counter address
  
  int doSample(const Packet& p, int field, uint32 threshold);

  void doHash(const Packet& p, map<int, tHashInfo >& perField, map<int, vector<uint32> >& perFieldValues );
  // uses perSketchHashInfo
  // for each field, hashes, mods by maximum range
  // fills in perFieldHashValues

  void clearHash(map<int, tHashInfo >& perField, map<int, vector<uint32> >& perFieldValues);

  void doSRAM(const Packet& p, int task_id);
  // uses perTaskCounterInfo
  // for each task, based on packet p, perFieldHashValues
  // updates perTaskSRAM

  uint64 getField(const Packet& p, int field);
 public:
  DataPlane();
  ~DataPlane();

  int sampled;
  // normal hash functions return a sketch_id < 100
  // reversible ... > 100
  // for measurement objects to call
  void addSampling(int field, uint32 threshold);
  int addHashFunctions(const tHashInfo& hashInfo); 


  // which set of hash values to use? sketch_id
  int setSRAMSize(int size); 
  // which sram set of tables? task_id
  int setPacketProcessing(int task_id, const vector<tCounterInfo>& counterInfo); 
  // how to parse them? id for which hash values, #rows, counters_per_row, counter_size, update_type

  // for main function to call
  void hashSeeds(const vector<uint64>& seedsA, const vector<uint64>& seedsB);
  void processPacket(const Packet& p, int task_id);

  void getHashByField();
  void getHashByField(map<int, tHashInfo>& perSketch, map<int, tHashInfo>& perField, map<int, vector<uint32> >& perFieldValues);

  void getHashValues(int sketch_id, vector<uint32>& hashValues);
  // will lookup perSketchHashInfo for config, with that lookup field, get hash values
  // maybe public function, e.g., if reversible needs to know what seeds were used
  // will clear passed hashValues, and insert new ones, 
  // make sure perFieldHashValues or perFieldRevHashValues is okay before calling this
  // of course count min and bitmap need, when we query a packet or addr to verify or jlt
  vector<uint64> getHashA() const {return hashA;}
  vector<uint64> getHashB() const {return hashB;}
  uint64 getMangleSeed1() const {return mangler->seed1;}
  uint64 getMangleSeed2() const {return mangler->seed2;}

  vector<int> getSRAM(int task_id) {return perTaskSRAM[task_id];}
};

#endif
