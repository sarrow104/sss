//T:foreach.test.cpp
#include <iostream>

using namespace std;
#include "foreach.h"

int main(int argc, char* argv[])
{
    int array[] = {2, 3, 5, 7, 9, 8, };
    int y;
//1
    cout << "array={";
    foreach(y, array)
    {
       cout << y << ",";
    }
    cout << "\b}\n";

//2
    foreach(int& x, array)
    {
        x *= 2;
    }

//3
    cout << "array={";
    foreach(int x, array)
    {
        cout << x << ",";
    }
    cout << "\b}\n";

//4
    int arr[] = {1, 2, 3, 4, 5};
    foreach(int a, arr) {
        if( a == 3 )
            break;
        cout << a << endl;
    }
    return 0;
}

