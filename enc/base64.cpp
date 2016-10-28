#include "base64.hpp"

#include <string>
#include <cstring>
#include <assert.h>

namespace sss{ namespace enc {

static const char *get_table()
{
    static const char * table = "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "abcdefghijklmnopqrstuvwxyz" "0123456789" "+/";
    return table;
}

static inline unsigned int base64_cat_buffer(const unsigned char * b)
{
    return (b[0] << 16) | (b[1] << 8) | b[2];
}

static inline int base64_index6(unsigned int v, int i)
{
    return (v >> (6 * (3-i))) & 0x3Fu;
}

static inline char uint_32_to_base64char(unsigned int buffer, int i)
{
    return get_table()[base64_index6(buffer, i)];
}

std::string Base64::encode(const std::string& s)
{
    int type = int(s.length()) % 3;
    std::string ret;
    int out_len = ((s.length() - 1) % 3 + s.length()) / 3 * 4;
    ret.reserve(out_len);
    unsigned int buffer = 0u;
    for (int i = 0; i < int(s.length() - type); i += 3)
    {
        buffer = base64_cat_buffer((const unsigned char*)(&s[i]));
        ret += uint_32_to_base64char(buffer, 0);
        ret += uint_32_to_base64char(buffer, 1);
        ret += uint_32_to_base64char(buffer, 2);
        ret += uint_32_to_base64char(buffer, 3);
    }
    switch (type)
    {
    case 0:
        break;

    case 1:
        buffer = (unsigned int)(*s.rbegin()) << 16;
        ret += uint_32_to_base64char(buffer, 0);
        ret += uint_32_to_base64char(buffer, 1);
        ret += "==";
        break;

    case 2:
        buffer = (unsigned int)(*(s.rbegin() + 1)) << 16 | (unsigned int)(*s.rbegin()) << 8;
        ret += uint_32_to_base64char(buffer, 0);
        ret += uint_32_to_base64char(buffer, 1);
        ret += uint_32_to_base64char(buffer, 2);
        ret += "=";
        break;
    }
    return ret;
}

// 如何解码呢？
//  最开始，我以为table恰好满足降序或者升序；但结果不是这样：
//  'A':65, 'a':97,'0':48,'1':49,'+':43,'/':47
//  这样的话，为了追求速度，只能用创建并使用查找表了：
std::string Base64::decode(const std::string& s)
{
    assert(s.length() % 4 == 0);
    class b64_rer_tab{
    public:
        b64_rer_tab() {
            std::memset(this->buffer, 0xFFu, sizeof(this->buffer));
            for (size_t i = 0; i != std::strlen(get_table()); ++i)
                this->buffer[size_t(get_table()[i])] = i;
        }

        unsigned char operator () (unsigned char c)
        {
            assert(buffer[c] != 0xFFu);
            return buffer[c];
        }

    private:
        // 因为转换后的编码，都是可打印字符，所以检索的空间，使用256/2即可。
        // 但是，考虑到char的空间范围，是256个值。为了避免多个if语句，使用256个
        // 值，也是可以的。
        unsigned char buffer[256];
    };
    static b64_rer_tab b64_op;
    std::string ret;
    ret.reserve(s.length());

    int buffer_len = 0;
    unsigned int buffer = 0u;

    size_t end_pos = s.length();
    for( size_t i = 0 ; i != s.length(); ++i )
    {
        if (s[i] == '=')
        {
            end_pos = i;
            break;
        }
        buffer = (buffer << 6) | b64_op(s[i]);
        buffer_len += 6;

        // 这里的 if 语句，也可以用一个4分支的switch语句代替
        // 因为出现push-pop动作的位置，是按每4个字节循环的。
        if (buffer_len >= 8)
        {
            ret += (buffer >> (buffer_len - 8)) & 0xFFu;
            buffer_len -= 8;
        }
    }

    // FIXME
    // 应加上对出现'='位置的检查处理。――出现'='时，要么这就是结尾；要么之后，
    // 还有一个'='字符就到了结尾。
    assert(end_pos == s.length() ||             // 没有字符'='；
           end_pos == s.length() - 1 ||         // 字符'='出现在序列末尾
           (end_pos == s.length() - 2 && *s.rbegin() == '=')); // 最后两个字符都是'='；
    // FIXME
    // 对于解码结果，还少了一个约束条件――'='的个数，与解码后的长度有关。
    // 当然，也可以反过来验证――先计算末尾的'='个数，求出解码后字节长度；再用
    // 这个长度，与实际解码长度进行比较。
    // 当然，这个还与实际输入序列，能否简单的访问尾部有关。据此，选择合适的算法。

    return ret;
}

} }
