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
   // check equivalent and build hash simultaneously
   for(vector<unsigned int>::iterator it = AIGinDFSOrder.begin();it != AIGinDFSOrder.end();it++)
   {
      unsigned int match = 0;
      FaninKey targetKey(gates[*it]); // will used later if merged
      #if FRAIG_DEBUG
      cout << "Checking " << *it << "...\n";
      #endif
      if(gatesHash->check(targetKey, match))
      {
         cout << "Strashing: " << match << " merging " << *it << "...\n"; // *it is the "merged"
         CirGate* target = gates[*it];
         // gate must be AIG_GATE here
         gates[target->fanin[0]/2]->removeFanout(*it);
         if(target->fanin[1]/2 != target->fanin[0]/2)
         {
            gates[target->fanin[1]/2]->removeFanout(*it);
         }
         for(vector<unsigned int>::iterator it2 = target->fanout.begin();it2 != target->fanout.end();it2++)
         {
            gates[match]->fanout.push_back(*it2);
            gates[*it2]->replaceFanin(*it, 2*match+0); // fanin is 2*id+inv
            // reaclculation is not needed because fanouts are not in hash yet
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
      else
      {
         gatesHash->forceInsert(targetKey, *it);
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
   for(unsigned int i = 0;i < this->I;i++)
   {
      simValues[i] = 0; // later will use model value from sat solver to simulate
   }
   int count = 0;
   for(vector<unsigned int>::iterator it = AIGinDFSOrder.begin();it != AIGinDFSOrder.end();it++)
   {
      vector<unsigned int>* curGroup = gates[*it]->curFECGroup;
      if(curGroup)
      {
         if(curGroup->size() == 1) // merged beforehand
         {
            continue;
         }
         bool inv = (gates[*it]->lastSimValue == ~gates[(*curGroup)[0]/2]->lastSimValue);
         // find first gate not processed during fraig
         vector<unsigned int>::iterator it2 = curGroup->begin();
         for(;it2 != curGroup->end();it2++)
         {
            if(!gates[*it2/2]->touchedInFraig && *it2/2 != *it)
            {
               break;
            }
         }
         if(it2 == curGroup->end()) // all gates "touched"
         {
            continue;
         }
         gates[*it]->touchedInFraig = true;
         bool result = solveBySat(2*(*it)+inv, *it2);
         if(result) // true (SAT) means not FEC anymore
         {
            for(vector<unsigned int>::iterator it3 = PI.begin();it3 != PI.end();it3++)
            {
               unsigned int modelValue = satSolver->getValue(gates[*it3/2]->satVar);
               simValues[PImap[*it3/2]] += (modelValue << count);
            }
            count++;
         }
      }
      if(count == 32)
      {
         cout << "\nUpdating by SAT... ";
         realSim();
         for(unsigned int i = 0;i < this->I;i++)
         {
            simValues[i] = 0; // later will use model value from sat solver to simulate
         }
         count = 0;
      }
   }
   for(vector<vector<unsigned int>*>::iterator it = fecGroups.begin();it != fecGroups.end();it++)
   {
      delete *it;
   }
   // clear fraig flags on gates
   for(unsigned int i = 1;i <= M;i++)
   {
      if(gates[i])
      {
         gates[i]->touchedInFraig = false;
      }
   }
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/

bool CirMgr::solveBySat(unsigned int id1, unsigned int id2)
{
   if(!satSolver)
   {
      satInitialize();
   }
   satSolver->assumeRelease();
   if(id2/2 == 0) // const
   {              // id1 is always 2*(*it)+inv, and it AIG, so id1 never const
      cout << "\nProving " << id1/2 << " = " << (id1%2?"0":"1") << "...";
      satSolver->assumeProperty(gates[id1/2]->satVar, !(id1%2));
   }
   else
   {
      cout << "\nProving (" << (id1%2?"!":"") << id1/2 << ", "
           << (id2%2?"!":"") << id2/2 << ")...";
      Var f = satSolver->newVar();
      satSolver->addXorCNF(f, gates[id1/2]->satVar, id1%2, 
                             gates[id2/2]->satVar, id2%2);
      satSolver->assumeProperty(f, true);
   }
   bool result = satSolver->assumpSolve();
   cout << (result?"SAT!!":"UNSAT!!");
   return result;
}

void CirMgr::satInitialize()
{
   if(satSolver)
   {
      return;
   }
   satSolver = new SatSolver();
   satSolver->initialize();

   // Assign a Var to each gate
   // PO not included because POs are never merged
   // But PIs are included because they're referenced
   for(unsigned int i = 1;i <= M;i++)
   {
      if(gates[i])
      {
         gates[i]->satVar = satSolver->newVar();
      }
   }
   // Add AIGs into solver
   for(unsigned int i = 1;i <= M;i++)
   {
      if(gates[i])
      {
         if(gates[i]->gateType == AIG_GATE)
         {
            CirGate *g1 = gates[gates[i]->fanin[0]/2], 
                    *g2 = gates[gates[i]->fanin[1]/2];
            satSolver->addAigCNF(gates[i]->satVar, 
                  g1->satVar, gates[i]->fanin[0]%2, 
                  g2->satVar, gates[i]->fanin[1]%2);
         }
      }
   }
}
