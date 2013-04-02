#ifndef _OS_COUNTERREVANALYZE_H_
#define _OS_COUNTERREVANALYZE_H_
#include "common.h"

using namespace std;

typedef struct NodeType {
  list<uint8> keyList;
  uint8 numFound;  // Number of HT's in which this key has been found

  // List of heavy change buckets to which this key corresponds for each HT
  vector<list<uint32> > bktLists;

  NodeType(uint8 key, uint32 bkt, 
	   uint8 tbl, int numTbls)
  {
    keyList.push_back(key);
    numFound = 1;
    
    bktLists.resize(numTbls);
    bktLists[tbl].push_back(bkt);
  }

  NodeType()
  {
    numFound = 0;
  }

} NodeType;

typedef vector<list<NodeType*> > GraphType;

class OS_CounterRevAnalyze {

 public:
  uint8 lookupTable[REVERSIBLE_NUMROWS][REVERSIBLE_NUMKEYS]\
    [REVERSIBLE_BINSPERDIV];
  vector< vector<uint8> > revLookupTable[REVERSIBLE_NUMROWS]\
    [REVERSIBLE_NUMDIVS];

  uint64 seeds[REVERSIBLE_NUMDIVS];

  vector<uint32>* pCounts;
  OS_CounterRevAnalyze();
  ~OS_CounterRevAnalyze();

  void populateLookupTable(uint32 table, uint64 seed);
  void getPossibleKeys(uint32 table, list<uint8> &keys,\
		       uint32 div, uint32 bin);
  void changeDetection(uint32 table, list<uint32> &changes,\
		       double threshold);
  double findThreshold(uint32 table, int numAnoms,\
		       double minThreshold);

  bool getKeys(uint32 iterSize, uint32 *maxThresh,\
	       list<uint32> &tempLst);

  void update_counts(vector<uint32>& counts) {\
    pCounts = &counts;
  }

 void clear_counts() {\
    pCounts = NULL;
  }

  uint32 get_count(uint32 table, uint32 bin);

  void createNodes(GraphType &graph,\
		   const vector<list<uint32> > &bktLists);

  void createLinks(GraphType &graph, int startDiv);

  NodeType* rIntersect(NodeType *node1, NodeType *node2);

  void generateFullKeys(GraphType &graph,\
		      list<uint32> &resultList);


  void listIntersect(list<uint32> &outList,\
		     const list<uint32> &l1,\
		     const list<uint32> &l2);

  void clear_state();
};
#endif
