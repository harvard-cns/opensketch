#include "taskSuperSpreaders.h"

void TaskSuperSpreaders::getSamplingConfig(double k, double b, double delta, double& r, double& c1) {
  double log1delta = log(1.0/delta);
  double e = exp(1.0);
  
  // set c1
  if (b <= 3 ) 
    {
      double monster = (3*b + 2*b* sqrt(6*b) + 2 * b *b)/ ( (b-1) * (b-1))  ;
      c1 = log1delta * monster;
    }
  else
    {
      if ( (b > 3) && (b < (2 * e * e)) )
	{
	  double max1 = (  b > 2/ ( (1 - e/b) * (1-e/b) )  ) ? b: 2/ ( (1 - e/b) * (1-e/b)); 
	  c1 = log1delta * max1;
	}
      else
	{
	  c1 = 8 * log1delta;
	}
    }
  // set r
  if (b <= 3 ) 
    {
      r = c1/b +  sqrt ( 3 * c1 * log1delta / b) ;
    }
  else
    {
      if ( (b > 3) && (b < (2 * e * e)) )
	{
	  r = e * c1 / b; 
	}
      else
	{
	  r = c1/2;
	}
    } 
  r = ceil(r);

}

void TaskSuperSpreaders::setUserPreferencesDirectly1(int k, int b, int delta, int field1, int field2, int numRows, int countersPerRow, int numBits) {
  double r, c1;
  getSamplingConfig(k, b, delta, r, c1);
  setUserPreferencesDirectly(field1, numRows, countersPerRow, field2, numBits, r, c1/k);
}

void TaskSuperSpreaders::setUserPreferencesDirectly(int field1, int numRows, int countersPerRow, int field2, int numBits, int max, double ratio) {
  countMin.setup(field1, numRows, countersPerRow);
  bitmap.setup(field2, numBits, max);
  samplingRatio = ratio;
}

void TaskSuperSpreaders::configureDataPlane(DataPlane &dataPlane) {
  dataPlane.addSampling(countMin.getHashInfo().field*10\
			+bitmap.getHashInfo().field,   \
			MAXUINT32 * samplingRatio);

  cm_id = dataPlane.addHashFunctions(countMin.getHashInfo());
  bm_id = dataPlane.addHashFunctions(bitmap.getHashInfo());

  int taskSize = bitmap.getSize();
  taskSize *= countMin.getSize();
  task_id = dataPlane.setSRAMSize(taskSize);

  vector<tCounterInfo> counterInfos;
  tCounterInfo countMinCounterInfo = countMin.getCounterInfo();
  countMinCounterInfo.counterSize = bitmap.getSize();
  countMinCounterInfo.sketch_id = cm_id;
  countMinCounterInfo.updateType = UPDATETYPE_NEXT;
  countMinCounterInfo.nextOffset = 0; // shouldn't check nextOffset if type is UPDATETYPE_NEXT
  counterInfos.push_back(countMinCounterInfo);

  tCounterInfo bitmapCounterInfo = bitmap.getCounterInfo();
  bitmapCounterInfo.counterSize = 1;
  bitmapCounterInfo.sketch_id = bm_id;
  bitmapCounterInfo.nextOffset = 0; // okay to access, won't make any difference
  counterInfos.push_back(bitmapCounterInfo);


  dataPlane.setPacketProcessing(task_id, counterInfos);
}


int TaskSuperSpreaders::queryGivenKey(int key) {
  vector<int> counts;
  queue<int> addresses;

  addresses.push(0);
  vector<int> hashValues;

  getHashValues(key, countMin.getHashInfo(), hashValues);
  
  if (key == -873408477) {
    for (int i = 0; i < hashValues.size(); i++) {printf("%d, ", hashValues[i]);}
    printf(" are the hash values for this key %d.\n", key);
  }

  tCounterInfo countMinCounterInfo = countMin.getCounterInfo();
  countMinCounterInfo.counterSize = bitmap.getSize();
  countMinCounterInfo.nextOffset = 0;

  updateAddresses(addresses, countMinCounterInfo, hashValues);


  int addr;
  while(!addresses.empty()) {
    if (key == -873408477) {
	printf("getting counter at %d\n", addr);
      }
    addr = addresses.front();      
    addresses.pop();

    vector<int> countsForBitmap;
    int tmp;
    int bitsSet = 0;
    int totalBits = bitmap.getSize();
    for(int i = 0; i < totalBits; i++) {
      tmp = sramCounters[addr+i];
      bitsSet += tmp;
      countsForBitmap.push_back(tmp);
    }
    //    printf("%d set out of %d\n", bitsSet, bitmap.getSize());

    int bitmapEstimate = bitmap.query(countsForBitmap);
    counts.push_back(bitmapEstimate);
  }

  int ret = countMin.query(counts); 
  return ret;
}

int TaskSuperSpreaders::queryGivenPacket(const Packet& p){
  int key = getField(p, countMin.getHashInfo().field);
  return queryGivenKey(key);
}

