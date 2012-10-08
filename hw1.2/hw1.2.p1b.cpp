#include <iostream>
#include "hw1.2.p1a.h"
#include "hw1.2.p1b.h"

using namespace std;

void printSize()
{
    cout << "The size of class P1a is " << sizeof(P1a) << "." << endl;
    return;
}

template<class T>
void _internal_printAddrs(T* ptr, size_t _size, const string& arrName)
{
    cout << "============================" << endl
         << "Addresses of " << arrName << "[" << _size << "] are:" << endl;
    for(size_t i=0;i<_size;i++)
    {
        cout << "&" << arrName << "[" << i << "]: " << hex << &ptr[i] << endl;
    }
    cout << "&" << arrName << ": " << hex << &ptr << endl;
    return;        
}

void printStaticArraySize()
{
    P1a arr1b_1[5];
    _internal_printAddrs(arr1b_1, 5, "arr1b_1");
    return;
}

void printDynamicArraySize()
{
    P1a *arr1b_2 = new P1a[5];
    _internal_printAddrs(arr1b_2, 5, "arr1b_2");
    delete [] arr1b_2;
    return;
}

void printPointerArraySize()
{
    P1a **arr1b_3 = new P1a *[5];
    for (size_t i = 0; i < 5; ++i)
        arr1b_3[i] = new P1a;

    cout << "============================" << endl
         << "Addresses of arr1b_3[5] are:" << endl;
    for(size_t i=0;i<5;i++)
    {
        cout << "&arr1b_3[" << i << "]: " << hex << &arr1b_3[i] << endl;
    }

    cout << "Contents of arr1b_3[5] are: " << endl;
    for(size_t i=0;i<5;i++)
    {
        cout << "arr1b_3[" << i << "]: ";
        arr1b_3[i]->print();
        cout << endl;
    }
    cout << "&arr1b_3: " << hex << &arr1b_3 << endl;
    for(int i=0;i<5;i++)
    {
        delete arr1b_3[i];
    }
    delete [] arr1b_3;
    return;
}
