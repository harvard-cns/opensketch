#ifndef __TASKHEAVYHITTERSREV_H
#define __TASKHEAVYHITTERSREV_H

#include "taskHeavyHitters.h"
#include "sketchReversible.h"


class TaskHeavyHittersRev : public TaskHeavyHitters
{

 protected:
  SketchReversible reversible;
  int threshold;

  int rev_id;

  Mangler* mangler;

 public:

  // find configuration
  void  setUserPreferencesDirectly(int field, int numRows, int countersPerRow);
  void setUserPreferencesDirectly(int field, int numRows, int countersPerRow, int threshold);
  void getManglerFromDataPlane(const DataPlane &dataPlane);

  // configure data plane
  void  configureDataPlane(DataPlane &dataPlane);

  // analyze
  void getHeavyHitters(vector<int>&heavyHitters);
};

#endif 


