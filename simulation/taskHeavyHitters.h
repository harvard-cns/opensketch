#ifndef __TASKHEAVYHITTERS_H
#define __TASKHEAVYHITTERS_H

#include "common.h"
#include "task.h"
#include "sketchCountMin.h"
#include "dataPlane.h"

class TaskHeavyHitters : public Task
{

 protected:
  // Sketches used
  // Sketch ids
  SketchCountMin countMin;
  int cm_id; // not used

  // Fields
  int field;

  // Error
  double errorPc;
  double confidence;

 public:

  // Find configuration based on error
  void setUserPreferencesGivenError(int field, double errorPc, double confidence);
  void setUserPreferencesGivenSpace(int field, int spaceB, double confidence);
  void setUserPreferencesDirectly(int field, int numRows, int countersPerRow);

  // Configure data plane
  void configureDataPlane(DataPlane &dataPlane);

   
  // Query Given Key
  // Query Given Packet
  int queryGivenKey(int key);
  int queryGivenPacket(const Packet& p);


};

#endif 


