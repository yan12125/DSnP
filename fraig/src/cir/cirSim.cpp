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
#include <cmath>
#include <sys/time.h>
#include <sys/types.h>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include "myHash.h"

using namespace std;

#define SIM_PERFORMANCE 0

/*******************************/
/*   Global variable and enum  */
/*******************************/

class SimValueKey
{
public:
   SimValueKey(unsigned int n): value((n > 0x7fffffff)?(~n):(n))
   {
   }
   bool operator==(const SimValueKey& k) const
   {
      return (this->value == k.value);
   }
   size_t operator() () const
   {
      return value;
   }
private:
   unsigned int value;
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
   unsigned int max_fails = (unsigned int)sqrt(this->A);
   cout << "MAX_FAILS = " << max_fails << endl;
   unsigned int failsNow = 0;
   unsigned int nCycles = 0;
   bool firstTime = true;
   #if SIM_PERFORMANCE
   timeval tv;
   #endif
   while(1)
   {
      #if SIM_PERFORMANCE
      gettimeofday(&tv, NULL);
      cout << tv.tv_sec << " " << tv.tv_usec << endl;
      #endif
      for(unsigned int i = 0;i < this->I;i++)
      {
         #if RAND_MAX == 2147483647
         simValues[i] = rand()*2147483648 + rand();
         #else
         simValues[i] = rand()*32768*32768+rand()*32768+rand();
         #endif
      }
      #if SIM_PERFORMANCE
      gettimeofday(&tv, NULL);
      cout << tv.tv_sec << " " << tv.tv_usec << "\n" << endl;
      #endif
      unsigned int originNFec = fecGroups.size();
      realSim(32, firstTime);
      firstTime = false;
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
   cout << nCycles*32 << " pattern simualted.";
}

void
CirMgr::fileSim(ifstream& patternFile)
{
   cout << "\n"; // mysterious \n in ref???
   unsigned int nSim = 0;
   bool firstTime = true;
   // unsigned int are 32-bit long in most platform
   // http://blog.csdn.net/zhangzhenghe/article/details/6766581
   for(unsigned int i = 0;i<this->I;i++)
   {
      simValues[i] = 0;
   }
   while(1)
   {
      string curLine;
      patternFile >> curLine;
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
         unsigned long long int tmpBit = (curLine[i] == '1');
         tmpBit <<= (nSim%32);
         simValues[i] += tmpBit; // http://stackoverflow.com/questions/5369770/bool-to-int-conversion
      }
      nSim++;
      // start simulation
      if(nSim%32 == 0)
      {
         realSim(32, firstTime);
         firstTime = false;
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
      realSim(nSim%32, firstTime);
   }
   cout << ((PI.size() != 0 && PO.size() != 0)?nSim:0) << " patterns simulated." << endl;
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/

// return value indicate whether fecGroups changed or not
void CirMgr::realSim(unsigned int N, bool firstTime)
{
   #if SIM_PERFORMANCE
   timeval tv;
   gettimeofday(&tv, NULL);
   cout << "\nIn realSim(), clock = " << (tv.tv_sec - this->tv0.tv_sec) * 1000000 + (tv.tv_usec - this->tv0.tv_usec) << endl;
   #endif
   if(_simLog)
   {
      for(IdList::iterator it = PI.begin();it != PI.end();it++)
      {
         gates[*it/2]->lastSimValue = simValues[PImap[*it/2]];
      }
   }
   if(PO.size() == 0 || PI.size() == 0)
   {
      return;
   }
   simCache.reset();
   simCache.init(this->M);
   if(firstTime)
   {
      // create the first FEC group, occur when first time simulation or after fraig
      fecGroups.push_back(new IdList);
      IdList* firstGroup = *(fecGroups.begin());
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
   gateListSim(&PO, N);
   if(_simLog && PI.size() != 0)
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
            *_simLog << ((gates[this->M+j+1]->lastSimValue & (1LL << i))?'1':'0');
         }
         *_simLog << "\n";
      }
      _simLog->flush();
   }
   if(fecGroups.size() == 1)
   {
      for(IdList::iterator it = fecGroups.front()->begin()+1;it != fecGroups.front()->end();it++)
      {
         if(gates[*it/2]->lastSimValue == 0xffffffff)
         {
            *it ^= 0x1;
         }
      }
   }
   for(FecGroup::iterator it = fecGroups.begin();it != fecGroups.end();it++)
   {
      unsigned int hashSize = getHashSize((*it)->size());
      #if SIM_PERFORMANCE
      //cout << "Hash size = " << hashSize << ", clock = " << clock() << endl;
      #endif
      Hash<SimValueKey, IdList*> newFecGroups(hashSize);
      IdList curGroupCopy;
      curGroupCopy.reserve((*it)->size());
      curGroupCopy.push_back((*it)->front());
      for(IdList::iterator it2 = (*it)->begin()+1;it2 != (*it)->end();it2++)
      {
         unsigned int curID = *it2;
         CirGate* g = gates[curID/2];
         if(((curID%2 == 0) && g->lastSimValue == gates[(**it)[0]/2]->lastSimValue)
          ||((curID%2 == 1) && g->lastSimValue == ~gates[(**it)[0]/2]->lastSimValue))
         {
            curGroupCopy.push_back(curID);
            continue;
         }
         IdList *grpNew;
         if(!newFecGroups.check(SimValueKey(g->lastSimValue), grpNew))
         {
            grpNew = new IdList(1, curID & 0xfffffffe); // set inv bit to zero
            newFecGroups.forceInsert(SimValueKey(g->lastSimValue), grpNew);
            g->curFECGroup = grpNew;
         }
         else // exist in Hash, meaning FEC or inverse FEC
         {
            if(g->lastSimValue == ~gates[(*grpNew)[0]/2]->lastSimValue) // inverse FEC
            {
               grpNew->push_back(curID | 0x1);
            }
            else // normal FEC
            {
               grpNew->push_back(curID & 0xfffffffe); // set inv bit to zero
            }
            g->curFECGroup = grpNew;
         }
      }
      **it = curGroupCopy;
      for(Hash<SimValueKey, IdList*>::iterator hashIt = newFecGroups.begin();hashIt != newFecGroups.end(); ++hashIt)
      {
         if(((*hashIt).second)->size() > 1) // groups with only one element are not "FEC pairs"
         {
            fecGroups.push_back((*hashIt).second);
         }
         else
         {
            gates[((*hashIt).second)->front()/2]->curFECGroup = NULL;
            delete (*hashIt).second;
         }
      }
   }
   // cleanup groups with only one element
   for(FecGroup::iterator it = fecGroups.begin();it != fecGroups.end();)
   {
      if((*it)->size() == 1)
      {
         gates[(*it)->front()/2]->curFECGroup = NULL;
         delete *it;
         it = fecGroups.erase(it);
      }
      else
      {
         it++;
      }
   }
   if(PI.size())
   {
      cout << "Total #FEC Group = " << fecGroups.size() << "\n";
   }
}

void CirMgr::gateListSim(IdList* gateList, unsigned int N)
{
   if(gateList->size() == 0)
   {
      return;
   }
   unsigned int count = 0;
   for(IdList::iterator it = gateList->begin(); it != gateList->end(); it++)
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
      gates[gateID]->lastSimValue = gates[gateID]->fanin[0]%2?~tmpResult:tmpResult;
      count++;
   }
}

unsigned int CirMgr::gateSim(unsigned int gateID, unsigned int N)
{
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
   else
   {
      if(!simCache.read(uintKey(id[0]), tmpResult[0]))
      {
         if(g1->gateType == PI_GATE)
         {
            tmpResult[0] = simValues[PImap[id[0]]];
         }
         else
         {
            tmpResult[0] = gateSim(id[0], N);
         }
      }
   }
   if(g2->gateType == CONST_GATE || g2->gateType == UNDEF_GATE)
   {
      tmpResult[1] = 0;
   }
   else
   {
      if(!simCache.read(uintKey(id[1]), tmpResult[1]))
      {
         if(g2->gateType == PI_GATE)
         {
            tmpResult[1] = simValues[PImap[id[1]]];
            // lastSimValue for PI will be filled by realSim()
         }
         else
         {
            tmpResult[1] = gateSim(id[1], N);
         }
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
   simCache.write(uintKey(gateID), retval);
   // save to each gate
   gates[gateID]->lastSimValue = retval;
   return retval;
}
