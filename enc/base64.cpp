#include "base64.hpp"

#include <cassert>
#include <cstring>
#include <string>

namespace sss{
namespace enc {

static const char * const b64_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "abcdefghijklmnopqrstuvwxyz" "0123456789" "+/";
static const char * const b64url_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "abcdefghijklmnopqrstuvwxyz" "0123456789" "-_";
// Replaces “+” by “-” (minus)
// Replaces “/” by “_” (underline)

class b64_reverse_table_t{
public:
    explicit b64_reverse_table_t(const char* tab)
    {
        std::memset(this->buffer, 0xFFu, sizeof(this->buffer));
        for (size_t i = 0, len = std::strlen(tab); i != len; ++i) {
            this->buffer[size_t(tab[i])] = i;
        }
    }

    unsigned char operator () (unsigned char c) const
    {
        assert(buffer[c] != 0xFFu);
        return buffer[c];
    }

private:
    // 因为转换后的编码，都是可打印字符，所以检索的空间，使用256/2即可。
    // 但是，考虑到char的空间范围，是256个值。为了避免多个if语句，使用256个
    // 值，也是可以的。
    unsigned char buffer[256]{};
};

static inline unsigned int base64_cat_buffer(const unsigned char * b)
{
    return (b[0] << 16) | (b[1] << 8) | b[2];
}

static inline int base64_index6(unsigned int v, int i)
{
    return (v >> (6 * (3-i))) & 0x3Fu;
}

static inline char uint32_to_b64char_impl(const char* tab, unsigned int buffer, int i)
{
    return tab[base64_index6(buffer, i)];
}

std::string Base64::encode(const std::string& s)
{
    return base64_encode(s);
}

// 如何解码呢？
//  最开始，我以为table恰好满足降序或者升序；但结果不是这样：
//  'A':65, 'a':97,'0':48,'1':49,'+':43,'/':47
//  这样的话，为了追求速度，只能用创建并使用查找表了：
std::string Base64::decode(const std::string& s)
{
    return base64_decode(s);
}

static std::string base64_encode_impl(const std::string& s, const char* table)
{
    const auto byte_width = uint32_t(CHAR_BIT);
    const auto short_width = byte_width * 2U;
    int type = int(s.length()) % 3;
    std::string ret;
    int out_len = ((s.length() - 1u) % 3 + s.length()) / 3 * 4;
    ret.reserve(out_len);
    unsigned int buffer = 0U;
    for (int i = 0; i < int(s.length() - type); i += 3)
    {
        buffer = base64_cat_buffer(reinterpret_cast<const unsigned char*>(&s[i]));
        ret += uint32_to_b64char_impl(table, buffer, 0);
        ret += uint32_to_b64char_impl(table, buffer, 1);
        ret += uint32_to_b64char_impl(table, buffer, 2);
        ret += uint32_to_b64char_impl(table, buffer, 3);
    }
    switch (type)
    {
    case 0:
        break;

    case 1:
        buffer = static_cast<unsigned int>(*s.rbegin()) << short_width;
        ret += uint32_to_b64char_impl(table, buffer, 0);
        ret += uint32_to_b64char_impl(table, buffer, 1);
        ret += "==";
        break;

    case 2:
        buffer = static_cast<unsigned int>(*(s.rbegin() + 1)) << short_width | static_cast<unsigned int>(*s.rbegin()) << byte_width;
        ret += uint32_to_b64char_impl(table, buffer, 0);
        ret += uint32_to_b64char_impl(table, buffer, 1);
        ret += uint32_to_b64char_impl(table, buffer, 2);
        ret += "=";
        break;
    }
    return ret;
}

static std::string base64_decode_impl(const std::string& s, const b64_reverse_table_t& b64_rev_op)
{
    const auto ending_byte_width = 6U;
    const auto byte_mask = 0xFFU;

    assert(s.length() % 4 == 0);

    std::string ret;
    ret.reserve(s.length());

    int buffer_len = 0;
    size_t buffer = 0U;

    size_t end_pos = s.length();
    for( size_t i = 0 ; i != s.length(); ++i )
    {
        if (s[i] == '=')
        {
            end_pos = i;
            break;
        }
        buffer = (buffer << ending_byte_width) | b64_rev_op(s[i]);
        buffer_len += ending_byte_width;

        // 这里的 if 语句，也可以用一个4分支的switch语句代替
        // 因为出现push-pop动作的位置，是按每4个字节循环的。
        if (buffer_len >= CHAR_BIT)
        {
            ret += char((buffer >> size_t(buffer_len - CHAR_BIT)) & byte_mask);
            buffer_len -= CHAR_BIT;
        }
    }
    (void)end_pos;

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

std::string base64_encode(const std::string& s)
{
    return base64_encode_impl(s, b64_table);
}

std::string base64url_encode(const std::string& s)
{
    return base64_encode_impl(s, b64url_table);
}

std::string base64_decode(const std::string& s)
{
    const static b64_reverse_table_t b64_rev_op{b64_table};
    return base64_decode_impl(s, b64_rev_op);
}

std::string base64url_decode(const std::string& s)
{
    const static b64_reverse_table_t b64_rev_op{b64url_table};
    return base64_decode_impl(s, b64_rev_op);
}

} // namespace enc
} // namespace sss
