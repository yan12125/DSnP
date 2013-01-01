/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-2012 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <algorithm>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include "myHash.h"

using namespace std;

#define SIM_DEBUG 0
#define FEC_DEBUG 0
#define SIM_PERFORMANCE 0

/*******************************/
/*   Global variable and enum  */
/*******************************/

// class for use unsigned int as key in hash
class uintKey
{
public:
   uintKey(unsigned int _n = 0): n(_n)
   {
   }
   bool operator==(const uintKey& k) const
   {
      return (this->n == k.n);
   }
   size_t operator() () const
   {
      return n;
   }
private:
   unsigned int n;
};

extern size_t getHashSize(size_t s); // in util/util.cpp

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{
}

void
CirMgr::fileSim(ifstream& patternFile)
{
   unsigned int nSim = 0;
   // unsigned int are 32-bit long in most platform
   // http://blog.csdn.net/zhangzhenghe/article/details/6766581
   this->simValues = new unsigned int[this->I]();
   for(unsigned int i = 0;i<this->I;i++)
   {
      simValues[i] = 0;
   }
   while(1)
   {
      string curLine;
      getline(patternFile, curLine);
      if(curLine.length() != this->I)
      {
         if(!curLine.empty())
         {
            cerr << "\nError: Pattern(" << curLine <<  ") length(" << curLine.length() 
                 << ") does not match the number of inputs(" << this->I << ") in a circuit!!" << endl;
         }
         break;
      }
      size_t pos = 0;
      // http://stackoverflow.com/questions/8888748/how-to-check-if-given-c-string-or-char-contains-only-digits
      if((pos = curLine.find_first_not_of("01")) != string::npos)
      {
         cerr << "Error: Pattern(" << curLine << ") contains a non-0/1 character(\'" 
              << curLine[pos] << "\')." << endl;
         break;
      }
      for(unsigned int i = 0;i < this->I;i++)
      {
         #if SIM_DEBUG
         /*if(i == 0)
         {
            cout << hex << simValues[i] << dec << endl;
         }*/
         #endif
         unsigned int long long tmpBit = (curLine[i] == '1');
         tmpBit <<= (nSim%32);
         simValues[i] += tmpBit; // http://stackoverflow.com/questions/5369770/bool-to-int-conversion
         #if SIM_DEBUG
         /*if(i == 0)
         {
            cout << hex << simValues[i] << dec << "\n--------" << endl;
         }*/
         #endif
      }
      nSim++;
      // start simulation
      if(nSim%32 == 0)
      {
         realSim();
         // clear simValues
         for(unsigned int i = 0;i < this->I;i++)
         {
            simValues[i] = 0;
         }
      }
      if(patternFile.eof())
      {
         break;
      }
   }
   if(nSim%32 != 0)
   {
      #if SIM_DEBUG
      for(unsigned int i = 0;i < this->I;i++)
      {
         cout << hex << simValues[i] << dec << endl;
      }
      #endif
      realSim(nSim%32);
   }
   delete [] simValues;
   simValues = NULL;
   cout << "\n" << nSim << " patterns simulated." << endl;
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/

void CirMgr::realSim(unsigned int N)
{
   simCache = new Cache<uintKey, unsigned int>(getHashSize(this->M));
   if(fecGroups.size() == 0)
   {
      // create the first FEC group, occur when first time simulation or after fraig
      fecGroups.reserve(this->M + 1); // to avoid reallocation
      fecGroups.push_back(new vector<unsigned int>);
      vector<unsigned int>* firstGroup = *(fecGroups.begin());
      firstGroup->reserve(M+1);
      for(unsigned int i = 0;i <= M;i++)
      {
         if(gates[i])
         {
            if((gates[i]->gateType == AIG_GATE && gates[i]->dfsOrder != -1) || // only gates in DFS proceeded
                gates[i]->gateType == CONST_GATE)
            {
               firstGroup->push_back(2*i); // numbers in fecGroups are 2*id+inv
               gates[i]->curFECGroup = firstGroup;
            }
         }
      }
   }
   unsigned int* results = new unsigned int[this->O];
   unsigned int count = 0;
   for(vector<unsigned int>::iterator it = PO.begin();it != PO.end();it++)
   {
      #if SIM_PERFORMANCE
      cout << "Simulate for gate " << *it << ", clock = " << clock() << endl;
      #endif
      unsigned int tmpResult = 0;
      CirGate *g = gates[gates[*it]->fanin[0]/2];
      if(g->gateType == AIG_GATE)
      {
         tmpResult = this->gateSim(gates[*it]->fanin[0]/2, N);
      }
      else if(g->gateType == PI_GATE)
      {
         tmpResult = simValues[PImap[g->getID()]];
         // will save lastSimValue later
      }
      else if(g->gateType == CONST_GATE || g->gateType == UNDEF_GATE)
      {
         tmpResult = 0;
      }
      results[count] = gates[*it]->fanin[0]%2?~tmpResult:tmpResult;
      gates[*it]->lastSimValue = results[count];
      #if SIM_DEBUG
      cout << "Last sim value for gate " << *it << " = " << hex << results[count] << dec << ", Line " << __LINE__ << endl;
      cout << "[" << count << "] " << results[count] << endl;
      #endif
      count++;
   }
   for(vector<unsigned int>::iterator it = PI.begin();it != PI.end();it++)
   {
      gates[*it/2]->lastSimValue = simValues[PImap[*it/2]];
      #if SIM_DEBUG
      cout << "Last sim value for gate " << *it/2 << " = " << hex << gates[*it/2]->lastSimValue << dec << ", Line " << __LINE__ << endl;
      #endif
   }
   if(_simLog)
   {
      for(unsigned int i = 0;i < N;i++)
      {
         for(unsigned int j = 0;j < this->I;j++)
         {
            *_simLog << ((simValues[j] & (1LL << i))?'1':'0');
         }
         *_simLog << ' ';
         for(unsigned int j = 0;j < this->O;j++)
         {
            *_simLog << ((results[j] & (1LL << i))?'1':'0');
         }
         *_simLog << "\n";
      }
      _simLog->flush();
   }
   /*for(vector<vector<unsigned int>*>::iterator it = fecGroups.begin();it != fecGroups.end();)
   {
      unsigned int firstGate = (*it)->at(0)/2;
      bool hasNewGroup = false;
      (*it)->reserve((*it)->size()+1);
      vector<unsigned int>* curGroup = *it; // I don't know why, but *it would change afterwards
      unsigned int curGroupSize = curGroup->size(), curPos = 0;
      for(vector<unsigned int>::iterator it2 = curGroup->begin();it2 != curGroup->end();it2++, curPos++)
      {
         #if SIM_DEBUG
         cout << "Compare gate " << *it2/2 << endl;
         #endif
         if(gates[*it2/2]->lastSimValue == ~(gates[firstGate]->lastSimValue)) // totally different
         {
            *it2 ^= 0x1; // reverse last bit
            gates[*it2/2]->invInFECGroup = *it2%2;
            #if SIM_DEBUG
            if(*it2%2)
            {
               cout << "Set gate " << *it2/2 << " as inv in FEC group" << endl;
            }
            #endif
         }
         else if(gates[*it2/2]->lastSimValue != gates[firstGate]->lastSimValue)
         {
            if(!hasNewGroup)
            {
               fecGroups.push_back(new vector<unsigned int>);
               fecGroups.back()->reserve(curGroupSize + 1);
               hasNewGroup = true;
            }
            fecGroups.back()->push_back(*it2);
            gates[*it2/2]->curFECGroup = fecGroups.back();
            #if SIM_DEBUG
            printFECPairs();
            cout << "Swap " << *it2/2 << " at location " << curPos << " with location " << curGroupSize - 1 << endl;
            #endif
            *it2 = 0xffffffff; // the biggest number, meaning deleted (Such gate number is impossible on normal computers)
            #if SIM_DEBUG
            printFECPairs();
            #endif
            curGroupSize--;
         }
      }
      sort(curGroup->begin(), curGroup->end());
      // clear 0's in curGroup
      curGroup->erase(curGroup->begin() + curGroupSize, curGroup->end());
      // delete invalid groups
      if((*it)->size() == 1)
      {
         gates[(*it)->at(0)/2]->curFECGroup = NULL;
         delete *it;
         it = fecGroups.erase(it);
      }
      else
      {
         it++;
      }
   }*/
   if(fecGroups.size() == 1)
   {
      for(vector<unsigned int>::iterator it = fecGroups[0]->begin()+1;it != fecGroups[0]->end();it++)
      {
         if(gates[*it/2]->lastSimValue == 0xffffffff)
         {
            gates[*it/2]->invInFECGroup = true;
            *it ^= 0x1;
         }
      }
   }
   for(vector<vector<unsigned int>*>::iterator it = fecGroups.begin();it != fecGroups.end();it++)
   {
      //Hash<uintKey, vector<unsigned int>*> curFecGroups;
      //unsigned int curNGroups = fecGroups.size();
      /*for(vector<vector<unsigned int>*>::iterator it2 = fecGroups.begin();it2 != fecGroups.end();it2++)
      {
         curFecGroups.insert(uintKey(gates[(*it2)->front()/2]->lastSimValue), *it2);
      }*/
      Hash<uintKey, vector<unsigned int>*> newFecGroups(getHashSize((*it)->size()));
      for(vector<unsigned int>::iterator it2 = (*it)->begin()+1;it2 != (*it)->end();)
      {
         if(((*it2%2 == 0) && gates[*it2/2]->lastSimValue == gates[(*it)->at(0)/2]->lastSimValue)
          ||((*it2%2 == 1) && gates[*it2/2]->lastSimValue == ~gates[(*it)->at(0)/2]->lastSimValue))
         {
            it2++;
            continue;
         }
         vector<unsigned int> *grpNew = NULL, *grpNewInv = NULL, *grp = NULL;
         bool foundNew = newFecGroups.check(uintKey(gates[*it2/2]->lastSimValue), grpNew), 
              foundNewInv = newFecGroups.check(uintKey(~gates[*it2/2]->lastSimValue), grpNewInv);
         if(!foundNew && !foundNewInv)
         {
            #if FEC_DEBUG
            cout << "Create a new group for " << *it2/2 << " at line " << __LINE__ << endl;
            #endif
            grp = new vector<unsigned int>;
            newFecGroups.forceInsert(uintKey(gates[*it2/2]->lastSimValue), grp);
            grp->push_back(*it2 & 0xffffffe); // set inv bit to zero
            gates[*it2/2]->curFECGroup = grp;
            gates[*it2/2]->invInFECGroup = false;
            it2 = (*it)->erase(it2);
         }
         else if(foundNew)
         {
            grpNew->push_back(*it2);
            gates[*it2/2]->curFECGroup = grpNew;
            gates[*it2/2]->invInFECGroup = *it2%2;
            it2 = (*it)->erase(it2);
         }
         else if(foundNewInv)
         {
            grpNewInv->push_back(*it2 | 1);
            gates[*it2/2]->curFECGroup = grpNewInv;
            gates[*it2/2]->invInFECGroup = true;//!(*it2%2);
            it2 = (*it)->erase(it2);
         }
         /*else if(foundOldInv && grpOldInv == *it)
         {
            *it2 |= 1; // reverse inv bit
            gates[*it2/2]->invInFECGroup = true;
            it2++;
         }
         else if(foundOld && grpOld == *it)
         {
            it2++;
         }
         else
         {
            if(gates[*it2/2]->invInFECGroup)
            {
               if(foundOldInv && grpOldInv != *it)
               {
                  if(gates[grpOldInv->at(0)/2]->lastSimValue != ~gates[(*it)->at(0)/2]->lastSimValue)
                  {
                     #if FEC_DEBUG
                     cout << "Create a new group for " << *it2/2 << " at line " << __LINE__ << endl;
                     #endif
                     grp = new vector<unsigned int>; // grp is NULL now, which is not used anymore
                     newFecGroups.forceInsert(uintKey(gates[*it2/2]->lastSimValue), grp);
                     grp->push_back(*it2);
                     gates[*it2/2]->curFECGroup = grp;
                     gates[*it2/2]->invInFECGroup = *it2%2;
                     // remove from original group
                     it2 = (*it)->erase(it2);
                  }
                  else
                  {
                     it2++;
                  }
               }
               else // found in the same old group
               {
                  if(gates[grpOld->at(0)/2]->lastSimValue != gates[(*it)->at(0)/2]->lastSimValue)
                  {
                     #if FEC_DEBUG
                     cout << "Create a new group for " << *it2/2 << " at line " << __LINE__ << endl;
                     #endif
                     grp = new vector<unsigned int>; // grp is NULL now, which is not used anymore
                     newFecGroups.forceInsert(uintKey(gates[*it2/2]->lastSimValue), grp);
                     grp->push_back(*it2);
                     gates[*it2/2]->curFECGroup = grp;
                     gates[*it2/2]->invInFECGroup = *it2%2;
                     // remove from original group
                     it2 = (*it)->erase(it2);
                  }
                  else
                  {
                     it2++; // in such scenario, nothing shout be done
                  }
               }
            }
            else // not inv in fec group
            {
               if(foundOld && grpOld != *it)
               {
                  if(gates[grpOld->at(0)/2]->lastSimValue != gates[(*it)->at(0)/2]->lastSimValue)
                  {
                     #if FEC_DEBUG
                     cout << "Create a new group for " << *it2/2 << " at line " << __LINE__ << endl;
                     #endif
                     grp = new vector<unsigned int>; // grp is NULL now, which is not used anymore
                     newFecGroups.forceInsert(uintKey(gates[*it2/2]->lastSimValue), grp);
                     grp->push_back(*it2);
                     gates[*it2/2]->curFECGroup = grp;
                     gates[*it2/2]->invInFECGroup = *it2%2;
                     // remove from original group
                     it2 = (*it)->erase(it2);
                  }
                  else
                  {
                     it2++;
                  }
               }
               else // found in the same old group
               {
                  if(gates[grpOldInv->at(0)/2]->lastSimValue != ~gates[(*it)->at(0)/2]->lastSimValue)
                  {
                     #if FEC_DEBUG
                     cout << "Create a new group for " << *it2/2 << " at line " << __LINE__ << endl;
                     #endif
                     grp = new vector<unsigned int>; // grp is NULL now, which is not used anymore
                     newFecGroups.forceInsert(uintKey(gates[*it2/2]->lastSimValue), grp);
                     grp->push_back(*it2);
                     gates[*it2/2]->curFECGroup = grp;
                     gates[*it2/2]->invInFECGroup = *it2%2;
                     // remove from original group
                     it2 = (*it)->erase(it2);
                  }
                  else
                  {
                     it2++; // in such scenario, nothing shout be done
                  }
               }
            }
         }*/
         #if FEC_DEBUG
         cout << "===============\n";
         /*for(Hash<uintKey, vector<unsigned int>*>::iterator hashIt = curFecGroups.begin();hashIt != curFecGroups.end();hashIt++)
         {
            for(vector<unsigned int>::iterator itGate = (*hashIt)->begin();itGate != (*hashIt)->end();itGate++)
            {
               cout << *itGate << " ";
            }
            cout << endl;
         }
         cout << "---------------\n";*/
         for(Hash<uintKey, vector<unsigned int>*>::iterator hashIt = newFecGroups.begin();hashIt != newFecGroups.end();hashIt++)
         {
            for(vector<unsigned int>::iterator itGate = (*hashIt)->begin();itGate != (*hashIt)->end();itGate++)
            {
               cout << *itGate << " ";
            }
            cout << endl;
         }
         #endif
      }
      for(Hash<uintKey, vector<unsigned int>*>::iterator hashIt = newFecGroups.begin();hashIt != newFecGroups.end();hashIt++)
      {
         if((*hashIt)->size() > 1) // groups with only one element are not "FEC pairs"
         {
            fecGroups.push_back(*hashIt);
         }
      }
      // need to call sort before unique()
      // http://stackoverflow.com/questions/1041620/most-efficient-way-to-erase-duplicates-and-sort-a-c-vector
      /*sort(fecGroups.begin(), fecGroups.end());
      fecGroups.erase(unique(fecGroups.begin(), fecGroups.end()), fecGroups.end());*/
      #if FEC_DEBUG
      printFECPairs();
      cout.flush();
      #endif
   }
   // cleanup groups with only one element
   for(vector<vector<unsigned int>*>::iterator it = fecGroups.begin();it != fecGroups.end();)
   {
      if((*it)->size() == 1)
      {
         it = fecGroups.erase(it);
      }
      else
      {
         it++;
      }
   }
   cout << "\nTotal #FEC Group = " << fecGroups.size();
   delete [] results;
   delete simCache;
   simCache = NULL;
}

unsigned int CirMgr::gateSim(unsigned int gateID, unsigned int N)
{
   assert(simCache);
   unsigned int retval = -1;
   CirGate* g = gates[gateID];
   assert(g->gateType == AIG_GATE);
   unsigned int tmpResult[2];
   unsigned int id[2] = { g->fanin[0]/2, g->fanin[1]/2 };
   CirGate *g1 = gates[id[0]], *g2 = gates[id[1]];
   if(g1->gateType == CONST_GATE || g1->gateType == UNDEF_GATE)
   {
      tmpResult[0] = 0;
   }
   else if(g1->gateType == PI_GATE)
   {
      tmpResult[0] = simValues[PImap[id[0]]];
   }
   else
   {
      if(!simCache->read(uintKey(id[0]), tmpResult[0]))
      {
         tmpResult[0] = gateSim(id[0], N);
      }
   }
   if(g2->gateType == CONST_GATE || g2->gateType == UNDEF_GATE)
   {
      tmpResult[1] = 0;
   }
   else if(g2->gateType == PI_GATE)
   {
      tmpResult[1] = simValues[PImap[id[1]]];
      // lastSimValue for PI will be filled by realSim()
   }
   else
   {
      if(!simCache->read(uintKey(id[1]), tmpResult[1]))
      {
         tmpResult[1] = gateSim(id[1], N);
      }
   }
   switch((g->fanin[0]%2)*2+(g->fanin[1]%2))
   {
      case 0:
         retval = tmpResult[0] & tmpResult[1];
         break;
      case 1:
         retval = tmpResult[0] & (~tmpResult[1]);
         break;
      case 2:
         retval = (~tmpResult[0]) & tmpResult[1];
         break;
      case 3:
         retval = (~tmpResult[0]) & (~tmpResult[1]);
         break;
   }
   #if SIM_DEBUG
   cout << "Sim for gate " << gateID << " = " << retval << endl;
   #endif
   simCache->write(uintKey(gateID), retval);
   // save to each gate
   #if SIM_DEBUG
   cout << "Last sim value for gate " << gateID << " = " << hex << retval << dec << ", Line " << __LINE__ << endl;
   #endif
   gates[gateID]->lastSimValue = retval;
   return retval;
}
