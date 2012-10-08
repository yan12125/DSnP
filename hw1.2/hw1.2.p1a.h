#ifndef _P1a_h
#define _P1a_h

#include <string>
using namespace std;

class P1a
{
public:
    P1a() {}
    P1a(const string& s) : _str(s) {}
    void assign(const string&);
    void print() const;
    P1a& append(const P1a&);
private:
    int _dummy;
    string _str;
};

#endif
