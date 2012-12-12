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
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

#define PARSE_DEBUG 1

using namespace std;

extern CirMgr *cirMgr;

/**************************************/
/*   class CirGate member functions   */
/**************************************/
void
CirGate::reportGate() const
{
}

void
CirGate::reportFanin(int level) const
{
   assert (level >= 0);
}

void
CirGate::reportFanout(int level) const
{
   assert (level >= 0);
}


CirAndGate::CirAndGate(unsigned int o, unsigned int i1, unsigned int i2)
{
   #if PARSE_DEBUG
   cout << pin[0] << " = " << (inv[0]?"!(":"") << (inv[1]?"!":"") << pin[1] << " && " << (inv[2]?"!":"") << pin[2] << (inv[0]?")":"") << endl;
   #endif
   pin[0] = o/2;
   pin[1] = i1/2;
   pin[2] = i2/2;
   inv[0] = o%2;
   inv[1] = i1%2;
   inv[2] = i2%2;
}

CirAndGate::~CirAndGate()
{
}

void CirAndGate::printGate() const
{
}

CirIOGate::CirIOGate(unsigned int _id, enum gateType _type): id(_id/2), type(_type), inverted(_id%2), name("")
{
   #if PARSE_DEBUG
   cout << ((_type == IGate)?"Input":"Output") << " gate " << id << (inverted?" inverted":"") << endl;
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

void CirIOGate::printGate() const
{
}
