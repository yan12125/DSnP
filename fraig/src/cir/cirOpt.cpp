/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-2012 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include <set>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

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
   set<unsigned int> removed; // use set instead of vector... n^2 => n*log(n)
   for(set<unsigned int>::iterator it = notInDFS2.begin();it != notInDFS2.end();it++)
   {
      if(gates[*it]->gateType != PI_GATE)
      {
         cout << "Sweeping: " << gates[*it]->getTypeStr() << "(" << gates[*it]->getID() << ") removed...\n";
         removed.insert(*it);
      }
   }
   // AIGinDFSOrder doess not include gates not reachable from POs, so not processing it
   for(vector<unsigned int>::iterator it = undefs.begin();it != undefs.end();)
   {
      if(removed.find(*it) != removed.end())
      {
         it = undefs.erase(it);
      }
      else
      {
         it++;
      }
   }
   for(unsigned int i=1;i<=M;i++)
   {
      if(removed.find(i) != removed.end())
      {
         delete gates[i];
         gates[i] = NULL;
      }
   }
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
void
CirMgr::optimize()
{
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/

