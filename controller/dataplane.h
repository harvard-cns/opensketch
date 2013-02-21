#ifndef _DATAPLANE_H
#define _DATAPLANE_H
#include "common.h"
#include "os_mangler.h"

using namespace std;

class DataPlane {

  int numAddHashFunction;
  int numAddRevHashFunctions;
  int numUpdateSRAMs;
  int SramSize;
 
 public:
  DataPlane();
  ~DataPlane();

   
  int addHashFunction(const tHashInfo& hashInfo); 

  int addRevHashFunctions(const tHashInfo& hashInfo);

  int setSRAMSize(int size); 

  int updateSRAM(int addressFnIndex, int hashFnIndex,  int row, int countersPerRow, int counterSize, int updateTypeIndex);

  int updateSRAM(int addressFnIndex, int HashFnIndex1,  int Row1, int CountersPerRow1, int counterSize1, int HashFnIndex2,  int Row2, int CountersPerRow2, int counterSize2, int updateType);
  vector<int> getSRAM();

  /*
TWOLEVEL_FUNCTION, countMinHashInfo.hashFnIndexes[i],\ 
                         i, countMinCounterInfo.countersPerRow, \               
                         countMinCounterInfo.counterSize,  \                    
                         bitmapHashInfo.hashFnIndexes[0],\                      
                         0, bitmapCounterInfo.countersPerRow,\                  
                         bitmapCounterInfo.counterSize,\                        
                         UPDATETYPE_SET);   
  */  
  int getNumAddHashFunctions();
  int getNumAddRevHashFunctions();
  int getNumUpdateSRAMs();


    //  vector<uint64> getHashA() const {return hashA;}
    //  vector<uint64> getHashB() const {return hashB;}

};

#endif
