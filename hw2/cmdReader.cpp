/****************************************************************************
  FileName     [ cmdReader.cpp ]
  PackageName  [ cmd ]
  Synopsis     [ Define command line reader member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-2012 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <cassert>
#include <cstring>
#include "cmdParser.h"

using namespace std;

//----------------------------------------------------------------------
//    Extrenal funcitons
//----------------------------------------------------------------------
void mybeep();
char mygetc(istream&);
ParseChar checkChar(char, istream&);


//----------------------------------------------------------------------
//    Member Function for class Parser
//----------------------------------------------------------------------
void
CmdParser::readCmd()
{
   if (_dofile.is_open()) {
      readCmdInt(_dofile);
      _dofile.close();
   }
   else
      readCmdInt(cin);
}

void
CmdParser::readCmdInt(istream& istr)
{
   resetBufAndPrintPrompt();

   while (1) {
      char ch = mygetc(istr);
      ParseChar pch = checkChar(ch, istr);
      if (pch == INPUT_END_KEY) break;
      switch (pch) {
         case LINE_BEGIN_KEY :
         case HOME_KEY       : moveBufPtr(_readBuf); break;
         case LINE_END_KEY   :
         case END_KEY        : moveBufPtr(_readBufEnd); break;
         case BACK_SPACE_KEY : /* TODO */ if(_readBufPtr != _readBuf){ moveBufPtr(_readBufPtr-1); deleteChar(); } break;
         case DELETE_KEY     : deleteChar(); break;
         case NEWLINE_KEY    : addHistory();
                               cout << char(NEWLINE_KEY);
                               printPrompt(); break;
         case ARROW_UP_KEY   : moveToHistory(_historyIdx - 1); break;
         case ARROW_DOWN_KEY : moveToHistory(_historyIdx + 1); break;
         case ARROW_RIGHT_KEY: /* TODO */ moveBufPtr(_readBufPtr+1); break;
         case ARROW_LEFT_KEY : /* TODO */ moveBufPtr(_readBufPtr-1); break;
         case PG_UP_KEY      : moveToHistory(_historyIdx - PG_OFFSET); break;
         case PG_DOWN_KEY    : moveToHistory(_historyIdx + PG_OFFSET); break;
         case TAB_KEY        : /* TODO */ break;
         case INSERT_KEY     : // not yet supported; fall through to UNDEFINE
         case UNDEFINED_KEY:   mybeep(); break;
         default:  // printable character
            insertChar(char(pch)); break;
      }
      #ifdef TA_KB_SETTING
      taTestOnly();
      #endif
   }
}


// This function moves _readBufPtr to the "ptr" pointer
// It is used by left/right arrowkeys, home/end, etc.
//
// Suggested steps:
// 1. Make sure ptr is within [_readBuf, _readBufEnd].
//    If not, make a beep sound and return false. (DON'T MOVE)
// 2. Move the cursor to the left or right, depending on ptr
// 3. Update _readBufPtr accordingly. The content of the _readBuf[] will
//    not be changed
//
// [Note] This function can also be called by other member functions below
//        to move the _readBufPtr to proper position.
bool
CmdParser::moveBufPtr(char* const ptr)
{
   // TODO...
   if(ptr > _readBufEnd || ptr < _readBuf)
   {
      mybeep();
      return false;
   }
   _readBufPtr = ptr;
   // update output
   cout << "\e[0G"; // move to first char of the current line
   printPrompt();
   cout << _readBuf;
   cout << "\e[0K"; // clear after current cursor
   cout << "\e[" << strlen("cmd> ")+ _readBufPtr - _readBuf + 1 << "G"; // move cursor to specified location
   cout.flush();
   return true;
}


// [Notes]
// 1. Delete the char at _readBufPtr
// 2. mybeep() and return false if at _readBufEnd
// 3. Move the remaining string left for one character
// 4. The cursor should stay at the same position
// 5. Remember to update _readBufEnd accordingly.
// 6. Don't leave the tailing character.
// 7. Call "moveBufPtr(...)" if needed.
//
// For example,
//
// cmd> This is the command
//              ^                (^ is the cursor position)
//
// After calling deleteChar()---
//
// cmd> This is he command
//              ^
//
bool
CmdParser::deleteChar()
{
   // TODO...
   assert(*_readBufEnd == '\0');
   if(*_readBuf == '\0' || _readBufPtr == _readBuf + strlen(_readBuf)) // nothing to delete
   {
      return false;
   }
   for(char* pos = _readBuf;pos <= _readBufEnd;pos++)
   {
      if(pos >= _readBufPtr)
      {
         *pos = *(pos+1);
      }
   }
   *_readBufEnd = '\0';
   _readBufEnd--;
   moveBufPtr(_readBufPtr); // just update the string
   return true;
}

// 1. Insert character 'ch' at _readBufPtr
// 2. Move the remaining string right for one character
// 3. The cursor should move right for one position afterwards
//
// For example,
//
// cmd> This is the command
//              ^                (^ is the cursor position)
//
// After calling insertChar('k') ---
//
// cmd> This is kthe command
//               ^
//
void
CmdParser::insertChar(char ch, int rep)
{
   // TODO...
   if(_readBufEnd == _readBuf + READ_BUF_SIZE -1 ) // no more space to insert characters
   {
      mybeep();
      return;
   }
   for(char* pos = _readBufEnd+1;pos >= _readBufPtr+1;pos--)
   {
      *pos = *(pos-1);
   }
   *_readBufPtr = ch;
   _readBufEnd++;
   moveBufPtr(_readBufPtr+1);
}

// 1. Delete the line that is currently shown on the screen
// 2. Reset _readBufPtr and _readBufEnd to _readBuf
// 3. Make sure *_readBufEnd = 0
//
// For example,
//
// cmd> This is the command
//              ^                (^ is the cursor position)
//
// After calling deleteLine() ---
//
// cmd>
//      ^
//
void
CmdParser::deleteLine()
{
   // TODO...
   strcpy(_readBuf, "");
   _readBufPtr = _readBufEnd = _readBuf;
   moveBufPtr(_readBuf);
}


// This functions moves _historyIdx to index and display _history[index]
// on the screen.
//
// Need to consider:
// If moving up... (i.e. index < _historyIdx)
// 1. If already at top (i.e. _historyIdx == 0), beep and do nothing.
// 2. If at bottom, temporarily record _readBuf to history.
//    (Do not remove spaces, and set _tempCmdStored to "true")
// 3. If index < 0, let index = 0.
//
// If moving down... (i.e. index > _historyIdx)
// 1. If already at bottom, beep and do nothing
// 2. If index >= _history.size(), let index = _history.size() - 1.
//
// Assign _historyIdx to index at the end.
//
// [Note] index should not = _historyIdx
//
void
CmdParser::moveToHistory(int index)
{
   // TODO...
   assert(index!=_historyIdx);
   if(_history.size()==0)
   {
      mybeep();
      return;
   }
   if((unsigned int) _historyIdx == _history.size() &&
      (unsigned int) index < _history.size() && 
      !_tempCmdStored) 
   {
      _tempCmdStored = true;
      _history.push_back(_readBuf);
   }
   if((unsigned int) _historyIdx+1 == _history.size() && 
      (unsigned int) index >= _history.size() && 
      _tempCmdStored)
   {
      strcpy(_readBuf, "");
      _readBufPtr = _readBufEnd = _readBuf;
      moveBufPtr(_readBufEnd);
      _historyIdx++;
      return;
   }
   if((unsigned int) _historyIdx == _history.size() && 
      (unsigned int) index >= _history.size())
   {  
      // already at bottom
      mybeep();
      return;
   }
   if(_historyIdx == 0 && index < 0)
   {
      // already at top
      mybeep();
      return;
   }
   if((unsigned int)index > _history.size())
   {
      index = _history.size();
   }
   if(index < 0 )
   {
      index = 0;
   }
   strcpy(_readBuf, _history[index].c_str());
   _historyIdx = index;
   _readBufEnd = _readBuf + strlen(_readBuf);
   moveBufPtr(_readBufEnd);
   cout << "\e[s\e[40G[ ";
   for(unsigned int i=0;i<_history.size();i++)
   {
      cout << "\"" << _history[i] << "\"";
      if(i!=_history.size()-1)
      {
         cout << ", ";
      }
      cout.flush();
   }
   cout << " ]\e[u";
}


// This function adds the string in _readBuf to the _history.
// The size of _history may or may not change. Depending on whether 
// there is a temp history string.
//
// 1. Remove ' ' at the beginning and end of _readBuf
// 2. If not a null string, add string to _history.
//    Be sure you are adding to the right entry of _history.
// 3. If it is a null string, don't add anything to _history.
// 4. Make sure to clean up "temp recorded string" (added earlier by up/pgUp,
//    and reset _tempCmdStored to false
// 5. Reset _historyIdx to _history.size() // for future insertion
// 6. Reset _readBufPtr and _readBufEnd to _readBuf
// 7. Make sure *_readBufEnd = 0 ==> _readBuf becomes null string
//
void
CmdParser::addHistory()
{
   // TODO...
   char trimmedStr[READ_BUF_SIZE] = "";
   char *pos1 = NULL, *pos2 = NULL;
   // pos1 will become the pointer to first non blank char, 
   // and pos2 will become the pointer to last non blank char

   for(pos1 = _readBuf;pos1 <= _readBufEnd-1 && *pos1==' ';pos1++){}
   if(pos1 == _readBufEnd) // empty string
   {
      return;
   }
   if(_tempCmdStored)
   {
      _history.pop_back();
   }
   for(pos2 = _readBufEnd-1;pos2 >= pos1 && *pos2==' ';pos2--){}
   for(char* pos = pos1;pos <= pos2;pos++)
   {
      trimmedStr[pos-pos1] = *pos;
   }
   *(pos2+1) = '\0';
   cout << endl << trimmedStr;
   _history.push_back(trimmedStr);
   _historyIdx = _history.size();
   *_readBuf = '\0';
   _readBufEnd = _readBufPtr = _readBuf;
   return;
}


// 1. Replace current line with _history[_historyIdx] on the screen
// 2. Set _readBufPtr and _readBufEnd to end of line
//
// [Note] Do not change _history.size().
//
void
CmdParser::retrieveHistory()
{
   deleteLine();
   strcpy(_readBuf, _history[_historyIdx].c_str());
   cout << _readBuf;
   _readBufPtr = _readBufEnd = _readBuf + _history[_historyIdx].size();
}
