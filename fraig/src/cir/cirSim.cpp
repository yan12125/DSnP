/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-2012 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{
}

void
CirMgr::fileSim(ifstream& patternFile)
{
   unsigned int nSim = 0;
   unsigned char** simValues = new unsigned char*[32];
   for(unsigned int i = 0;i < 32;i++)
   {
      simValues[i] = new unsigned char[this->I/8]();
   }
   while(1)
   {
      string curLine;
      getline(patternFile, curLine);
      if(curLine.length() != this->I)
      {
         cerr << "Error: Pattern(" << curLine <<  ") length(" << curLine.length() 
              << ")does not match the number of inputs(" << this->I << ") in a circuit!!" << endl;
         break;
      }
      size_t pos = 0;
      // http://stackoverflow.com/questions/8888748/how-to-check-if-given-c-string-or-char-contains-only-digits
      if((pos = curLine.find_first_not_of("01")) != string::npos)
      {
         cerr << "Error: Pattern(" << curLine << ") contains a non-0/1 character(\'" 
              << curLine[pos] << "\')." << endl;
         break;
      }
      for(unsigned int i = 0;i < this->I;i++)
      {
         simValues[nSim%32][i/8] += (curLine[i] == '1'); // http://stackoverflow.com/questions/5369770/bool-to-int-conversion
         simValues[nSim%32][i/8] *= 2;
      }
      nSim++;
      // start simulation
      realSim(simValues);
      if(patternFile.eof())
      {
         break;
      }
   }
   for(unsigned int i = 0;i < 32;i++)
   {
      delete [] simValues[i];
   }
   delete [] simValues;
   cout << nSim << " patterns simulated." << endl;
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/

void CirMgr::realSim(unsigned char* simValues[32])
{
}
