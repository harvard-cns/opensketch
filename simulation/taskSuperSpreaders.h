#ifndef __TASKSUPERSPREADERS_H
#define __TASKSUPERSPREADERS_H

#include "task.h"
#include "sketchCountMin.h"
#include "sketchBitmap.h"


class TaskSuperSpreaders : public Task
{
 protected:
  double samplingRatio;
  SketchCountMin countMin;
  SketchBitmap bitmap;
  
  int cm_id; // not used
  int bm_id;

  // this and dataplane each have their own version of computing hash function, synced of course
  int field1;
  int field2;


 public:

  void getSamplingConfig(double k, double b, double delta, double& r, double& c1);
  void setUserPreferencesDirectly1(int k, int b, int delta, int field1, int field2, int numRows, int countersPerRow, int numBits);
  void setUserPreferencesDirectly(int field1, int numRows, int countersPerRow, \
				  int field2, int numBits, int max, double ratio);
  void configureDataPlane(DataPlane &dataPlane);

  int queryGivenKey(int key);
  int queryGivenPacket(const Packet& p);
};

#endif 


