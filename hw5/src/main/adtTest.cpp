/****************************************************************************
  FileName     [ adtTest.cpp ]
  PackageName  [ main ]
  Synopsis     [ Define AdtTest functions and commands ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-2012 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <cassert>
#include "util.h"
#include "adtTest.h"

// Define global and static variables and functions
//
AdtTest adtTest;

int AdtTestObj::_strLen = 5;

bool
initAdtCmd()
{     
   if (!(cmdMgr->regCmd("ADTReset", 4, new AdtResetCmd) &&
         cmdMgr->regCmd("ADTAdd", 4, new AdtAddCmd) &&
         cmdMgr->regCmd("ADTDelete", 4, new AdtDeleteCmd) &&
         cmdMgr->regCmd("ADTPrint", 4, new AdtPrintCmd)
      )) {
      cerr << "Registering \"adt\" commands fails... exiting" << endl;
      return false;
   }
   return true;
}

//----------------------------------------------------------------------
//    class AdtTestObj member functions
//----------------------------------------------------------------------
AdtTestObj::AdtTestObj()
{
   _str.resize(_strLen);
   for (int i = 0; i < _strLen; ++i)
      _str[i] = 'a' + rnGen(26);
}

ostream& operator << (ostream& os, const AdtTestObj& o)
{
   return (os << o._str);
}

//----------------------------------------------------------------------
//    ADTReset <(size_t strLen)>
//----------------------------------------------------------------------
CmdExecStatus
AdtResetCmd::exec(const string& option)
{
   // check option
   string token;
   if (!CmdExec::lexSingleOption(option, token, false))
      return CMD_EXEC_ERROR;
   int len;
   if (!myStr2Int(token, len) || len <= 0)
      return CmdExec::errorOption(CMD_OPT_ILLEGAL, token);
   adtTest.reset(len);
   return CMD_EXEC_DONE;
}

void
AdtResetCmd::usage(ostream& os) const
{
   os << "Usage: ADTReset <(size_t strLen)>" << endl;
}

void
AdtResetCmd::help() const
{
   cout << setw(15) << left << "ADTReset: " << "(ADT test) reset ADT\n";
}


//----------------------------------------------------------------------
//    ADTAdd <-String (string str) | -Random (size_t repeats)>
//----------------------------------------------------------------------
CmdExecStatus
AdtAddCmd::exec(const string& option)
{
   // check option
   vector<string> options;
   if (!CmdExec::lexOptions(option, options, 2))
      return CMD_EXEC_ERROR;

   // options.size() must = 2
   if (myStrNCmp("-String", options[0], 2) == 0) {
      string str = options[1];
      if (!adtTest.insert(str)) {
         cerr << "Error: \"" << AdtTestObj(str) << "\" already exists!\n";
         return CMD_EXEC_ERROR;
      }
   }
   else if (myStrNCmp("-Random", options[0], 2) == 0) {
      int repeats;
      if (!myStr2Int(options[1], repeats) || repeats <= 0)
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
      for (int i = 0; i < repeats; ++i)
         adtTest.add();
   }
   else
      return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);

   return CMD_EXEC_DONE;
}

void
AdtAddCmd::usage(ostream& os) const
{
   os << "Usage: ADTAdd <-String (string str) | -Random (size_t repeats)>\n";
}

void
AdtAddCmd::help() const
{
   cout << setw(15) << left << "ADTAdd: " << "(ADT test) add objects\n";
}


//----------------------------------------------------------------------
//    ADTDelete < -All | -String (string str) |
//              <<-MINimal | -MAXimal | -Random> (size_t repeats) >>
//----------------------------------------------------------------------
CmdExecStatus
AdtDeleteCmd::exec(const string& option)
{
   // check option
   vector<string> options;
   if (!CmdExec::lexOptions(option, options))
      return CMD_EXEC_ERROR;

   size_t nOpts = options.size();
   if (nOpts == 0)
      return CmdExec::errorOption(CMD_OPT_MISSING, "");
   if (myStrNCmp("-All", options[0], 2) == 0) {
      if (nOpts > 1)
         return CmdExec::errorOption(CMD_OPT_EXTRA,options[1]);
      adtTest.deleteAll();
   }
   else if (myStrNCmp("-String", options[0], 2) == 0) {
      if (nOpts == 1)
         return CmdExec::errorOption(CMD_OPT_MISSING, options[0]);
      if (nOpts > 2)
         return CmdExec::errorOption(CMD_OPT_EXTRA,options[1]);
      if (!adtTest.deleteObj(options[1])) {
         cerr << "Error: \"" << AdtTestObj(options[1]) << "\" is not found!\n";
         return CMD_EXEC_ERROR;
      }
   }
   else {
      enum { DO_MIN, DO_MAX, DO_RAN } delType;
      if (myStrNCmp("-MINimal", options[0], 4) == 0) delType = DO_MIN;
      else if (myStrNCmp("-MAXimal", options[0], 4) == 0) delType = DO_MAX;
      else if (myStrNCmp("-Random", options[0], 2) == 0) delType = DO_RAN;
      else return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
      if (nOpts == 1)
         return CmdExec::errorOption(CMD_OPT_MISSING, options[0]);
      int iOpt;
      if (!myStr2Int(options[1], iOpt) || iOpt <= 0)
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
      if (nOpts > 2)
         return CmdExec::errorOption(CMD_OPT_EXTRA,options[1]);
      switch (delType) {
         case DO_MIN: adtTest.deleteFront(iOpt); break;
         case DO_MAX: adtTest.deleteBack(iOpt); break;
         case DO_RAN: adtTest.deleteRandom(iOpt); break;
      }
   }
   return CMD_EXEC_DONE;
}

void
AdtDeleteCmd::usage(ostream& os) const
{
   os << "ADTDelete < -All | -String (string str) |" << endl
      << "          <<-MINimal | -MAXimal | -Random> (size_t repeats) >>\n";
}

void
AdtDeleteCmd::help() const
{
   cout << setw(15) << left << "ADTDelete: " << "(ADT test) delete objects\n";
}


//----------------------------------------------------------------------
//    ADTPrint [-Reversed]
//----------------------------------------------------------------------
CmdExecStatus
AdtPrintCmd::exec(const string& option)
{  
   // check option
   vector<string> options;
   if (!CmdExec::lexOptions(option, options))
      return CMD_EXEC_ERROR;
   bool reversed = false, verbose = false;
   unsigned nToken = options.size();
   if (nToken > 2)
      return CmdExec::errorOption(CMD_OPT_EXTRA, options[2]);
   for (unsigned i = 0; i < nToken; ++i) {
      if (myStrNCmp("-Reversed", options[i], 2) == 0)
         reversed = true;
      #ifdef TEST_BST
      else if (myStrNCmp("-Verbose", options[i], 2) == 0)
         verbose = true;
      #endif // TEST_BST
      else
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[i]);
   }

   adtTest.print(reversed, verbose);
   return CMD_EXEC_DONE;
}

void
AdtPrintCmd::usage(ostream& os) const
{
   os << "Usage: ADTPrint [-Reversed]" << endl;
}

void
AdtPrintCmd::help() const
{
   cout << setw(15) << left << "ADTPrint: " << "(ADT test) print ADT\n";
}
