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
   cout << "==================================================\n";
}

void
CirGate::reportFanin(int level, int indent, bool invert, list<const CirGate*> *reported) const
{
   assert (level >= 0);
   bool topLevel = false;
   if(!reported)
   {
      topLevel = true;
      reported = new list<const CirGate*>;
   }
   cout << string(indent, ' ') << (invert?"!":"") << this->getTypeStr() << ' ' << this->getID();
   if(level == 0)
   {
      cout << '\n';
      return;
   }
   // #1Gp9L6MI (EE_DSnP) by ypf791
   if(this->gateType == CONST_GATE || this->gateType == PI_GATE)
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
   for(vector<unsigned int>::const_iterator it = fanin.begin();it != fanin.end();it++)
   {
      CirGate* g = cirMgr->getGate((*it)/2);
      if(g)
      {
         g->reportFanin(level-1, indent+2, (*it)%2, reported);
      }
   }
   if(topLevel)
   {
      delete reported;
   }
}

void
CirGate::reportFanout(int level, int indent, bool invert, list<const CirGate*> *reported) const
{
   assert (level >= 0);
   bool topLevel = false;
   if(!reported)
   {
      topLevel = true;
      reported = new list<const CirGate*>;
   }
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
      if(g)
      {
         g->reportFanout(level-1, indent+2, this->isInvert(g), reported);
      }
   }
   if(topLevel)
   {
      delete reported;
   }
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

int CirAndGate::getID() const
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

int CirIOGate::getID() const
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

int CirConstGate::getID() const
{
   return (value?1:0);
}

CirUndefGate::CirUndefGate(unsigned int _id): CirGate(UNDEF_GATE, 0), id(_id)
{
   #if PARSE_DEBUG
   cout << "Undefined " << id << endl;
   #endif
}

int CirUndefGate::getID() const
{
   return id;
}
