/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-2012 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

#include "cirDef.h"

extern CirMgr *cirMgr;

class CirIOGate;

// TODO: Define your own data members and member functions
class CirMgr
{
public:
   CirMgr();
   ~CirMgr();

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const { return 0; }

   // Member functions about circuit construction
   bool readCircuit(const string&);

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void writeAag(ostream&) const;

   operator int()
   {
      return hasCircuit?1:0;
   }
private:
   fstream* fCir;
   bool hasCircuit;
   unsigned int M; // maximum variable index
   unsigned int I; // number of inputs
   unsigned int L; // number of latches
   unsigned int O; // number of outputs
   unsigned int A; // number of AND gates
   CirGate** gates;
   vector<CirIOGate*> PI;
   vector<CirIOGate*> PO;
};

#endif // CIR_MGR_H
