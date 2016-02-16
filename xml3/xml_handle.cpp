#include "xml_handle.hpp"
#include "xml_parser.hpp"
#include "exception.hpp"

#include <sss/Exception.hpp>
#include <sss/path.hpp>
#include <iostream>

namespace sss{
    namespace xml3 {

        const char * EleHandle::D_PREFIX = "sss::xml3::EleHandle";

            sss::xml3::node * EleHandle::nodeCreator(sss::xml3::node_type_t type, const std::string& data)
            {
                // SSS_LOG_FUNC_TRACE(sss::log::log_ERROR);
                sss::xml3::xml_doc * pdoc = this->_data ? this->_data->get_doc() : 0;
                if (!pdoc) {
                    return 0;
                }
                sss::xml3::node * ret = 0;
                switch (type) {
                case sss::xml3::type_PI:
                    ret = pdoc->create_PI(data);
                    break;

                case sss::xml3::type_cdata:
                    ret = pdoc->create_cdata(data);
                    break;

                case sss::xml3::type_comment:
                    ret = pdoc->create_comment(data);
                    break;

                case sss::xml3::type_doctype:
                    ret = pdoc->create_doctype(data);
                    break;

                case sss::xml3::type_info:
                    ret = pdoc->create_info(data);
                    break;

                case sss::xml3::type_node:
                    ret = pdoc->create_node(data);
                    break;

                case sss::xml3::type_text:
                    ret = pdoc->create_text(data);
                    break;

                default:
                    {
                        std::ostringstream oss;
                        oss << D_PREFIX << "::" << __func__ << "( type error " << type << ")";
                        throw std::runtime_error(oss.str());
                    }
                }
                return ret;
            }

DocHandle::DocHandle()
    : _pdoc(0), _openmode(std::ios_base::in | std::ios_base::out)
{
    this->_pdoc = new xml3::xml_doc;
}

DocHandle::DocHandle(const std::string& fname, std::ios_base::openmode mode)
    : _pdoc(0), _openmode(mode)
{
    //! /usr/include/c++/4.8/fstream|240
    /*
     *  Table 92, adapted here, gives the relation between openmode
     *  combinations and the equivalent fopen() flags.
     *  (NB: lines app, in|out|app, in|app, binary|app, binary|in|out|app,
     *  and binary|in|app per DR 596)
     *  +---------------------------------------------------------+
     *  | ios_base Flag combination            stdio equivalent   |
     *  |binary  in  out  trunc  app                              |
     *  +---------------------------------------------------------+
     *  |             +                        w                  |
     *  |             +           +            a                  |
     *  |                         +            a                  |
     *  |             +     +                  w                  |
     *  |         +                            r                  |
     *  |         +   +                        r+                 |
     *  |         +   +     +                  w+                 |
     *  |         +   +           +            a+                 |
     *  |         +               +            a+                 |
     *  +---------------------------------------------------------+
     *  |   +         +                        wb                 |
     *  |   +         +           +            ab                 |
     *  |   +                     +            ab                 |
     *  |   +         +     +                  wb                 |
     *  |   +     +                            rb                 |
     *  |   +     +   +                        r+b                |
     *  |   +     +   +     +                  w+b                |
     *  |   +     +   +           +            a+b                |
     *  |   +     +               +            a+b                |
     *  +---------------------------------------------------------+
     */
    // NOTE，不同于标准的文件输入输出流，xml文档，是不大可能在内存中建立于外存
    // 的一一映射的——虽然在内存中貌似是随机读取，但是，写入的时候，还是得一并
    // ，流式写入；
    // 为了规范，in打开的不能写入——但是out有更高优先级，即，out打开，可以同时
    // 读取；
    //
    // app没有意义了——xml的写操作，很少会在文本后面添加数据的。
    // 至于trunc，由于不可能一边写，一边读，所以，这个选项也没用
    //
    // binary也没有意义；xml_parser内部本来就是二进制读写，并进行了转码，以保证是utf8；
    //
    // in  out
    // -        读取（不修改）
    //     -    写，不读取(直接覆盖)
    // -   -    读写
    this->load(fname);
}

DocHandle::~DocHandle()
{
    this->clear();
}

void DocHandle::clear()
{
    delete _pdoc;
    this->_pdoc = 0;
}

DocHandle& DocHandle::parse(const std::string& xml_str)
{
    this->clear();
    sss::xml3::xml_parser xp;
    this->_pdoc = dynamic_cast<sss::xml3::xml_doc*>(xp.parse(xml_str));
    return *this;
}

node * DocHandle::parseSubtree(const std::string& xml_str)
{
    if (!this->_pdoc) {
        this->_pdoc = new sss::xml3::xml_doc;
    }
    return this->_pdoc->create_subtree(xml_str);
}

// TODO FIXME 是否应该封装解析和创建xml_doc对象的过程？
//
// 因为：
// 1. 操作基本；
// 2. 规范行为
//
// 所谓规范行为，主要问题在于，从提取文本，到解析，到生成xml树形结构，如果出错怎么办？
// 首先，需要明确，上述流程不是所有情况，它只不过是从外存创建对象树的过程；还有
// 从内存中，自己构造；或者直接解析传过来的字符串；
//
// 即，如果数据是来自外存中，那么，如果读取错误（比如源文件不存在、没权限而不可
// 读），那么，程序本身是没能力，也不应该去解决；只能完全保管这个问题，让软件开
// 发者确定之后，再报告给使用者，走这个流程。
// 即，这是一个不可恢复错误；
//
// 如果在解析这里出问题怎么办？
// 这里出错，还有好几种情况；
// 1. 空白的文件； -> 可以返回一个空白的xml_doc指针，然后用户可以选择往里面塞东西；
// 2. 错误的，非xml的，比如图片或者文本；
// 3. 部分成功；即，解析不完整；
// 4. 还有后续
//
// 分别代表，4种异常……
//
// 线段树 XMLSegment
//
// 解析不完整，线段树还可用吗？比如，遇到错误，还未闭合？我需要补上闭合吗？
//
// 我是二段式解析；
//
// 感觉只有，先对象，然后解析内部的方式，才能保证不完全构建xml树；
//
// 所以，我还是暂时别管不完全构造吧！
//
// 需要的异常，应该从哪里继承呢？
// logic_error
//      是程序设计上出错了——相当于程序函数之间，不能满足各自的协议了；返回值
//      、参数什么的，出现了不应该的值；
//
//      即，运行结果(中间结果)，与期望不符。
//
// runtime_error
//      程序员可能预料到，但无法解决的(比如，未考虑到内存用尽、线程用尽等等)问
//      题。与logic_error是不同的……
//
// 可见，上述，我分析出来的xml解析相关发生的异常，都应该归于runtime_error。
DocHandle& DocHandle::load(const std::string& fname)
{
    // SSS_LOG_FUNC_TRACE(sss::log::log_ERROR);

    std::string full_name = sss::path::full_of_copy(fname);
    sss::path_type t = sss::path::file_exists(full_name);
    if (_openmode & std::ios_base::in && t) {
        switch (t) {
        case sss::PATH_TO_FILE:
            if (sss::path::filereadable(full_name)) {
                try {
                    std::string xml_str;
                    sss::path::file2string(full_name, xml_str);
                    this->parse(xml_str);
                }
                catch (sss::xml3::ParsingError& e) {
                    std::cerr << e.what() << std::endl;
                    this->clear();
                    this->_pdoc = new xml3::xml_doc;
                }
                catch (sss::xml3::ConstructingError& e) {
                    std::cerr << e.what() << std::endl;
                    this->clear();
                    this->_pdoc = new xml3::xml_doc;
                }
                catch (sss::ExceptionNotSupportMethod& e) {
                    std::cerr << e.what() << std::endl;
                    this->clear();
                    this->_pdoc = new xml3::xml_doc;
                }
                catch (...) {
                    std::cerr
                        << "unexpect error " << __func__ << ":"
                        << __FILE__ << "," << __LINE__
                        << std::endl;
                    throw;
                }
            }
            else {
                std::ostringstream oss;
                oss << "Unable to open file `" << full_name << "` to read!" << std::endl;
                throw std::runtime_error(oss.str());
            }
            break;

        case sss::PATH_TO_DIRECTORY:
            {
                std::ostringstream oss;
                oss << "Path `" << full_name << "` is a directory! cannot read" << std::endl;
                throw std::runtime_error(oss.str());
            }
            break;

        default:
            break;
        }
    }
    else if (_openmode & std::ios_base::out) {
        this->_pdoc = new xml3::xml_doc;
    }

    if (this->_pdoc) {
        this->_fname = full_name;
    }
    return *this;
}

DocHandle& DocHandle::save(const std::string& fname)
{
    // FIXME
    // 文件名路径比较……
    if (fname == this->_fname) {
        return this->save();
    }
    else if (!fname.empty() && this->_pdoc && this->_pdoc->root()) {
        this->_pdoc->save(fname);
    }
    return *this;
}

DocHandle& DocHandle::save()
{
    if (!this->_fname.empty() && this->_pdoc && this->_pdoc->root() && (this->_openmode & std::ios_base::out)) {
        this->_pdoc->save(_fname);
    }
    return *this;
}

const DocHandle& DocHandle::write(const std::string& fname, const char * sep) const
{
    if (this->_pdoc) {
        this->_pdoc->write(fname, sep);
    }
    return *this;
}

const DocHandle& DocHandle::print(std::ostream& o) const
{
    if (this->_pdoc) {
        this->_pdoc->print(o);
    }
    return *this;
}

EleHandle DocHandle::locate(const std::string& path) const
{
    return EleHandle(this->_pdoc ? this->_pdoc->locate(path) : 0);
}

EleHandle DocHandle::root() const
{
    return EleHandle(this->_pdoc->root());
}

DocHandle& DocHandle::root(const std::string& data)
{
    if (this->_pdoc && !this->_pdoc->root()) {
        this->_pdoc->root(data);
    }

    return *this;
}

    }
}
