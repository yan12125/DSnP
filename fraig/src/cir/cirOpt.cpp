/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-2012 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include <set>
#include <algorithm>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

#define OPT_DEBUG 0 

using namespace std;

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
void
CirMgr::sweep()
{
   bool* removed = new bool[M+O+1];
   for(unsigned int i = 0;i < M+O+1;i++)
   {
      removed[i] = false;
   }
   for(set<unsigned int>::iterator it = notInDFS2.begin();it != notInDFS2.end();it++)
   {
      if(gates[*it]->gateType != PI_GATE)
      {
         cout << "Sweeping: " << gates[*it]->getTypeStr() << "(" << gates[*it]->getID() << ") removed...\n";
         removed[*it] = true;
      }
   }
   // AIGinDFSOrder does not include gates not reachable from POs, so not processing it
   for(vector<unsigned int>::iterator it = undefs.begin();it != undefs.end();)
   {
      if(removed[*it])
      {
         it = undefs.erase(it);
      }
      else
      {
         it++;
      }
   }
   for(vector<unsigned int>::iterator it = floatingFanin.begin();it != floatingFanin.end();)
   {
      if(removed[*it])
      {
         it = floatingFanin.erase(it);
      }
      else
      {
         it++;
      }
   }
   for(vector<unsigned int>::iterator it = notInDFS.begin();it != notInDFS.end();)
   {
      // notInDFS is in fact defined but not used
      if(removed[*it])
      {
         #if OPT_DEBUG
         cout << "Remove " << *it << " from notInDFS\n";
         #endif
         it = notInDFS.erase(it);
      }
      else
      {
         #if OPT_DEBUG
         cout << *it << " not removed from notInDFS\n";
         #endif
         it++;
      }
   }
   for(unsigned int i=0;i<=M;i++)
   {
      if(gates[i])
      {
         gates[i]->removeFanout(removed);
      }
      if(removed[i]) // removed must be a valid gate
      {
         assert(gates[i]);
         assert(gates[i]->gateType != PI_GATE);
         delete gates[i];
         gates[i] = NULL;
      }
   }
   for(vector<unsigned int>::iterator it = PI.begin();it != PI.end();it++)
   {
      if(gates[*it/2]->fanout.empty())
      {
         notInDFS.push_back(*it/2);
      }
   }
   sort(notInDFS.begin(), notInDFS.end());
   notInDFS.erase(unique(notInDFS.begin(), notInDFS.end()), notInDFS.end());
   delete [] removed;
   buildNotInDFS2();
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
void
CirMgr::optimize()
{
   // AIGinDFSOrder will be rebuilt later, so not changing it in this optimization
   for(vector<unsigned int>::iterator it = AIGinDFSOrder.begin();it != AIGinDFSOrder.end();it++)
   {
      if(gates[*it]->fanin[0] == 1 || gates[*it]->fanin[1] == 1) // type A
      {
         unsigned int target = 0;
         if(gates[*it]->fanin[0] == 1)
         {
            target = gates[*it]->fanin[1];
         }
         else
         {
            target = gates[*it]->fanin[0];
         }
         cout << "Simplifying: " << target/2 << " merging " << (target%2?"!":"") << *it << "...\n";
         vector<unsigned int>* _fanout = &(gates[*it]->fanout);
         for(vector<unsigned int>::iterator it2 = _fanout->begin();it2 != _fanout->end();it2++)
         {
            gates[*it2]->replaceFanin(*it, target);
         }
         gates[target/2]->replaceFanout(*it, _fanout);
         if(target/2 != 0)
         {
            vector<unsigned int>::iterator itConstFanout = find(gates[0]->fanout.begin(), gates[0]->fanout.end(), *it);
            assert(itConstFanout != gates[0]->fanout.end());
            gates[0]->fanout.erase(itConstFanout);
         }
         delete gates[*it];
         gates[*it] = NULL;
      }
      else if(gates[*it]->fanin[0] == 0 || gates[*it]->fanin[1] == 0) // type B
      {
         unsigned int target = 0;
         if(gates[*it]->fanin[0] == 0)
         {
            target = gates[*it]->fanin[1];
         }
         else
         {
            target = gates[*it]->fanin[0];
         }
         cout << "Simplifying: 0 merging " << *it << "...\n";
         vector<unsigned int>* _fanout = &(gates[*it]->fanout);
         for(vector<unsigned int>::iterator it2 = _fanout->begin();it2 != _fanout->end();it2++)
         {
            gates[*it2]->replaceFanin(*it, 0);
            gates[0]->fanout.push_back(*it2);
         }
         vector<unsigned int> empty;
         gates[target/2]->replaceFanout(*it, &empty);
         if(target/2 != 0)
         {
            vector<unsigned int>::iterator itConstFanout = find(gates[0]->fanout.begin(), gates[0]->fanout.end(), *it);
            assert(itConstFanout != gates[0]->fanout.end());
            gates[0]->fanout.erase(itConstFanout);
         }
         delete gates[*it];
         gates[*it] = NULL;
      }
      else if(gates[*it]->fanin[0] == gates[*it]->fanin[1]) // type C
      {
         int target = gates[*it]->fanin[0];
         cout << "Simplifying: " << target/2 << " merging " << (target%2?"!":"") << *it << "...\n";
         vector<unsigned int>* _fanout = &(gates[*it]->fanout);
         for(vector<unsigned int>::iterator it2 = _fanout->begin();it2 != _fanout->end();it2++)
         {
            gates[*it2]->replaceFanin(*it, target);
         }
         gates[target/2]->replaceFanout(*it, _fanout);
         delete gates[*it];
         gates[*it] = NULL;
      }
      else if(gates[*it]->fanin[0]/2 == gates[*it]->fanin[1]/2) // type D
      {
         assert(gates[*it]->fanin[0]%2 != gates[*it]->fanin[1]%2);
         unsigned int target = gates[*it]->fanin[0];
         cout << "Simplifying: 0 merging " << *it << "...\n";
         vector<unsigned int>* _fanout = &(gates[*it]->fanout);
         for(vector<unsigned int>::iterator it2 = _fanout->begin();it2 != _fanout->end();it2++)
         {
            gates[*it2]->replaceFanin(*it, 0);
            gates[0]->fanout.push_back(*it2);
         }
         vector<unsigned int> empty;
         gates[target/2]->replaceFanout(*it, &empty);
         // Fanouts of CONST wouldn't include *it, so not processing
         delete gates[*it];
         gates[*it] = NULL;
      }
   }
   // undefs with no fanouts should be deleted
   for(vector<unsigned int>::iterator it = undefs.begin();it != undefs.end();)
   {
      if(gates[*it]->fanout.empty())
      {
         assert(gates[*it]);
         delete gates[*it];
         gates[*it] = NULL;
         it = undefs.erase(it);
      }
      else
      {
         it++;
      }
   }
   buildDFSwrapper();
   buildFloatingFanin();
   buildDefinedButNotUsed();
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/

