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
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include "myHash.h"

using namespace std;

#define SIM_DEBUG 0
#define SIM_PERFORMANCE 0

/*******************************/
/*   Global variable and enum  */
/*******************************/

class GateIDKey
{
public:
   GateIDKey(unsigned int _gateID = 0): gateID(_gateID)
   {
   }
   bool operator==(const GateIDKey& k) const
   {
      return (this->gateID == k.gateID);
   }
   size_t operator() () const
   {
      return gateID;
   }
private:
   unsigned int gateID;
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
   cout << nSim << " patterns simulated." << endl;
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/

void CirMgr::realSim(unsigned int N)
{
   simCache = new Cache<GateIDKey, unsigned int>(getHashSize(this->M));
   fecGroups.reserve(this->M); // to avoid reallocation
   fecGroups.push_back(new vector<unsigned int>);
   vector<unsigned int>* firstGroup = *(fecGroups.begin());
   firstGroup->reserve(M+1);
   for(unsigned int i = 0;i <= M;i++)
   {
      if(gates[i])
      {
         if(gates[i]->gateType == AIG_GATE) // only AIG gates needs fraig
         {
            firstGroup->push_back(2*i); // numbers in fecGroups are 2*id+inv
            gates[i]->curFECGroup = firstGroup;
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
   vector<vector<unsigned int>* >::iterator originEnd = fecGroups.end(); // end() would change if new group are added
   for(vector<unsigned int>** it = &fecGroups.front();it != &fecGroups.front() + fecGroups.size();)
   {
      unsigned int firstGate = (*it)->at(0)/2;
      bool hasNewGroup = false;
      (*it)->reserve((*it)->size()+1);
      vector<unsigned int>* curGroup = *it; // I don't know why, but *it would change afterwards
      for(vector<unsigned int>::iterator it2 = curGroup->begin();it2 != curGroup->end();)
      {
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
            it2++; // still need to move to next element
         }
         else if(gates[*it2/2]->lastSimValue != gates[firstGate]->lastSimValue)
         {
            if(!hasNewGroup)
            {
               fecGroups.push_back(new vector<unsigned int>);
               fecGroups.back()->reserve(curGroup->size()+1);
               hasNewGroup = true;
            }
            fecGroups.back()->push_back(*it2);
            gates[*it2/2]->curFECGroup = fecGroups.back();
            it2 = curGroup->erase(it2);
         }
         else
         {
            it2++;
         }
      }
      // for validity of pointer curGroup, add const should do at last
      if(gates[firstGate]->lastSimValue == 0xffffffff)
      {
         for(vector<unsigned int>::iterator it = curGroup->begin()++;it != curGroup->end();it++)
         {
            *it ^= 0x1;
         }
      }
      if(gates[firstGate]->lastSimValue == 0 || gates[firstGate]->lastSimValue == 0xffffffff)
      {
         gates[0]->curFECGroup = curGroup;
         gates[0]->invInFECGroup = false;
         curGroup->insert(curGroup->begin(), 0);
      }
      // delete invalid groups
      if((*it)->size() == 1)
      {
         fecGroups.erase(fecGroups.begin()+(it - &(fecGroups.front())));
      }
      else
      {
         it++;
      }
   }
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
      if(!simCache->read(GateIDKey(id[0]), tmpResult[0]))
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
      if(!simCache->read(GateIDKey(id[1]), tmpResult[1]))
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
   simCache->write(GateIDKey(gateID), retval);
   // save to each gate
   #if SIM_DEBUG
   cout << "Last sim value for gate " << gateID << " = " << hex << retval << dec << ", Line " << __LINE__ << endl;
   #endif
   gates[gateID]->lastSimValue = retval;
   return retval;
}
