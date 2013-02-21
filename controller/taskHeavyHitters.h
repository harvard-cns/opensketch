#ifndef __TASKHEAVYHITTERS_H
#define __TASKHEAVYHITTERS_H

#include "common.h"
#include "sketchCountMin.h"
#include "dataplane.h"

class TaskHeavyHitters
{
  SketchCountMin countMin;
  int task_id; // to get sram from dataplane
  int cm_id; // not used
  // this and dataplane each have their own version of computing hash function, synced of course
  int field;
  double errorPc;
  double confidence;
  vector<uint64> hashA, hashB;
  vector<int> sramCounters;

  void updateAddresses(queue<int>& addresses, const tCounterInfo& counterInfo, const vector<int>& hashValues);
  void getHashValues(int x, const tHashInfo& hashInfo, vector<int>& hashValues);
  int getField(const Packet& p, int field);

 public:
  TaskHeavyHitters();
  ~TaskHeavyHitters();
  
  void setUserPreferencesGivenError(int field, double errorPc, double confidence);
  void setUserPreferencesGivenSpace(int field, int spaceB, double confidence);
  void setUserPreferencesDirectly(int field, int numRows, int countersPerRow);

  void configureDataPlane(DataPlane &dataPlane);
  void getHashSeedsFromDataPlane(const DataPlane &dataPlane);
  void updateCountersFromDataPlane(DataPlane &dataPlane);

  int getTaskId();

  int queryGivenKey(int key);
  int queryGivenPacket(const Packet& p);
};

#endif 


