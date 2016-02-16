#ifndef  __EXCEPTION_HPP_1450796254__
#define  __EXCEPTION_HPP_1450796254__

#include <stdexcept>

namespace sss {
    namespace xml3 {
        // 解析时，发生的错误
        class ParsingError : public std::runtime_error
        {
        public:
            ParsingError(const std::string& msg)
                : std::runtime_error(msg)
            {
            }
            ~ParsingError() throw()
            {
            }
        };

        class SelectorParseError : public std::runtime_error
        {
        public:
            SelectorParseError(const std::string& msg)
                : std::runtime_error(msg)
            {
            }
            ~SelectorParseError() throw()
            {
            }
        };

        // 构建xml树时，发生的错误
        class ConstructingError : public std::runtime_error
        {
        public:
            ConstructingError(const std::string& msg)
                : std::runtime_error(msg)
            {
            }
            ~ConstructingError() throw()
            {
            }
        };
    }
}



#endif  /* __EXCEPTION_HPP_1450796254__ */
