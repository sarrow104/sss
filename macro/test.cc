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

#include  "operator.h"

class A
{
public:
    enum _d_ { __a, __b, __c };
    A(_d_ mask){};
};

SSS_BIT_MACK_ARGUMENT_OPERATORS(A::_d_);

int main(int argc, char *argv[])
{
    A a(A::__a), b(A::__a | A::__b);

    return 0;
}

