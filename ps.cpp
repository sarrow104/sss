#include "ps.hpp"

#include <sss/util/Escaper.hpp>

namespace sss
{
    namespace ps
    {
        // system 在linux和windows下，都是调用自己的命令行解释器，对目录进行解释；
        // linux下，自然就是shell；
        // shell下，对参数的特殊字符，有两种处理办法；一种是用引号（单双皆可）
        // ，来消除歧义；一种，就是用反斜杠进行转义；
        //
        // 不过，有几个字符比较特殊，应按照需要来转义；
        // '>','<','|',还有"&&"，"||"
        // 前三个是重新定向管道；后面两个是短路执行；
        // 绝大多数情况下（很少有人的文件名，直接是'>',...；），都OK；但来自网
        // 络的文件名，确实可能含有'&'字符；
        // 即，真有可能需要区分情况，以决定是否用引号包裹，或者转义；
        void StringPipe::streamAddParam(std::ostream& oss, const std::string& param)
        {
#if 0
            if (param.empty()) {
                return;
            }
            // oss << ' '; 是否补上' '间隔，由上层决定；

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
