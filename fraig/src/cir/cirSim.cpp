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
   this->simValues = new unsigned int[this->I]();
   unsigned int max_fails = 20; // test first
   unsigned int failsNow = 0;
   unsigned int nCycles = 0;
   while(1)
   {
      for(unsigned int i = 0;i < this->I;i++)
      {
         #if RAND_MAX == 2147483647
         simValues[i] = rand()*2147483648 + rand();
         #else
         simValues[i] = rand()*32768*32768+rand()*32768+rand();
         #endif
      }
      if(!realSim(32, fecGroups.size() != 0)) // realSim() return false if fecGroups are not changed
      {            // ^^^^^^^^^^^^^^ if simulate only fec, fec groups should not be empty
         failsNow++;
      }
      else
      {
         failsNow = 0;
      }
      nCycles++;
      if(failsNow >= max_fails)
      {
         break;
      }
   }
   delete [] simValues;
   cout << "\n" << nCycles*32 << " pattern simualted.";
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

// return value indicate whether fecGroups changed or not
bool CirMgr::realSim(unsigned int N, bool onlyFEC)
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
   if(!onlyFEC || _simLog)
   {
      gateListSim(&PO, NORMAL_LIST, N, results, true); // the function really do simulation
   }
   else
   {
      for(vector<vector<unsigned int>*>::iterator it = fecGroups.begin();it != fecGroups.end();it++)
      {
         gateListSim(*it, DOUBLED_LIST, N, results, true);
      }
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
   bool fecTouched = false;
   if(fecGroups.size() == 1)
   {
      fecTouched = true;
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
      Hash<uintKey, vector<unsigned int>*> newFecGroups(getHashSize((*it)->size()));
      for(vector<unsigned int>::iterator it2 = (*it)->begin()+1;it2 != (*it)->end();)
      {
         if(((*it2%2 == 0) && gates[*it2/2]->lastSimValue == gates[(*it)->at(0)/2]->lastSimValue)
          ||((*it2%2 == 1) && gates[*it2/2]->lastSimValue == ~gates[(*it)->at(0)/2]->lastSimValue))
         {
            it2++;
            continue;
         }
         fecTouched = true;
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
         #if FEC_DEBUG
         cout << "===============\n";
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
            fecTouched = true;
            fecGroups.push_back(*hashIt);
         }
      }
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
         fecTouched = true;
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
   return fecTouched;
}

void CirMgr::gateListSim(vector<unsigned int>* gateList, enum ListType type, unsigned int N, unsigned int* results, bool processPI)
{
   unsigned int count = 0;
   for(vector<unsigned int>::iterator it = gateList->begin();it != gateList->end();it++)
   {
      unsigned int gateID = 0;
      switch(type)
      {
         case NORMAL_LIST:
            gateID = *it;
            break;
         case DOUBLED_LIST:
            gateID = *it/2;
      }
      if(gates[gateID]->gateType == CONST_GATE || gates[gateID]->gateType == UNDEF_GATE)
      {
         continue;
      }
      #if SIM_PERFORMANCE
      cout << "Simulate for gate " << gateID << ", clock = " << clock() << endl;
      #endif
      unsigned int tmpResult = 0;
      CirGate *g = gates[gates[gateID]->fanin[0]/2];
      if(g->gateType == AIG_GATE)
      {
         tmpResult = this->gateSim(gates[gateID]->fanin[0]/2, N);
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
      results[count] = gates[gateID]->fanin[0]%2?~tmpResult:tmpResult;
      gates[gateID]->lastSimValue = results[count];
      #if SIM_DEBUG
      cout << "Last sim value for gate " << gateID << " = " << hex << results[count] << dec << ", Line " << __LINE__ << endl;
      cout << "[" << count << "] " << results[count] << endl;
      #endif
      count++;
   }
   if(processPI)
   {
      for(vector<unsigned int>::iterator it = PI.begin();it != PI.end();it++)
      {
         gates[*it/2]->lastSimValue = simValues[PImap[*it/2]];
         #if SIM_DEBUG
         cout << "Last sim value for gate " << *it/2 << " = " << hex << gates[*it/2]->lastSimValue << dec << ", Line " << __LINE__ << endl;
         #endif
      }
   }
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
