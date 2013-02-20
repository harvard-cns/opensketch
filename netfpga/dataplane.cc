#include "dataplane.h"

#include "nf2.h"
#include "nf2util.h"
using namespace std;

#define OPENSKETCH_FILED_ID_REG 	0x2000300
#define	OPENSKETCH_NUMHASHVALUES_REG 	0x2000304
#define	OPENSKETCH_RANGE_REG 		0x2000308
#define OPENSKETCH_NUMROWS_REG		0x200030C


#define OPENSKETCH_COUNTERSPERROW_REG	0x2000310
#define OPENSKETCH_COUNTERSIZE_REG	0x2000314
#define OPENSKETCH_HASH_IDX_REG		0x2000320
#define OPENSKETCH_I_REG		0x2000324

#define OPENSKETCH_COUNTERSPERROW2_REG	0x2000328
#define OPENSKETCH_COUNTERSIZE2_REG	0x200032C
#define OPENSKETCH_HASH_IDX2_REG	0x2000330
#define OPENSKETCH_I2_REG		0x2000334

#define OPENSKETCH_UPDATETYPE_REG	0x2000338
#define OPENSKETCH_ADDRFUNC_LIST_REG	0x200033C

#define OPENSKETCH_FUNC_LIST_REG	0x2000340
#define OPENSKETCH_CONSTANT_VALUE_REG	0x2000344
#define OPENSKETCH_FUNC_LIST_REG	0x2000348
#define OPENSKETCH_REV_FILED_ID_REG 	0x200034C
#define	OPENSKETCH_REV_NUMHASHVALUES_REG 0x2000350
#define	OPENSKETCH_REV_RANGE_REG 	0x2000354
#define	OPENSKETCH_SRAM_SIZE_REG 	0x2000358


#define SRAM_BASE_ADDR            	0x1000000

static struct nf2device nf2;
//	unsigned addr;
//	unsigned value;
//readReg(&nf2, addr, &value);
//writeReg(&nf2, addr, value);
//
//
DataPlane::DataPlane(){
  numAddHashFunction = 0;
  numAddRevHashFunctions = 0;
  numUpdateSRAMs = 0;
  SramSize = 0;
}
DataPlane::~DataPlane(){};

   
int DataPlane::addHashFunction(const tHashInfo& hashInfo) {
	
  	writeReg(&nf2, OPENSKETCH_FILED_ID_REG, hashInfo.field);
	writeReg(&nf2, OPENSKETCH_NUMHASHVALUES_REG, hashInfo.numHashValues);
	writeReg(&nf2, OPENSKETCH_RANGE_REG, hashInfo.range);
	numAddHashFunction++;
  
}

int DataPlane::addRevHashFunctions(const tHashInfo& hashInfo) {
  	writeReg(&nf2, OPENSKETCH_FILED_ID_REG, hashInfo.field);
	writeReg(&nf2, OPENSKETCH_NUMHASHVALUES_REG, hashInfo.numHashValues);
	writeReg(&nf2, OPENSKETCH_RANGE_REG, hashInfo.range);

	numAddRevHashFunctions++;
}

int DataPlane::setSRAMSize(int size){
	writeReg(&nf2, OPENSKETCH_SRAM_SIZE_REG,size);
	SramSize = size;
}

int DataPlane::updateSRAM(int addressFnIndex, int hashFnIndex,  int row, int countersPerRow, int counterSize, int updateTypeIndex) {
	writeReg(&nf2, OPENSKETCH_ADDRFUNC_LIST_REG, addressFnIndex);
	writeReg(&nf2, OPENSKETCH_HASH_IDX_REG, hashFnIndex);
	writeReg(&nf2, OPENSKETCH_I_REG, row);
	writeReg(&nf2, OPENSKETCH_COUNTERSPERROW_REG, countersPerRow);
	writeReg(&nf2, OPENSKETCH_COUNTERSIZE_REG, counterSize);
	writeReg(&nf2, OPENSKETCH_FUNC_LIST_REG, updateTypeIndex);
  	numUpdateSRAMs++;
}

int DataPlane::updateSRAM(int addressFnIndex, int HashFnIndex1,  int Row1, int CountersPerRow1, int counterSize1, int HashFnIndex2,  int Row2, int CountersPerRow2, int counterSize2, int updateType) {

	writeReg(&nf2, OPENSKETCH_ADDRFUNC_LIST_REG, addressFnIndex);

	writeReg(&nf2, OPENSKETCH_HASH_IDX_REG, HashFnIndex1);
	writeReg(&nf2, OPENSKETCH_I_REG, Row1);
	writeReg(&nf2, OPENSKETCH_COUNTERSPERROW_REG, CountersPerRow1);
	writeReg(&nf2, OPENSKETCH_COUNTERSIZE_REG, counterSize1);

	writeReg(&nf2, OPENSKETCH_HASH_IDX2_REG, HashFnIndex2);
	writeReg(&nf2, OPENSKETCH_I2_REG, Row2);
	writeReg(&nf2, OPENSKETCH_COUNTERSPERROW2_REG, CountersPerRow2);
	writeReg(&nf2, OPENSKETCH_COUNTERSIZE2_REG, counterSize2);

	writeReg(&nf2, OPENSKETCH_FUNC_LIST_REG, updateType);

  	numUpdateSRAMs++;
}

vector<int> DataPlane::getSRAM() {
	unsigned int value;
	vector<int> out;
	for (int i = 0; i < SramSize; i++) {
		readReg(&nf2, SRAM_BASE_ADDR + i, &value);
		out.push_back((int)value);
	}
	return out;
}

int DataPlane::getNumAddHashFunctions() {
  return numAddHashFunction;
}
int DataPlane::getNumAddRevHashFunctions(){
  return numAddRevHashFunctions;
}
int DataPlane::getNumUpdateSRAMs() {
  return numUpdateSRAMs;
}


    //  vector<uint64> getHashA() const {return hashA;}
    //  vector<uint64> getHashB() const {return hashB;}

