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
   bool bBST = false;
   if(argc >= 2)
   {
      count = atoi(argv[1]);
   }
   if(argc >= 3)
   {
      if(!strncmp(argv[2], "-bst", 4))
      {
         bBST = true;
      }
   }
   int strLen = rand()%20+1;
   char* testStr = new char[strLen+1];
   cout << "adtr " << strLen << "\n";
   vector<string> existStrs;
   for(int i=1;i<count;i++)
   {
      int p1 = rand()%100;
      if(p1<75)
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
      else if(p1<80)
      {
         cout << "adtp " << (bBST?"-v ":"") << ((rand()%2)?"-r":"") << "\n";
      }
      else if(p1<97) // the speed of delete is much faster than add
      {
         if(!existStrs.empty())
         {
            int p2 = rand()%10;
            if(p2<7) // delete a single string
            {
               int n = rand()%existStrs.size();
               memset(testStr, 0, strLen);
               strcpy(testStr, existStrs[n].c_str());
               existStrs.erase(existStrs.begin()+n);
               cout << "adtd -s " << testStr << "\n";
            }
            else if(p2<9) // -min or -max
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
         cout << "adtp";
         if(bBST)
         {
            cout << " -v";
         }
         cout << "\nadtr " << strLen << "\n";
         existStrs.clear();
         i++; // an extra adtp command
      }
   }
   cout << "adtp\nq -f" << endl;
   delete [] testStr;
   return 0;
}
