/****************************************************************************
  FileName     [ memCmd.cpp ]
  PackageName  [ mem ]
  Synopsis     [ Define memory test commands ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-2012 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include "memCmd.h"
#include "memTest.h"
#include "cmdParser.h"
#include "util.h"

using namespace std;

extern MemTest mtest;  // defined in memTest.cpp

bool
initMemCmd()
{
   if (!(cmdMgr->regCmd("MTReset", 3, new MTResetCmd) &&
         cmdMgr->regCmd("MTNew", 3, new MTNewCmd) &&
         cmdMgr->regCmd("MTDelete", 3, new MTDeleteCmd) &&
         cmdMgr->regCmd("MTPrint", 3, new MTPrintCmd)
      )) {
      cerr << "Registering \"mem\" commands fails... exiting" << endl;
      return false;
   }
   return true;
}


//----------------------------------------------------------------------
//    MTReset [(size_t blockSize)]
//----------------------------------------------------------------------
CmdExecStatus
MTResetCmd::exec(const string& option)
{
   // check option
   string token;
   if (!CmdExec::lexSingleOption(option, token))
      return CMD_EXEC_ERROR;
   if (token.size()) {
      int b;
      if (!myStr2Int(token, b) || b < int(sizeof(MemTestObj))) {
         cerr << "Illegal block size (" << token << ")!!" << endl;
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, token);
      }
      #ifdef MEM_MGR_H
      mtest.reset(toSizeT(b));
      #else
      mtest.reset();
      #endif // MEM_MGR_H
   }
   else
      mtest.reset();
   return CMD_EXEC_DONE;
}

void
MTResetCmd::usage(ostream& os) const
{  
   os << "Usage: MTReset [(size_t blockSize)]" << endl;
}

void
MTResetCmd::help() const
{  
   cout << setw(15) << left << "MTReset: " 
        << "(memory test) reset memory manager" << endl;
}


//----------------------------------------------------------------------
//    MTNew <(size_t numObjects)> [-Array (size_t arraySize)]
//----------------------------------------------------------------------
CmdExecStatus
MTNewCmd::exec(const string& option)
{
   // TODO
   vector<string> tokens;
   size_t arraySize = 0;
   size_t nObjs = 0;
   if (!CmdExec::lexOptions(option, tokens, 0))
      return CMD_EXEC_ERROR;
   if(tokens.size() == 0)
   {
      return CmdExec::errorOption(CMD_OPT_MISSING, "");
   }
   vector<string> availParams;
   availParams.push_back("-ARRAY");
   bool shouldntAppear[1] = { false };
   for(unsigned int i=0;i<tokens.size();)
   {
      if(i >= 3) // all parameters are provided, so others are extra
      {
         return CmdExec::errorOption(CMD_OPT_EXTRA, tokens[i]);
      }
      int value = -1;
      int status = parseParam(tokens, availParams, i, value, shouldntAppear);
      switch(status)
      {
      case -1:
         return CmdExec::errorOption(CMD_OPT_MISSING, tokens[i]);
      case -2:
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[i+1]);
      case -4:
         return CmdExec::errorOption(CMD_OPT_EXTRA, tokens[i]);
      case 0:
         if(value >= 1)
         {
            arraySize = value;
            shouldntAppear[0] = true;
            i += 2;
            continue;
         }
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[i+1]);
      case -3:
         stringstream ss(tokens[i]);
         ss >> value;
         if(!ss.bad() && !ss.fail() && ss.eof() && value >= 1)
         {
            if(nObjs >= 1)
            {
               return CmdExec::errorOption(CMD_OPT_EXTRA, tokens[i]);
            }
            nObjs = value;
            i++;
            continue;
         }
         else
         {
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[i]);
         }
      }
   }
   try
   {
      if(arraySize >= 1)
      {
         mtest.newArrs(nObjs, arraySize);
      }
      else
      {
         mtest.newObjs(nObjs);
      }
   }
   catch(std::bad_alloc e)
   {
      return CMD_EXEC_ERROR;
   }
   return CMD_EXEC_DONE;
}

void
MTNewCmd::usage(ostream& os) const
{  
   os << "Usage: MTNew <(size_t numObjects)> [-Array (size_t arraySize)]\n";
}

void
MTNewCmd::help() const
{  
   cout << setw(15) << left << "MTNew: " 
        << "(memory test) new objects" << endl;
}


//----------------------------------------------------------------------
//    MTDelete <-Index (size_t objId) | -Random (size_t numRandId)> [-Array]
//----------------------------------------------------------------------
CmdExecStatus
MTDeleteCmd::exec(const string& option)
{
   // TODO
   vector<string> tokens;
   int objId = -1, numRandId = -1;
   bool isArray = false;
   string optionStr, optionStr2; // for outputs after parsing parameters
   if (!CmdExec::lexOptions(option, tokens, 0))
      return CMD_EXEC_ERROR;
   if(tokens.size() == 0)
   {
      return CmdExec::errorOption(CMD_OPT_MISSING, "");
   }
   vector<string> availParams;
   availParams.push_back("-INDEX");
   availParams.push_back("-RANDOM");
   availParams.push_back("-ARRAY-"); // tailing '-' means no additional parameter required
   bool shouldntAppear[3] = { false, false, false };
   for(size_t i=0;i<tokens.size();)
   {
      int value = -1;
      int status = parseParam(tokens, availParams, i, value, shouldntAppear);
      switch(status)
      {
      case -1:
         return CmdExec::errorOption(CMD_OPT_MISSING, tokens[i]);
      case -2:
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[i+1]);
      case -3:
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[i]);
      case -4:
         return CmdExec::errorOption(CMD_OPT_EXTRA, tokens[i]);
      case 0:
         if(value >= 0)
         {
            objId = value;
            optionStr = tokens[i];
            optionStr2 = tokens[i+1];
            shouldntAppear[0] = shouldntAppear[1] = true;
            i+=2;
            continue;
         }
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[i+1]);
      case 1:
         if(value >= 1)
         {
            numRandId = value;
            optionStr = tokens[i];
            optionStr2 = tokens[i+1];
            shouldntAppear[0] = shouldntAppear[1] = true;
            i+=2;
            continue;
         }
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[i+1]);
      case 2:
         isArray = true;
         shouldntAppear[2] = true;
         i++;
         continue;
      }
   }
   if(objId == -1 && numRandId == -1)
   {
      return CmdExec::errorOption(CMD_OPT_MISSING, "");
   }
   if(objId != -1)
   {
      int size = -1;
      if(isArray)
      {
         size = mtest.getArrListSize();
         if(objId >= size)
         {
            cout << "Size of array list (" << size << ") is <= " << objId << "!!" << endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, optionStr2);
         }
         mtest.deleteArr(objId);
      }
      else
      {
         size = mtest.getObjListSize();
         if(objId >= size)
         {
            cout << "Size of object list (" << size << ") is <= " << objId << "!!" << endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, optionStr2);
         }
         mtest.deleteObj(objId);
      }
   }
   if(numRandId != -1)
   {
      if(isArray && mtest.getArrListSize() == 0)
      {
         cout << "Size of array list is 0!!" << endl;
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, optionStr);
      }
      if(!isArray && mtest.getObjListSize() == 0) // isArray must be false here
      {
         cout << "Size of object list is 0!!" << endl;
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, optionStr);
      }
      for(int i=0;i<numRandId;i++)
      {
         if(isArray)
         {
            mtest.deleteArr(rnGen(mtest.getArrListSize()));
         }
         else
         {
            mtest.deleteObj(rnGen(mtest.getObjListSize()));
         }
      }
   }
   return CMD_EXEC_DONE;
}

void
MTDeleteCmd::usage(ostream& os) const
{  
   os << "Usage: MTDelete <-Index (size_t objId) | "
      << "-Random (size_t numRandId)> [-Array]" << endl;
}

void
MTDeleteCmd::help() const
{  
   cout << setw(15) << left << "MTDelete: " 
        << "(memory test) delete objects" << endl;
}


//----------------------------------------------------------------------
//    MTPrint
//----------------------------------------------------------------------
CmdExecStatus
MTPrintCmd::exec(const string& option)
{
   // check option
   if (option.size())
      return CmdExec::errorOption(CMD_OPT_EXTRA, option);
   mtest.print();

   return CMD_EXEC_DONE;
}

void
MTPrintCmd::usage(ostream& os) const
{  
   os << "Usage: MTPrint" << endl;
}

void
MTPrintCmd::help() const
{  
   cout << setw(15) << left << "MTPrint: " 
        << "(memory test) print memory manager info" << endl;
}


