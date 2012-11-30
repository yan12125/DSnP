#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cstring>
using namespace std;

int main(int argc, char* argv[])
{
   srand(time(NULL));
   int count = 100;
   if(argc == 2)
   {
      count = atoi(argv[1]);
   }
   int strLen = rand()%20+1;
   char* testStr = new char[strLen+1];
   vector<string> existStrs;
   for(int i=0;i<count;i++)
   {
      int p1 = rand()%100;
      if(p1<40)
      {
         memset(testStr, 0, strLen+1);
         int length = rand()%strLen+1;
         for(int j=0;j<length;j++)
         {
            testStr[j] = (rand()%26)+((rand()%2)?'a':'A');
         }
         if(find(existStrs.begin(), existStrs.end(), string(testStr)) != existStrs.end()) // found
         {
            i--;
         }
         else
         {
            cout << "adta -s " << testStr << "\n";
            existStrs.push_back(testStr);
         }
      }
      else if(p1<50)
      {
         cout << "adtp " << ((rand()%2)?"-r":"") << "\n";
      }
      else if(p1<90)
      {
         if(!existStrs.empty())
         {
            int p2 = rand()%10;
            if(p2<4) // delete a single string
            {
               int n = rand()%existStrs.size();
               memset(testStr, 0, strLen);
               strcpy(testStr, existStrs[n].c_str());
               existStrs.erase(existStrs.begin()+n);
               cout << "adtd -s " << testStr << "\n";
            }
            else if(p2<8) // -min or -max
            {
               sort(existStrs.begin(), existStrs.end());
               int m = rand()%existStrs.size()+1;
               if(p2<6) // for -min
               {
                  cout << "adtd -min " << m << "\n";
                  existStrs.erase(existStrs.begin(), existStrs.begin()+m);
               }
               else
               {
                  cout << "adtd -max " << m << "\n";
                  existStrs.erase(existStrs.begin()+existStrs.size()-m, existStrs.end());
               }
            }
            else
            {
               cout << "adtd -a\n";
               existStrs.clear();
            }
         }
         else
         {
            i--; // because no operation in this cycle
         }
      }
      else
      {
         strLen = rand()%20+1;
         delete [] testStr;
         testStr = new char[strLen+1];
         memset(testStr, 0, strLen+1);
         cout << "adtr " << strLen << "\n";
         existStrs.clear();
      }
   }
   cout << "adtp\nq -f" << endl;
   delete [] testStr;
   return 0;
}
