#include "os_counterrevanalyze.h"

typedef vector<list<NodeType*> > GraphType;

OS_CounterRevAnalyze::OS_CounterRevAnalyze() {
  uint32 table, j, k;
  for (table = 0; table < REVERSIBLE_NUMROWS; table++) {
    for (j = 0; j < REVERSIBLE_NUMDIVS; j++) {
      revLookupTable[table][j].resize(8);
      for (k = 0; k < 8; k++) {
	//revLookupTable[table][j][k].resize(256);
      }
    }
  }

}

OS_CounterRevAnalyze::~OS_CounterRevAnalyze(){}

void OS_CounterRevAnalyze::populateLookupTable(uint32 table,\
					       uint64 seed) {

  int64views h;
  int64views vx;
  uint32 i, j, k;
  uint8 i8, a8, k8, tmp;
  //  seeds[table] = seed[table];
  // h(ip) = h1(ip1).h2(ip2).h3(ip3).h4(ip4) = val
  // i: 0 thru 255 is what's in ip1, ip2 etc.
  // j: 0 thru 3 is "which tuple" of ip
  // k: 0 thru 7 is h(tuple)
  // lookupTable[table][i][j] --> k
  // revLookupTable[table][j][k] --> {i, i', i''}

  //  for (table = 0; table < rev::NUMTABLES; table++) {
    h.as_int64 = seed;
    for (i = 0; i < REVERSIBLE_NUMKEYS; i++) {
      vx.as_int32s[0] = i;
      i8 = vx.as_int8s[0];
      for (j = 0; j < REVERSIBLE_NUMDIVS; j++) {
	a8 = h.as_int8s[j];
	k8 = os_dietz8to3(i, a8);
	k = k8 & 7;
	lookupTable[table][i][j] = k8;
	revLookupTable[table][j][k].push_back(i8);
      }
    }
    //}
}


void OS_CounterRevAnalyze::getPossibleKeys(uint32 table,\
					list<uint8> &keys,\
					uint32 div, uint32 bin){
  uint32 i;
  vector<uint8>* lKeys;
  
  uint8 bin8 = ((bin >> (div*3))  & 7);

  lKeys = &(revLookupTable[table][div][bin8]);
  /*cout << lKeys->size() << " possible keys for div's bin # "	\
    << (bin8 & 7) << ".\n";*/
  for (i = 0; i < lKeys->size(); i++)
    keys.push_back((*lKeys)[i]);
}

void OS_CounterRevAnalyze::changeDetection(uint32 table,\
					list<uint32> &changes,\
					double threshold){
  uint32 i;
  for(i = 0; i < REVERSIBLE_COUNTERSPERROW; i++)
    if (get_count(table, i) > threshold)
      changes.push_back(i);
}

double OS_CounterRevAnalyze::findThreshold(uint32 table,\
					int numAnoms,\
					double minThreshold){
  uint32 count, numChgs, i;
  vector<double> heavy_counts;
  heavy_counts.clear();
  //  cout << "table "  << table << "\n";
  for(i = 0; i < REVERSIBLE_COUNTERSPERROW; i++){
    count = get_count(table, i);
    // cout << "count " << i << ": " << count << ", ";
    if (count > minThreshold)
      heavy_counts.push_back(count);
  }
  //   cout << "\n";
  
  
  numChgs = heavy_counts.size();
  cout << "number of changes > " << minThreshold << ": " << numChgs << "\n";
  if (numChgs == 0) return -1;
  else if (numChgs <= numAnoms) return minThreshold;
  
  sort(heavy_counts.begin(), heavy_counts.end());
  return(heavy_counts[numChgs-numAnoms]);

  // want not more than numAnoms returned
  // (1, 3, 2, 2, 4) say numAnoms is 2, 
  // then threshold is [numChgs-numAnoms] --> 2
  // will return only "4", not "4, 2, 2"

}

bool OS_CounterRevAnalyze::getKeys(uint32 iterSize,\
				uint32 *maxThresh,\
				list<uint32> &tempLst){
  bool done = false;
  uint32 table;
  uint32 heavybins = 0;
  vector<list<uint32> > bktLists(REVERSIBLE_NUMROWS);
  unsigned int nextThresh;

  for(table = 0; table < REVERSIBLE_NUMROWS; table++ ){
    nextThresh = findThreshold(table, iterSize, *maxThresh);
    if( nextThresh == -1.0 ) {done = true; break;}
    else *maxThresh = nextThresh;
  }
  if (done) return done;

  /*cout << "threshold for no more than "		\
       << iterSize << " heavy bins in a table is "\
       << *maxThresh << "\n";*/

  for(table = 0; table < REVERSIBLE_NUMROWS; table++ ){
    bktLists[table].clear();
    changeDetection(table, bktLists[table], *maxThresh);
    heavybins += bktLists[table].size();
  }
  if (!heavybins) return (done=true);

  /*cout << "found " << bktLists[0].size() << " heavy bins"	\
    " in table " << 0 << " and " << heavybins << " in all.\n";*/

  GraphType graph;
  uint32 div;

  graph.clear();

  /*cout << "cleared graph successfully.\n";*/

  createNodes(graph, bktLists);

  /*cout << "created nodes successfully.\n";*/

  for(div = 0; div < REVERSIBLE_NUMDIVS; div++)
    createLinks(graph, div);

  /*cout << "created links successfully.\n";*/

  tempLst.clear();
  generateFullKeys(graph,tempLst);

  /*cout << "generated full keys successfully.\n";*/

  return done;
}

uint32 OS_CounterRevAnalyze::get_count(uint32 table, uint32 bin) {	\
  if (pCounts) return (*pCounts)[(table*REVERSIBLE_COUNTERSPERROW)+bin];
  else return 100000;
}

void OS_CounterRevAnalyze::createNodes(GraphType &graph,		\
		 const vector<list<uint32> > &bktLists){
  uint32 div, table;
  map<uint8, NodeType*> nodeMap;
  list<uint32>::const_iterator bktListIt;
  list<uint8> keys;
  list<uint8>::iterator keyIt;

  graph.resize(4);

  for(div = 0; div < REVERSIBLE_NUMDIVS; div++){
    nodeMap.clear();
    for(table = 0; table < REVERSIBLE_NUMROWS; table++){
      for(bktListIt = bktLists[table].begin();\
	  bktListIt != bktLists[table].end();\
	  bktListIt++){
	/*	cout << "div: " << div		\
	     << ", table: " << table\
	     << ", *bktListIt: " << *bktListIt\
	     << ".\n";*/
	keys.clear();
	getPossibleKeys(table, keys, div, *bktListIt);

       	//cout << "got possible keys for " << *bktListIt << ".\n";

	for(keyIt = keys.begin(); keyIt != keys.end();\
	    keyIt++) {
	  NodeType *node = nodeMap[*keyIt];
	  if( node == NULL ) {
	    node = new NodeType(*keyIt, *bktListIt,\
				table, REVERSIBLE_NUMROWS);
	    nodeMap[*keyIt] = node;
	  }
	  else {
	    if( node->bktLists[table].empty() ) node->numFound++;
	    node->bktLists[table].push_back(*bktListIt);
	  }
	}
      }
    }

    map<uint8, NodeType*>::iterator nodeMapIt;
    for(nodeMapIt = nodeMap.begin();\
	nodeMapIt != nodeMap.end();\
	nodeMapIt++) {
      NodeType *node = nodeMapIt->second;
      if( (REVERSIBLE_NUMROWS - node->numFound) <= REVERSIBLE_R )
	graph[div].push_back(node);
      else delete node;
    }
  }
}
	  
void OS_CounterRevAnalyze::createLinks(GraphType &graph, int startDiv){
  if( startDiv >= 3 )
    return;  // cannot start at the last level

  list<NodeType*> *l1List = &(graph[startDiv]);
  list<NodeType*> *l2List = &(graph[startDiv+1]);

  list<NodeType*> linkList;

  for( list<NodeType*>::iterator l1It = l1List->
	 begin();
       l1It != l1List->end();
       l1It++ )
    {

      for( list<NodeType*>::iterator l2It = l2List->
	     begin();
	   l2It != l2List->end();
	   l2It++ )
        {

	  NodeType *newNode = rIntersect(*l1It, *l2It);
	  if( newNode != NULL )
	    linkList.push_back(newNode);
        }  // End for...l2It

      delete *l1It;  // no longer need this node in level 1
    }

  // Delete all of the nodes in L2
  for( list<NodeType*>
	 ::iterator l2It = l2List->begin();
       l2It != l2List->end();
       l2It++ )
    delete *l2It;

  l1List->clear();
  l2List->clear();
  l2List->insert(l2List->end(), linkList.begin(), linkList.end());
}

NodeType* OS_CounterRevAnalyze::rIntersect(NodeType *node1, NodeType *node2){
  NodeType *newNode = new NodeType;
  newNode->bktLists.resize(REVERSIBLE_NUMROWS);

  for( int tbl = 0; tbl < REVERSIBLE_NUMROWS; tbl++ )
    {
      list<uint32> *node1Lst = &(node1->bktLists[tbl]);
      list<uint32> *node2Lst = &(node2->bktLists[tbl]);
      list<uint32> *newLst = &(newNode->bktLists[tbl]);

      listIntersect(*newLst, *node1Lst, *node2Lst);

      if( newLst->empty() )
        {
	  if( newNode->numFound >= REVERSIBLE_R )
            {
	      delete newNode;
	      return NULL;
            }
	  else
	    newNode->numFound++;
        }
    }
  
  list<uint8> *keyLst = &(newNode->keyList);
  keyLst->insert(keyLst->end(), node1->keyList.begin(), node1->keyList.end());
  keyLst->insert(keyLst->end(), node2->keyList.begin(), node2->keyList.end());

  return newNode;
}

void OS_CounterRevAnalyze::generateFullKeys(GraphType &graph,\
					 list<uint32> &resultList){
  uint32 fullKey, div;
  uint8 *keyDivided = (uint8 *)&fullKey;
  list<NodeType*>::iterator lstIt;
  list<uint8> *keyList;
  list<uint8>::iterator keyIt;
  for(lstIt = graph[3].begin(); lstIt != graph[3].end();
       lstIt++ )
    {
      keyList = &((*lstIt)->keyList);
      if( keyList->size() == 4 ){
	div = 0;
	for(keyIt = keyList->begin(); keyIt != keyList->end();	\
	    keyIt++, div++ )
	  keyDivided[div] = *keyIt;
	resultList.push_back(fullKey);
      }
      //delete *lstIt;
    }	
}



void OS_CounterRevAnalyze::listIntersect(list<uint32> &outList,	\
				      const list<uint32> &l1,	\
				      const list<uint32> &l2){
  outList.clear();

  list<uint32>::const_iterator l1It = l1.begin();
  list<uint32>::const_iterator l2It = l2.begin();

  while( true )
    {
      if( (l1It == l1.end()) || (l2It == l2.end()) )
	return;

      if( *l1It > *l2It )
	l2It++;
      else if( *l1It < *l2It )
	l1It++;
      else
        {
	  outList.push_back(*l1It);
	  l1It++;
	  l2It++;
        }
    }

};
