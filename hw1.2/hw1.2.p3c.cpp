#include <iostream>
#include <vector>
#include <string>

using namespace std;

template <class T>
class Compare
{
public:
    virtual bool operator()(const T a, const T b) const = 0;
};

template <class T>
class Less : public Compare<T>
{
public:
    virtual bool operator()(const T a, const T b) const
    {
        return a < b;
    }
};

template <class T>
class Greater : public Compare<T>
{
public:
    virtual bool operator()(const T a, const T b) const
    {
        return a > b;
    }
};

template <class T>
class NoSort : public Compare<T>
{
public:
    virtual bool operator()(const T a, const T b) const
    {
        return true;
    }
};

template <class T>
void mySwap(T& a, T& b)
{
    T temp = a;
    a = b;
    b = temp;
}

template <class T>
void selectionSort(vector<T>& array, const Compare<T>& compare)
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

template<class T>
void sort_and_print(vector<T>& array, const Compare<T>& compare)
{
    cout << endl;
    selectionSort(array, compare);
    for(int i=0;i<array.size();i++)
    {
        cout << array[i] << " ";
    }
    cout << endl;
}

template<class T>
void input(vector<T>& array)
{
    for(int i=0;i<array.size();i++)
    {
        cin >> array[i];
    }
}

int main()
{
    int number = 0;
    cout << "How many strings? ";
    cin >> number;
    vector<string> arr_strings(number);
    input(arr_strings);

    cout << "Before sort:";
    sort_and_print(arr_strings, NoSort<string>());

    cout << "Ascending sort:";
    sort_and_print(arr_strings, Less<string>());

    cout << endl << "How many doubles? ";
    cin >> number;
    vector<double> arr_doubles(number);
    input(arr_doubles);

    cout << "Before sort:";
    sort_and_print(arr_doubles, NoSort<double>());

    cout << "Ascending sort:";
    sort_and_print(arr_doubles, Less<double>());

    cout << endl;
    return 0;
}
