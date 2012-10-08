/****************************************************************************
  FileName     [ hw1.2.p1a.main.cpp]
  PackageName  [ HW1.2 ]
  Synopsis     [ For problem 1(a) of HW1.2 in DSnP class ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2011-2012 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include "hw1.2.p1a.h"

int main()
{
   P1a p1("I");
   P1a p2(" love");

   p1.append(p2).append(P1a(" DSnP!!"));

   p1.print();
}

