#ifndef __TASKSUPERSPREADERSREV_H
#define __TASKSUPERSPREADERSREV_H

#include "taskSuperSpreaders.h"
#include "sketchReversible.h"


class TaskSuperSpreadersRev : public TaskSuperSpreaders
{
  SketchReversible reversible;
  int rev_id;
  int threshold;
  Mangler* mangler;

 public:

  void setUserPreferencesDirectly1(int k, int b, int delta, int field1, int field2, int numRows, int countersPerRow, int numBits, int threshold);
  void setUserPreferencesDirectly(int field1, int numRows, int countersPerRow, \
				  int field2, int numBits, double ratio,\
				  int threshold);

  void configureDataPlane(DataPlane &dataPlane);
  void getManglerFromDataPlane(const DataPlane &dataPlane);
  void getSuperSpreaders(vector<int>&superSpreaders);
};

#endif 


