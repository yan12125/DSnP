/****************************************************************************
  FileName     [ adtTest.h ]
  PackageName  [ main ]
  Synopsis     [ Define AdtTest class and commands ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-2012 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#ifndef ADT_TEST_H
#define ADT_TEST_H

#include <iostream>
#include <string>
#include "cmdParser.h"


using namespace std;

//----------------------------------------------------------------------
//    Type define for linear ADT test program
//----------------------------------------------------------------------

#if defined  TEST_DLIST

      #undef   TEST_ARRAY
      #undef   TEST_BST
      #undef   RANDOM_ACCESS

      #define  ADT         "dlist"
      #define  AdtType     DList

      #include "dlist.h"

#elif defined  TEST_ARRAY

      #undef   TEST_DLIST
      #undef   TEST_BST
      #define  RANDOM_ACCESS

      #define  ADT         "array"
      #define  AdtType     Array

      #include "array.h"

#elif defined  TEST_BST

      #undef   TEST_DLIST
      #undef   TEST_ARRAY
      #undef   RANDOM_ACCESS

      #define  ADT         "bst"
      #define  AdtType     BSTree

      #include "bst.h"

#endif // TEST_DLIST


//----------------------------------------------------------------------
//    Classes for ADT test program
//----------------------------------------------------------------------
class AdtTestObj
{
public:
   AdtTestObj();
   AdtTestObj(const string& s)
   : _str(s) { if (int(_str.size()) > _strLen) _str.resize(_strLen); }
   AdtTestObj(const AdtTestObj& o) : _str(o._str) {}

   bool operator < (const AdtTestObj& o) const { return (_str < o._str); }
   bool operator == (const AdtTestObj& o) const { return (_str == o._str); }

   static void setLen(int len) { _strLen = len; }

   friend ostream& operator << (ostream& os, const AdtTestObj& o);

private:
   string      _str;  // _data should alywas be between [0, _dataRange - 1]
   static int  _strLen;
};

class AdtTest
{
public:
   void reset(int len) { deleteAll(); AdtTestObj::setLen(len); }

   bool add() { return insert(AdtTestObj()); }  // insert random number
   bool insert(const AdtTestObj& o) { return _container.insert(o); }

   void deleteAll() { _container.clear(); }
   bool deleteObj(const AdtTestObj& o) { return _container.erase(o); }
   void deleteFront(size_t repeat = 1) {
      for (size_t i = 0; i < repeat; ++i) _container.pop_front(); }
   void deleteBack(size_t repeat = 1) {
      for (size_t i = 0; i < repeat; ++i) _container.pop_back(); }
   void deleteRandom(size_t repeat = 1) {
      size_t s = _container.size();
      for (size_t i = 0; i < repeat; ++i) {
         size_t pos = rnGen(s);
         if (_container.erase(getPos(pos))) --s;
      }
   }

   void print(bool reverse = false, bool verbose = false) {
      #ifdef TEST_BST
      if (verbose)
         _container.print();  // for BST only
      #endif
      cout << "=== ADT (" << ADT << ") ===" << endl;
      if (reverse) printBackward();
      else printForward();
      cout << endl;
   }

private:
   AdtType<AdtTestObj>   _container;

   // private functions
   // return end() if 'pos' passes the end
   AdtType<AdtTestObj>::iterator getPos(size_t pos) {

      #ifdef RANDOM_ACCESS
         if (pos >= _container.size()) return _container.end();
         return (_container.begin() + pos);
      #else
         size_t i = 0;
         AdtType<AdtTestObj>::iterator li = _container.begin();
         AdtType<AdtTestObj>::iterator lj = _container.end();
         while ((li != lj) && (i++ != pos)) ++li;
         return li;
      #endif // RANDOM_ACCESS
   }
   void printForward() {
      size_t idx = 0;
      AdtType<AdtTestObj>::iterator li = _container.begin();
      for (; li != _container.end(); ++li)
         printData(idx++, li, 4);
   }
   void printBackward() {
      if (_container.empty()) return;
      size_t idx = _container.size() - 1;
      size_t r = (idx + 1) % 5;
      AdtType<AdtTestObj>::iterator li = _container.end(); --li;
      #ifdef RANDOM_ACCESS
         for (int i = idx; i >= 0; --i)
            printData(idx--, li--, r);
      #else // for DList and BST
         do {
            printData(idx, li, r);
            --idx;
         } while (li-- != _container.begin());
      #endif // RANDOM_ACCESS
   }
   void printData(size_t idx, AdtType<AdtTestObj>::iterator li, size_t r) {
      cout << "[" << setw(3) << right << idx << "] = "
           << setw(3) << right << *li << "   ";
      if (idx % 5 == r) cout << endl;
   }
};


//----------------------------------------------------------------------
//    Classes for linear ADT test commands
//----------------------------------------------------------------------
CmdClass(AdtResetCmd);
CmdClass(AdtAddCmd);
CmdClass(AdtDeleteCmd);
CmdClass(AdtPrintCmd);

#endif // ADT_TEST_H
