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
   FaninKey(CirGate* g)
   {
      assert(g->gateType == AIG_GATE);
      this->fanin[0] = g->fanin[0];
      this->fanin[1] = g->fanin[1];
   }
   FaninKey()
   {
      fanin[0] = fanin[1] = 0;
   }
   ~FaninKey()
   {
   }
   FaninKey& operator=(const FaninKey& k)
   {
      this->fanin[0] = k.fanin[0];
      this->fanin[1] = k.fanin[1];
      return *this;
   }
   size_t operator() () const
   {
      size_t retVal = fanin[0]+fanin[1]+(fanin[0]%256)*(fanin[1]%256);
      #if FRAIG_DEBUG
      cout << "Hash key for " << fanin[0] << " and " << fanin[1] << " = " << retVal << endl;
      #endif
      return retVal;
   }
   bool operator==(const FaninKey& f) const
   {
      return (fanin[0] == f.fanin[0] && fanin[1] == f.fanin[1]) || 
             (fanin[1] == f.fanin[0] && fanin[0] == f.fanin[1]);
   }
private:
   unsigned int fanin[2];
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
      #if FRAIG_DEBUG
      cout << "Checking " << *it << "...\n";
      #endif
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
               FaninKey oldKey;
               if(gates[*it2]->gateType == AIG_GATE) // recalculation needed only for AIG_GATE
               {
                  oldKey = gates[*it2]; // for reaclculation later
               }
               gates[*it2]->replaceFanin(*it, 2*match+0); // fanin is 2*id+inv
               if(gates[*it2]->gateType == AIG_GATE)
               {
                  gatesHash->reCalculateHash(oldKey, FaninKey(gates[*it2]));
               }
               #if FRAIG_DEBUG
               gates[*it2]->reportFanin(1);
               #endif
            }
            delete gates[*it];
            gates[*it] = NULL;
            #if HASH_DEBUG
            gatesHash->printAll();
            #endif
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
