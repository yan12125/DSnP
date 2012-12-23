/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2010-2012 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHash.h"
#include "util.h"

#define FRAIG_DEBUG 0

using namespace std;

class FaninKey
{
public:
   FaninKey(CirGate* g): fanin(&(g->fanin))
   {
      assert(g->gateType == AIG_GATE);
   }
   ~FaninKey()
   {
   }
   size_t operator() () const
   {
      return (*fanin)[0]+(*fanin)[1]+((*fanin)[0]%256)*((*fanin)[1]%256);
   }
   bool operator==(const FaninKey& f) const
   {
      return ((*fanin)[0] == (*f.fanin)[0] && (*fanin)[1] == (*f.fanin)[1])||
             ((*fanin)[1] == (*f.fanin)[0] && (*fanin)[0] == (*f.fanin)[1]);
   }
private:
   vector<unsigned int>* fanin;
};

/*******************************/
/*   Global variable and enum  */
/*******************************/
extern vector<unsigned int> emptyVector;

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
void
CirMgr::strash()
{
   Hash<FaninKey, unsigned int>* gatesHash = new Hash<FaninKey, unsigned int>;
   // build hash of gates
   for(vector<unsigned int>::iterator it = AIGinDFSOrder.begin();it != AIGinDFSOrder.end();it++)
   {
      gatesHash->insert(FaninKey(gates[*it]), *it);
   }
   for(vector<unsigned int>::iterator it = AIGinDFSOrder.begin();it != AIGinDFSOrder.end();it++)
   {
      unsigned int match = 0;
      FaninKey targetKey(gates[*it]); // will used later if merged
      if(gatesHash->check(targetKey, match))
      {
         if(match != *it) // *it are to be removed, by Ref
         {                // != : matching someone else, not matching myself
            cout << "Strashing: " << match << " merging " << *it << "...\n";
            CirGate* target = gates[*it];
            // gate must be AIG_GATE here
            gates[target->fanin[0]/2]->replaceFanout(*it, &emptyVector);
            if(target->fanin[1]/2 != target->fanin[0]/2)
            {
               gates[target->fanin[1]/2]->replaceFanout(*it, &emptyVector);
            }
            for(vector<unsigned int>::iterator it2 = target->fanout.begin();it2 != target->fanout.end();it2++)
            {
               gates[match]->fanout.push_back(*it2);
               gates[*it2]->replaceFanin(*it, 2*match+0); // fanin is 2*id+inv
               #if FRAIG_DEBUG
               gates[*it2]->reportFanin(2);
               #endif
            }
            delete gates[*it];
            gates[*it] = NULL;
         }
      }
   }
   delete gatesHash;
   buildDFSwrapper();
   buildNotInDFS2();
   buildFloatingFanin();
}

void
CirMgr::fraig()
{
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/
