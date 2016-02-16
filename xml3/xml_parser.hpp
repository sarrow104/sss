//#ifndef  __XML_PARSER_HPP_1437717812__
//#define  __XML_PARSER_HPP_1437717812__
//
//#define _USING_tmplateSegmentXML_
//#ifndef  _USING_tmplateSegmentXML_
//#include "xml_parser.hpp.ok"
//#else
//#include "xml_parser.hpp.tmplateSegmentXML"
//#endif
//
//
//#endif  /* __XML_PARSER_HPP_1437717812__ */

// 关于类族的讨论
//
// TinyXML2 用的是document节点，代表解析得到的xml树；注意，每一个节点，都含有一
// 个_document指针，用来指向该节点。
//
// 我呢，觉得以document类型为根节点的方式，有些别扭——毕竟，节点类型不同，不方
// 便将节点，从一棵树挪往另外一棵xml树；
//
// 既然每个节点都有_document指针，那么，我干脆，就不再专门有document节点；我直
// 接让第一个节点，代表根节点，然后读取其_document指针，所指向的数据即可。
//
// 以获取 <?xml version="1.0" encoding="utf8"?>，<!DOCTYPE ...>等信息。这些信息
// ，本来就是属于整个xml树的；至于只有在文件打印的时候，输出在开头，只不过是基
// 于关键信息，越早知道越好的考虑。
//
// 那么，要实现只输出一次document信息的方法，很简单。使用print_impl函数，传入一
// 个"递归"深度信息即可；
//
// 还有，关于Visitor模式；Visitor有几个关键点：
//
// 1. 类族；
// 2. 动态派送；
//
// 至于写法，访问者的动作是visit;而被访问方是accept；
//
// 如何实现Visitor模式的模板方法？
//
// 要点中，类族，肯定是用户设计的；作为库提供者，最多像Qt那样，提供一个基类；并
// 且，访问者模式，是基类，完成按类型派送方法；
//
// 像 TinyXML2 那样，TinyXML2库的使用者，无法是根据已经提供的Visitor基类，完成
// 其他的Visitor实体类；
//
// C++ visitor模式的限制在于，基类不知道子类有哪些；于是，枚举子类，进行派送，
// 变得不可能。
//
// 同样，也没法动态检测某类，是否可以调用某一个函数。
//
// 还有，TinyXML2中，Attribute 与 Node基类，到底有啥方法的异同？Element呢？
//
// TinyXML2中
//
//  XMLDocument doc;
//  doc.Parse(xml);
//  ...
//  XMLPrinter printer1(0, true);
//  doc.Accept(&printer1);
//
// 是通过具体的子类，XMLDocument，来调用Accept方法，于是，就不用额外写派送的代码；
//
// 如果，是通过基类指针，则需要根据基类指针，写一个根据类型派送的方法；

#ifndef  __XML_PARSER_HPP_1400994344__
#define  __XML_PARSER_HPP_1400994344__

#include "xml_node.hpp"
#include "xml_doc.hpp"

#include <sss/stream/skipper.hpp>
#include <sss/container/dtree.hpp>

#include <list>
#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>

// NOTE 测试用！
#define protected public

// 解析xml文件；并生成一个xml_doc对象或者xml3::node * 对象；
namespace sss {
    namespace xml3 {

        class xml_parser {

        public:
            typedef sss::stream::skipper<std::string::const_iterator> skipper_t;
            typedef sss::stream::Rewinder<std::string::const_iterator> Rewinder_t;

        public:
            class falldown_name_table : private std::map<std::string, int>{
            public:
                typedef std::map<std::string, int> value_type;

                    enum fd_f_value {
                        PARSE_BOM = 1,
                        PARSE_DOCUMENT,
                        PARSE_CHAR,
                        PARSE_S,
                        PARSE_NAMESTARTCHAR,
                        PARSE_NAMECHAR,
                        PARSE_NAME,
                        PARSE_NAMES,
                        PARSE_NMTOKEN,
                        PARSE_NMTOKENS,
                        PARSE_ENTITYVALUE,
                        PARSE_ATTVALUE,
                        PARSE_SYSTEMLITERAL,
                        PARSE_PUBIDLITERAL,
                        PARSE_PUBIDCHAR,
                        PARSE_CHARDATA,
                        PARSE_COMMENT,
                        PARSE_PI,
                        PARSE_PITARGET,
                        PARSE_CDSECT,
                        PARSE_CDSTART,
                        PARSE_CDATA,
                        PARSE_CDEND,
                        PARSE_PROLOG,
                        PARSE_XMLDECL,
                        PARSE_VERSIONINFO,
                        PARSE_EQ,
                        PARSE_VERSIONNUM,
                        PARSE_MISC,
                        PARSE_DOCTYPEDECL,
                        PARSE_DECLSEP,
                        PARSE_INTSUBSET,
                        PARSE_MARKUPDECL,
                        PARSE_EXTSUBSET,
                        PARSE_EXTSUBSETDECL,
                        PARSE_SDDECL,
                        PARSE_ELEMENT,
                        PARSE_STAG,
                        PARSE_ATTRIBUTE,
                        PARSE_ETAG,
                        PARSE_CONTENT,
                        PARSE_ATTRIBLIST,
                        PARSE_EMPTYELEMTAG,
                        PARSE_ELEMENTDECL,
                        PARSE_CONTENTSPEC,
                        PARSE_CHILDREN,
                        PARSE_CP,
                        PARSE_TIMES_SUFFIX,
                        PARSE_CHOICE,
                        PARSE_SEQ,
                        PARSE_MIXED,
                        PARSE_MIXED_SUFFIX,
                        PARSE_ATTLISTDECL,
                        PARSE_ATTDEF,
                        PARSE_ATTTYPE,
                        PARSE_STRINGTYPE,
                        PARSE_TOKENIZEDTYPE,
                        PARSE_ENUMERATEDTYPE,
                        PARSE_NOTATIONTYPE,
                        PARSE_ENUMERATION,
                        PARSE_DEFAULTDECL,
                        PARSE_CONDITIONALSECT,
                        PARSE_INCLUDESECT,
                        PARSE_IGNORESECT,
                        PARSE_IGNORESECTCONTENTS,
                        PARSE_IGNORE,
                        PARSE_CHARREF,
                        PARSE_REFERENCE,
                        PARSE_ENTITYREF,
                        PARSE_PEREFERENCE,
                        PARSE_ENTITYDECL,
                        PARSE_GEDECL,
                        PARSE_PEDECL,
                        PARSE_ENTITYDEF,
                        PARSE_PEDEF,
                        PARSE_EXTERNALID,
                        PARSE_NDATADECL,
                        PARSE_TEXTDECL,
                        PARSE_EXTPARSEDENT,
                        PARSE_ENCODINGDECL,
                        PARSE_ENCNAME,
                        PARSE_NOTATIONDECL,
                        PARSE_PUBLICID,
                        PARSE_LETTER,
                        PARSE_BASECHAR,
                        PARSE_IDEOGRAPHIC,
                        PARSE_COMBININGCHAR,
                        PARSE_DIGIT,
                        PARSE_EXTENDER,
                        PARSE_SDDECL_VALUE
                    };

            private:
                falldown_name_table();

                ~falldown_name_table()
                {
                }

            public:
                static falldown_name_table& get_instance() {
                    static falldown_name_table single;
                    return single;
                }
                static int conv(const std::string& val) {
                    value_type::const_iterator it = get_instance().find(val);
                    if (it != get_instance().end()) {
                        return it->second;
                    }
                    else {
                        return 0;
                    }
                }

                static const std::string& conv(int val) {
                    value_type & instance = get_instance();
                    static std::string not_find = "NULL";
                    for (value_type::const_iterator it = instance.begin();
                         it != instance.end();
                         ++it)
                    {
                        if (it->second == val) {
                            return it->first;
                        }
                    }
                    return not_find;
                }
            };

        public:
            // NOTE 这并不是真正意义上的线段树；
            // 按理说，此处的线段树，应该是这个样子：
            // 根节点，关键元素，存储开始、结束位置；然后data域，存放函数名；
            // 就是说，按照sql的关键字说法，是开始、结束、以及函数名，共同构成关键字！
            // 然后，开始、结束，构成包含关系；首先，我假设，不存在完全相同的范围；
            // 于是：
            //
            // 可以构建比较关系吗，两个表示范围的整数？
            //
            // 这样的，我觉得，可以在递归下降的时候，就构造这个所谓的"线段树"。
            // 进入函数的时候，其实"线段树"父亲，就确定了；当然，还有开始位置，
            // 只不过，结束位置，只有等到函数退出的时候，才知道；
            //! 线段XML
            //
            // TODO FIXME
            //
            // 这个线段树，还是保存 std::string& , start_pos, end_pos 的好；
            // 这样，对xml文件的部分替换，也可以实现。
            class SegmentXML;
            typedef sss::can::Handle<SegmentXML> SegHandle;

            class SegmentXML : public sss::can::dtree<SegmentXML> {
                typedef sss::can::dtree<SegmentXML> Base_t;
                friend class sss::can::Handle<SegmentXML>;

            public:
                SegmentXML(const char * func_name,
                           std::string::const_iterator sbeg,
                           int _start, int _end = -1)
                    : func_id(falldown_name_table::conv(func_name)),
                      s_beg(sbeg),
                      start(_start),
                      end(_end),
                      ws_bytes_before(0),
                      ws_bytes_after(0),
                      is_ok(false)
                {
                    check_status();
                }

                SegmentXML()
                    : func_id(0), start(-1), end(-1), is_ok(false)
                {
                    check_status();
                }

                ~SegmentXML()
                {
                    this->clear_children();
                }

            public:
                void validator(int type) {
                    if (this->get_func() != type) {
                        SSS_LOG_ERROR("expect \"%s\"; but \"%s\"; at %p\n",
                                      xml_parser::falldown_name_table::conv(type).c_str(),
                                      xml_parser::falldown_name_table::conv(this->get_func()).c_str(),
                                      this);
                        this->print(std::cout, this->s_beg);
                    }
                    assert(this->get_func() == type);
                    assert(this->get_end() != -1);
                }

                void trim()
                {
                    while (this->start != this->end &&
                           std::isspace(*(this->s_beg + this->start))) {
                        this->start ++;
                        this->ws_bytes_before ++;
                    }
                    while (this->start != this->end &&
                           std::isspace(*(this->s_beg + this->end - 1))) {
                        this->end --;
                        this->ws_bytes_after ++;
                    }
                }

                bool empty()
                {
                    return this->start == this->end;
                }

                void check_status()
                {
                    is_ok = (!func_id && start != -1 && end != -1);
                }

                bool set_start(int _start)
                {
                    bool ret = is_offset_ok(_start) && !is_offset_ok(this->start);
                    if (ret) {
                        this->start = _start;
                    }
                    return ret;
                }

                bool set_end(int _end)
                {
                    bool ret = is_offset_ok(_end) && !is_offset_ok(this->end);
                    if (ret) {
                        this->end = _end;
                    }
                    return ret;
                }

                int get_end() const
                {
                    return this->end;
                }
                int get_start() const
                {
                    return this->start;
                }

                bool set_func(int _id)
                {
                    bool ret = !this->func_id && (_id > 0);
                    if (ret) {
                        this->func_id = _id;
                    }
                    return ret;
                }

                bool set_func(const char * func_name)
                {
                    return this->set_func(falldown_name_table::conv(func_name));
                }

                int get_func() const
                {
                    return this->func_id;
                }

                std::string get_func_name() const
                {
                    return xml_parser::falldown_name_table::conv(this->get_func());
                }

                SegmentXML * set_parent(SegmentXML * p)
                {
                    return this->Base_t::parent(p);
                }

                void print(std::ostream& out, std::string::const_iterator s_beg);

                std::string get_slice() const
                {
                    return std::string(this->s_beg + this->start,
                                       this->s_beg + this->end);
                }

                std::string get_slice(int beg_off, int end_off) const
                {
                    return std::string(this->s_beg + this->start + beg_off,
                                       this->s_beg + this->end - end_off);
                }

                std::string get_sample() const
                {
                    return std::string(this->s_beg + this->start, this->s_beg + this->start + 20) + "...";
                }

                void string_slice_print(std::ostream& out, std::string::const_iterator s_beg);

                SegmentXML * get_parent()
                {
                    return this->Base_t::parent();
                }

                bool remove_child(SegmentXML * child)
                {
                    return this->Base_t::removeChild(child);
                }

                SegmentXML * first_child()
                {
                    return this->Base_t::firstChild();
                }
                SegmentXML * last_child()
                {
                    return this->Base_t::lastChild();
                }

                SegmentXML * nextSibling()
                {
                    return this->Base_t::nextSibling();
                }

                SegmentXML * prevSibling()
                {
                    return this->Base_t::prevSibling();
                }

                void clear()
                {
                    this->func_id = 0;
                    this->start = -1;
                    this->end = -1;
                }

                bool append(SegmentXML * child)
                {
                    return this->Base_t::appendChild(child, 0);
                }

                bool pop()
                {
                    bool ret = false;
                    if (this->lastChild()) {
                        this->Base_t::removeChild(this->lastChild());
                        ret = true;
                    }
                    return ret;
                }

                int size()
                {
                    return this->Base_t::size();
                }

                int clear_children()
                {
                    int ret = this->size();
                    this->Base_t::clear();
                    return ret;
                }

                bool find(SegmentXML * child)
                {
                    return this->Base_t::isChild(child);
                }

            protected:
                static bool is_offset_ok(int _offset)
                {
                    return _offset != -1;
                }

            protected:
                int     func_id;
                std::string::const_iterator s_beg;
                int     start;
                int     end;
                int     ws_bytes_before;
                int     ws_bytes_after;
                bool    is_ok;
            };

            // 目的，用于falldown函数中，在函数开始和结尾的时候，在SegmentXML
            // 树中，注册当前函数的调用信息——
            // 它需要知道：
            // 1. 调用函数的名字；通过 __func__ 宏；
            // 2. 匹配开始的位置；通过 s_beg;
            // 3. 匹配结束的位置；还是通过 s_beg;
            // 并且，在当前选路失败的情况下，还可以从解析树中，删除该
            // SegmentXML对象；
            // 另外，匹配是否成功、匹配的开始结束位置，都可以通过 Rewinder_t
            // r(s_beg)对象，获取到；
            // 剩下的，就是这个 SegmentXML树的信息；鉴于解析，都是从
            // parse_document函数开始的；可以先从这里把树建立起来。
            //
            // 即，xml_parser需要一个SegmentXML的跟节点指针——初始值，0；
            // 接着，xml_parser需要一个指向当前工作SegmentXML的指针（父
            // SegmentXML对象）；
            //
            // 就是说，SegmentXML_Helper构造函数，会往父指针下面，添加一个新节点；
            // 它需要：
            // 1. xml_parser 指针一枚；
            // 2. ...
            class SegmentXML_Helper
            {
            public:
                SegmentXML_Helper(xml_parser*              _xp, //
                                  const char *             _func_name,
                                  xml_parser::Rewinder_t & _r);
                ~SegmentXML_Helper();

            public:
                // NOTE 本函数调用时机，在~SegmentXML_Helper()之前
                // 构造函数，要么创建一个seg，要么从pathMem中获取一个走过的路径；
                // 此时，有三种情况：
                // 1. seg == 0
                // 说明路径走过，并且失败；此时不应当new SegmentXML 和 插入seg树；
                // 即，SegmentXML_Helper()和~SegmentXML_Helper()实际都不操作；
                // 2. seg != 0 && seg->get_end() == -1
                // 说明，这个seg，是new来的！需要 ~SegmentXML_Helper() 完成
                // SegmentXML::set_end() 操作。
                // 3. seg != 0 && seg->get_end() != -1
                // 说明，这个seg是从memPath弄过来的。此时，构造函数应当已经完成
                // 了seg树的操作，以及Rewinder_t r的操作。
                // 无需 ~SegmentXML_Helper() 。
                //
                // 综合考虑，本函数，其实只有其析构函数在使用……
                bool isOK() const {
                    return !this->seg || this->seg->get_end() != -1;
                }

            private:
                xml_parser  * xp;
                SegmentXML * seg;
                xml_parser::Rewinder_t & r;
            };

            class XMLpathMemKey : private std::pair<int, int>
            {
            public:
                typedef std::pair<int, int> Base_t;

            public:
                XMLpathMemKey(int func_id, int offset)
                    : Base_t(func_id, offset)
                {
                }

                XMLpathMemKey()
                    : Base_t(0, 0)
                {
                }

                XMLpathMemKey(const XMLpathMemKey& k)
                    : Base_t(k.funcID(), k.offset())
                {
                }

                XMLpathMemKey & operator = (const XMLpathMemKey& k)
                {
                    if (this != &k) {
                        this->funcID(k.funcID());
                        this->offset(k.offset());
                    }
                    return *this;
                }

                bool operator > (const XMLpathMemKey& k) const {
                    if (this->funcID() > k.funcID()) {
                        return true;
                    }
                    else if (this->funcID() == k.funcID()) {
                        return this->offset() > k.offset();
                    }
                    else {
                        return false;
                    }
                }

                void validAssert() const
                {
                    assert(this->funcID());
                }

                bool operator < (const XMLpathMemKey& k) const {
                    return k.operator>(*this);
                }

                bool operator == (const XMLpathMemKey& k) const {
                    return Base_t(*this) == Base_t(k);
                }

            public:
                int funcID() const
                {
                    return this->Base_t::first;
                }
                int funcID(int id)
                {
                    std::swap(id, this->Base_t::first);
                    return id;
                }

                int offset() const
                {
                    return this->Base_t::second;
                }
                int offset(int off)
                {
                    std::swap(off, this->Base_t::second);
                    return off;
                }
            };

            // 只记录错误的路径，和错误路径下面，可以重用的部分；
            //
            // std::map<XMLpathMemKey, SegmentXML*>::value_type;
            // - first 表示调用情况
            // - second == 0 表示之前的失败了。
            //          != 0 表示可以复用；
            class XMLpathMem: private std::map<XMLpathMemKey, SegmentXML*>
            {
            public:
                typedef std::map<XMLpathMemKey, SegmentXML*> value_type;

            public:
                XMLpathMem()
                {
                }

                ~XMLpathMem()
                {
                }

            public:
                void clear();
                bool add(const XMLpathMemKey&, SegmentXML*) throw(const char *);
                bool has_trival(const XMLpathMemKey&, SegmentXML *&);
                bool usePath(const XMLpathMemKey&);
            };

            // 本质上，是将传入的字符串，解释、并分析，最后组建一个xml3::node 对
            // 象；
            // NOTE 可以是xml_doc类型；
            // 那么，就需要进行分析；
            //
            // 首先，分析是否是bom；
            // 此时，需要预读3个字节；以判断bom；
            // 然后，尝试读取<?..?>标记；（以"<?"开头，并且以"?>"结尾，忽略空白字符）
            // 接着，就开始解析<...>标签；此处，使用递归完成即可；

        public:
            xml_parser();
            ~xml_parser();

        public:
            void clear();

        public:
            // 将传入的字符串，按xml方式解析，并生成到 传入的 xml_doc 对象上
            // 并返回该对象
            xml3::node * parse(std::string::const_iterator s_ini,
                               std::string::const_iterator s_fin);

            xml3::node * parse(const std::string&);

            //! 关于，从字符串，生成xml子树的需求分析；
            // 有些时候，（比如动态java；返回字符串，再根据字符串生成子树，再整体插入）
            // 此时，我需要让xml_parser知道，我已经有xml_doc 对象了！
            // 当然，也可以有如下策略：
            // parse_document(s_beg, s_end)
            // ...
            // if seg->first == PARSE_PROLOG
            //    swap() // 或者，改用handle，然后整个替换？否则，需要处理IDmap_t对象等；
            // else if seg->first == PARSE_ELEMENT
            //    set_p_doc = xml_doc * ref;
            //    Seg2XML_element(); // 貌似不能是Seg2XML_content 开头！因为需要一个“子树根节点”
            // else
            //    wrong！
            // 第一种情况，此时，会有两棵树：老的树，指向旧的
            // xml_doc，那么新的树呢？
            //
            // 算了，简单起见，还是从element开始解析吧！
            //
            // 为避免歧义，函数名定位：parseSubtree()，即，子树解析
            xml3::node * parseSubtree(xml_doc * doc, const std::string& str)
            {
                return this->parseSubtree(doc, str.begin(), str.end());
            }

            xml3::node * parseSubtree(xml_doc *,
                                      std::string::const_iterator s_ini,
                                      std::string::const_iterator s_fin);

        protected:
            // 完成转换；
            xml3::node * Seg2XML(SegmentXML * seg_root); // == Seg2XML_document()
            xml3::node * Seg2XMLsubtree(xml3::xml_doc * pdoc, xml_parser::SegmentXML * seg_root);

            bool        Seg2XML_prolog(SegmentXML *& seg);
            bool        Seg2XML_element(SegmentXML *& seg);
            bool        Seg2XML_Misc(SegmentXML *& seg);
            bool        Seg2XML_content(SegmentXML *& seg);
            // NOTE 非引用！
            bool        Seg2XML_AttribList(SegmentXML * seg);

        protected:
            // 解析bom文件头；返回消耗的字节数
            int    parse_bom(std::string::const_iterator& s_beg,
                             std::string::const_iterator s_end);

        protected:
            // document    ::= prolog element Misc*
            int parse_document(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // Char       ::=          0x9 | 0xA | 0xD | [0x20-0xD7FF] | [0xE000-0xFFFD] | [0x10000-0x10FFFF]
            /* any Unicode character, excluding the surrogate blocks, FFFE, and FFFF. */
            int parse_Char(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // NOTE 以下函数的返回值，都表示，匹配的时候，消耗的字节数；
            // S           ::=  (0x20 | 0x9 | 0xD | 0xA)+
            int parse_S(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            //NameStartChar    ::=   ":" | [A-Z] | "_" | [a-z] | [0xC0-0xD6] | [0xD8-0xF6] | [0xF8-0x2FF] | [0x370-0x37D]
            //                      | [0x37F-0x1FFF] | [0x200C-0x200D] | [0x2070-0x218F] | [0x2C00-0x2FEF] | [0x3001-0xD7FF]
            //                    | [0xF900-0xFDCF] | [0xFDF0-0xFFFD] | [0x10000-0xEFFFF]
            int parse_NameStartChar(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            //NameChar   ::=   NameStartChar | "-" | "." | [0-9] | 0xB7 | [0x0300-0x036F] | [0x203F-0x2040]
            int parse_NameChar(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            //Name     ::=   NameStartChar (NameChar)*
            int parse_Name(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            //Names    ::=   Name (0x20 Name)*
            int parse_Names(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            //Nmtoken          ::=   (NameChar)+
            int parse_Nmtoken(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            //Nmtokens   ::=   Nmtoken (0x20 Nmtoken)*
            int parse_Nmtokens(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // EntityValue   ::=   '"' ([^%&"] | PEReference | Reference)* '"'
            //                  |  "'" ([^%&'] | PEReference | Reference)* "'"
            int parse_EntityValue(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // AttValue        ::=   '"' ([^<&"] | Reference)* '"'
            //                       |  "'" ([^<&'] | Reference)* "'"
            int parse_AttValue(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // SystemLiteral           ::=   ('"' [^"]* '"') | ("'" [^']* "'")
            int parse_SystemLiteral(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // PubidLiteral    ::=   '"' PubidChar* '"' | "'" (PubidChar - "'")* "'"
            int parse_PubidLiteral(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // PubidChar       ::=   0x20 | 0xD | 0xA | [a-zA-Z0-9] | [-'()+,./:=?;!*#@$_%]
            int parse_PubidChar(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // CharData   ::=   [^<&]* - ([^<&]* ']]>' [^<&]*)
            int parse_CharData(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // Comment         ::=   '<!--' ((Char - '-') | ('-' (Char - '-')))* '-->'
            int parse_Comment(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // PI      ::=   '<?' PITarget (S (Char* - (Char* '?>' Char*)))? '?>'
            int parse_PI(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // PITarget   ::=   Name - (('X' | 'x') ('M' | 'm') ('L' | 'l'))
            int parse_PITarget(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // CDSect          ::=   CDStart CData CDEnd
            int parse_CDSect(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // CDStart         ::=   '<![CDATA['
            int parse_CDStart(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // CData           ::=   (Char* - (Char* ']]>' Char*))
            int parse_CData(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // CDEnd           ::=   ']]>'
            int parse_CDEnd(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // prolog          ::=   XMLDecl? Misc* (doctypedecl Misc*)?
            int parse_prolog(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // XMLDecl         ::=   '<?xml' VersionInfo EncodingDecl? SDDecl? S? '?>'
            int parse_XMLDecl(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // VersionInfo  ::= S 'version' Eq ("'" VersionNum "'" | '"' VersionNum '"')
            int parse_VersionInfo(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // Eq      ::=   S? '=' S?
            int parse_Eq(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // VersionNum   ::= '1.' [0-9]+
            int parse_VersionNum(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // Misc    ::=   Comment | PI | S
            int parse_Misc(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // doctypedecl   ::=  '<!DOCTYPE' S Name (S ExternalID)? S? ('[' intSubset ']' S?)? '>'
            int parse_doctypedecl(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // DeclSep         ::=     PEReference | S
            int parse_DeclSep(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // intSubset   ::=    (markupdecl | DeclSep)*
            int parse_intSubset(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // markupdecl   ::=   elementdecl | AttlistDecl | EntityDecl | NotationDecl | PI | Comment
            int parse_markupdecl(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // extSubset       ::=          TextDecl? extSubsetDecl
            int parse_extSubset(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // extSubsetDecl           ::=          ( markupdecl | conditionalSect | DeclSep)*
            int parse_extSubsetDecl(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // SDDecl          ::=          S 'standalone' Eq SDDecl_Value
            int parse_SDDecl(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // SDDecl_Value    ::= ("'" ('yes' | 'no') "'") | ('"' ('yes' | 'no') '"')
            int parse_SDDecl_Value(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // element         ::=          EmptyElemTag
            //              | STag content ETag
            int parse_element(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // STag    ::=          '<' Name (S Attribute)* S? '>'
            // ==>
            // STag    ::=          '<' Name AttribList? S? '>'
            int parse_STag(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // Attribute       ::=          Name Eq AttValue
            int parse_Attribute(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // ETag       ::=          '</' Name S? '>'
            int parse_ETag(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // content    ::=          CharData? ((element | Reference | CDSect | PI | Comment) CharData?)*
            int parse_content(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // AttribList       ::=          (S Attribute)*
            int parse_AttribList(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // EmptyElemTag       ::=          '<' Name (S Attribute)* S? '/>'
            // ==>
            // EmptyElemTag       ::=          '<' Name AttribList? S? '/>'
            int parse_EmptyElemTag(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // elementdecl        ::=          '<!ELEMENT' S Name S contentspec S? '>'
            int parse_elementdecl(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // contentspec        ::=          'EMPTY' | 'ANY' | Mixed | children
            int parse_contentspec(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // children   ::=          (choice | seq) ('?' | '*' | '+')?
            int parse_children(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // cp         ::=          (Name | choice | seq) cp_suffix
            int parse_cp(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // cp_suffix ::= ('?' | '*' | '+')?
            int parse_Times_suffix(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // choice     ::=          '(' S? cp ( S? '|' S? cp )+ S? ')'
            int parse_choice(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // seq        ::=          '(' S? cp ( S? ',' S? cp )* S? ')'
            int parse_seq(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // Mixed      ::=   '(' S? '#PCDATA' (S? '|' S? Name)* S? ')' Mixed_suffix
            //                 | '(' S? '#PCDATA' S? ')'
            int parse_Mixed(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // Mixed_suffix ::= '*'
            int parse_Mixed_suffix(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // AttlistDecl   ::=       '<!ATTLIST' S Name AttDef* S? '>'
            int parse_AttlistDecl(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // AttDef     ::=          S Name S AttType S DefaultDecl
            int parse_AttDef(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // AttType    ::=          StringType | TokenizedType | EnumeratedType
            int parse_AttType(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // StringType         ::=          'CDATA'
            int parse_StringType(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // TokenizedType      ::=          'ID'
            //                 | 'IDREF'
            //                 | 'IDREFS'
            //                 | 'ENTITY'
            //                 | 'ENTITIES'
            //                 | 'NMTOKEN'
            //                 | 'NMTOKENS'
            int parse_TokenizedType(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // EnumeratedType     ::=          NotationType | Enumeration
            int parse_EnumeratedType(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // NotationType       ::=          'NOTATION' S '(' S? Name (S? '|' S? Name)* S? ')'
            int parse_NotationType(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // Enumeration        ::=          '(' S? Nmtoken (S? '|' S? Nmtoken)* S? ')'
            int parse_Enumeration(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // DefaultDecl   ::=       '#REQUIRED' | '#IMPLIED'
            //                 | (('#FIXED' S)? AttValue)
            int parse_DefaultDecl(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // conditionalSect    ::=          includeSect | ignoreSect
            int parse_conditionalSect(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // includeSect        ::=          '<![' S? 'INCLUDE' S? '[' extSubsetDecl ']]>'
            int parse_includeSect(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // ignoreSect         ::=          '<![' S? 'IGNORE' S? '[' ignoreSectContents* ']]>'
            int parse_ignoreSect(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // ignoreSectContents         ::=          Ignore ('<![' ignoreSectContents ']]>' Ignore)*
            int parse_ignoreSectContents(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // Ignore     ::=          Char* - (Char* ('<![' | ']]>') Char*)
            int parse_Ignore(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // CharRef    ::=   '&#' [0-9]+ ';'
            //                 | '&0x' [0-9a-fA-F]+ ';'
            int parse_CharRef(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // Reference          ::=          EntityRef | CharRef
            int parse_Reference(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // EntityRef          ::=          '&' Name ';'
            int parse_EntityRef(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // PEReference        ::=          '%' Name ';'
            int parse_PEReference(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // EntityDecl ::=   GEDecl | PEDecl
            int parse_EntityDecl(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // GEDecl     ::=   '<!ENTITY' S Name S EntityDef S? '>'
            int parse_GEDecl(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // PEDecl     ::=   '<!ENTITY' S '%' S Name S PEDef S? '>'
            int parse_PEDecl(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // EntityDef  ::=   EntityValue | (ExternalID NDataDecl?)
            int parse_EntityDef(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // PEDef      ::=   EntityValue | ExternalID
            int parse_PEDef(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // ExternalID  ::=   'SYSTEM' S SystemLiteral
            //         | 'PUBLIC' S PubidLiteral S SystemLiteral
            int parse_ExternalID(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // NDataDecl   ::=   S 'NDATA' S Name
            int parse_NDataDecl(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // TextDecl   ::=   '<?xml' VersionInfo? EncodingDecl S? '?>'
            int parse_TextDecl(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // extParsedEnt   ::=   TextDecl? content
            int parse_extParsedEnt(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // EncodingDecl   ::=   S 'encoding' Eq ('"' EncName '"' | "'" EncName "'" )
            int parse_EncodingDecl(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // EncName   ::=   [A-Za-z] ([A-Za-z0-9._] | '-')* /* Encoding name contains only Latin characters */
            int parse_EncName(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // NotationDecl       ::=          '<!NOTATION' S Name S (ExternalID | PublicID) S? '>'
            int parse_NotationDecl(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // PublicID           ::=          'PUBLIC' S PubidLiteral
            int parse_PublicID(std::string::const_iterator& s_beg, std::string::const_iterator s_end);

            // Letter     ::=   BaseChar | Ideographic
            int parse_Letter(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // BaseChar   ::=   [0x0041-0x005A] | [0x0061-0x007A] | [0x00C0-0x00D6] | [0x00D8-0x00F6] | [0x00F8-0x00FF]
            //                | [0x0100-0x0131] | [0x0134-0x013E] | [0x0141-0x0148] | [0x014A-0x017E] | [0x0180-0x01C3]
            //                | [0x01CD-0x01F0] | [0x01F4-0x01F5] | [0x01FA-0x0217] | [0x0250-0x02A8] | [0x02BB-0x02C1]
            //                | 0x0386 | [0x0388-0x038A] | 0x038C | [0x038E-0x03A1] | [0x03A3-0x03CE] | [0x03D0-0x03D6]
            //                | 0x03DA | 0x03DC | 0x03DE | 0x03E0 | [0x03E2-0x03F3] | [0x0401-0x040C] | [0x040E-0x044F]
            //                | [0x0451-0x045C] | [0x045E-0x0481] | [0x0490-0x04C4] | [0x04C7-0x04C8] | [0x04CB-0x04CC]
            //                | [0x04D0-0x04EB] | [0x04EE-0x04F5] | [0x04F8-0x04F9] | [0x0531-0x0556] | 0x0559 | [0x0561-0x0586]
            //                | [0x05D0-0x05EA] | [0x05F0-0x05F2] | [0x0621-0x063A] | [0x0641-0x064A] | [0x0671-0x06B7]
            //                | [0x06BA-0x06BE] | [0x06C0-0x06CE] | [0x06D0-0x06D3] | 0x06D5 | [0x06E5-0x06E6] | [0x0905-0x0939]
            //                | 0x093D | [0x0958-0x0961] | [0x0985-0x098C] | [0x098F-0x0990] | [0x0993-0x09A8] | [0x09AA-0x09B0]
            //                | 0x09B2 | [0x09B6-0x09B9] | [0x09DC-0x09DD] | [0x09DF-0x09E1] | [0x09F0-0x09F1] | [0x0A05-0x0A0A]
            //                | [0x0A0F-0x0A10] | [0x0A13-0x0A28] | [0x0A2A-0x0A30] | [0x0A32-0x0A33] | [0x0A35-0x0A36]
            //                | [0x0A38-0x0A39] | [0x0A59-0x0A5C] | 0x0A5E | [0x0A72-0x0A74] | [0x0A85-0x0A8B] | 0x0A8D
            //                | [0x0A8F-0x0A91] | [0x0A93-0x0AA8] | [0x0AAA-0x0AB0] | [0x0AB2-0x0AB3] | [0x0AB5-0x0AB9]
            //                | 0x0ABD | 0x0AE0 | [0x0B05-0x0B0C] | [0x0B0F-0x0B10] | [0x0B13-0x0B28] | [0x0B2A-0x0B30]
            //                | [0x0B32-0x0B33] | [0x0B36-0x0B39] | 0x0B3D | [0x0B5C-0x0B5D] | [0x0B5F-0x0B61] | [0x0B85-0x0B8A]
            //                | [0x0B8E-0x0B90] | [0x0B92-0x0B95] | [0x0B99-0x0B9A] | 0x0B9C | [0x0B9E-0x0B9F] | [0x0BA3-0x0BA4]
            //                | [0x0BA8-0x0BAA] | [0x0BAE-0x0BB5] | [0x0BB7-0x0BB9] | [0x0C05-0x0C0C] | [0x0C0E-0x0C10]
            //                | [0x0C12-0x0C28] | [0x0C2A-0x0C33] | [0x0C35-0x0C39] | [0x0C60-0x0C61] | [0x0C85-0x0C8C]
            //                | [0x0C8E-0x0C90] | [0x0C92-0x0CA8] | [0x0CAA-0x0CB3] | [0x0CB5-0x0CB9] | 0x0CDE | [0x0CE0-0x0CE1]
            //                | [0x0D05-0x0D0C] | [0x0D0E-0x0D10] | [0x0D12-0x0D28] | [0x0D2A-0x0D39] | [0x0D60-0x0D61]
            //                | [0x0E01-0x0E2E] | 0x0E30 | [0x0E32-0x0E33] | [0x0E40-0x0E45] | [0x0E81-0x0E82] | 0x0E84
            //                | [0x0E87-0x0E88] | 0x0E8A | 0x0E8D | [0x0E94-0x0E97] | [0x0E99-0x0E9F] | [0x0EA1-0x0EA3]
            //                | 0x0EA5 | 0x0EA7 | [0x0EAA-0x0EAB] | [0x0EAD-0x0EAE] | 0x0EB0 | [0x0EB2-0x0EB3] | 0x0EBD
            //                | [0x0EC0-0x0EC4] | [0x0F40-0x0F47] | [0x0F49-0x0F69] | [0x10A0-0x10C5] | [0x10D0-0x10F6]
            //                | 0x1100 | [0x1102-0x1103] | [0x1105-0x1107] | 0x1109 | [0x110B-0x110C] | [0x110E-0x1112]
            //                | 0x113C | 0x113E | 0x1140 | 0x114C | 0x114E | 0x1150 | [0x1154-0x1155] | 0x1159 | [0x115F-0x1161]
            //                | 0x1163 | 0x1165 | 0x1167 | 0x1169 | [0x116D-0x116E] | [0x1172-0x1173] | 0x1175 | 0x119E
            //                | 0x11A8 | 0x11AB | [0x11AE-0x11AF] | [0x11B7-0x11B8] | 0x11BA | [0x11BC-0x11C2] | 0x11EB
            //                | 0x11F0 | 0x11F9 | [0x1E00-0x1E9B] | [0x1EA0-0x1EF9] | [0x1F00-0x1F15] | [0x1F18-0x1F1D]
            //                | [0x1F20-0x1F45] | [0x1F48-0x1F4D] | [0x1F50-0x1F57] | 0x1F59 | 0x1F5B | 0x1F5D | [0x1F5F-0x1F7D]
            //                | [0x1F80-0x1FB4] | [0x1FB6-0x1FBC] | 0x1FBE | [0x1FC2-0x1FC4] | [0x1FC6-0x1FCC] | [0x1FD0-0x1FD3]
            //                | [0x1FD6-0x1FDB] | [0x1FE0-0x1FEC] | [0x1FF2-0x1FF4] | [0x1FF6-0x1FFC] | 0x2126 | [0x212A-0x212B]
            //                | 0x212E | [0x2180-0x2182] | [0x3041-0x3094] | [0x30A1-0x30FA] | [0x3105-0x312C] | [0xAC00-0xD7A3]
            int parse_BaseChar(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // Ideographic   ::=   [0x4E00-0x9FA5] | 0x3007 | [0x3021-0x3029]
            int parse_Ideographic(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // CombiningChar ::=   [0x0300-0x0345] | [0x0360-0x0361] | [0x0483-0x0486] | [0x0591-0x05A1] | [0x05A3-0x05B9]
            //                | [0x05BB-0x05BD] | 0x05BF | [0x05C1-0x05C2] | 0x05C4 | [0x064B-0x0652] | 0x0670 | [0x06D6-0x06DC]
            //                | [0x06DD-0x06DF] | [0x06E0-0x06E4] | [0x06E7-0x06E8] | [0x06EA-0x06ED] | [0x0901-0x0903] | 0x093C
            //                | [0x093E-0x094C] | 0x094D | [0x0951-0x0954] | [0x0962-0x0963] | [0x0981-0x0983] | 0x09BC | 0x09BE
            //                | 0x09BF | [0x09C0-0x09C4] | [0x09C7-0x09C8] | [0x09CB-0x09CD] | 0x09D7 | [0x09E2-0x09E3] | 0x0A02
            //                | 0x0A3C | 0x0A3E | 0x0A3F | [0x0A40-0x0A42] | [0x0A47-0x0A48] | [0x0A4B-0x0A4D] | [0x0A70-0x0A71]
            //                | [0x0A81-0x0A83] | 0x0ABC | [0x0ABE-0x0AC5] | [0x0AC7-0x0AC9] | [0x0ACB-0x0ACD] | [0x0B01-0x0B03]
            //                | 0x0B3C | [0x0B3E-0x0B43] | [0x0B47-0x0B48] | [0x0B4B-0x0B4D] | [0x0B56-0x0B57] | [0x0B82-0x0B83]
            //                | [0x0BBE-0x0BC2] | [0x0BC6-0x0BC8] | [0x0BCA-0x0BCD] | 0x0BD7 | [0x0C01-0x0C03] | [0x0C3E-0x0C44]
            //                | [0x0C46-0x0C48] | [0x0C4A-0x0C4D] | [0x0C55-0x0C56] | [0x0C82-0x0C83] | [0x0CBE-0x0CC4]
            //                | [0x0CC6-0x0CC8] | [0x0CCA-0x0CCD] | [0x0CD5-0x0CD6] | [0x0D02-0x0D03] | [0x0D3E-0x0D43]
            //                | [0x0D46-0x0D48] | [0x0D4A-0x0D4D] | 0x0D57 | 0x0E31 | [0x0E34-0x0E3A] | [0x0E47-0x0E4E] | 0x0EB1
            //                | [0x0EB4-0x0EB9] | [0x0EBB-0x0EBC] | [0x0EC8-0x0ECD] | [0x0F18-0x0F19] | 0x0F35 | 0x0F37 | 0x0F39
            //                | 0x0F3E | 0x0F3F | [0x0F71-0x0F84] | [0x0F86-0x0F8B] | [0x0F90-0x0F95] | 0x0F97 | [0x0F99-0x0FAD]
            //                | [0x0FB1-0x0FB7] | 0x0FB9 | [0x20D0-0x20DC] | 0x20E1 | [0x302A-0x302F] | 0x3099 | 0x309A
            int parse_CombiningChar(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // Digit   ::=   [0x0030-0x0039] | [0x0660-0x0669] | [0x06F0-0x06F9] | [0x0966-0x096F] | [0x09E6-0x09EF]
            //                | [0x0A66-0x0A6F] | [0x0AE6-0x0AEF] | [0x0B66-0x0B6F] | [0x0BE7-0x0BEF] | [0x0C66-0x0C6F]
            //                | [0x0CE6-0x0CEF] | [0x0D66-0x0D6F] | [0x0E50-0x0E59] | [0x0ED0-0x0ED9] | [0x0F20-0x0F29]
            int parse_Digit(std::string::const_iterator& s_beg, std::string::const_iterator s_end);
            // Extender  ::=   0x00B7 | 0x02D0 | 0x02D1 | 0x0387 | 0x0640 | 0x0E46 | 0x0EC6 | 0x3005 | [0x3031-0x3035]
            //                | [0x309D-0x309E] | [0x30FC-0x30FE]
            int parse_Extender(std::string::const_iterator& s_beg, std::string::const_iterator s_end);


        public:
            typedef int (xml_parser::*parser_Func_t) (std::string::const_iterator& s_beg, std::string::const_iterator s_end);

        public:
            void set_sbeg(std::string::const_iterator s_beg)
            {
                this->sbeg_bak = s_beg;
            }

            std::string::const_iterator get_sbeg()
            {
                return this->sbeg_bak;
            }

            int get_offset(std::string::const_iterator it)
            {
                return std::distance(this->sbeg_bak, it);
            }

        public:
            SegmentXML * seg_get_root() const
            {
                return this->seg_root;
            }
            void seg_set_root(SegmentXML * seg)
            {
                this->seg_root = seg;
            }

            SegmentXML * seg_get_current() const
            {
                return this->seg_current;
            }
            void seg_set_current(SegmentXML * seg)
            {
                this->seg_current = seg;
            }

            void seg_print(std::ostream& o) const
            {
                this->seg_get_root()->print(o, this->sbeg_bak);
            }

        protected:
            inline int loop_times(std::string::const_iterator& s_beg, std::string::const_iterator s_end,
                                  parser_Func_t func)
            {
                int times = 0;
                while ( (this->*func)(s_beg, s_end)) {
                        times++;
                }
                return times;
            }
            inline void accumluate(int & val)
            {
                val += this->cur_consume;
                this->cur_consume = 0;
            }
            inline int consume(const char * func_name, int cnt)
            {
                this->cur_consume = cnt;
                if (cnt && func_name) {
                    this->last_func_name = func_name;
                }
                return this->cur_consume;
            }
            inline int consume() const
            {
                return this->cur_consume;
            }

            inline bool or_throw(const char * msg) {
                if (p_it) {
//                    std::cout
//                        << __func__ << "(" << (last_func_name ? last_func_name : "") << ")"
//                        << " encounter " << *(*p_it) << *(*p_it + 1)
//                        << " msg = " << msg << std::endl;
                    this->set_last_err(this->get_offset(*p_it),
                                       last_func_name ? last_func_name : "",
                                       msg);
                }
                throw msg;
                return true;
            }

            inline void set_p_it(std::string::const_iterator & it) {
                this->p_it = &it;
            }

        protected:
            void set_last_err(int off, const char * func_name, const std::string& msg);

            void print_last_err(std::ostream& out) const;

        private:
            std::string                 bom_str;
            // std::string                 charset_str; not use

            skipper_t sk;
            int cur_consume;
            std::string::const_iterator * p_it; // 用于指示问题发生的位置；
            std::string::const_iterator sbeg_bak; // 保存序列开始位置，并传递给SegmentXML树

            // 解析用
            // 注意，这些指针，最好改为 handle模式！
            // 参考 jsonpp::JHandle
            xml3::node *                p_root;         // 根节点
            xml3::node *                p_cur;          // 临时工作节点
            xml3::node *                p_info;         // info节点
            xml3::node *                p_doctype;      // doctype节点
            xml3::xml_doc *             p_document;     // doctype节点

            const char *                last_func_name;

            SegmentXML *                seg_root;
            SegmentXML *                seg_current;

            // NOTE 记录最远匹配位置，发生的错误，虽然想法很好，但是，代码还需要进一步改造才行！
            // 因为当前，很多地方，用的不是抛出异常的方式；
            // 如果仅记录匹配字符数为0，发生的函数呢？
            int                         last_err_offset;        // 最后一次错误发生位置
            const char *                last_err_func_name;     // 最后一次错误函数名；
            std::string                 last_err_msg;           // 最后一次错误消息；

        public:
            XMLpathMem                  pathMem;
        };
    }
}


#endif  /* __XML_PARSER_HPP_1400994344__ */
