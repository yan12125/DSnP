#include <iostream>
#include <vector>

using namespace std;

class Compare
{
public:
    virtual bool operator()(const int a, const int b) const = 0;
protected:
};

class Less : public Compare
{
public:
    virtual bool operator()(const int a, const int b) const
    {
        return a < b;
    }
protected:
};

class Greater : public Compare
{
public:
    virtual bool operator()(const int a, const int b) const
    {
        return a > b;
    }
protected:
};

void mySwap(int& a, int& b)
{
    int temp = a;
    a = b;
    b = temp;
}

void selectionSort(vector<int>& array, const Compare& compare)
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
    cout << endl << "Ascending sort:" << endl;
    selectionSort(arr_numbers, Less());
    for(int i=0;i<number;i++)
    {
        cout << arr_numbers[i] << " ";
    }
    cout << endl << "Descending sort:" << endl;
    selectionSort(arr_numbers, Greater());
    for(int i=0;i<number;i++)
    {
        cout << arr_numbers[i] << " ";
    }
    cout << endl;
    return 0;
}
