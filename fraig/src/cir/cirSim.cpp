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
   fecGroups.push_back(new set<unsigned int>);
   for(unsigned int i = 0;i <= M;i++)
   {
      if(gates[i])
      {
         (*fecGroups.begin())->insert(i);
      }
   }
   unsigned int* results = new unsigned int[this->O];
   unsigned int count = 0;
   for(vector<unsigned int>::iterator it = PO.begin();it != PO.end();it++)
   {
      unsigned int tmpResult = 0;
      CirGate *g = gates[gates[*it]->fanin[0]/2];
      if(g->gateType == AIG_GATE)
      {
         tmpResult = this->gateSim(gates[*it]->fanin[0]/2, N);
         #if SIM_DEBUG
         cout << "Last sim value for gate " << *it << " = " << hex << tmpResult << dec << endl;
         #endif
         gates[*it]->lastSimValue = tmpResult;
      }
      else if(g->gateType == PI_GATE)
      {
         tmpResult = simValues[PImap[g->getID()]];
         #if SIM_DEBUG
         cout << "Last sim value for gate " << g->getID() << " = " << hex << tmpResult << dec << endl;
         #endif
         g->lastSimValue = tmpResult;
         gates[*it]->lastSimValue = tmpResult;
      }
      else if(g->gateType == CONST_GATE || g->gateType == UNDEF_GATE)
      {
         tmpResult = 0;
      }
      results[count] = gates[*it]->fanin[0]%2?~tmpResult:tmpResult;
      #if SIM_DEBUG
      cout << "[" << count << "] " << results[count] << endl;
      #endif
      count++;
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
   delete [] results;
   delete simCache;
   simCache = NULL;
}

unsigned int CirMgr::gateSim(unsigned int gateID, unsigned int N)
{
   assert(simCache);
   unsigned int retval = -1;
   GateIDKey k(gateID);
   /*if(simCache->read(k, retval))
   {
      #if SIM_DEBUG
      cout << "Read from sim cache: " << gateID << " = " << retval << endl;
      #endif
      return retval;
   }*/
   CirGate* g = gates[gateID];
   /*if(g->gateType == PO_GATE)
   {
      unsigned int tmpResult = gateSim(g->fanin[0]/2);
      retval = g->fanin[0]%2?(~tmpResult):tmpResult;
   }
   else */if(g->gateType == AIG_GATE)
   {
      unsigned int tmpResult[2];
      CirGate *g1 = gates[g->fanin[0]/2], *g2 = gates[g->fanin[1]/2];
      if(g1->gateType == CONST_GATE || g1->gateType == UNDEF_GATE)
      {
         tmpResult[0] = 0;
      }
      else if(g1->gateType == PI_GATE)
      {
         tmpResult[0] = simValues[PImap[g1->getID()]];
         #if SIM_DEBUG
         cout << "Last sim value for gate " << g->getID() << " = " << hex << tmpResult[0] << dec << endl;
         #endif
         g1->lastSimValue = tmpResult[0];
      }
      else
      {
         if(!simCache->read(GateIDKey(g->fanin[0]/2), tmpResult[0]))
         {
            tmpResult[0] = gateSim(g->fanin[0]/2, N);
         }
      }
      if(g2->gateType == CONST_GATE || g2->gateType == UNDEF_GATE)
      {
         tmpResult[1] = 0;
      }
      else if(g2->gateType == PI_GATE)
      {
         tmpResult[1] = simValues[PImap[g2->getID()]];
         #if SIM_DEBUG
         cout << "Last sim value for gate " << g->getID() << " = " << hex << tmpResult[1] << dec << endl;
         #endif
         g2->lastSimValue = tmpResult[1];
      }
      else
      {
         if(!simCache->read(GateIDKey(g->fanin[1]/2), tmpResult[1]))
         {
            tmpResult[1] = gateSim(g->fanin[1]/2, N);
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
   }
   /*else if(g->gateType == PI_GATE)
   {
      map<unsigned int, unsigned int>::iterator it = PImap.find(g->getID()*2);
      assert(it != PImap.end());
      retval = simValues[it->second];
   }
   else if(g->gateType == CONST_GATE)
   {
      retval = 0;
   }*/
   else
   {
      retval = -1;
      assert(false);
   }
   #if SIM_DEBUG
   cout << "Sim for gate " << gateID << " = " << retval << endl;
   #endif
   simCache->write(k, retval);
   // save to each gate
   #if SIM_DEBUG
   cout << "Last sim value for gate " << gateID << " = " << hex << retval << dec << endl;
   #endif
   gates[gateID]->lastSimValue = retval;
   return retval;
}
