/****************************************************************************
  FileName     [ util.h ]
  PackageName  [ util ]
  Synopsis     [ Define function interfaces and global variables for util ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-2012 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#ifndef UTIL_H
#define UTIL_H

#include <istream>
#include <vector>
#include <algorithm>
#include <sstream>
#include "rnGen.h"
#include "myUsage.h"

using namespace std;

// Extern global variable defined in util.cpp
extern RandomNumGen  rnGen;
extern MyUsage       myUsage;

// In myString.cpp
extern int myStrNCmp(const string& s1, const string& s2, unsigned n);
extern size_t myStrGetTok(const string& str, string& tok, size_t pos = 0,
                          const char del = ' ');
extern bool myStr2Int(const string& str, int& num);
extern bool isValidVarName(const string& str);

// In myGetChar.cpp
extern char myGetChar(istream&);
extern char myGetChar();

// Write directly in header file is easier (no need to change makefile), 
// though I dont't like this...
template<class T>
int parseParam(const vector<string>& tokens, const vector<string>& availParams, size_t startPos, T& ret)
{
   string tokenCopy(tokens[startPos]);
   transform(tokenCopy.begin(), tokenCopy.end(), tokenCopy.begin(), ::toupper);
   for(unsigned int i=0;i!=availParams.size();i++)
   {
      string curParam = availParams[i];
      bool singular = false;
      if(curParam[curParam.size()-1] == '-') // no additional parameters required
      {
         curParam = curParam.substr(0, curParam.size()-1);
         singular = true;
      }
      if(tokenCopy.find(curParam.substr(0, 2))==0 && curParam.find(tokenCopy)==0)
      {
         if(singular)
         {
            return i;
         }
         // below are for non singular parameters
         if(tokens.size()==startPos)
         {
            return -1; // MISSING
         }
         stringstream ss(tokens[startPos+1]);
         ss >> ret;
         if(!ss.bad() && !ss.fail() && ss.eof())
         {
            return i; // SUCCESS
         }
         else
         {
            return -2; // ILLEGAL
         }
      }
   }
   return -3; // NOT A PARAM
}

#endif // UTIL_H
