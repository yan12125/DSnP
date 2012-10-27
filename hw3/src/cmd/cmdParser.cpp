/****************************************************************************
  FileName     [ cmdParser.cpp ]
  PackageName  [ cmd ]
  Synopsis     [ Define command parsing member functions for class CmdParser ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-2012 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <cassert>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <algorithm>
#include <sstream>
#include "util.h"
#include "cmdParser.h"

using namespace std;

//----------------------------------------------------------------------
//    Global cmd Manager
//----------------------------------------------------------------------
CmdParser* cmdMgr = new CmdParser("mcalc> ");


//----------------------------------------------------------------------
//    Extrenal funcitons
//----------------------------------------------------------------------
void mybeep();


//----------------------------------------------------------------------
//    Member Function for class cmdParser
//----------------------------------------------------------------------
// return false if file cannot be opened
// Please refer to the comments in "DofileCmd::exec", cmdCommon.cpp
bool
CmdParser::openDofile(const string& dof)
{
   // TODO...
   if(_dofileStack.size()>=1021) // 1021 is from the result of ref/modCalc-64
   {
      return false;
   }
   _dofileStack.push(_dofile);
   _dofile = new ifstream(dof.c_str());
   if(_dofile->fail())
   {
      delete _dofile;
      _dofile = NULL;
      _dofile = _dofileStack.top();
      _dofileStack.pop();
      return false;
   }
   return _dofile;
}

// Must make sure _dofile != 0
void
CmdParser::closeDofile()
{
   // TODO...
   if(_dofile!=NULL)
   {
      if(_dofile->is_open())
      {
         _dofile->close();
         delete _dofile;
         _dofile = NULL; // execOneCmd determine whether a file is open by this pointer
         _dofile = _dofileStack.top();
         _dofileStack.pop();
      }
   }
}

// Return false if registration fails
bool
CmdParser::regCmd(const string& cmd, unsigned nCmp, CmdExec* e)
{
   // Make sure cmd hasn't been registered and won't cause ambiguity
   string str = cmd;
   unsigned s = str.size();
   if (s < nCmp) return false;
   while (true) {
      if (getCmd(str)) return false;
      if (s == nCmp) break;
      str.resize(--s);
   }

   // Change the first nCmp characters to upper case to facilitate
   //    case-insensitive comparison later.
   // The strings stored in _cmdMap are all upper case
   //
   assert(str.size() == nCmp);  // str is now mandCmd
   string& mandCmd = str;
   for (unsigned i = 0; i < nCmp; ++i)
      mandCmd[i] = toupper(mandCmd[i]);
   string optCmd = cmd.substr(nCmp);
   assert(e != 0);
   e->setOptCmd(optCmd);

   // insert (mandCmd, e) to _cmdMap; return false if insertion fails.
   return (_cmdMap.insert(CmdRegPair(mandCmd, e))).second;
}

// Return false on "quit" or if excetion happens
CmdExecStatus
CmdParser::execOneCmd()
{
   bool newCmd = false;
   if (_dofile != 0)
      newCmd = readCmd(*_dofile);
   else
      newCmd = readCmd(cin);

   // execute the command
   if (newCmd) {
      string option;
      CmdExec* e = parseCmd(option);
      if (e != 0)
         return e->exec(option);
   }
   return CMD_EXEC_NOP;
}

// For each CmdExec* in _cmdMap, call its "help()" to print out the help msg.
// Print an endl at the end.
void
CmdParser::printHelps() const
{
   // TODO...
   for(CmdMap::const_iterator it=_cmdMap.begin();it!=_cmdMap.end();it++)
   {
      it->second->help();
   }
}

void
CmdParser::printHistory(int nPrint) const
{
   assert(_tempCmdStored == false);
   if (_history.empty()) {
      cout << "Empty command history!!" << endl;
      return;
   }
   int s = _history.size();
   if ((nPrint < 0) || (nPrint > s))
      nPrint = s;
   for (int i = s - nPrint; i < s; ++i)
      cout << "   " << i << ": " << _history[i] << endl;
}


//
// Parse the command from _history.back();
// Let string str = _history.back();
//
// 1. Read the command string (may contain multiple words) from the leading
//    part of str (i.e. the first word) and retrive the corresponding
//    CmdExec* from _cmdMap
//    ==> If command not found, print to cerr the following message:
//        Illegal command!! "(string cmdName)"
//    ==> return it at the end.
// 2. Call getCmd(cmd) to retrieve command from _cmdMap.
//    "cmd" is the first word of "str".
// 3. Get the command options from the trailing part of str (i.e. second
//    words and beyond) and store them in "option"
//
CmdExec*
CmdParser::parseCmd(string& option)
{
   assert(_tempCmdStored == false);
   assert(!_history.empty());
   string str = _history.back();

   // TODO...
   assert(str[0] != 0 && str[0] != ' ');
   CmdExec* e=NULL;
   stringstream iss(str);
   string cmd;
   iss >> cmd;
   e=getCmd(cmd);
   if(e==NULL)
   {
      cout << "Illegal command!! (" << cmd << ")" << endl;
   }
   else
   {
      // get the remaining part
      // http://www.velocityreviews.com/forums/t639320-simple-stringstream-question.html
      if(static_cast<unsigned long>(iss.tellg())>=str.size()) // no options
      {
         option = "";
      }
      else
      {
         option = iss.str().substr(iss.tellg());
      }
      return e;
   }
   return NULL;
}

// This function is called by pressing 'Tab'.
// It is to list the partially matched commands.
// "str" is the partial string before current cursor position. It can be 
// a null string, or begin with ' '. The beginning ' ' will be ignored.
//
// Several possibilities after pressing 'Tab'
// (Let $ be the cursor position)
// 1. [Before] Null cmd
//    cmd> $
//    -----------
//    [Before] Cmd with ' ' only
//    cmd>     $
//    [After Tab]
//    ==> List all the commands, each command is printed out by:
//           cout << setw(12) << left << cmd;
//    ==> Print a new line for every 5 commands
//    ==> After printing, re-print the prompt and place the cursor back to
//        original location (including ' ')
//
// 2. [Before] partially matched (multiple matches)
//    cmd> h$                   // partially matched
//    [After Tab]
//    HELp        HIStory       // List all the parially matched commands
//    cmd> h$                   // and then re-print the partial command
//    -----------
//    [Before] partially matched (multiple matches)
//    cmd> h$aaa                // partially matched with trailing characters
//    [After Tab]
//    HELp        HIStory       // List all the parially matched commands
//    cmd> h$aaa                // and then re-print the partial command
//
// 3. [Before] partially matched (single match)
//    cmd> he$
//    [After Tab]
//    cmd> heLp $
//    -----------
//    [Before] partially matched with trailing characters (single match)
//    cmd> he$hahah
//    [After Tab]
//    cmd> heLp $hahaha
//    ==> Automatically complete on the same line
//    ==> The auto-expanded part follow the strings stored in cmd map and
//        cmd->_optCmd. Insert a space after "heLp"
//
// 4. [Before] No match
//    cmd> hek$
//    [After Tab]
//    ==> Beep and stay in the same location
//
// 5. [Before] Already matched
//    cmd> help asd$fg
//    [After] Print out the usage for the already matched command
//    Usage: HELp [(string cmd)]
//    cmd> help asd$fg
//
// 6. [Before] Cursor NOT on the first word and NOT matched command
//    cmd> he haha$kk
//    [After Tab]
//    ==> Beep and stay in the same location
//
void
CmdParser::listCmd(const string& str)
{
   // TODO...
   string keyword; // string before cursor
   keyword = _readBuf;
   transform(keyword.begin(), keyword.end(), keyword.begin(), ::toupper);
   keyword = keyword.substr(0, _readBufPtr - _readBuf); // extract part before cursor
   size_t pos = keyword.find_first_not_of(' ');
   if(pos==string::npos) // blank
   {
      int i = 0;
      cout << endl;
      for(CmdMap::iterator it = _cmdMap.begin();it != _cmdMap.end();it++)
      {
         // manipulator only affect one output
         cout << setw(12) << left << string(it->first)+string(it->second->getOptCmd());
         if(i%5==4)
         {
            cout << endl;
         }
         i++;
      }
      reprintCmd();
      return; // for reducing indent level
   }

   size_t pos2 = keyword.find_first_of(' '); // first word is in interval [ pos, pos2 ]
   keyword = keyword.substr(pos, pos2 - pos); // extract first word

   if(pos2 >= static_cast<unsigned int>(_readBufPtr-_readBuf)) // at first word
   {
      // start to find all matches
      vector<string> matchedCmds;
      for(CmdMap::iterator it = _cmdMap.begin();it!=_cmdMap.end();it++)
      {
         bool match = false;
         if(it->first.find(keyword)==0)  // match if mnemonic code starts with keyword
         {
            match = true;
         }
         if(keyword.find(it->first)==0)
         {
            if(it->second->checkOptCmd(keyword.substr(it->first.size())))
            {
               // or ( keyword starts with it->first and optional cmd matches )
               match = true;
            }
         }
         if(match)
         {
            matchedCmds.push_back(string(it->first)+string(it->second->getOptCmd()));
         }
      }
      if(matchedCmds.size()==0)
      {
         mybeep();
         return;
      }
      else if(matchedCmds.size()==1)
      {
         // print the remaining part
         for(size_t ptr = keyword.size();ptr!=matchedCmds[0].size();ptr++)
         {
            insertChar(matchedCmds[0][ptr]);
         }
         insertChar(' '); // for next argument
      }
      else // multiple matches
      {
         int i=0;
         cout << endl;
         for(vector<string>::iterator it = matchedCmds.begin();it!=matchedCmds.end();it++)
         {
            cout << setw(12) << left << *it;
            if(i%5==4)
            {
               cout << endl;
            }
            i++;
         }
         reprintCmd();
      }
   }
   else
   {
      CmdExec* e;
      if((e=getCmd(keyword))!=NULL) // first word match a command
      {
         cout << endl;
         e->help();
         reprintCmd();
      }
      else // first word isn't a command
      {
         mybeep();
      }
   }
}

// cmd is a copy of the original input
//
// return the corresponding CmdExec* if "cmd" matches any command in _cmdMap
// return 0 if not found.
//
// Please note:
// ------------
// 1. The mandatory part of the command string (stored in _cmdMap) must match
// 2. The optional part can be partially omitted.
//    ==> Checked by the CmdExec::checkOptCmd(const string&) function
// 3. All string comparison are "case-insensitive".
//
CmdExec*
CmdParser::getCmd(string cmd)
{
   CmdExec* e = 0;
   // TODO...
   // call CmdExec::checkOptCmd(const string&) if needed...
   // http://stackoverflow.com/questions/735204/convert-a-string-in-c-to-upper-case
   transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
   for(CmdMap::iterator it=_cmdMap.begin();it!=_cmdMap.end();it++)
   {
       // cmd starts with it->first
      if(cmd.find(it->first)==0 && it->second->checkOptCmd(cmd.substr(it->first.size())))
      {
         e = it->second;
         break;
      }
   }
   return e;
}


//----------------------------------------------------------------------
//    Member Function for class CmdExec
//----------------------------------------------------------------------
// Return false if error options found
// "optional" = true if the option is optional XD
// "optional": default = true
//
bool
CmdExec::lexSingleOption
(const string& option, string& token, bool optional) const
{
   size_t n = myStrGetTok(option, token);
   if (!optional) {
      if (token.size() == 0) {
         errorOption(CMD_OPT_MISSING, "");
         return false;
      }
   }
   if (n != string::npos) {
      errorOption(CMD_OPT_EXTRA, option.substr(n));
      return false;
   }
   return true;
}

// if nOpts is specified (!= 0), the number of tokens must be exactly = nOpts
// Otherwise, return false.
//
bool
CmdExec::lexOptions
(const string& option, vector<string>& tokens, size_t nOpts) const
{
   string token;
   size_t n = myStrGetTok(option, token);
   while (token.size()) {
      tokens.push_back(token);
      n = myStrGetTok(option, token, n);
   }
   if (nOpts != 0) {
      if (tokens.size() < nOpts) {
         errorOption(CMD_OPT_MISSING, "");
         return false;
      }
      if (tokens.size() > nOpts) {
         errorOption(CMD_OPT_EXTRA, tokens[nOpts]);
         return false;
      }
   }
   return true;
}

CmdExecStatus
CmdExec::errorOption(CmdOptionError err, const string& opt) const
{
   switch (err) {
      case CMD_OPT_MISSING:
         cerr << "Missing option";
         if (opt.size()) cerr << " after (" << opt << ")";
         cerr << "!!" << endl;
      break;
      case CMD_OPT_EXTRA:
         cerr << "Extra option!! (" << opt << ")" << endl;
      break;
      case CMD_OPT_ILLEGAL:
         cerr << "Illegal option!! (" << opt << ")" << endl;
      break;
      case CMD_OPT_FOPEN_FAIL:
         cerr << "Error: cannot open file \"" << opt << "\"!!" << endl;
      break;
      default:
         cerr << "Unknown option error type!! (" << err << ")" << endl;
      exit(-1);
   }
   return CMD_EXEC_ERROR;
}

// Called by "getCmd()"
// Check if "check" is a matched substring of "_optCmd"...
// if not, return false.
// 
// Perform case-insensitive checks
//
bool
CmdExec::checkOptCmd(const string& check) const
{
   // TODO...
   string upperCheck(check); // transform wouldn't allocate memory
   string upperOptCmd(_optCmd);
   transform(check.begin(), check.end(), upperCheck.begin(), ::toupper);
   transform(_optCmd.begin(), _optCmd.end(), upperOptCmd.begin(), ::toupper);
   if(upperOptCmd.find(upperCheck)==0) // check must be the first few chars in _optCmd
   {
      return true;
   }
   else
   {
      return false;
   }
}
