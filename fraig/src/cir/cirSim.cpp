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
      #if SIM_PERFORMANCE
      cout << "\nGenerate random value done, clock = " << clock();
      #endif
      unsigned int originNFec = fecGroups.size();
      realSim(32, false);
      if(fecGroups.size() == originNFec)
      {
         failsNow++;
      }
      else
      {
         failsNow = 0;
      }
      nCycles++;
      if(failsNow >= max_fails || fecGroups.size() == 1) // size = 1 means first time emulation in realSim()
      {
         break;
      }
   }
   cout << "\n" << nCycles*32 << " pattern simualted.";
}

void
CirMgr::fileSim(ifstream& patternFile)
{
   unsigned int nSim = 0;
   // unsigned int are 32-bit long in most platform
   // http://blog.csdn.net/zhangzhenghe/article/details/6766581
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
   cout << "\n" << nSim << " patterns simulated." << endl;
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/

// return value indicate whether fecGroups changed or not
void CirMgr::realSim(unsigned int N, bool isRandom)
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
   gateListSim(&PO, N, !isRandom || _simLog); // the function really do simulation
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
   if(fecGroups.size() == 1)
   {
      for(vector<unsigned int>::iterator it = fecGroups[0]->begin()+1;it != fecGroups[0]->end();it++)
      {
         if(gates[*it/2]->lastSimValue == 0xffffffff)
         {
            *it ^= 0x1;
         }
      }
   }
   for(vector<vector<unsigned int>*>::iterator it = fecGroups.begin();it != fecGroups.end();it++)
   {
      unsigned int hashSize = getHashSize((*it)->size());
      #if SIM_PERFORMANCE
      //cout << "Hash size = " << hashSize << ", clock = " << clock() << endl;
      #endif
      Hash<uintKey, vector<unsigned int>*> newFecGroups(hashSize);
      vector<unsigned int> curGroupCopy;
      curGroupCopy.reserve((*it)->size());
      for(vector<unsigned int>::iterator it2 = (*it)->begin();it2 != (*it)->end();it2++)
      {
         unsigned int curID = *it2;
         CirGate* g = gates[curID/2];
         if(((curID%2 == 0) && g->lastSimValue == gates[(**it)[0]/2]->lastSimValue)
          ||((curID%2 == 1) && g->lastSimValue == ~gates[(**it)[0]/2]->lastSimValue))
         {
            #if FEC_DEBUG
            cout << "Push " << curID << " to curGroupCopy" << endl;
            #endif
            curGroupCopy.push_back(curID);
            continue;
         }
         vector<unsigned int> *grpNew;
         if(!newFecGroups.check(uintKey(g->lastSimValue), grpNew))
         {
            if(!newFecGroups.check(uintKey(~g->lastSimValue), grpNew))
            {
               #if FEC_DEBUG
               cout << "Create a new group for " << curID/2 << " at line " << __LINE__ << endl;
               #endif
               #if SIM_PERFORMANCE
               cout << "Create a new group, clock = " << clock() << endl;
               #endif
               grpNew = new vector<unsigned int>(1, curID & 0xfffffffe); // set inv bit to zero
               newFecGroups.forceInsert(uintKey(g->lastSimValue), grpNew);
               g->curFECGroup = grpNew;
            }
            else
            {
               grpNew->push_back(curID | 1);
               g->curFECGroup = grpNew;
            }
         }
         else
         {
            grpNew->push_back(curID);
            g->curFECGroup = grpNew;
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
      **it = curGroupCopy;
      for(Hash<uintKey, vector<unsigned int>*>::iterator hashIt = newFecGroups.begin();hashIt != newFecGroups.end();hashIt++)
      {
         if((*hashIt)->size() > 1) // groups with only one element are not "FEC pairs"
         {
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
         it = fecGroups.erase(it);
      }
      else
      {
         it++;
      }
   }
   cout << "\nTotal #FEC Group = " << fecGroups.size();
   delete simCache;
   simCache = NULL;
}

void CirMgr::gateListSim(vector<unsigned int>* gateList, unsigned int N, bool processPI)
{
   unsigned int count = 0;
   unsigned int *firstGate = &gateList->front(), *lastGate = &gateList->back();
   for(unsigned int* it = firstGate;it <= lastGate;it++)
   {
      unsigned int gateID = *it;
      if(gates[gateID]->gateType == CONST_GATE || gates[gateID]->gateType == UNDEF_GATE)
      {
         continue;
      }
      #if SIM_PERFORMANCE
      //cout << "Simulate for gate " << gateID << ", clock = " << clock() << endl;
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
