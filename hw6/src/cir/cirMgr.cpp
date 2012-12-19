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
#include <sstream>
#include <errno.h>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

#define PARSE_DEBUG 0
#define ERROR_DEBUG 1

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

   DUMMY_END, 
   SYNTAX_ERROR = -1
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
//static char buf[1024];
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
      case SYNTAX_ERROR:
         cerr << "Syntax error in line " << lineNo << endl;
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
   if(!gates) // no circuit exists, might be opening file failed
   {
      return;
   }
   for(unsigned int i=0;i<M+O+1;i++)
   {
      delete gates[i];
   }
   delete [] gates;
}

bool
CirMgr::readCircuit(const string& fileName)
{
   bool errorOnParse = false;
   fCir = new fstream(fileName.c_str(), ios::in);
   if(!fCir->good())
   {
      cerr << "Cannot open design \"" << fileName << "\"!!" << endl;
      delete fCir;
      fCir = NULL;
      return false;
   }
   string curLine;
   lineNo = 1;
   unsigned int nAndGates = 0;
   enum { header, input, latch, output, andGate, symbol, comment } curSec = header;
   while(1)
   {
      if(fCir->eof()) // premature ending
      {
         #if ERROR_DEBUG
         cout << "Line " << __LINE__ << "\n";
         #endif
         errorOnParse = true;
         parseError(SYNTAX_ERROR);
         fCir->close();
         delete fCir;
         fCir = NULL;
         break;
      }
      getline(*fCir, curLine);
      if(curLine.empty() || curLine[0] == ' ' || curLine[curLine.size()-1] == ' ' || curLine.find("  ") != string::npos)
      {
         #if ERROR_DEBUG
         cout << "Line " << __LINE__ << "\n";
         #endif
         errorOnParse = true;
         parseError(SYNTAX_ERROR);
         break;
      }
      switch(curSec)
      {
         case header:
            if(5 == sscanf(curLine.c_str(), "aag %d %d %d %d %d", &M, &I, &L, &O, &A))
            {
               // successfully parsed
               gates = new CirGate*[M+O+1]; // additional one for primitives (0 and 1)
               gates[0] = new CirConstGate(false);
               for(unsigned int i=1;i<M+O+1;i++)
               {
                  gates[i] = NULL;
               }
               PI.reserve(I);
               PO.reserve(O);
               curSec = input;
            }
            else
            {
               #if ERROR_DEBUG
               cout << "Line " << __LINE__ << "\n";
               #endif
               errorOnParse = true;
               parseError(SYNTAX_ERROR);
               curSec = comment;
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
               if(errno != 0)
               {
                  #if ERROR_DEBUG
                  cout << "Line " << __LINE__ << "\n";
                  #endif
                  errorOnParse = true;
                  parseError(SYNTAX_ERROR);
                  curSec = comment;
                  break;
               }
               gates[id/2] = new CirIOGate(id, lineNo);
               PI.push_back(id);
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
               if(errno != 0)
               {
                  #if ERROR_DEBUG
                  cout << "Line " << __LINE__ << "\n";
                  #endif
                  errorOnParse = true;
                  parseError(SYNTAX_ERROR);
                  curSec = comment;
                  break;
               }
               int pos = M+PO.size()+1;
               gates[pos] = new CirIOGate(id, pos, lineNo);
               PO.push_back(pos);
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
                  gates[o/2] = new CirAndGate(o, i1, i2, lineNo);
                  nAndGates++;
               }
               else
               {
                  #if ERROR_DEBUG
                  cout << "Line " << __LINE__ << "\n";
                  #endif
                  errorOnParse = true;
                  parseError(SYNTAX_ERROR);
                  curSec = comment;
               }
               break;
            }
         }
         case symbol:
         {
            char type = '\0';
            unsigned int id = 0;
            char* name = new char[curLine.size()+1]; // safe length but not most compact
            if(curLine == "c") // start of comment section
            {
               curSec = comment;
            }
            else if(3 == sscanf(curLine.c_str(), "%c%u %s", &type, &id, name))
            {
               switch(type)
               {
                  case 'i': // input gate
                     if(id >= PI.size())
                     {
                        #if ERROR_DEBUG
                        cout << "Line " << __LINE__ << "\n";
                        #endif
                        errorOnParse = true;
                        parseError(SYNTAX_ERROR);
                        break;
                     }
                     reinterpret_cast<CirIOGate*>(gates[PI[id]/2])->setName(name);
                     break;
                  case 'o': // output gate
                     if(id >= PO.size())
                     {
                        #if ERROR_DEBUG
                        cout << "Line " << __LINE__ << "\n";
                        #endif
                        errorOnParse = true;
                        parseError(SYNTAX_ERROR);
                        break;
                     }
                     reinterpret_cast<CirIOGate*>(gates[PO[id]])->setName(name);
                     break;
                  default:
                     #if ERROR_DEBUG
                     cout << "Line " << __LINE__ << "\n";
                     #endif
                     errorOnParse = true;
                     parseError(SYNTAX_ERROR);
                     break;
               }
               delete [] name;
               break;
            }
            else
            {
               if(!fCir->eof()) // no more symbols
               {
                  #if ERROR_DEBUG
                  cout << "Line " << __LINE__ << "\n";
                  #endif
                  errorOnParse = true;
                  parseError(SYNTAX_ERROR);
               }
            }
            delete [] name;
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
   if(errorOnParse)
   {
      if(fCir)
      {
         fCir->close();
         delete fCir;
         fCir = NULL;
      }
      return false;
   }
   /*********** Build undefGates ***********/
   for(unsigned int i=0;i<=M+O;i++)
   {
      if(gates[i])
      {
         if(gates[i]->gateType == AIG_GATE)
         {
            CirAndGate* tmp = reinterpret_cast<CirAndGate*>(gates[i]);
            if(!gates[tmp->pin[1]])
            {
               gates[tmp->pin[1]] = new CirUndefGate(tmp->pin[1]);
               undefs.push_back(tmp->pin[1]);
            }
            if(!gates[tmp->pin[2]])
            {
               gates[tmp->pin[2]] = new CirUndefGate(tmp->pin[2]);
               undefs.push_back(tmp->pin[2]);
            }
         }
         if(gates[i]->gateType == PO_GATE)
         {
            CirIOGate* tmp = reinterpret_cast<CirIOGate*>(gates[i]);
            if(!gates[tmp->id])
            {
               gates[tmp->id] = new CirUndefGate(tmp->id);
               undefs.push_back(tmp->id);
            }
         }
      }
   }
   /******** Parse Fanouts for AIGs ********/
   for(unsigned int i=0;i<=M;i++) // M is the maximum id of gates
   {
      if(gates[i])
      {
         if(gates[i]->gateType == AIG_GATE)
         {
            CirAndGate* tmp = reinterpret_cast<CirAndGate*>(gates[i]);
            if(gates[tmp->pin[1]]) // i1
            {
               gates[tmp->pin[1]]->fanout.push_back(i);
            }
            if(gates[tmp->pin[2]]) // i2
            {
               gates[tmp->pin[2]]->fanout.push_back(i);
            }
            #if PARSE_DEBUG
            //cout << "------------" << endl;
            #endif
         }
      }
   }
   /********* Parse Fanouts for POs *********/
   for(vector<unsigned int>::iterator it = PO.begin();it != PO.end();it++)
   {
      CirIOGate* tmp = reinterpret_cast<CirIOGate*>(gates[*it]);
      if(gates[tmp->id])
      {
         #if PARSE_DEBUG
         cout << tmp->id << " fanout " << tmp->n << endl;
         #endif
         gates[tmp->id]->fanout.push_back(tmp->n);
      }
   }
   /****** Parse Fanins for And gates *******/
   // Put inverting information in fanins vector is required, 
   // because fanins can be same but with different inverting
   for(unsigned int i=1;i<=M+O;i++)
   {
      if(gates[i])
      {
         if(gates[i]->gateType == AIG_GATE)
         {
            CirAndGate* tmp = reinterpret_cast<CirAndGate*>(gates[i]);
            #if PARSE_DEBUG
            cout << i << " fanin " << tmp->pin[1] << (tmp->inv[1]?" inverted":"") << "\n"
                 << i << " fanin " << tmp->pin[2] << (tmp->inv[2]?" inverted":"") << endl;
            #endif
            tmp->fanin.push_back(tmp->pin[1]*2+tmp->inv[1]);
            tmp->fanin.push_back(tmp->pin[2]*2+tmp->inv[2]);
         }
         if(gates[i]->gateType == PO_GATE)
         {
            CirIOGate* tmp = reinterpret_cast<CirIOGate*>(gates[i]);
            #if PARSE_DEBUG
            cout << i << " fanin " << tmp->id << endl;
            #endif
            tmp->fanin.push_back(tmp->id*2+tmp->inverted);
         }
      }
   }
   /*********** Build DFS Order *************/
   unsigned int lastID = 0;
   for(vector<unsigned int>::iterator it = PO.begin();it != PO.end();it++)
   {
      lastID = buildDFSOrder(gates[*it], lastID);
   }
   // clean DFS flags
   for(unsigned int i = 0;i <= M+O;i++)
   {
      if(gates[i])
      {
         gates[i]->dfsOrder = 0;
      }
   }
   /******* Analyzing floating gates ********/
   // Part I: A gate that cannot be reached from any PO (defined but not used, or no fanouts)
   for(unsigned int i = 1;i <= M;i++)
   {
      if(gates[i])
      {
         if(gates[i]->gateType != UNDEF_GATE && gates[i]->fanout.empty())
         {
            notInDFS.push_back(i);
         }
      }
   }
   // Part II: A gate with a floating fanin
   for(vector<unsigned int>::iterator it = undefs.begin();it != undefs.end();it++)
   {
      vector<unsigned int>& fanoutList = gates[*it]->fanout;
      for(vector<unsigned int>::iterator it2 = fanoutList.begin();it2 != fanoutList.end();it2++)
      {
         floatingFanin.push_back(*it2);
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
  Total      162
*********************/
void
CirMgr::printSummary() const
{
   unsigned int aig_count = 0;
   for(unsigned int i=0;i<=M;i++)
   {
      if(gates[i])
      {
         if(gates[i]->gateType == AIG_GATE)
         {
            aig_count++;
         }
      }
   }
   const int padding = 9;
   cout << "\nCircuit Statistics\n" // wierd blank? ref!
        << "==================\n"
        << "  PI   " << right << setw(padding) << PI.size() << "\n"
        << "  PO   " << right << setw(padding) << PO.size() << "\n"
        << "  AIG  " << right << setw(padding) << aig_count << "\n"
        << "------------------\n"
        << "  Total" << right << setw(padding) << PI.size()+PO.size()+aig_count << endl;
}

void
CirMgr::printNetlist() const
{
   unsigned int count = 0;
   cout << "\n";
   for(vector<unsigned int>::const_iterator it = dfsOrder.begin();it != dfsOrder.end();it++)
   {
      cout << "[" << count << "] " << left << setw(4) << gates[*it]->getTypeStr() << gates[*it]->getID();
      switch(gates[*it]->gateType)
      {
         case PI_GATE:
         case PO_GATE:
         {
            CirIOGate* tmp = reinterpret_cast<CirIOGate*>(gates[*it]);
            if(tmp->gateType == PO_GATE)
            {
               cout << " " 
                    << ((gates[tmp->id]->gateType == UNDEF_GATE)?"*":"")
                    << (tmp->inverted?"!":"") << tmp->id;
            }
            if(tmp->name != "")
            {
               cout << " (" << tmp->name << ")";
            }
            cout << "\n";
            break;
         }
         case AIG_GATE:
         {
            CirAndGate* tmp = reinterpret_cast<CirAndGate*>(gates[*it]);
            cout << " " 
                 << ((gates[tmp->pin[1]]->gateType == UNDEF_GATE)?"*":"")
                 << (tmp->inv[1]?"!":"") << tmp->pin[1] << " " 
                 << ((gates[tmp->pin[2]]->gateType == UNDEF_GATE)?"*":"")
                 << (tmp->inv[2]?"!":"") << tmp->pin[2] << "\n"; // i1 and i2
            break;
         }
         case CONST_GATE:
            cout << "\n"; // nothing special to do
            break;
         default:
            assert(false); // shouldn't reach here
            break;
      }
      count++;
   }
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for(vector<unsigned int>::const_iterator it = PI.begin();it != PI.end();it++)
   {
      cout << " " << (*it)/2; // because no /2 in readCircuit(), don't know why
   }
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for(vector<unsigned int>::const_iterator it = PO.begin();it != PO.end();it++)
   {
      cout << " " << *it;
   }
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
   if(!floatingFanin.empty())
   {
      cout << "Gates with floating fanin(s):";
      for(vector<unsigned int>::const_iterator it = floatingFanin.begin();it != floatingFanin.end();it++)
      {
         cout << ' ' << *it;
      }
      cout << "\n";
   }
   if(!notInDFS.empty())
   {
      cout << "Gates defined but not used  :";
      for(vector<unsigned int>::const_iterator it = notInDFS.begin();it != notInDFS.end();it++)
      {
         cout << ' ' << *it;
      }
      cout << "\n";
   }
}

void
CirMgr::writeAag(ostream& outfile) const
{
   outfile << "aag " << M << " " << I << " " << L << " " << O << " " << AIGinDFSOrder.size() << "\n";
   stringstream symbols;
   int count = 0;
   for(vector<unsigned int>::const_iterator it = PI.begin();it != PI.end();it++)
   {
      CirIOGate* tmp = reinterpret_cast<CirIOGate*>(gates[*it/2]);
      outfile << tmp->id*2+tmp->inverted << "\n";
      if(tmp->name != "")
      {
         symbols << 'i' << count << ' ' << tmp->name << "\n";
         count++;
      }
   }
   count = 0;
   for(vector<unsigned int>::const_iterator it = PO.begin();it != PO.end();it++)
   {
      CirIOGate* tmp = reinterpret_cast<CirIOGate*>(gates[*it]);
      outfile << tmp->id*2+tmp->inverted << "\n";
      if(tmp->name != "")
      {
         symbols << 'o' << count << ' ' << tmp->name << "\n";
         count++;
      }
   }
   for(vector<unsigned int>::const_iterator it = AIGinDFSOrder.begin();it != AIGinDFSOrder.end();it++)
   {
      CirAndGate* tmp = reinterpret_cast<CirAndGate*>(gates[*it]);
      outfile << (tmp->pin[0]*2+tmp->inv[0]) << ' '
              << (tmp->pin[1]*2+tmp->inv[1]) << ' '
              << (tmp->pin[2]*2+tmp->inv[2]) << "\n";
   }
   outfile << symbols.str(); // should not have \n here because blank lines are not allowed in aag files
}

CirGate* CirMgr::getGate(unsigned int gid) const
{
   if(gid <= M+O)
   {
      if(gates[gid])
      {
         return gates[gid];
      }
   }
   #if PARSE_DEBUG
   cout << "Gate " << gid << " not found" << endl;
   #endif
   return NULL;
}

unsigned int CirMgr::buildDFSOrder(CirGate* g, unsigned int curID)
{
   for(vector<unsigned int>::iterator it = g->fanin.begin();it != g->fanin.end();it++)
   {
      if(gates[(*it)/2]->dfsOrder == -1 &&        // not visited
         gates[(*it)/2]->gateType != UNDEF_GATE)  // undefined gates are not counted
      {
         curID = buildDFSOrder(gates[(*it)/2], curID);
      }
   }
   g->dfsOrder = curID;
   unsigned int idOfG = g->getID();
   this->dfsOrder.push_back(idOfG);
   if(g->gateType == AIG_GATE) // for cirwrite
   {
      AIGinDFSOrder.push_back(idOfG);
   }
   curID++;
   return curID; // return maximum ID
}
