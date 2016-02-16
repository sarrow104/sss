#ifndef  __UTILES_CPP_1443968625__
#define  __UTILES_CPP_1443968625__

#include "utiles.hpp"

#include <iostream>

namespace
{
    // bool less_XP(const string& lhs, const string& rhs);
    // 返回字符等级（符号最为优先、其次数字、再次为字母）
    inline int charRankLevel(char ch)
    {
        return  ispunct(ch) ? 0 :
            isdigit(ch) ? 1 :
            isalpha(ch) ? 2 :
            -1;
    }
}

namespace sss{
    namespace path {

bool less_XP(std::string const& lhs, std::string const & rhs)
{
    using std::string;
    using std::make_pair;
    using std::pair;
    using std::cerr;
    using std::endl;

    typedef string::const_iterator citer_t;
    citer_t left[2] = {lhs.begin()},
            right[2] = {rhs.begin()};
    int zeroStringDiff = 0;
    while (left[0] != lhs.end() && right[0] != rhs.end()) {
        //数字和数字比较
        if (isdigit(*left[0]) && isdigit(*right[0])) {
            int leftZeroLen = 0;
            while (left[0] != lhs.end() && *left[0] == '0') {
                ++left[0];
                ++leftZeroLen;
            }
            int rightZeroLen = 0;
            while (right[0] != rhs.end() && *right[0] == '0') {
                ++right[0];
                ++rightZeroLen;
            }

            if (zeroStringDiff == 0)
                zeroStringDiff = leftZeroLen - rightZeroLen;

            left[1] = left[0];
            while (left[1] != lhs.end() && isdigit(*left[1])) {
                ++left[1];
            }
            right[1] = right[0];
            while (right[1] != rhs.end() && isdigit(*right[1])) {
                ++right[1];
            }

            int leftDigitLen = distance(left[0], left[1]);

            int rightDigitLen = distance(right[0], right[1]);

            int digitLenDiff = leftDigitLen - rightDigitLen;
            if (digitLenDiff < 0) {
                return true;
            }
            else if (digitLenDiff == 0) {
                pair<citer_t, citer_t> pit = mismatch(left[0], left[1], right[0]);
                if (pit.first != left[1]) {
                    return *pit.first < *pit.second;
                }
            }
            else if (digitLenDiff > 0) {
                return false;
            }

            left[0] = left[1];
            right[0] = right[1];
        }
        //其他情况（数字、标点、字母之间的比较。）
        else {
            int leftLevel  = charRankLevel(*left[0]);
            int rightLevel = charRankLevel(*right[0]);

            if (leftLevel == -1 || rightLevel == -1) {
                cerr << "bad character !" << endl;
            }

            int levelDiff = leftLevel - rightLevel;
            if (levelDiff < 0) {
                return true;
            }
            else if (levelDiff == 0) {
                if (int diff = toupper(*left[0]) - toupper(*right[0])) {
                    return diff < 0;
                }
            }
            else if (levelDiff > 0) {
                return false;
            }

            ++left[0];
            ++right[0];
        }
    }

    //循环结束
    int lengthDiff = std::distance(left[0], lhs.end())
        - std::distance(right[0], rhs.end());
    if (lengthDiff < 0) {
        return true;
    }
    else if (lengthDiff == 0) {//此时必然有左右两个串都到了结尾。
        if (zeroStringDiff) {
            //零串长的，反而小
            return zeroStringDiff > 0;
        }
        //串全等，返回false;
        return false;
    }
    else if (lengthDiff < 0) {
        return false;
    }

    return false;
}

    }
}

#endif  /* __UTILES_CPP_1443968625__ */
