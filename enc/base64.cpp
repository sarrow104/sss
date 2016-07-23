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

// ��ν����أ�
//  �ʼ������Ϊtableǡ�����㽵��������򣻵��������������
//  'A':65, 'a':97,'0':48,'1':49,'+':43,'/':47
//  �����Ļ���Ϊ��׷���ٶȣ�ֻ���ô�����ʹ�ò��ұ��ˣ�
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
        // ��Ϊת����ı��룬���ǿɴ�ӡ�ַ������Լ����Ŀռ䣬ʹ��256/2���ɡ�
        // ���ǣ����ǵ�char�Ŀռ䷶Χ����256��ֵ��Ϊ�˱�����if��䣬ʹ��256��
        // ֵ��Ҳ�ǿ��Եġ�
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

        // ����� if ��䣬Ҳ������һ��4��֧��switch������
        // ��Ϊ����push-pop������λ�ã��ǰ�ÿ4���ֽ�ѭ���ġ�
        if (buffer_len >= 8)
        {
            ret += (buffer >> (buffer_len - 8)) & 0xFFu;
            buffer_len -= 8;
        }
    }

    // FIXME
    // Ӧ���϶Գ���'='λ�õļ�鴦����������'='ʱ��Ҫô����ǽ�β��Ҫô֮��
    // ����һ��'='�ַ��͵��˽�β��
    assert(end_pos == s.length() ||             // û���ַ�'='��
           end_pos == s.length() - 1 ||         // �ַ�'='����������ĩβ
           (end_pos == s.length() - 2 && *s.rbegin() == '=')); // ��������ַ�����'='��
    // FIXME
    // ���ڽ�������������һ��Լ����������'='�ĸ�����������ĳ����йء�
    // ��Ȼ��Ҳ���Է�������֤�����ȼ���ĩβ��'='���������������ֽڳ��ȣ�����
    // ������ȣ���ʵ�ʽ��볤�Ƚ��бȽϡ�
    // ��Ȼ���������ʵ���������У��ܷ�򵥵ķ���β���йء��ݴˣ�ѡ����ʵ��㷨��

    return ret;
}

} }
