#ifndef  __SPLITER_HPP_1327379487__
#define  __SPLITER_HPP_1327379487__

//v1:2012-01-24
//Sarrow

#include <string>
#include <vector>
#include <iterator>
#include <algorithm>

namespace sss{
    template<typename IteratorT>
    class RangeSpliter
    {
    public:
        typedef std::pair<IteratorT, IteratorT> range_t;
        typedef typename std::iterator_traits<IteratorT>::value_type value_type;

        RangeSpliter(IteratorT it_beg, IteratorT it_end, value_type delims)
            : _it_beg(it_beg), _it_end(it_end), _delims(delims)
        {
        }
        ~RangeSpliter()
        {
        }

    public:
        bool fetch_next(range_t & range)
        {
            return this->fetch(range);
        }

        bool fetch(range_t & range)
        {
            if (_it_beg >= _it_end) {
                return false;
            }

            IteratorT it_next = std::find(_it_beg, _it_end, _delims);

            range.first  = _it_beg;
            range.second = it_next;

            if (it_next != _it_end)
            {
                _it_beg = it_next;
                std::advance(_it_beg, 1);
                return true;
            }
            else
            {
                _it_beg = _it_end;
                return true;
            }
        }

    public:
        IteratorT _it_beg;
        IteratorT _it_end;
        value_type _delims;
    };

class Spliter
{
public:
    // ע�⣬���ڲ�֣������ַ�ʽ��
    // 1. һ��delimsֻ����һ���ַ�
    // 2. delims�������ֵ����ʱ��Ϊ���������
    //    a. ��������span   ���� �������Ķ��delims������һ����
    //    b. ����������span
    // ���⣬�����һ���ַ�����delims����ô֮ǰ�㲻��һ��stem�أ�
    // ���������ǿ��ַ��������㲻����Բ���أ�
    Spliter(const std::string& str, char delims)
        : s(str), d(delims), ini(0)
    {
    }

    ~Spliter()
    {
    }

public:
    bool fetch_next(std::string& stem)
    {
        if (this->s.length() == 0 || this->ini == std::string::npos)
            return false;
        size_type fin = this->s.find(this->d, ini);
        if (fin != std::string::npos)
        {
            stem.assign(this->s, ini, fin - ini);
            this->ini = fin + 1;
            return true;
        }
        else
        {
            //stem.assign(this->s, ini, this->s.length() - ini);
            stem.assign(this->s.substr(ini));
            this->ini = std::string::npos;
            return true;
        }
    }

private:
    typedef std::string::size_type size_type;
    std::string s;
    char        d;
    size_type   ini;
};

// TODO
// �������������У������ַ����з֣�
// ע�⣬�Ⲣ���������������������������ǵ�ת���ַ����еĻ���
//
// ��ʱ������һ���ֽڴ���һ���ַ������������������ַ�������һ���ַ���
//
// �����Ҫ���ָ��ж��ˣ�
// ��ʱ�������ֽ���취��
// 1. �ṩ����escape_charsetting���ַ�����װ�ࣻ���������������ȡ�ֽڣ�
// 2. ÿ���ֶ���д����
class SpliterSequence
{
};

namespace util {
    template<typename T>
        inline
        std::string join(T ini, T fin, const std::string& sep)
        {
            std::string ret;
            bool is_empty = true;
            while (ini != fin)
            {
                if (!is_empty)
                {
                    ret += sep;
                }
                ret += *ini;
                is_empty = false;
                ++ini;
            }
            return ret;
        }

    template<typename T, typename Op>
        inline
        std::string join(T ini, T fin, const std::string& sep, Op o)
        {
            std::string ret;
            bool is_empty = true;
            while (ini != fin)
            {
                if (!is_empty)
                {
                    ret += sep;
                }
                ret += o(ini);
                is_empty = false;
                ++ini;
            }
            return ret;
        }

    //inline int split(const std::string& s, char del, std::vector<std::string>& vs)
    //{
    //    std::vector<std::string> ret;
    //    Spliter sp(s, del);
    //    std::string stem;
    //    while(sp.fetch_next(stem))
    //    {
    //        ret.push_back(stem);
    //    }
    //    ret.swap(vs);
    //    return vs.size();
    //}

    template<typename C>
    inline int split(const std::string& s, char del, C& vs)
    {
        Spliter sp(s, del);
        std::string stem;
        int count = 0;
        while(sp.fetch_next(stem))
        {
            std::back_inserter(vs) = stem;
            ++count;
        }
        return count;
    }
} // namespace util

} // namespace sss;


#ifdef __SPLITER_TEST__
#include <algorithm>
#include <iostream>
#include <iterator>

int main(int argc, char *argv[])
{
    std::string s("2012-01-24");

    sss::Spliter sp(s, '-');
    std::string stem;
    while(sp.fetch_next(stem))
    {
        std::cout << "{"<< stem << "}" << std::endl;
    }
}

#endif

#endif  /* __SPLITER_HPP_1327379487__ */
