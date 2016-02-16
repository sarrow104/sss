#include <iosfwd>
#include <string>
#include <sstream>

namespace sss{
    namespace util {
        // ����linux�����в�����ת��ո���ַ�����
        // �����ascii�ַ�����ת�壻
        // �������⣺
        //    ������ת���ַ�����δ���
        //    ����c_str�У�"\\"������ת����ַ�������ַ�����ǡ����"\\"��β����
        //    ��ô����
        //    ��Ȼ������˫���ű���Ҳ����Ҫת��ģ������ⲻ�����⣻
        //    ���ǣ�printf�еĸ�ʽ�ַ��������������ˣ���ʽ�ַ����У�����"%"Ϊת
        //    ���ַ��ģ�
        //    ����˵��"%%"��ʾһ��"%"����������ģ����ʾ�ض����������
        class Escaper
        {
        public:
            // escape_setting �ĵ�һ���ַ���ת���ַ��������Ķ��Ǳ�ת���ַ���
            // ͨ��ת���ַ�����
            Escaper();
            explicit Escaper(const std::string& escape_setting);
            ~Escaper();

        public:
            void assign(const std::string& escape_setting = "");
            bool good() const;

            bool isEscapeChar(char ch) const;
            bool isNeedEscape(char ch) const;

        public:
            void escapeToStream(std::ostream& o, const std::string& s);
            void unescapeToStream(std::ostream& o, const std::string& s);

            std::string escape(const std::string& s);
            std::string unescape(const std::string& s);

        private:
            char _escape_char;
            char _escape_table[128];

        public:
            static
            std::string escape(const std::string& escape_setting, const std::string& s)
            {
                std::ostringstream oss;
                Escaper tmp_esc(escape_setting);
                tmp_esc.escapeToStream(oss, s);
                return oss.str();
            }
            static
            std::string unescape(const std::string& escape_setting, const std::string& s)
            {
                std::ostringstream oss;
                Escaper tmp_esc(escape_setting);
                tmp_esc.unescapeToStream(oss, s);
                return oss.str();
            }
        };

        bool isQuoted(const std::string& s, char escape = '\\');
        inline bool isDquoted(const std::string& s, char escape = '\\')
        {
            return isQuoted(s, escape) && s[0] == '"';
        }
        inline bool isSquoted(const std::string& s, char escape = '\\')
        {
            return isQuoted(s, escape) && s[0] == '\'';
        }
        std::string& dquote(std::string& s, char escape = '\\');
        std::string& squote(std::string& s, char escape = '\\');
        std::string& unquote(std::string& s, char escape = '\\');

        inline std::string dquote_copy(const std::string& s, char escape = '\\')
        {
            std::string ret(s);
            return dquote(ret, escape);
        }
        inline std::string squote_copy(const std::string& s, char escape = '\\')
        {
            std::string ret(s);
            return squote(ret, escape);
        }
        inline std::string unquote_copy(const std::string& s, char escape = '\\')
        {
            std::string ret(s);
            return unquote(ret, escape);
        }
    }
}

