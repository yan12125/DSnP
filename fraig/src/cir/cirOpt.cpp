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
   buildNotInDFS2();
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
         merge(target & 0xfffffffe, 2*(*it)+(target%2), "Simplifying");
      }
      else if(gates[*it]->fanin[0] == 0 || gates[*it]->fanin[1] == 0) // type B
      {
         merge(0, 2*(*it), "Simplifying");
      }
      else if(gates[*it]->fanin[0] == gates[*it]->fanin[1]) // type C
      {
         int target = gates[*it]->fanin[0];
         merge(target & 0xfffffffe, 2*(*it)+(target%2), "Simplifying");
      }
      else if(gates[*it]->fanin[0]/2 == gates[*it]->fanin[1]/2) // type D
      {
         assert(gates[*it]->fanin[0]%2 != gates[*it]->fanin[1]%2);
         merge(0, 2*(*it), "Simplifying");
         // Fanouts of CONST wouldn't include *it, so not processing
      }
   }
   // undefs with no fanouts should be deleted
   for(vector<unsigned int>::iterator it = undefs.begin();it != undefs.end();)
   {
      if(gates[*it]->fanout.empty())
      {
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

void CirMgr::merge(unsigned int id1, unsigned int id2, string why)
{
   cout << why << ": " << inv(id1) << id1/2 << " merging " << inv(id2) << id2/2 << "..." << endl;
   vector<unsigned int>* _fanout = &gates[id2/2]->fanout;
   unsigned int* _fanin = gates[id2/2]->fanin;
   if(id1 == 0)
   {
      if(_fanin[0]/2 == 0 || _fanin[1]/2 == 0)
      {
         unsigned int target;
         if(_fanin[0]/2 == 0)
         {
            target = _fanin[1];
         }
         else
         {
            target = _fanin[0];
         }
         gates[0]->replaceFanout(id2/2, _fanout);
         for(IdIterator it = _fanout->begin();it != _fanout->end();it++)
         {
            gates[*it]->replaceFanin(id2/2, id2%2);
         }
         if(target/2 != 0)
         {
            gates[target/2]->replaceFanout(id2/2, NULL);
         }
      }
      else // two fanins of id2 are trivially inverse 
      {    // (same gate with different phase) or proved by SAT
         for(IdIterator it = _fanout->begin();it != _fanout->end();it++)
         {
            gates[*it]->replaceFanin(id2/2, 0);
            gates[0]->fanout.push_back(*it); // CONST 0 is not in original fanout, so add directly
         }
         gates[_fanin[0]/2]->replaceFanout(id2/2, NULL);
         if(_fanin[1]/2 != _fanin[0]/2) // proved by SAT
         {
            gates[_fanin[1]/2]->replaceFanout(id2/2, NULL);
         }
      }
   }
   else if(_fanin[0]/2 == id1/2 || _fanin[1]/2 == id1/2)
   {
      unsigned int target;
      if(_fanin[0]/2 == id1/2)
      {
         target = _fanin[1];
      }
      else
      {
         target = _fanin[0];
      }
      gates[id1/2]->replaceFanout(id2/2, _fanout);
      if(target == 1)
      {
         gates[0]->replaceFanout(id2/2, NULL);
      }
      else if(_fanin[0]/2 != _fanin[1]/2) // some cases proved in SAT...
      {
         // here target is (not trivially) equivalent to CONST !0
         gates[target/2]->replaceFanout(id2/2, NULL);
      }
      for(IdIterator it = _fanout->begin();it != _fanout->end();it++)
      {
         gates[*it]->replaceFanin(id2/2, id1 ^ (id1%2 != id2%2));
      }
   }
   else
   {
      gates[_fanin[0]/2]->replaceFanout(id2/2, NULL);
      if(_fanin[0]/2 != _fanin[1]/2)
      {
         gates[_fanin[1]/2]->replaceFanout(id2/2, NULL);
      }
      for(IdIterator it = _fanout->begin();it != _fanout->end();it++)
      {
         gates[id1/2]->fanout.push_back(*it);
         gates[*it]->replaceFanin(id2/2, id1);
      }
   }
   // deleting...
   delete gates[id2/2];
   gates[id2/2] = NULL;
}
