#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>

#include "../spliter.hpp"

#include <gtest/gtest.h>

TEST(spliter, basic)
{
    std::string s("2012-01-24");

    std::ostringstream oss;

    sss::Spliter sp(s, '-');
    std::string stem;
    while (sp.fetch_next(stem)) {
        oss << "{" << stem << "}" << std::endl;
    }
    GTEST_ASSERT_EQ(oss.str(), "{2012}\n{01}\n{24}\n");
}
