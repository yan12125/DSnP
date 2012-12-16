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
#include <list>
#include "cirDef.h"

using namespace std;

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGate
{
public:
   friend class CirMgr;
   CirGate(enum GateType _gateType, unsigned int _line): gateType(_gateType), lineNo(_line){}
   virtual ~CirGate() {}

   // Basic access methods
   string getTypeStr() const;
   unsigned getLineNo() const { return 0; }
   virtual int getID() const = 0;
   bool isInvert(CirGate*) const; // for fanout only, because fanin save inverted information in

   // Printing functions
   void reportGate() const;
   void reportFanin(int level, int indent = 0, bool invert = false, list<const CirGate*> *reported = NULL) const;
   void reportFanout(int level, int indent = 0, bool invert = false, list<const CirGate*> *reported = NULL) const;

private:
   CirGate(){};

protected:
   vector<unsigned int> fanin;
   vector<unsigned int> fanout;
   enum GateType gateType;
   unsigned int lineNo;
};

class CirAndGate: public CirGate
{
public:
   friend class CirMgr;
   friend class CirGate;
   CirAndGate(unsigned int, unsigned int, unsigned int, unsigned int);
   ~CirAndGate();
   virtual int getID() const;
protected:
   CirAndGate();
   unsigned int pin[3]; // pins: o, i1, i2 respectively
   bool inv[3];
};

class CirIOGate: public CirGate
{
public:
   friend class CirMgr;
   friend class CirGate;
   CirIOGate(unsigned int, unsigned int); // for PI
   CirIOGate(unsigned int, int, unsigned int); // for PO
   ~CirIOGate();
   void setName(const string&);
   virtual int getID() const;
protected:
   unsigned int id;
   bool inverted;
   string name;
   int n; // id for PO
};

class CirConstGate: public CirGate
{
public:
   friend class CirMgr;
   friend class CirGate;
   CirConstGate(bool _value): CirGate(CONST_GATE, 0), value(_value){}
   ~CirConstGate(){};
   virtual int getID() const;
private:
   CirConstGate();
   bool value;
};

class CirUndefGate: public CirGate
{
public:
   friend class CirMgr;
   friend class CirGate;
   CirUndefGate(unsigned int);
   ~CirUndefGate(){};
   virtual int getID() const;
private:
   CirUndefGate();
   unsigned int id;
};

#endif // CIR_GATE_H
