/****************************************************************************
  FileName     [ hw1.2.p1.bug.cpp]
  PackageName  [ HW1.2 ]
  Synopsis     [ For problem 1(c) of HW1.2 in DSnP class ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2011-2012 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <cstdlib>
#include "hw1.2.p1a.h"

using namespace std;

int main()
{
   P1a *p1 = (P1a*)malloc(sizeof(P1a));
   P1a *p2 = (P1a*)malloc(sizeof(P1a));

   p1->assign("I");
   p2->assign(" love");

   (*p1).append(*p2).append(P1a(" DSnP!!"));

   p1->print();
}

