/****************************************************************************
  FileName     [ calcCmd.cpp ]
  PackageName  [ calc ]
  Synopsis     [ Define modular calculator commands ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-2012 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <iomanip>
#include "util.h"
#include "calcCmd.h"
#include "calcModNum.h"

bool
initCalcCmd()
{
   // TODO...
   return true;
}

//----------------------------------------------------------------------
//    MSET <(int modulus)>
//----------------------------------------------------------------------
CmdExecStatus
MsetCmd::exec(const string& option)
{
   // check option
   string token;
   if (!CmdExec::lexSingleOption(option, token, false))
      return CMD_EXEC_ERROR;

   int m;
   if (!myStr2Int(token, m) || (m <= 0))
      return CmdExec::errorOption(CMD_OPT_ILLEGAL, token);

   if (ModNum::getModulus() != m) {
      ModNum::setModulus(m);
      // modulus changed; all the numbers need to be reset
      ModNum::resetVapMap();
   }

   return CMD_EXEC_DONE;
}

void
MsetCmd::usage(ostream& os) const
{
   os << "Usage: MSET <(int modulus)>" << endl;
}

void
MsetCmd::help() const
{
   cout << setw(15) << left << "MSET: "
        << "set the modulus of the modular number calculator" << endl;
}


//----------------------------------------------------------------------
//    MVARiable <(string var)> <(string var) | (int val)>
//----------------------------------------------------------------------
CmdExecStatus
MvarCmd::exec(const string& option)
{
   // check option
   vector<string> options;
   if (!CmdExec::lexOptions(option, options, 2))
      return CMD_EXEC_ERROR;

   // check option 1
   if (!isValidVarName(options[0]))
      return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
   // check option 2
   ModNum v;
   if (!ModNum::getStrVal(options[1], v))
      return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);

   ModNum::setVarVal(options[0], v);
   cout << options[0] << " = " << v << endl;

   return CMD_EXEC_DONE;
}

void
MvarCmd::usage(ostream& os) const
{
   os << "Usage: MVARiable <(string var)> <(string var) | (int val)>"
      << endl;
}

void
MvarCmd::help() const
{
   cout << setw(15) << left << "MVARiable: "
        << "set the variable value of the modular number calculator"
        << endl;
}


//----------------------------------------------------------------------
//    MADD <(string y)> <(string a) | (int va)> <(string b) | (int vb)>
//----------------------------------------------------------------------
CmdExecStatus
MaddCmd::exec(const string& option)
{
   // TODO...

   return CMD_EXEC_DONE;
}

void
MaddCmd::usage(ostream& os) const
{
   os << "Usage: MADD "
      << "<(string y)> <(string a) | (int va)> <(string b) | (int vb)>"
      << endl;
}

void
MaddCmd::help() const
{
   cout << setw(15) << left << "MADD: "
        << "perform modular number addition" << endl;
}


//----------------------------------------------------------------------
//    MSUBtract <(string y)> <(string a) | (int va)> <(string b) | (int vb)>
//----------------------------------------------------------------------
CmdExecStatus
MsubCmd::exec(const string& option)
{
   // TODO...

   return CMD_EXEC_DONE;
}

void
MsubCmd::usage(ostream& os) const
{
   os << "Usage: MSUBtract "
      << "<(string y)> <(string a) | (int va)> <(string b) | (int vb)>"
      << endl;
}

void
MsubCmd::help() const
{
   cout << setw(15) << left << "MSUBtract: "
        << "perform modular number subtraction" << endl;
}


//----------------------------------------------------------------------
//    MMULTiply <(string y)> <(string a) | (int va)> <(string b) | (int vb)>
//----------------------------------------------------------------------
CmdExecStatus
MmultCmd::exec(const string& option)
{
   // TODO...
   
   return CMD_EXEC_DONE;
}

void
MmultCmd::usage(ostream& os) const
{
   os << "Usage: MMULTiply "
      << "<(string y)> <(string a) | (int va)> <(string b) | (int vb)>"
      << endl;
}

void
MmultCmd::help() const
{
   cout << setw(15) << left << "MMULTiply: "
        << "perform modular number multiplication" << endl;
}


//----------------------------------------------------------------------
//    MCOMPare <(string var1) | (int val1)> <(string var2) | (int val2)>
//----------------------------------------------------------------------
CmdExecStatus
McmpCmd::exec(const string& option)
{
   // TODO...

   return CMD_EXEC_DONE;
}

void
McmpCmd::usage(ostream& os) const
{
   os << "Usage: MCOMPare "
      << "<(string var1) | (int val1)> <(string var2) | (int val2)>"
      << endl;
}

void
McmpCmd::help() const
{
   cout << setw(15) << left << "MCOMPare: "
        << "compare if two variables or values are equal"
        << endl;
}


//----------------------------------------------------------------------
//    MPrint [(string var)...]
//----------------------------------------------------------------------
CmdExecStatus
MprintCmd::exec(const string& option)
{
   vector<string> options;
   CmdExec::lexOptions(option, options);
   size_t n = options.size();
   if (n) {
      for (size_t i = 0; i < n; ++i) {
         ModNum val;
         if (isValidVarName(options[i]) && ModNum::getVarVal(options[i], val))
            cout << options[i] << " = " << val << endl;
         else
            CmdExec::errorOption(CMD_OPT_ILLEGAL, options[i]);
      }
   }
   else
      ModNum::printVars();
   return CMD_EXEC_DONE;
}

void
MprintCmd::usage(ostream& os) const
{
   os << "Usage: MPrint [(string var)...]" << endl;
}

void
MprintCmd::help() const
{
   cout << setw(15) << left << "MPrint: "
        << "print the variables of the modular number calculator"
        << endl;
}


