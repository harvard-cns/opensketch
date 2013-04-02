#ifndef __TASK_H
#define __TASK_H

#include "common.h"
#include "dataPlane.h"

class Task
{
 protected:

  // Sketches used
  // Sketch ids
  // Fields
  // Error

  int task_id; // to get sram from dataplane
 
  vector<uint64> hashA, hashB;
  vector<int> sramCounters;

  void updateAddresses(queue<int>& addresses, const tCounterInfo& counterInfo, const vector<int>& hashValues);
  void getHashValues(int x, const tHashInfo& hashInfo, vector<int>& hashValues);
  int getField(const Packet& p, int field);

 public:
  Task();
  ~Task();
  
  // Configure based on error

  void setUserPreferencesDirectly(int field, int numRows, int countersPerRow);
  void configureDataPlane(DataPlane &dataPlane);
  void getHashSeedsFromDataPlane(const DataPlane &dataPlane);
  void updateCountersFromDataPlane(DataPlane &dataPlane);
  int getTaskId();

  // Query Given Key
  // Query Given Packet
};

#endif 


