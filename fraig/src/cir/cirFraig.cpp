/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2010-2012 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include <algorithm>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHash.h"
#include "util.h"

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
      if(gatesHash->check(targetKey, match))
      {
         merge(2*match, 2*(*it), "Strashing");
      }
      else
      {
         gatesHash->forceInsert(targetKey, *it);
      }
   }
   delete gatesHash;
   buildDFSwrapper();
   buildFloatingFanin();
   buildDefinedButNotUsed();
}

void
CirMgr::fraig()
{
   if(fecGroups.size() == 0) // no fec pairs identified by simulation, exiting...
   {
      return;
   }
   vector<pair<unsigned int, unsigned int> > toBeMerged; // save UNSAT pairs
   for(unsigned int i = 0;i < this->I;i++)
   {
      simValues[i] = 0; // later will use model value from sat solver to simulate
   }
   int count = 0;
   bool ending = true; // Is all AIGs are processed in fraig?
   do
   {
      ending = true;
      for(vector<unsigned int>::iterator it = AIGinDFSOrder.begin();it != AIGinDFSOrder.end();it++)
      {
         if(gates[*it]->touchedInFraig)
         {
            continue;
         }
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
            unsigned int fec[2] = { 2*(*it)+inv, *it2 }; // fec[1] will be merged
            if(fec[1] == 0) // CONST 0 should not be merged
            {
               fec[1] = fec[0];
               fec[0] = 0;
            }
            else if((fec[0] != 0) && (gates[fec[0]/2]->dfsOrder > gates[fec[1]/2]->dfsOrder))
            {
               // to prevent difficult situation, only delete parent
               unsigned int temp = fec[0];
               fec[0] = fec[1];
               fec[1] = temp;
            }
            bool result = solveBySat(fec[0], fec[1]);
            if(result) // true (SAT) means not FEC anymore
            {
               for(vector<unsigned int>::iterator it3 = PI.begin();it3 != PI.end();it3++)
               {
                  unsigned int modelValue = satSolver->getValue(gates[*it3/2]->satVar);
                  simValues[PImap[*it3/2]] += (modelValue << count);
               }
               count++;
               if(*it2/2 != 0) // CONST 0 can always used in proving
               {
                  gates[*it]->touchedInFraig = true;
               }
            }
            else // UNSAT: merging
            {
               toBeMerged.push_back(make_pair(fec[0], fec[1]));
               // remove it from FEC pairs
               IdList* curGroup = gates[fec[1]/2]->curFECGroup;
               curGroup->erase(find(curGroup->begin(), curGroup->end(), fec[1]));
               gates[fec[1]/2]->curFECGroup = NULL;
               // fec groups with size 1 will be deleted in realSim, so not precessed here
            }
         }
         if(count == 32)
         {
            cout << "\nUpdating by SAT... ";
            realSim(32, false);
            for(unsigned int i = 0;i < this->I;i++)
            {
               simValues[i] = 0; // later will use model value from sat solver to simulate
            }
            // printFECPairs();
            count = 0;
            // clear fraig flags on gates
            for(unsigned int i = 0;i <= M;i++)
            {
               if(gates[i])
               {
                  gates[i]->touchedInFraig = false;
                  //gates[i]->curFECGroup = NULL;
               }
            }
            ending = false;
            break;
         }
      }
   }while(!ending);
   cout << "\nUpdating by SAT... ";
   realSim(count, false);
   // It's possible that fecGroups still not empty: "model value collision"
   // I just discard them
   if(!fecGroups.empty())
   {
      for(FecGroup::iterator it = fecGroups.begin();it != fecGroups.end();it++)
      {
         for(IdIterator it2 = (*it)->begin();it2 != (*it)->end();it2++)
         {
            gates[*it2/2]->curFECGroup = NULL;
         }
         delete *it;
         *it = NULL;
      }
      fecGroups.clear();
      cout << "Cleaning... Total #FEC Group = 0\n";
   }
   // Merging...
   for(vector<pair<unsigned int, unsigned int> >::iterator it = toBeMerged.begin();it != toBeMerged.end();it++)
   {
      merge(it->first, it->second, "Fraig");
   }
   buildDFSwrapper();
   buildFloatingFanin();
   buildDefinedButNotUsed();
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
   satSolver->assumeProperty(gates[0]->satVar, false);
   if(id1/2 == 0) // const
   {              // id2 is always 2*(*it)+inv, and it AIG, so id1 never const
      cout << "\nProving " << id2/2 << " = " << (id2%2?"0":"1") << "...";
      satSolver->assumeProperty(gates[id2/2]->satVar, !(id2%2));
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
   // CONST 0 is required because it's used in assumeProperty false
   gates[0]->satVar = satSolver->newVar();
   for(IdIterator it = PI.begin();it != PI.end();it++)
   {
      gates[*it/2]->satVar = satSolver->newVar();
   }
   for(IdIterator it = AIGinDFSOrder.begin();it != AIGinDFSOrder.end();it++)
   {
      gates[*it]->satVar = satSolver->newVar();
   }
   // Add AIGs into solver
   for(IdIterator it = AIGinDFSOrder.begin();it != AIGinDFSOrder.end();it++)
   {
      CirGate *g1 = gates[gates[*it]->fanin[0]/2], 
              *g2 = gates[gates[*it]->fanin[1]/2];
      satSolver->addAigCNF(gates[*it]->satVar, 
            g1->satVar, gates[*it]->fanin[0]%2, 
            g2->satVar, gates[*it]->fanin[1]%2);
   }
}
