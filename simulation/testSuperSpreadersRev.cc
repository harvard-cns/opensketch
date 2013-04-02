#include "common.h"
#include "taskSuperSpreadersRev.h"
#include "dataPlane.h"

using namespace std;

int main() {
  TaskSuperSpreadersRev ss;
  DataPlane dp;
  ss.getHashSeedsFromDataPlane(dp);
  ss.getManglerFromDataPlane(dp);

  int field1 = FIELD_SRCIP;
  int numRows = 3;
  int countersPerRow = 109226;

  int field2 = FIELD_DSTIP;
  int numBits = 60;

  // from cmu superspreaders
  // for k = 200, b = 2, delta = 0.2
  // sampling ratio (src-dst) is 44.83/200
  // r = 33

  // Magic numbers from 4.2/ sketch manager
  ss.setUserPreferencesDirectly(field1, numRows, countersPerRow,\
				field2, numBits, 44.83/200, 33);

  ss.configureDataPlane(dp);
  dp.getHashByField();
  int MINEPOCH = 0;
  int MAXEPOCH = 0;
  char TRACEPREFIX[] = "/Users/lavya810/traces/equinix-sanjose.dirA.20090917-130200";

  // iterate through files to do per packet processing
  for (int i = MINEPOCH; i <= MAXEPOCH; i++) {
    char filename[200];
    sprintf(filename,"%s.%d",TRACEPREFIX,i);
    printf("Processing Epoch %d File%s\n",i,filename);
    

    ifstream infile;
    infile.open(filename, ifstream::in);
    string line;
    Packet p;
    char srcip[20],dstip[20];
    unsigned int sport,dport,proto;
    unsigned int srcip1,srcip2,srcip3,srcip4;
    unsigned int dstip1,dstip2,dstip3,dstip4;  

    struct in_addr tmp,tmp1;    
    int packetCount = 0;

    if (infile.is_open()) {
      printf("File opened.\n");

      while(infile.good()) {
	getline(infile, line);
 	//printf("Reading line %s\n", line.c_str());
	
	const char *cStringLine = line.c_str();
	packetCount++;

	if(packetCount % 20000 == 0) {printf("%d done\n", packetCount);}

	if ( sscanf(cStringLine,"%d.%d.%d.%d,%d.%d.%d.%d,%d,%d,%d",&srcip1,&srcip2,&srcip3,&srcip4,&dstip1,&dstip2,&dstip3,&dstip4,&sport,&dport,&proto) == 11) {
	  sprintf(srcip,"%d.%d.%d.%d",srcip1,srcip2,srcip3,srcip4);
	  sprintf(dstip,"%d.%d.%d.%d",dstip1,dstip2,dstip3,dstip4);

	  if (!inet_aton(srcip,&tmp)) 
	    {
	      printf("bad ip address .. %s\n",srcip);
	    }

	  p.srcip = tmp.s_addr;
	  if(!inet_aton(dstip,&tmp1)) 
	    {
	      printf("bad ip address .. %s\n",dstip);
	    }
	  p.dstip = tmp1.s_addr;
	  p.srcport = sport;
	  p.dstport = dport;
	  p.proto = proto;

	  //printf("Calling dp to process packet\n");
	  // for k = 200, b = 2, delta = 0.2
	  // sampling ratio is 44.83/200
	  // r = 33
	  dp.processPacket(p, ss.getTaskId());
	}
      }
      printf("infile is no longer good.\n");
      printf("sampled %d out of %d i.e. %f fraction\n", dp.sampled, packetCount, (1.0*dp.sampled/ packetCount));

    }
      ss.updateCountersFromDataPlane(dp);
     

      // not about IP addresses
      // 35.216.240.203
      // using inet_aton
      // first change to 203.240.216.35
      // then IP address to integer http://www.webdnstools.com/dnstools/ipcalc
      // gives 3421558819
      // then 3421558819 - 2^32
      // -873408477

      printf("sources with many destinations\n");


      inet_aton("223.72.66.4", &tmp);      
      printf("%s or %d: %d\n", "223.72.66.4", tmp.s_addr, ss.queryGivenKey(tmp.s_addr));
      printf("check tmp.s_addr %d is %ld\n",  tmp.s_addr, -4223514401);
      printf("%d: %u\n", tmp.s_addr, ss.queryGivenKey(tmp.s_addr));
      printf("%d's count should be %d\n\n", tmp.s_addr, 205);


      inet_aton("219.46.141.122", &tmp);      
      printf("%s or %d: %u\n", "219.46.141.122", tmp.s_addr, ss.queryGivenKey(tmp.s_addr));
      printf("check tmp.s_addr %d is %d\n",  tmp.s_addr, 2056072923);
      printf("%d: %u\n", tmp.s_addr, ss.queryGivenKey(tmp.s_addr));
      printf("%d's count should be %d\n\n", tmp.s_addr, 450);


      inet_aton("35.216.240.203", &tmp);      
      printf("%s or %d: %u\n", "35.216.240.203", tmp.s_addr, ss.queryGivenKey(tmp.s_addr));
      printf("check tmp.s_addr %d is %d\n",  tmp.s_addr, -873408477);
      printf("%d: %u\n", tmp.s_addr, ss.queryGivenKey(tmp.s_addr));
      printf("%d's count should be %d\n\n", tmp.s_addr, 4993);
      printf("if count is %u, it filled up.\n", MAXUINT32);


      vector<int> superSpreaders;
      ss.getSuperSpreaders(superSpreaders);
      vector<int>::iterator it;

      for (it = superSpreaders.begin(); it != superSpreaders.end(); it++)
	{
	  char tmp[20];
	  os_ipint2string(*it,tmp);
	  printf("%s\n",tmp);
	}

      infile.close();
  }

  

  return 0;

}
