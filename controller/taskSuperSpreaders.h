#ifndef __TASKSUPERSPREADERS_H
#define __TASKSUPERSPREADERS_H

#include "common.h"
#include "sketchCountMin.h"
#include "sketchBitmap.h"
#include "dataplane.h"

class TaskSuperSpreaders
{
  SketchCountMin countMin;
  SketchBitmap bitmap;

  int task_id;

  // this and dataplane each have their own version of computing hash function, synced of course
  int field1;
  int field2;

  vector<int> sramCounters;

  void getHashValues(int x, const tHashInfo& hashInfo, vector<int>& hashValues);
  int getField(const Packet& p, int field);

 public:
  TaskSuperSpreaders();
  ~TaskSuperSpreaders();
  
  void setUserPreferencesDirectly(int field1, int numRows, int countersPerRow, \
				  int field2, int numBits);

  void configureDataPlane(DataPlane &dataPlane);
  void updateCountersFromDataPlane(DataPlane &dataPlane);

  int getTaskId();

  int queryGivenKey(int key);
  int queryGivenPacket(const Packet& p);
};

#endif 


