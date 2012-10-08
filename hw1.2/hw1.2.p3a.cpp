#include <iostream>
#include <vector>

using namespace std;

void mySwap(int& a, int& b)
{
    int temp = a;
    a = b;
    b = temp;
}

bool compare(const int a, const int b)
{
    return a < b;
}

void selectionSort(vector<int>& array)
{
    for (size_t i = 0, n = array.size(); i < n - 1; ++i) {
        size_t pivot = i;
        for (size_t j = i+1; j < n; ++j) {
            if (!compare(array[pivot], array[j]))
                pivot = j;
        }
        if (pivot != i)
            mySwap(array[pivot], array[i]);
    }
}

int main()
{
    int number = 0;
    cout << "How many numbers? ";
    cin >> number;
    vector<int> arr_numbers(number);
    for(int i=0;i<number;i++)
    {
        cin >> arr_numbers[i];
    }
    cout << endl << "Before sort:" << endl;
    for(int i=0;i<number;i++)
    {
        cout << arr_numbers[i] << " ";
    }
    cout << endl << "After sort:" << endl;
    selectionSort(arr_numbers);
    for(int i=0;i<number;i++)
    {
        cout << arr_numbers[i] << " ";
    }
    return 0;
}
