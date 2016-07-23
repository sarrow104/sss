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

        // NOTE �����ϲ���̲��븨����Ϣ��
        // ��Ϊ�ײ���ã���û�б�Ҫ��Ҳ������֪���ϲ�Ļ����ġ������в���̲���
        // ��ɵɵ��ԭ���׳��쳣����ô���ϲ㣬ֱ���û������ܸ�����֪�����������
        // Ϣ��������
        // ����˵���쳣��Ϣ��Ҳ��Ҫ��װ������
        //
        // ����ʽ�����֣�һ���ǽػ�ײ���Ϣ���𿪣����ϵ�ǰ��֪���ģ��п��ܶ�
        // �ϲ��а�������Ϣ�������´����һ���µ��쳣��������׳���
        //
        // ����һ�֣����Ǹ���ײ���Ϣ����Ĺ��ܣ���������ʱ������Ϣ��
        //! ǰ��
        virtual void insert(const std::string& s) throw()
        {
            this->message = s + this->message;
        }

        //! ��
        virtual void append(const std::string& s) throw()
        {
            this->message += s;
        }
    private:
        std::string message;
    };
    // ��ξ��ֻ�����ģ�����أ�
    //  template class Exception<MyClass>;

    // �û���֧�ֵĳ�Ա�����쳣�׳�
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
