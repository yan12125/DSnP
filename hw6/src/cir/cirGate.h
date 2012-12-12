/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-2012 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"

using namespace std;

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGate
{
public:
   CirGate() {}
   virtual ~CirGate() {}

   // Basic access methods
   string getTypeStr() const { return ""; }
   unsigned getLineNo() const { return 0; }

   // Printing functions
   virtual void printGate() const = 0;
   void reportGate() const;
   void reportFanin(int level) const;
   void reportFanout(int level) const;

private:

protected:

};

class CirAndGate: public CirGate
{
public:
   CirAndGate(unsigned int, unsigned int, unsigned int);
   ~CirAndGate();
   virtual void printGate() const;
protected:
private:
   CirAndGate();
   unsigned int pin[3]; // pins: o, i1, i2 respectively
   bool inv[3];
};

class CirIOGate: public CirGate
{
public:
   enum gateType { IGate, OGate };
   CirIOGate(unsigned int, enum gateType);
   ~CirIOGate();
   void setName(const string&);
   virtual void printGate() const;
protected:
   unsigned int id;
   gateType type;
   bool inverted;
   string name;
};

#endif // CIR_GATE_H
