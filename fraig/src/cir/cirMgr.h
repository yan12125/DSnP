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
#include <set>
#include <map>
#include <list>

using namespace std;

#include "cirDef.h"
#include "myHash.h"

extern CirMgr *cirMgr;

class CirIOGate;
class CirAndGate;
class CirConstGate;
class GateIDKey;

// TODO: Define your own data members and member functions
class CirMgr
{
public:
   CirMgr();
   ~CirMgr();

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const;

   // Member functions about circuit construction
   bool readCircuit(const string&);

   // Member functions about circuit optimization
   void sweep();
   void optimize();

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }

   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void writeAag(ostream&) const;

   operator int()
   {
      return hasCircuit?1:0;
   }
private:
   ofstream           *_simLog;
   fstream* fCir;
   bool hasCircuit;
   unsigned int M; // maximum variable index
   unsigned int I; // number of inputs
   unsigned int L; // number of latches
   unsigned int O; // number of outputs
   unsigned int A; // number of AND gates
   CirGate** gates;
   vector<unsigned int> PI;
   vector<unsigned int> PO;
   vector<unsigned int> undefs;
   vector<unsigned int> dfsOrder;
   vector<unsigned int> dfsOrderWithUndefs;
   vector<unsigned int> AIGinDFSOrder;
   vector<unsigned int> notInDFS;      // defined but not used
   set<unsigned int> notInDFS2;        // real not in DFS list, undefs considered
   vector<unsigned int> floatingFanin;
   unsigned int* PImap;  // which PI is id N?
   Cache<GateIDKey, unsigned int>* simCache;
   unsigned int* simValues;
   vector<vector<unsigned int>* > fecGroups;
   
   /* helper functions */
   unsigned int buildDFSOrder(CirGate*, unsigned int, vector<unsigned int>*, bool);
   void buildDFSwrapper();
   void buildFloatingFanin();
   void buildDefinedButNotUsed();
   void buildNotInDFS2();

   // simulation functions
   void realSim(unsigned int N = 32);
   unsigned int gateSim(unsigned int gateID, unsigned int N);
};

#endif // CIR_MGR_H
