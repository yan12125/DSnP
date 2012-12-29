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
class FaninKey;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGate
{
public:
   friend class CirMgr;
   friend class FaninKey;
   CirGate(enum GateType _gateType, unsigned int _line): gateType(_gateType), lineNo(_line), dfsOrder(-1), lastSimValue(0){}
   virtual ~CirGate() {}

   // Basic access methods
   string getTypeStr() const;
   unsigned getLineNo() const { return 0; }
   virtual unsigned int getID() const = 0;
   bool isInvert(CirGate*) const; // for fanout only, because fanin save inverted information in

   // Printing functions
   void reportGate() const;
   void reportFanin(int level) const;
   void reportFanout(int level) const;
   void reportFaninInternal(int level, int indent, bool invert, list<const CirGate*> *reported) const;
   void reportFanoutInternal(int level, int indent, bool invert, list<const CirGate*> *reported) const;

   void removeFanout(bool* removed); // used in sweep
   void removeFanout(unsigned int orig); // remove single fanout
   //void removeFanin(unsigned int); // used in strash
   void replaceFanin(unsigned int orig, unsigned int repl); // used in opt
   void replaceFanout(unsigned int orig, vector<unsigned int>* _fanout); // used in opt, strash

private:
   CirGate(){};

protected:
   unsigned int fanin[2];
   vector<unsigned int> fanout;
   enum GateType gateType;
   unsigned int lineNo;
   int dfsOrder;  // not visited is -1, NOT 0!!!
   unsigned int lastSimValue;
};

class CirAndGate: public CirGate
{
public:
   friend class CirMgr;
   friend class CirGate;
   CirAndGate(unsigned int, unsigned int, unsigned int, unsigned int);
   ~CirAndGate();
   virtual unsigned int getID() const;
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
   virtual unsigned int getID() const;
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
   virtual unsigned int getID() const;
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
   virtual unsigned int getID() const;
private:
   CirUndefGate();
   unsigned int id;
};

#endif // CIR_GATE_H
