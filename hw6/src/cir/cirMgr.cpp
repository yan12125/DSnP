/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-2012 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include <string>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
CirMgr::CirMgr() :fCir(NULL), hasCircuit(false), M(0), I(0), L(0), O(0), A(0), gates(NULL)
{
}

CirMgr::~CirMgr()
{
   for(unsigned int i=0;i<M+1;i++)
   {
      delete gates[i];
   }
   delete [] gates;
   for(vector<CirIOGate*>::iterator it = PI.begin();it != PI.end();it++)
   {
      delete *it;
   }
   PI.clear();
   for(vector<CirIOGate*>::iterator it = PO.begin();it != PO.end();it++)
   {
      delete *it;
   }
   PO.clear();
}

bool
CirMgr::readCircuit(const string& fileName)
{
   fCir = new fstream(fileName.c_str(), ios::in);
   string curLine;
   unsigned int nAndGates = 0;
   enum { header, input, latch, output, andGate, symbol, comment } curSec = header;
   while(1)
   {
      getline(*fCir, curLine);
      switch(curSec)
      {
         case header:
            if(5 == sscanf(curLine.c_str(), "aag %d %d %d %d %d", &M, &I, &L, &O, &A))
            {
               // successfully parsed
               gates = new CirGate*[M+1]; // additional one for primitives (0 and 1)
               for(unsigned int i=0;i<M+1;i++)
               {
                  gates[i] = NULL;
               }
               PI.reserve(I);
               PO.reserve(O);
               curSec = input;
            }
            else
            {
               cout << "Line " << __LINE__ << "\n";
            }
            break;
         case input:
            if(PI.size() == I)
            {
               curSec = latch; // this line hasn't been parsed yet, so no break;
            }
            else
            {
               int id = strtol(curLine.c_str(), NULL, 10);
               PI.push_back(new CirIOGate(id, CirIOGate::IGate));
               break;
            }
         case latch:
            // not implemented, give it a empty statement
            curSec = output;
         case output:
            if(PO.size() == O)
            {
               curSec = andGate;
            }
            else
            {
               int id = strtol(curLine.c_str(), NULL, 10);
               PO.push_back(new CirIOGate(id, CirIOGate::OGate));
               break;
            }
         case andGate:
         {
            if(nAndGates == A)
            {
               curSec = symbol;
            }
            else
            {
               int o = 0, i1 = 0, i2 = 0;
               if(3 == sscanf(curLine.c_str(), "%d %d %d", &o, &i1, &i2))
               {
                  gates[o/2] = new CirAndGate(o, i1, i2);
                  nAndGates++;
               }
               else
               {
                  cout << "Line " << __LINE__ << "\n";
               }
               break;
            }
         }
         case symbol:
         {
            char type = '\0';
            int id = 0;
            char* name = new char[curLine.size()+1]; // safe length but not most compact
            if(curLine == "c") // start of comment section
            {
               curSec = comment;
            }
            else if(3 == sscanf(curLine.c_str(), "%c%d %s", &type, &id, name))
            {
               switch(type)
               {
                  case 'i': // input gate
                     reinterpret_cast<CirIOGate*>(PI[id])->setName(name);
                     break;
                  case 'o': // output gate
                     reinterpret_cast<CirIOGate*>(PO[id])->setName(name);
                     break;
                  default:
                     cout << "Line " << __LINE__ << "\n";
                     break;
               }
               break;
            }
            else
            {
               if(!fCir->eof()) // no more symbols
               {
                  cout << "Line " << __LINE__ << "\n";
               }
            }
            delete name;
         }
         case comment:
            break;
      }
      lineNo++;
      if(fCir->eof() || curSec == comment)
      {
         fCir->close();
         delete fCir;
         fCir = NULL;
         break;
      }
   }
   return true;
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      167
*********************/
void
CirMgr::printSummary() const
{
}

void
CirMgr::printNetlist() const
{
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
}

void
CirMgr::writeAag(ostream& outfile) const
{
}
