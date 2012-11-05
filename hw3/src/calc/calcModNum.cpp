/****************************************************************************
  FileName     [ calcModNum.cpp ]
  PackageName  [ calc ]
  Synopsis     [ Define member functions for class ModNum ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-2012 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <sstream>
#include "util.h"
#include "calcModNum.h"

// TODO: Initialize the static data members of class ModNum
//       (Note: let default _modulus = 100000000)
// TODO: Define the member functions of class ModNum
int ModNum::_modulus = 100000000;
CalcMap ModNum::_varMap;

ostream& operator << (ostream& os, const ModNum& n)
{
   os << n._num;
   return os;
}

ModNum::ModNum(int i)
{
   _num = i % _modulus;
   if(_num<0) // in c++, negative gives out negative remaining
   {
      _num += _modulus;
   }
}

ModNum ModNum::operator + (const ModNum& n) const 
{
   return ModNum(this->_num + n._num);
}

ModNum& ModNum::operator += (const ModNum& n) 
{
   this->_num = (this->_num + n._num)%_modulus;
   if(this->_num<0)
   {
      this->_num += _modulus;
   }
   return *this;
}

ModNum ModNum::operator - (const ModNum& n) const 
{
   return ModNum(this->_num - n._num);
}

ModNum& ModNum::operator -= (const ModNum& n) 
{
   this->_num = (this->_num - n._num)%_modulus;
   if(this->_num<0)
   {
      this->_num += _modulus;
   }
   return *this;
}

ModNum ModNum::operator * (const ModNum& n) const 
{
   return ModNum(this->_num * n._num);
}

ModNum& ModNum::operator *= (const ModNum& n) 
{
   this->_num = (this->_num * n._num)%_modulus;
   if(this->_num<0)
   {
      this->_num += _modulus;
   }
   return *this;
}

bool ModNum::operator == (const ModNum& n) const 
{
   return (this->_num == n._num);
}

bool ModNum::operator != (const ModNum& n) const 
{
   return (this->_num != n._num);
}

ModNum& ModNum::operator = (const ModNum& n) 
{
   this->_num = n._num;
   return *this;
}

void ModNum::setVarVal(const string& s, const ModNum& n)
{
   _varMap[s]._num = n._num;
}

bool ModNum::getVarVal(const string& s, ModNum& n)
{
   bool retval = false;
   if(_varMap.find(s)!=_varMap.end()) // found
   {
      n._num = _varMap[s]._num;
      retval = true;
   }
   else // not found
   {
      retval = false;
   }
   return retval;
}


bool ModNum::getStrVal(const string& s, ModNum& n)
{
   if(isValidVarName(s)) // it's a variable name
   {
      return getVarVal(s, n);
   }
   else
   {
      long value = 0;
      stringstream ss(s);
      ss >> value;
      if(!ss.fail() && !ss.bad() && ss.eof()) // it's a number (check eof() is needed. eg. 4a3 will give 4)
      {
         n._num = value % _modulus;
         if(n._num<0)
         {
            n._num += _modulus;
         }
         return true;
      }
      else // neither (invalid input)
      {
         return false;
      }
   }
}


void ModNum::printVars()
{
   for(CalcMap::iterator it = _varMap.begin();it!=_varMap.end();it++)
   {
      cout << it->first << " = " << it->second << endl;
   }
}

void ModNum::resetVapMap()
{
   _varMap.clear();
}

