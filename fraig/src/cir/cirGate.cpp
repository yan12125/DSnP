/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-2012 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include <list>
#include <algorithm>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

#define PARSE_DEBUG 0
#define OPT_DEBUG 0

using namespace std;

extern CirMgr *cirMgr;

/**************************************/
/*   class CirGate member functions   */
/**************************************/
void
CirGate::reportGate() const
{
   stringstream secondline;
   cout << "==================================================\n";
   secondline << "= " << this->getTypeStr() << "(" << this->getID() << ")";
   if(this->gateType == PO_GATE || this->gateType == PI_GATE)
   {
      const CirIOGate* tmp = reinterpret_cast<const CirIOGate*>(this);
      if(tmp->name != "")
      {
         secondline << "\"" << tmp->name << "\"";
      }
   }
   secondline << ", line " << this->lineNo;
   cout << secondline.str();
   int len = secondline.str().size();
   if(len < 49) // 50 chars width, but last char is =
   {
      cout << string(49-len, ' ') << "=\n";
   }
   cout << "= FECs:                                          =\n"
        << "= Value: ";
   for(int i = 31;i >= 0;i--)
   {
      cout << ((lastSimValue & (1 << i))?'1':'0');
      if(i%4 == 0 && i != 0)
      {
         cout << '_';
      }
   }
   cout << " =\n==================================================\n";
}

void CirGate::reportFanin(int level) const
{
   assert (level >= 0);
   list<const CirGate*> reported;
   reportFaninInternal(level, 0, false, &reported);
   reported.clear();
}

void CirGate::reportFanout(int level) const
{
   assert (level >= 0);
   list<const CirGate*> reported;
   reportFanoutInternal(level, 0, false, &reported);
   reported.clear();
}

void
CirGate::reportFaninInternal(int level, int indent, bool invert, list<const CirGate*> *reported) const
{
   assert (level >= 0);
   cout << string(indent, ' ') << (invert?"!":"") << this->getTypeStr() << ' ' << this->getID();
   if(level == 0)
   {
      cout << '\n';
      return;
   }
   // #1Gp9L6MI (EE_DSnP) by ypf791
   if(this->gateType == CONST_GATE || this->gateType == PI_GATE || this->gateType == UNDEF_GATE)
   {
      level = 0;
   }
   else if(find(reported->begin(), reported->end(), this) != reported->end())
   {
      cout << " (*)";
      level = 0;
   }
   cout << '\n';
   if(level == 0)
   {
      return;
   }
   reported->push_back(this);
   /*for(vector<unsigned int>::const_iterator it = fanin.begin();it != fanin.end();it++)
   {
      CirGate* g = cirMgr->getGate((*it)/2);
      // g must be a valid gate here
      g->reportFaninInternal(level-1, indent+2, (*it)%2, reported);
   }*/
   if(this->gateType == AIG_GATE)
   {
      cirMgr->getGate(fanin[0]/2)->reportFaninInternal(level-1, indent+2, fanin[0]%2, reported);
      cirMgr->getGate(fanin[1]/2)->reportFaninInternal(level-1, indent+2, fanin[1]%2, reported);
   }
   else if(this->gateType == PO_GATE)
   {
      cirMgr->getGate(fanin[0]/2)->reportFaninInternal(level-1, indent+2, fanin[0]%2, reported);
   }
}

void
CirGate::reportFanoutInternal(int level, int indent, bool invert, list<const CirGate*> *reported) const
{
   assert (level >= 0);
   cout << string(indent, ' ') << (invert?"!":"") << this->getTypeStr() << ' ' << this->getID();
   if(level == 0)
   {
      cout << '\n';
      return;
   }
   if(find(reported->begin(), reported->end(), this) != reported->end())
   {
      cout << " (*)";
      level = 0;
   }
   cout << '\n';
   if(level == 0)
   {
      return;
   }
   reported->push_back(this);
   for(vector<unsigned int>::const_iterator it = fanout.begin();it != fanout.end();it++)
   {
      CirGate* g = cirMgr->getGate(*it);
      // g must be a valid gate here
      g->reportFanoutInternal(level-1, indent+2, this->isInvert(g), reported);
   }
}

void CirGate::removeFanout(bool* removed)
{
   for(vector<unsigned int>::iterator it = fanout.begin();it != fanout.end();)
   {
      if(removed[*it])
      {
         it = fanout.erase(it);
      }
      else
      {
         it++;
      }
   }
}

void CirGate::replaceFanin(unsigned int orig, unsigned int repl)
{
   // repl is in the form 2*id+inv, while orig is id
   #if OPT_DEBUG
   cout << "Replacing fanin of " << this->getTypeStr() << " " << this->getID() << ", " << orig << "=>" << repl << endl;
   #endif
   if(orig == repl/2)
   {
      return;
   }
   if(this->gateType == AIG_GATE)
   {
      CirAndGate* tmp = reinterpret_cast<CirAndGate*>(this);
      if(fanin[0]/2 == orig)
      {
         tmp->pin[1] = repl/2; // pin and inv are o, i1, i2
         tmp->inv[1] = (repl+tmp->inv[1])%2;
         fanin[0] = 2*tmp->pin[1]+tmp->inv[1];
      }
      else if(fanin[1]/2 == orig)
      {
         tmp->pin[2] = repl/2;
         tmp->inv[2] = (repl+tmp->inv[2])%2;
         fanin[1] = 2*tmp->pin[2]+tmp->inv[2];
      }
      else // impossible
      {
         assert(false);
      }
   }
   else if(this->gateType == PO_GATE)
   {
      CirIOGate* tmp = reinterpret_cast<CirIOGate*>(this);
      if(fanin[0]/2 == orig)
      {
         tmp->id = repl/2;
         tmp->inverted = (repl+tmp->inverted)%2;
         fanin[0] = 2*tmp->id+tmp->inverted;
      }
      else
      {
         assert(false);
      }
   }
   else
   {
      assert(false);
   }
}

void CirGate::replaceFanout(unsigned int orig, vector<unsigned int>* _fanout)
{
   #if OPT_DEBUG
   cout << "Replacing fanout of " << this->getTypeStr() << " " << this->getID() << ", " << orig << "=>";
   for(vector<unsigned int>::iterator it = _fanout->begin();it != _fanout->end();it++)
   {
      cout << " " << *it << ",";
   }
   cout << endl;
   #endif
   // 
   bool hasDeleted = false;
   for(vector<unsigned int>::iterator it = fanout.begin();it != fanout.end();)
   {
      if(*it == orig)
      {
         it = fanout.erase(it);
         hasDeleted = true;
      }
      else
      {
         it++;
      }
   }
   assert(hasDeleted);
   for(vector<unsigned int>::iterator it = _fanout->begin();it != _fanout->end();it++)
   {
      this->fanout.push_back(*it);
   }
   #if OPT_DEBUG
   /*for(vector<unsigned int>::iterator it = fanout.begin();it != fanout.end();it++)
   {
      cout << *it << " ";
   }
   cout << "\n";*/
   #endif
   //sort(fanout.begin(), fanout.end());
   //fanout.erase(unique(fanout.begin(), fanout.end()), fanout.end());
   #if OPT_DEBUG
   for(vector<unsigned int>::iterator it = fanout.begin();it != fanout.end();it++)
   {
      cout << *it << " ";
   }
   cout << "\n";
   #endif
}

/*void CirGate::removeFanin(unsigned int target)
{
   for(vector<unsigned int>::iterator it = fanin.begin();it != fanin.end();)
   {
      if(*it/2 == target)
      {
         fanin.erase(it);
         return;
      }
      else
      {
         it++;
      }
   }
   assert(false); // specified gate must be in fanin
}*/

void CirGate::removeFanout(unsigned int orig)
{
   bool hasDelete = false;
   for(vector<unsigned int>::iterator it = fanout.begin();it != fanout.end();)
   {
      if(*it == orig)
      {
         it = fanout.erase(it);
         hasDelete = true;
      }
      else
      {
         it++;
      }
   }
   assert(hasDelete);
}

string CirGate::getTypeStr() const
{
   switch(this->gateType)
   {
      case PI_GATE:
         return "PI";
         break;
      case PO_GATE:
         return "PO";
         break;
      case AIG_GATE:
         return "AIG";
         break;
      case CONST_GATE:
         return "CONST";
         break;
      case UNDEF_GATE:
         return "UNDEF";
         break;
      default:
         return "";
         break;
   }
}

bool CirGate::isInvert(CirGate* out) const
{
   if(out->gateType == PO_GATE)
   {
      return (reinterpret_cast<CirIOGate*>(out))->inverted;
   }
   if(out->gateType == AIG_GATE)
   {
      CirAndGate* tmp = reinterpret_cast<CirAndGate*>(out);
      if(tmp->pin[1] == this->getID())
      {
         return tmp->inv[1];
      }
      if(tmp->pin[2] == this->getID())
      {
         return tmp->inv[2];
      }
   }
   return false;
}

CirAndGate::CirAndGate(unsigned int o, unsigned int i1, unsigned int i2, unsigned int _line): CirGate(AIG_GATE, _line)
{
   pin[0] = o/2;
   pin[1] = i1/2;
   pin[2] = i2/2;
   inv[0] = o%2;
   inv[1] = i1%2;
   inv[2] = i2%2;
   #if PARSE_DEBUG
   cout << pin[0] << " = " << (inv[0]?"!(":"") << (inv[1]?"!":"") << pin[1] << " && " << (inv[2]?"!":"") << pin[2] << (inv[0]?")":"") << endl;
   #endif
}

CirAndGate::~CirAndGate()
{
}

unsigned int CirAndGate::getID() const
{
   return pin[0];
}

CirIOGate::CirIOGate(unsigned int _id, unsigned int _line): CirGate(PI_GATE, _line), id(_id/2), inverted(_id%2), name(""), n(-1)
{
   #if PARSE_DEBUG
   cout << "Input" << " gate " << id << (inverted?" inverted":"") << endl;
   #endif
}

CirIOGate::CirIOGate(unsigned int _id, int _n, unsigned int _line): CirGate(PO_GATE, _line), id(_id/2), inverted(_id%2), name(""), n(_n)
{
   #if PARSE_DEBUG
   cout << "Output" << " gate " << id << ", " << n << (inverted?" inverted":"") << endl;
   #endif
}

CirIOGate::~CirIOGate()
{
}

void CirIOGate::setName(const string& _name)
{
   name = _name;
   #if PARSE_DEBUG
   cout << "setName: " << _name << endl;
   #endif
}

unsigned int CirIOGate::getID() const
{
   if(this->gateType == PO_GATE)
   {
      return this->n;
   }
   else
   {
      return this->id;
   }
}

unsigned int CirConstGate::getID() const
{
   return 0; // const gate has ID 0
}

CirUndefGate::CirUndefGate(unsigned int _id): CirGate(UNDEF_GATE, 0), id(_id)
{
   #if PARSE_DEBUG
   cout << "Undefined " << id << endl;
   #endif
}

unsigned int CirUndefGate::getID() const
{
   return id;
}
