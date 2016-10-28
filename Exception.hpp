#ifndef  __EXCEPTION_HPP_1365127813__
#define  __EXCEPTION_HPP_1365127813__

#include <exception>
#include <string>

namespace sss {

    class Exception : public std::exception
    {
    public:
        Exception(const std::string& s) throw()
            : message(s)
        {}
        virtual ~Exception() throw()
        {}

    public:
        virtual const char * what() const throw()
        {
            return this->message.c_str();
        }

        // NOTE 方便上层过程插入辅助信息。
        // 因为底层调用，是没有必要，也不可能知道上层的环境的。但是中层过程不可
        // 能傻傻地原样抛出异常，那么再上层，直到用户，可能根本不知道这个错误信
        // 息的由来。
        // 就是说，异常消息，也需要封装起来。
        //
        // 处理方式有两种，一种是截获底层消息，拆开，加上当前层知道的，有可能对
        // 上层有帮助的信息，再重新打包成一个新的异常对象；最后抛出。
        //
        // 另外一种，就是改造底层消息对象的功能，让它能随时附加消息。
        //! 前插
        virtual void insert(const std::string& s) throw()
        {
            this->message = s + this->message;
        }

        //! 后附
        virtual void append(const std::string& s) throw()
        {
            this->message += s;
        }
    private:
        std::string message;
    };
    // 如何具现化上述模版类呢？
    //  template class Exception<MyClass>;

    // 用户不支持的成员函数异常抛出
    class ExceptionNotSupportMethod : public Exception
    {
    public:
        ExceptionNotSupportMethod(const std::string& msg)
            : Exception(msg + " not support!")
        {
        }
    };
}


#endif  /* __EXCEPTION_HPP_1365127813__ */
