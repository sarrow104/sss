#include "ps.hpp"

#include <sss/util/Escaper.hpp>

namespace sss
{
    namespace ps
    {
        // system ��linux��windows�£����ǵ����Լ��������н���������Ŀ¼���н��ͣ�
        // linux�£���Ȼ����shell��
        // shell�£��Բ����������ַ��������ִ���취��һ���������ţ���˫�Կɣ�
        // �����������壻һ�֣������÷�б�ܽ���ת�壻
        //
        // �������м����ַ��Ƚ����⣬Ӧ������Ҫ��ת�壻
        // '>','<','|',����"&&"��"||"
        // ǰ���������¶���ܵ������������Ƕ�·ִ�У�
        // �����������£��������˵��ļ�����ֱ����'>',...��������OK����������
        // ����ļ�����ȷʵ���ܺ���'&'�ַ���
        // �������п�����Ҫ����������Ծ����Ƿ������Ű���������ת�壻
        void StringPipe::streamAddParam(std::ostream& oss, const std::string& param)
        {
#if 0
            if (param.empty()) {
                return;
            }
            // oss << ' '; �Ƿ���' '��������ϲ������

            bool is_need_quote =
                param.find(' ') != std::string::npos ||
                param.find('(') != std::string::npos ||
                param.find(')') != std::string::npos;

            if (is_need_quote) {
                oss << '"';
            }
            for (size_t i = 0; i != param.length(); ++i) {
                if ('"' == param[i]) {
                    oss << '\\';
                }
                oss << param[i];
            }
            if (is_need_quote) {
                oss << '"';
            }
#else
            // static sss::util::Escaper esp(R"(\ "'[](){}?*$)"); // -std=c++11
            static sss::util::Escaper esp("\\ \"'[](){}?*$&");
            esp.escapeToStream(oss, param);
#endif
        }
        void StringPipe::streamAddParams(std::ostream& o, int argc, char *argv[])
        {
            for (int i = 0; i < argc; ++i) {
                o << " ";
                StringPipe::streamAddParam(o, argv[i]);
            }
        }
    }
}
