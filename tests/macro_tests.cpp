// TODO

/*
 * =====================================================================================
 *
 *       Filename:  test.cpp
 *
 *    Description:  test operator.h
 *
 *        Version:  1.0
 *        Created:  2008-7-8 1:14:24 中国标准时间
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  first_name last_name (fl), fl@my-company.com
 *        Company:  my-company
 *
 * =====================================================================================
 */

#include "../macro/operator.hpp"
#include "../macro/foreach.hpp"

#include <gtest/gtest.h>

#include <iostream>

class A
{
public:
    enum _d_ { _a, _b, _c };
    explicit A(_d_ mask){};
};

SSS_BIT_MACK_ARGUMENT_OPERATORS(A::_d_);

TEST(macro, operator)
{
    A a(A::_a), b(A::_a | A::_b);
}

TEST(macro, foreach)
{
    using namespace std;

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
}
