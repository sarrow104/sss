//#include "xml_parser.hpp"
//#ifndef  _USING_tmplateSegmentXML_
//#include "xml_parser.cpp.ok"
//#else
//#include "xml_parser.cpp.tmplateSegmentXML"
//#endif

#include "xml_parser.hpp"
#include "xml_doc.hpp"
#include "exception.hpp"

#include <sss/Exception.hpp>
#include <sss/util/Memory.hpp>
#include <sss/utlstring.hpp>
#include <sss/log.hpp>

#include <assert.h>

#include <algorithm>
#include <stdexcept>
#include <set>

// FIXME 怎么办？有些是预读的！就是说，允许解析失败！
// 即，有些是必须完成匹配（匹配一次），有些是允许（0或1次）；
// 有些允许任意多次（0到n次）
// 如果，C语言支持 函数式编程，那么一切好说；现在么，
// 1. 使用异常，再增加一个参数，表示允许的次数——反正就3种情况；
// 2. 仿函数式编程，利用传入指针；用另外两个函数，来实现匹配0-1次；
//    和0-n次；
//    不过，如果，连续匹配多个，就成了问题；
//    还有构建对象的时机呢？
//    预读机制？
// ~/project/LanguageImplementationPatterns/CPP_rewrite/LL2/LookaheadParser.hpp|28
// 上面的代码里面，用的是，预读，然后消耗的战术；
// if (LA(A) == ...) {
//    ...;
// }
// while(LA(A) == ...) {
//    ...;
//    ...;
// }
// ~/project/LanguageImplementationPatterns/CPP_rewrite/readme.txt|51
// 另外，还有消除做递归 等技术：
//
//! 消除“左递归”
//
// from
//
// > expr: ID '++'		// match "x++"
// >     | ID '--'		// match "x--"
// >     ;
//
// to
//
// > expr: ID ('++' |'--' ) ; // match "x++" or "x--"
//
//
// ----------------------------------------------------------------------
//! nest list grammar -- page 59 LL(1)
//
// list	 : '[' elements ']' ;		// match bracketed list
// elements : element (',' element)* ;     // match comma-separated list
// element  : NAME | list ;                // element is name or nested list
//
// ** nest list grammar -- page 60 LL(K) k>1
//
// list     : '[' elements ']' ;           // match bracketed list
// elements : element (',' element)* ;     // match comma-separated list
// element  : NAME '=' NAME                // match assignment such as a=b
//          | NAME
//          | list
//          ;
//
// 好了，剩下的问题就是，如何在递归下降的同时，生成结构体？还有错误处理、路径选
// 择问题；
// 首先要明白一点的是，递归下降的每一个子函数，都是消耗了0-n个字符；这些函数的被
// 调用本身，就反映了一种"状态"——如果把整个解析看做是状态机的话；
// 那本parttern的指导书里面，使用的是(预读＋消耗)的战术：先预读一段信息；如果符
// 合要求，就消耗掉这段数据；改进版本，则是记录下，这些信息，已经被解析（理解）
// 为某某标记了——如果，有两个分支，他们的开始的判断规则，是有一致性的话，只不
// 过稍微后面一点，才体现出不同；
//
// 比如，EmptyElemTag 和 STag，都是 "<" Name properties? 开头，只不过前者是
// "/>" 结尾，后者是 ">"结尾；
// 当然，这里讨论的是xml的解析；是否 EmptyElemTag 其实只是一个打印效果的关系——
// 就好比对于xhtml来说，换行符用 <br/>还是 <br></br>，浏览器都是接受的。
// 当然，如果数据结构（或者说类型），一定不同，那么应该怎么办？
//
// 递归下降的时候，如何同时创建结构？
//
// 1. 先让我的parser继承自 sss::skipper;
// 2. 之前的版本，我是先整体tokenizer，然后token -> xml结构体；即，是将原始的字
// 符流，解析为 int + std::string的值对；然后，xml_parser 则利用一个xml::node *
// tmp_node指针，不断地根据token序列信息，进行操作——添加属性、增加节点；回到上
// 层节点；等等操作；
//
// 如此，新版的xml_parser，虽然有N多的子函数，我相信，也可以如此处理！
//! 关于路径记忆版
// 当成功判断可复用串（以空白符为间隔，可以吗？），记录下开始位置，以及成功匹配
// 的函数名；
// 当消耗这些串的时候，从容器中删除这些节点即可；如果以空白符为界，那么以开始位
// 置，作为key，也是可以的。（或者以偏移）
//----------------------------------------------------------------------
//! 如何处理DTD复用？
// 外部的DTD，用的是域名为关键字；
// 内部呢？可以将DTD字符串，为关键字；（先解析，然后输出，再用md5等信息抽取算法
// ）
// 对于无DTD的xml，也是可以用此种方式表达的


#define XML_PARSER_SEG_HELPER(r) SegmentXML_Helper \
    _helper_(this, __func__, r);                \
    if (_helper_.isOK()) {                  \
        return this->consume(__func__, r.count());        \
    }

namespace sss {

    namespace xml3 {
        class AttNameValidator
        {
        public:
            AttNameValidator(sss::xml3::xml_parser* xp)
                : _xp(xp)
            {
                assert(_xp);
                assert(_xp->seg_get_current());
                assert(_xp->seg_get_current()->get_func() == xml_parser::falldown_name_table::PARSE_ATTRIBLIST);
                assert(_xp->seg_get_current()->parent());
                assert(_xp->seg_get_current()->parent()->firstChild());
                xml_parser::SegmentXML * name_seg = _xp->seg_get_current()->parent()->firstChild();
                name_seg->validator(xml_parser::falldown_name_table::PARSE_NAME);
                _node_name = name_seg->get_slice();
            }

            ~AttNameValidator()
            {
            }

            void checkIn()
            {
                xml_parser::SegmentXML * attribute = _xp->seg_get_current()->lastChild();
                assert(attribute);
                assert(attribute->get_func() == xml_parser::falldown_name_table::PARSE_ATTRIBUTE);
                assert(attribute->firstChild());
                attribute->firstChild()->validator(xml_parser::falldown_name_table::PARSE_NAME);
                std::string attribute_name = attribute->firstChild()->get_slice();
                if (_attribute_names.find(attribute_name) != _attribute_names.end()) {
                    throw sss::xml3::ConstructingError("encounter the same attribute_name:" + attribute_name);
                }
                _attribute_names.insert(attribute_name);
            }

        protected:
            sss::xml3::xml_parser * _xp;
            std::string _node_name;
            std::set<std::string> _attribute_names;
        };

        xml_parser::SegmentXML_Helper::SegmentXML_Helper(xml_parser*          _xp, //
                                                         const char *         _func_name,
                                                         xml_parser::Rewinder_t & _r)
            : xp(_xp), seg(0), r(_r)
        {
#if _DEBUG_
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            SSS_LOG_DEBUG("from %s\n", _func_name);
#endif
            int func_id = xml_parser::falldown_name_table::conv(_func_name);
            int offset  = this->xp->get_offset(this->r.get_it_beg());
            XMLpathMemKey key(func_id, offset);
            if (this->xp->pathMem.has_trival(key, this->seg)) {
                this->xp->pathMem.usePath(key);
                if (this->seg) {
                    assert(this->seg->get_start() >= 0);
                    assert(this->seg->get_end() >= this->seg->get_start());

                    SSS_LOG_DEBUG("reuse %s at %d, consume %d\n",
                                  xml_parser::falldown_name_table::conv(this->seg->get_func()).c_str(),
                                  this->seg->get_start(),
                                  this->seg->get_end() - this->seg->get_start());

                    r.consume(this->seg->get_end() - this->seg->get_start());

                    assert(this->xp->seg_get_current());
                    //assert(this->xp->seg_get_current()->get_end() != -1);
                    // seg_get_current() 不变！
                    this->xp->seg_get_current()->append(seg);
                }
                else {
                    // NOTE 多重分支的话，恰好前面几个分支，都是同样的第二级函数，比如parse_Name，那么
                    // 确实会发生死在同一种支路上，并且是同一位置！
                    r.rollback();
                }
                r.commit();
            }
            else {
                this->seg = new SegmentXML(_func_name,
                                           this->xp->get_sbeg(),
                                           offset);
                if (!this->xp->seg_get_root()) {
                    this->xp->seg_set_root(this->seg);
                    assert(!this->xp->seg_get_current());
                    this->xp->seg_set_current(0);
                }

                if (this->xp->seg_get_current()) {
                    assert(this->xp->seg_get_current()->get_end() == -1);
                    this->xp->seg_get_current()->append(seg);
                }
                this->xp->seg_set_current(seg);
            }

        }

        xml_parser::SegmentXML_Helper::~SegmentXML_Helper()
        {
            //SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            //SSS_LOG_DEBUG("from %s\n",
            //              xml_parser::falldown_name_table::conv(this->seg->get_func()).c_str());

            if (this->isOK()) {
                return;
            }

            SegmentXML * parent = this->seg->get_parent();
            if (this->r) {
                int offset_end = this->xp->get_offset(this->r.get_it_end());
                this->seg->set_end(offset_end);
                //SSS_LOG_DEBUG("%s\n", "r.commit()");

                // parse_CharData 当内部，全部是空白字符组成，删除该节点！
                if (this->seg->get_func() == falldown_name_table::PARSE_CHARDATA) {
                    this->seg->trim();
                    if (parent && this->seg->empty()) {
                        parent->remove_child(this->seg);
                    }
                }
            }
            else {
                this->seg->unlink();
                // 记录失败的路径点
                this->xp->pathMem.add(XMLpathMemKey(this->seg->get_func(), this->seg->get_start()), 0);

                SegmentXML * left = 0;
                // 默认子路径都是成功的；记录之；
                for (SegmentXML * node = this->seg->first_child(); node; node = left) {
                    left = node->nextSibling();
                    node->unlink();
                    this->xp->pathMem.add(XMLpathMemKey(node->get_func(), node->get_start()),
                                          node);
                }
                delete this->seg;
            }

            this->xp->seg_set_current(parent);
        }

        void xml_parser::SegmentXML::string_slice_print(std::ostream& out, std::string::const_iterator s_beg)
        {
            std::advance(s_beg, this->start);
            for (int i = 0; i < this->end - this->start; ++i, ++s_beg) {
                out << *s_beg;
            }
        }

        void xml_parser::SegmentXML::print(std::ostream& out, std::string::const_iterator s_beg)
        {
            out << falldown_name_table::conv(this->func_id)
                << "[" << this->start << "," << this->end << "]";

            switch (this->func_id)
            {
            //case falldown_name_table::PARSE_NAME:
            //    this->string_slice_print(out, s_beg);
            //    break;

            default:
                if (!this->hasChildren()) {
                    this->string_slice_print(out, s_beg);
                }
                break;
            }
            out << std::endl;

            if (this->size()) {
                out << "{";
                for (SegmentXML * p = this->first_child();
                     p;
                     p = p->nextSibling())
                {
                    p->print(out, s_beg);
                }
                out << "}" << std::endl;
            }
        }

        xml_parser::falldown_name_table::falldown_name_table()
        {
            value_type & val(*this);
            val["parse_bom"] = PARSE_BOM;
            val["parse_document"] = PARSE_DOCUMENT;
            val["parse_Char"] = PARSE_CHAR;
            val["parse_S"] = PARSE_S;
            val["parse_NameStartChar"] = PARSE_NAMESTARTCHAR;
            val["parse_NameChar"] = PARSE_NAMECHAR;
            val["parse_Name"] = PARSE_NAME;
            val["parse_Names"] = PARSE_NAMES;
            val["parse_Nmtoken"] = PARSE_NMTOKEN;
            val["parse_Nmtokens"] = PARSE_NMTOKENS;
            val["parse_EntityValue"] = PARSE_ENTITYVALUE;
            val["parse_AttValue"] = PARSE_ATTVALUE;
            val["parse_SystemLiteral"] = PARSE_SYSTEMLITERAL;
            val["parse_PubidLiteral"] = PARSE_PUBIDLITERAL;
            val["parse_PubidChar"] = PARSE_PUBIDCHAR;
            val["parse_CharData"] = PARSE_CHARDATA;
            val["parse_Comment"] = PARSE_COMMENT;
            val["parse_PI"] = PARSE_PI;
            val["parse_PITarget"] = PARSE_PITARGET;
            val["parse_CDSect"] = PARSE_CDSECT;
            val["parse_CDStart"] = PARSE_CDSTART;
            val["parse_CData"] = PARSE_CDATA;
            val["parse_CDEnd"] = PARSE_CDEND;
            val["parse_prolog"] = PARSE_PROLOG;
            val["parse_XMLDecl"] = PARSE_XMLDECL;
            val["parse_VersionInfo"] = PARSE_VERSIONINFO;
            val["parse_Eq"] = PARSE_EQ;
            val["parse_VersionNum"] = PARSE_VERSIONNUM;
            val["parse_Misc"] = PARSE_MISC;
            val["parse_doctypedecl"] = PARSE_DOCTYPEDECL;
            val["parse_DeclSep"] = PARSE_DECLSEP;
            val["parse_intSubset"] = PARSE_INTSUBSET;
            val["parse_markupdecl"] = PARSE_MARKUPDECL;
            val["parse_extSubset"] = PARSE_EXTSUBSET;
            val["parse_extSubsetDecl"] = PARSE_EXTSUBSETDECL;
            val["parse_SDDecl"] = PARSE_SDDECL;
            val["parse_element"] = PARSE_ELEMENT;
            val["parse_STag"] = PARSE_STAG;
            val["parse_Attribute"] = PARSE_ATTRIBUTE;
            val["parse_ETag"] = PARSE_ETAG;
            val["parse_content"] = PARSE_CONTENT;
            val["parse_AttribList"] = PARSE_ATTRIBLIST;
            val["parse_EmptyElemTag"] = PARSE_EMPTYELEMTAG;
            val["parse_elementdecl"] = PARSE_ELEMENTDECL;
            val["parse_contentspec"] = PARSE_CONTENTSPEC;
            val["parse_children"] = PARSE_CHILDREN;
            val["parse_cp"] = PARSE_CP;
            val["parse_Times_suffix"] = PARSE_TIMES_SUFFIX;
            val["parse_choice"] = PARSE_CHOICE;
            val["parse_seq"] = PARSE_SEQ;
            val["parse_Mixed"] = PARSE_MIXED;
            val["parse_Mixed_suffix"] = PARSE_MIXED_SUFFIX;
            val["parse_AttlistDecl"] = PARSE_ATTLISTDECL;
            val["parse_AttDef"] = PARSE_ATTDEF;
            val["parse_AttType"] = PARSE_ATTTYPE;
            val["parse_StringType"] = PARSE_STRINGTYPE;
            val["parse_TokenizedType"] = PARSE_TOKENIZEDTYPE;
            val["parse_EnumeratedType"] = PARSE_ENUMERATEDTYPE;
            val["parse_NotationType"] = PARSE_NOTATIONTYPE;
            val["parse_Enumeration"] = PARSE_ENUMERATION;
            val["parse_DefaultDecl"] = PARSE_DEFAULTDECL;
            val["parse_conditionalSect"] = PARSE_CONDITIONALSECT;
            val["parse_includeSect"] = PARSE_INCLUDESECT;
            val["parse_ignoreSect"] = PARSE_IGNORESECT;
            val["parse_ignoreSectContents"] = PARSE_IGNORESECTCONTENTS;
            val["parse_Ignore"] = PARSE_IGNORE;
            val["parse_CharRef"] = PARSE_CHARREF;
            val["parse_Reference"] = PARSE_REFERENCE;
            val["parse_EntityRef"] = PARSE_ENTITYREF;
            val["parse_PEReference"] = PARSE_PEREFERENCE;
            val["parse_EntityDecl"] = PARSE_ENTITYDECL;
            val["parse_GEDecl"] = PARSE_GEDECL;
            val["parse_PEDecl"] = PARSE_PEDECL;
            val["parse_EntityDef"] = PARSE_ENTITYDEF;
            val["parse_PEDef"] = PARSE_PEDEF;
            val["parse_ExternalID"] = PARSE_EXTERNALID;
            val["parse_NDataDecl"] = PARSE_NDATADECL;
            val["parse_TextDecl"] = PARSE_TEXTDECL;
            val["parse_extParsedEnt"] = PARSE_EXTPARSEDENT;
            val["parse_EncodingDecl"] = PARSE_ENCODINGDECL;
            val["parse_EncName"] = PARSE_ENCNAME;
            val["parse_NotationDecl"] = PARSE_NOTATIONDECL;
            val["parse_PublicID"] = PARSE_PUBLICID;
            val["parse_Letter"] = PARSE_LETTER;
            val["parse_BaseChar"] = PARSE_BASECHAR;
            val["parse_Ideographic"] = PARSE_IDEOGRAPHIC;
            val["parse_CombiningChar"] = PARSE_COMBININGCHAR;
            val["parse_Digit"] = PARSE_DIGIT;
            val["parse_Extender"] = PARSE_EXTENDER;
            val["parse_SDDecl_Value"] = PARSE_SDDECL_VALUE;
        }

        void xml_parser::XMLpathMem::clear()
        {
            value_type & val(*this);
            for (value_type::iterator it = val.begin(); it != val.end(); ++it) {
                delete it->second;
            }
            val.clear();
        }

        bool xml_parser::XMLpathMem::add(const xml_parser::XMLpathMemKey& key, xml_parser::SegmentXML* seg)
            throw(const char *)
        {
            value_type & val(*this);
            key.validAssert();
            // NOTE seg 是否为0，都有意义，所以不用assert

            value_type::iterator it = val.find(key);
            assert(it == val.end());
            val.insert(std::make_pair(key, seg));
            if (!seg) {
                SSS_LOG_DEBUG("%s %d faild path.\n",
                              xml_parser::falldown_name_table::conv(key.funcID()).c_str(),
                              key.offset());
            }
            else {
                SSS_LOG_DEBUG("%s [%d, %d] success %s\n",
                              xml_parser::falldown_name_table::conv(seg->get_func()).c_str(),
                              seg->get_start(), seg->get_end(),
                              seg->get_slice().c_str());
            }
            return true;
        }

        bool xml_parser::XMLpathMem::has_trival(const xml_parser::XMLpathMemKey& key, xml_parser::SegmentXML *& seg)
        {
            bool ret = false;
            value_type & val(*this);
            key.validAssert();

            value_type::iterator it = val.find(key);
            if (it != val.end()) {
                seg = it->second;
                ret = true;
            }

            return ret;
        }

        bool xml_parser::XMLpathMem::usePath(const xml_parser::XMLpathMemKey& key)
        {
            value_type & val(*this);
            key.validAssert();
            // NOTE seg 是否为0，都有意义，所以不用assert

            value_type::iterator it = val.find(key);
            assert(it != val.end());
            val.erase(it);
            return true;
        }

        xml_parser::xml_parser()
            : p_it(0),
            p_root(0), p_cur(0), p_info(0), p_doctype(0), p_document(0),
            last_func_name(0),
            seg_root(0),
            seg_current(0),
            last_err_offset(-1),
            last_err_func_name(0)
        {
        }

        xml_parser::~xml_parser()
        {
        }

        // 解析 xml_str_data ，构建于 tmp_doc 对象
        xml3::node * xml_parser::parse(const std::string& xml_str_data)
        {
            return this->parse(xml_str_data.begin(),
                               xml_str_data.end());
        }

        void xml_parser::clear()
        {
            delete this->seg_root;
            this->seg_set_root(0);
            this->seg_set_current(0);

            this->p_document = 0;
            this->p_cur = 0;
            this->p_info = 0;
            this->p_root = 0;
            this->p_doctype = 0;

            this->pathMem.clear();

            this->last_err_msg = "";
            this->last_err_func_name = 0;
            this->last_err_offset = -1;
        }

        xml3::node * xml_parser::Seg2XMLsubtree(xml3::xml_doc * pdoc, xml_parser::SegmentXML * seg_root)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            try {
                assert(pdoc);
                assert(seg_root);
                this->p_document = pdoc;
                seg_root->validator(xml_parser::falldown_name_table::PARSE_ELEMENT);

                // 要不，我这样？再创建一个xml_doc对象，作为子树的容器？
                // 在解析完成之后，返回内部子树，然后，销毁整个容器即可！
                // 反正我的xml对象树，也没有限制xml_doc下面在指针不能还指向xml_doc对象？
                this->p_root = new xml3::xml_doc; // TODO 用handle来包裹，避免内存泄露
                this->p_cur = p_root;

                this->Seg2XML_element(seg_root);  // 将current以element元素，一次解析下去；

                if (this->p_root->size() > 1u) {
                    std::ostringstream oss;
                    oss << __func__ << ":  encounter multy node!";
                    throw sss::xml3::ConstructingError(oss.str());
                }
                sss::scoped_ptr<sss::xml3::node> ret(this->p_root->firstChild());
                ret->unlink();

                this->clear();

                return ret.release();
            }
            catch (std::exception& e) {
                std::cout << e.what() << std::endl;
                return 0;
            }
        }

        xml3::node * xml_parser::Seg2XML(xml_parser::SegmentXML * seg_root)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            try {
                assert(seg_root);
                seg_root->validator(xml_parser::falldown_name_table::PARSE_DOCUMENT);

                xml_parser::SegmentXML * current = SegHandle(seg_root).firstChild().get();
                current->validator(xml_parser::falldown_name_table::PARSE_PROLOG);
                if (!this->p_document) {
                    this->p_document = new xml3::xml_doc;
                    this->p_cur = this->p_document;
                }
                this->Seg2XML_prolog(current); // 将prolog子树，抽取出来，生成合适的节点，放在p_document下面；
                this->Seg2XML_element(current);// 将current以element元素，一次解析下去；
                this->Seg2XML_Misc(current);   // 解析剩余的SegmentXML树；——注意，只是循环的包裹！不用上下移动指针；

                xml3::xml_doc * ret = 0;
                std::swap(ret, this->p_document);

                this->clear();

                // NOTE bnf产生式，并不纯洁！
                // document   ::=  prolog element Misc*
                // prolog     ::=  XMLDecl? Misc* (doctypedecl Misc*)?
                // element    ::=  EmptyElemTag
                //               | STag content ETag
                // Misc       ::=  Comment | PI | S // NOTE Misc没有捕获
                //
                // 可见，document 产生式，本质就是一系列节点的链表！——如果把可能
                // 的树形doctypedecl、element看做是单独整体的话。
                // 而且，产生式，竟然不要求XMLDecl？！

                return ret;
            }
            catch (std::exception& e) {
                std::cout << e.what() << std::endl;
                return 0;
            }
        }

        // 链表顺序检查seg；需要移动seg指针，至少返回一个当前位置指针！
        // 然后让外面完成移动；
        // 不过，就无法完成链式操作——一个，接一个；
        // prolog     ::=  XMLDecl? Misc* (doctypedecl Misc*)?
        bool xml_parser::Seg2XML_prolog(xml_parser::SegmentXML *& current)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            for (xml_parser::SegmentXML * seg = current->firstChild(); seg; seg = seg->nextSibling()) {
                switch (seg->get_func()) {
                case xml_parser::falldown_name_table::PARSE_XMLDECL:
                    SSS_LOG_DEBUG("%s\n", "PARSE_XMLDECL");
                    this->p_info = this->p_document->create_info("xml");
                    // XMLDecl         ::=   '<?xml' VersionInfo EncodingDecl? SDDecl? S? '?>'
                    // VersionInfo     ::=   S 'version' Eq ("'" VersionNum "'" | '"' VersionNum '"')
                    // EncodingDecl    ::=   S 'encoding' Eq ('"' EncName '"' | "'" EncName "'" )
                    // SDDecl          ::=   S 'standalone' Eq SDDecl_Value
                    // SDDecl_Value    ::= ("'" ('yes' | 'no') "'") | ('"' ('yes' | 'no') '"')
                    this->p_document->append_child(p_info);
                    {
                        xml_parser::SegmentXML * xmldecl_sub = seg->first_child();

                        xmldecl_sub->validator(xml_parser::falldown_name_table::PARSE_VERSIONINFO);
                        assert(xmldecl_sub->firstChild());
                        xmldecl_sub->firstChild()->validator(xml_parser::falldown_name_table::PARSE_VERSIONNUM);
                        this->p_info->set("version", xmldecl_sub->firstChild()->get_slice());
                        xmldecl_sub = xmldecl_sub->nextSibling();

                        if (xmldecl_sub && xmldecl_sub->get_func() == xml_parser::falldown_name_table::PARSE_ENCODINGDECL) {
                            assert(xmldecl_sub->firstChild());
                            xmldecl_sub->firstChild()->validator(xml_parser::falldown_name_table::PARSE_ENCNAME);
                            this->p_info->set("encoding", xmldecl_sub->firstChild()->get_slice());
                            xmldecl_sub = xmldecl_sub->nextSibling();
                        }
                        if (xmldecl_sub && xmldecl_sub->get_func() == xml_parser::falldown_name_table::PARSE_SDDECL) {
                            assert(xmldecl_sub->firstChild());
                            xmldecl_sub->firstChild()->validator(xml_parser::falldown_name_table::PARSE_SDDECL_VALUE);
                            this->p_info->set("standalone", xmldecl_sub->firstChild()->get_slice(1, 1));
                        }
                    }
                    this->p_document->info(this->p_info);
                    break;

                case xml_parser::falldown_name_table::PARSE_DOCTYPEDECL:
                    SSS_LOG_DEBUG("%s\n", "PARSE_DOCTYPEDECL");
                    this->p_doctype = this->p_document->create_doctype(seg->get_slice());
                    this->p_document->append_child(p_doctype);
                    // NOTE doctype也应该进行特殊处理。比如可以获取dtd信息（内部，或者外部的url字符串）
                    break;

                default:
                    {
                        // FIXME 可能要用到 goto！
                        bool ret = this->Seg2XML_Misc(seg);
                        if (!ret) {
                            goto out_side;
                        }
                    }
                    break;
                }
            }
out_side:
            current = current->nextSibling();
            return true;
        }

        bool  xml_parser::Seg2XML_Misc(SegmentXML *& current)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            if (!current) {
                return false;
            }
            for (xml_parser::SegmentXML * seg = current; seg; seg = seg->nextSibling()) {
                switch (seg->get_func()) {
                case xml_parser::falldown_name_table::PARSE_COMMENT:
                    this->p_cur->append_child(this->p_document->create_comment(seg->get_slice(4, 3))); // ??
                    break;

                case xml_parser::falldown_name_table::PARSE_PI:
                    this->p_cur->append_child(this->p_document->create_PI(seg->get_slice(2, 2))); // ??
                    break;

                default:
                    break;
                }
            }
            return true;
        }

        // 链表移动；消耗至少一个节点；然后挪到下一个节点；
        // element    ::=  EmptyElemTag
        //               | STag content ETag
        bool xml_parser::Seg2XML_element(xml_parser::SegmentXML *& current)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            xml_parser::SegmentXML * cur_bak = current;
            current = current->firstChild();

            xml3::node * p_cur_bak = this->p_cur;

            switch (current->get_func()) {
            case xml_parser::falldown_name_table::PARSE_EMPTYELEMTAG:
            case xml_parser::falldown_name_table::PARSE_STAG:
                {
                    // NOTE 在这里，Stag 下的 Name 和 AttribList 只是作为其属性存在
                    // ，而不应该看为解析树的子节点！
                    assert(current->firstChild());
                    current->firstChild()->validator(xml_parser::falldown_name_table::PARSE_NAME);
                    this->p_cur = this->p_document->create_node(current->firstChild()->get_slice());

                    if (!this->p_root) {
                        this->p_root = this->p_cur;
                        //this->p_document->root(this->p_root);
                    }
                    p_cur_bak->append_child(this->p_cur);
                }

                if (current->firstChild()->nextSibling()) {
                    this->Seg2XML_AttribList(current->firstChild()->nextSibling());
                }

                if (current->get_func() == xml_parser::falldown_name_table::PARSE_STAG) {
                    current = current->nextSibling();
                    this->Seg2XML_content(current);
                    assert(current);
                    current->validator(xml_parser::falldown_name_table::PARSE_ETAG);
                }

                break;

            default:
                throw sss::xml3::ConstructingError("unexpect seg node :" +
                                                   xml_parser::falldown_name_table::conv(current->get_func()));
            }
            current = cur_bak->nextSibling();
            this->p_cur = p_cur_bak;
            return true;
        }

        // content    ::=          CharData? ((element | Reference | CDSect | PI | Comment) CharData?)*
        bool xml_parser::Seg2XML_content(xml_parser::SegmentXML *& current)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            for (xml_parser::SegmentXML *& seg = current; seg;) {
                switch (seg->get_func()) {
                case xml_parser::falldown_name_table::PARSE_CHARDATA:
                    assert(this->p_cur);
                    this->p_cur->append_child(this->p_document->create_text(seg->get_slice())); // ??
                    seg = seg->nextSibling();
                    break;

                case xml_parser::falldown_name_table::PARSE_ELEMENT:
                    assert(this->p_cur);
                    this->Seg2XML_element(seg);
                    break;

                case xml_parser::falldown_name_table::PARSE_REFERENCE:
                    throw sss::ExceptionNotSupportMethod("xml_parser::falldown_name_table::PARSE_REFERENCE not support.");
                    break;

                case xml_parser::falldown_name_table::PARSE_CDSECT:
                    assert(this->p_cur);
                    assert(seg->first_child());
                    seg->first_child()->validator(xml_parser::falldown_name_table::PARSE_CDATA);
                    this->p_cur->append_child(this->p_document->create_cdata(seg->first_child()->get_slice()));
                    seg = seg->nextSibling();
                    break;


                case xml_parser::falldown_name_table::PARSE_PI:
                    this->p_cur->append_child(this->p_document->create_PI(seg->get_slice(2, 2))); // ??
                    seg = seg->nextSibling();
                    break;

                case xml_parser::falldown_name_table::PARSE_COMMENT:
                    this->p_cur->append_child(this->p_document->create_comment(seg->get_slice(4, 3))); // ??
                    seg = seg->nextSibling();
                    break;

                default:
                    goto out_side;
                }
            }
out_side:
            return true;
        }

        // 这个有些特殊，只处理一个节点下面的链表；
        bool xml_parser::Seg2XML_AttribList(xml_parser::SegmentXML * current)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            if (!current) {
                return false;
            }
            current->validator(xml_parser::falldown_name_table::PARSE_ATTRIBLIST);

            for (xml_parser::SegmentXML * seg = current->firstChild(); seg; seg = seg->nextSibling()) {
                seg->validator(xml_parser::falldown_name_table::PARSE_ATTRIBUTE);
                assert(seg->size() == 2);
                assert(this->p_cur);
                this->p_cur->set(seg->first_child()->get_slice(), seg->last_child()->get_slice(1, 1));
            }
            // NOTE do not change *current*
            return true;
        }
        // ----------------------------------------------------------------------

        // 先处理字节流的bom
        // 然后将字节流，转换成token流
        //
        // 第一遍循环，分析节点对应关系，是否正确
        // 检测 info.charset —— 提供默认值
        // 第二遍循环，构建节点；
        // 检查 root 是否唯一；
        //
        //! 上面是解析xml的流程；其实解析xhtml的话，也可以套用这个流程；
        // 不过，info 节点的检测，需要做小修改：应该最后来判断这个节点的存在性
        // ，以及出现的位置合不合适。
        //
        // 如：
        // 先处理字节流的bom
        // 然后将字节流，转换成token流
        //
        // 第一遍循环，分析节点对应关系，是否正确
        // 检测 doctype
        // 第二遍循环，构建节点；
        // 检查 root 是否唯一；
        // 如果doctype 不存在，则提供一个默认值；
        // 如果doctype 位置不对，则将其挪至开始。
        //
        //! 额，或者说，上述合法性检测，应该挪到实体对象的构造函数中完成
        //（当然，也可以借助一个叫validata的成员函数）
        // 那样的话，parser函数，就能显得比较简洁；

        xml3::node * xml_parser::parse(std::string::const_iterator s_ini,
                                       std::string::const_iterator s_fin)
        {
            try {
                this->set_sbeg(s_ini);
                this->set_p_it(s_ini);

                this->parse_bom(s_ini, s_fin);
                if (!this->parse_document(s_ini, s_fin)) {
                    std::ostringstream oss;
                    this->print_last_err(oss);
                    throw sss::xml3::ParsingError(oss.str());
                    this->clear();
                }

                // FIXME TODO
                // 这里，应该对 SegmentXML 树，进行整理，然后生成 node 对象树；
                // 以及，对解析环境，进行初始化
                return Seg2XML(this->seg_get_root());
            }
            catch (std::exception & e) {
                std::cout << e.what() << std::endl;
                throw;
            }
        }

        xml3::node * xml_parser::parseSubtree(xml3::xml_doc * pdoc, std::string::const_iterator s_ini, std::string::const_iterator s_fin)
        {
            try {
                this->set_sbeg(s_ini);
                this->set_p_it(s_ini);

                if (!this->parse_element(s_ini, s_fin)) {
                    std::ostringstream oss;
                    this->print_last_err(oss);
                    throw sss::xml3::ParsingError(oss.str());
                    this->clear();
                }

                return Seg2XMLsubtree(pdoc, this->seg_get_root());
            }
            catch (std::exception & e) {
                std::cout << e.what() << std::endl;
                throw;
            }
        }

        // 解析 bom，并返回读取过的字节数；
        int    xml_parser::parse_bom(std::string::const_iterator& ini,
                                     std::string::const_iterator fin)
        {
            for (sss::xml3::bom_str_table_t::iterator it = sss::xml3::bom_str_table.begin();
                 it != sss::xml3::bom_str_table.end();
                 ++it)
            {
                if (sk.skip_str(ini, fin, it->second)) {
                    this->bom_str = it->first;
                    return true;
                }
                else {
                    continue;
                }
            }
            return false;
        }

        // NOTE 如下函数特点：
        // 1. 返回消耗的字节数；
        // 2. 空格分割开的块，用额外的数据结构保存；
        //    主要是因为，内部次数策略，为"*" 的情况，意味着0次到N次；即，允许没有消耗！
        //    如果可以用while(...)来实现，那么，它就要求，内部函数，即便匹配失败，也不能抛出异常！
        //    因为对于上层匹配来说，0次，没有消耗也是允许的！
        // 综上所述，我的每一个函数，都不能抛出异常！
        // 如果匹配失败，那么很简单，消耗字符的指针，回到原位即可！
        // 也就是说，我需要一个对象，它在正常的时候（有消耗），其析构函数，啥都
        // 不做；不正常（消耗数为0），它将字符指针回到原位！
        // 如何办到？
        // 这样，还是让Rewinder 提供保存中间必要结果的功能；内部是一个数组；记
        // 录的时候，就是往后面添加数据；
        // 需要记录函数名，以及字符串（先用耗费空间的std::string，以后再看能否
        // 有节省空间的办法，并且不用回溯）
        //
        // document    ::= prolog element Misc*
        int xml_parser::parse_document(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            try {
                this->parse_prolog(s_beg, s_end) || or_throw("requre parse_prolog");
                this->parse_element(s_beg, s_end) || or_throw("requre parse_element");
                while (this->parse_Misc(s_beg, s_end));
                r.commit();
            }
            catch(skipper_t::Exception& e) {
                this->set_last_err(this->get_offset(s_beg), __func__, e.what());
                r.rollback();
            }
            catch(const char * msg) {
                this->set_last_err(this->get_offset(s_beg), __func__, msg);
                r.rollback();
            }
            catch(std::length_error & e) {
                this->set_last_err(this->get_offset(s_beg), __func__, e.what());
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }

        // Char       ::=          0x9 | 0xA | 0xD | [0x20-0xD7FF] | [0xE000-0xFFFD] | [0x10000-0x10FFFF]
        /* any Unicode character, excluding the surrogate blocks, FFFE, and FFFF. */
        int xml_parser::parse_Char(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            Rewinder_t r(s_beg);
            // FIXME
            r.commit(sk.skip_char(s_beg, s_end, "\x09\x0A\x0D") ||
                     sk.skip_char_range(s_beg, s_end, '\x20', '\xFD'));
            return this->consume(__func__, r.count());
        }

        // S           ::=  (0x20 | 0x9 | 0xD | 0xA)+
        int xml_parser::parse_S(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);

            Rewinder_t r(s_beg);
            r.commit(sk.skip_set(s_beg, s_end, "\x09\x0A\x0D\x0A\x20"));
            return this->consume(__func__, r.count());
        }

        //NameStartChar    ::=   ":" | [A-Z] | "_" | [a-z] | [0xC0-0xD6] | [0xD8-0xF6] | [0xF8-0x2FF] | [0x370-0x37D]
        //                      | [0x37F-0x1FFF] | [0x200C-0x200D] | [0x2070-0x218F] | [0x2C00-0x2FEF] | [0x3001-0xD7FF]
        //                    | [0xF900-0xFDCF] | [0xFDF0-0xFFFD] | [0x10000-0xEFFFF]
        int xml_parser::parse_NameStartChar(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            // FIXME
            Rewinder_t r(s_beg);
            r.commit(sk.skip_char(s_beg, s_end, ':') ||
                     sk.skip_if(s_beg, s_end, ::isalpha) ||
                     sk.skip_char(s_beg, s_end, '_') ||
                     sk.skip_char_range(s_beg, s_end, '\xC0', '\xD6') ||
                     sk.skip_char_range(s_beg, s_end, '\xD8', '\xF6') ||
                     sk.skip_char_range(s_beg, s_end, '\xF8', '\xFD'));
            return this->consume(__func__, r.count());
        }

        //NameChar   ::=   NameStartChar | "-" | "." | [0-9] | 0xB7 | [0x0300-0x036F] | [0x203F-0x2040]
        int xml_parser::parse_NameChar(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            Rewinder_t r(s_beg);
            r.commit(this->parse_NameStartChar(s_beg, s_end) ||
                     sk.skip_char(s_beg, s_end, '-') ||
                     sk.skip_char(s_beg, s_end, '.') ||
                     sk.skip_if_times(s_beg, s_end, ::isdigit, 1) ||
                     sk.skip_char(s_beg, s_end, '\xB7'));
            return this->consume(__func__, r.count());
        }

        //Name     ::=   NameStartChar (NameChar)*
        int xml_parser::parse_Name(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            if (this->parse_NameStartChar(s_beg, s_end)) {
                while (this->parse_NameChar(s_beg, s_end));
                r.commit();
            }
            else {
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }

        // NOTE 未被使用
        //Names    ::=   Name (0x20 Name)*
        int xml_parser::parse_Names(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            Rewinder_t r(s_beg);
            if (this->parse_Name(s_beg, s_end)) {
                while (true) {
                    Rewinder_t r2(s_beg);
                    if (sk.skip_char(s_beg, s_end, '\x20') &&
                        this->parse_Name(s_beg, s_end))
                    {
                        r2.commit();
                    }
                    else {
                        r2.rollback();
                        break;
                    }
                }
                r.commit();
            }
            else {
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }

        //Nmtoken          ::=   (NameChar)+
        int xml_parser::parse_Nmtoken(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            if (this->parse_NameChar(s_beg, s_end)) {
                while (this->parse_NameChar(s_beg, s_end));
                r.commit();
            }
            else {
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }

        // NOTE 未被使用
        //Nmtokens   ::=   Nmtoken (0x20 Nmtoken)*
        int xml_parser::parse_Nmtokens(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            if (this->parse_Nmtoken(s_beg, s_end)) {
                while (true) {
                    Rewinder_t r2(s_beg);
                    if (sk.skip_char(s_beg, s_end, '\x20') &&
                        this->parse_Nmtoken(s_beg, s_end))
                    {
                        r2.commit();
                    }
                    else {
                        r2.rollback();
                        break;
                    }
                }
                r.commit();
            }
            else {
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }

        // EntityValue   ::=   '"' ([^%&"] | PEReference | Reference)* '"'
        //                  |  "'" ([^%&'] | PEReference | Reference)* "'"
        int xml_parser::parse_EntityValue(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            try {
                if (sk.skip_char(s_beg, s_end, '"')) {
                    while (sk.skip_set_verse(s_beg, s_end, "%&\"") ||
                           this->parse_PEReference(s_beg, s_end) ||
                           this->parse_Reference(s_beg, s_end));

                    sk.skip_char(s_beg, s_end, '"', "requre right \"");
                    r.commit();
                }
                else if (sk.skip_char(s_beg, s_end, '\'')) {
                    while (sk.skip_set_verse(s_beg, s_end, "%&\'") ||
                           this->parse_PEReference(s_beg, s_end) ||
                           this->parse_Reference(s_beg, s_end));

                    sk.skip_char(s_beg, s_end, '\'', "requre right \'");
                    r.commit();
                }
                else {
                    throw "requre \' or \" by EntityValue";
                }
            }
            catch(skipper_t::Exception& e) {
                this->set_last_err(this->get_offset(s_beg), __func__, e.what());
                r.rollback();
            }
            catch(const char * msg) {
                this->set_last_err(this->get_offset(s_beg), __func__, msg);
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }

        // AttValue        ::=   '"' ([^<&"] | Reference)* '"'
        //                    |  "'" ([^<&'] | Reference)* "'"
        int xml_parser::parse_AttValue(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            try {
                if (sk.skip_char(s_beg, s_end, '"')) {
                    while (sk.skip_set_verse(s_beg, s_end, "<&\"") ||
                           this->parse_Reference(s_beg, s_end));

                    sk.skip_char(s_beg, s_end, '"', "requre right \"");
                    r.commit();
                }
                else if (sk.skip_char(s_beg, s_end, '\'')) {
                    while (sk.skip_set_verse(s_beg, s_end, "<&\'") ||
                           this->parse_Reference(s_beg, s_end));

                    sk.skip_char(s_beg, s_end, '\'', "requre right \'");
                    r.commit();
                }
                else {
                    throw "not in \' or \"";
                }
            }
            catch(skipper_t::Exception& e) {
                this->set_last_err(this->get_offset(s_beg), __func__, e.what());
                r.rollback();
            }
            catch(const char * msg) {
                this->set_last_err(this->get_offset(s_beg), __func__, msg);
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }
        // SystemLiteral           ::=   ('"' [^"]* '"') | ("'" [^']* "'")
        int xml_parser::parse_SystemLiteral(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            try {
                if (sk.skip_char(s_beg, s_end, '"')) {
                    sk.skip_set_verse(s_beg, s_end, "\"");
                    sk.skip_char(s_beg, s_end, '"', "requre right \"");
                    r.commit();
                }
                else if (sk.skip_char(s_beg, s_end, '\'')) {
                    sk.skip_set_verse(s_beg, s_end, "'");
                    sk.skip_char(s_beg, s_end, '\'', "requre right \'");
                    r.commit();
                }
                else {
                    throw "not in \' or \"";
                }
            }
            catch(skipper_t::Exception& e) {
                this->set_last_err(this->get_offset(s_beg), __func__, e.what());
                r.rollback();
            }
            catch(const char * msg) {
                this->set_last_err(this->get_offset(s_beg), __func__, msg);
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }

        // PubidLiteral    ::=   '"' PubidChar* '"' | "'" (PubidChar - "'")* "'"
        int xml_parser::parse_PubidLiteral(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            try {
                if (sk.skip_char(s_beg, s_end, '"')) {
                    while (this->parse_PubidChar(s_beg, s_end));
                    sk.skip_char(s_beg, s_end, '"', "requre right \"");
                    r.commit();
                }
                else if (sk.skip_char(s_beg, s_end, '\'')) {
                    while (!sk.is_begin_with(s_beg, s_end, "\"") && this->parse_PubidChar(s_beg, s_end));

                    sk.skip_char(s_beg, s_end, '\'', "requre right \'");
                    r.commit();
                }
                else {
                    throw "not in \' or \"";
                    r.rollback();
                }
            }
            catch(skipper_t::Exception& e) {
                this->set_last_err(this->get_offset(s_beg), __func__, e.what());
                r.rollback();
            }
            catch(const char * msg) {
                this->set_last_err(this->get_offset(s_beg), __func__, msg);
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }

        // PubidChar       ::=   0x20 | 0xD | 0xA | [a-zA-Z0-9] | [-'()+,./:=?;!*#@$_%]
        int xml_parser::parse_PubidChar(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            Rewinder_t r(s_beg);
            r.commit(sk.skip_char(s_beg, s_end, '\x20') ||
                     sk.skip_char(s_beg, s_end, '\x0D') ||
                     sk.skip_char(s_beg, s_end, '\x0A') ||
                     sk.skip_if_times(s_beg, s_end, ::isalnum, 1) ||
                     sk.skip_char(s_beg, s_end, "-'()+,./:=?;!*#@$_%"));
            return this->consume(__func__, r.count());
        }

        // NOTE 注意，减号-的意思；
        // 非 <& 的任意长序列，避开 中间包含了 ']]>' 的情况！
        // 即，序列中，不能包含']]>'串！
        // 如何翻译成编程语言？
        // CharData   ::=   [^<&]* - ([^<&]* ']]>' [^<&]*)
        // 当遇到']'的时候，特殊处理就好。只要不是连续三个
        // FIXME 错误！
        // 这里允许匹配0长！
        // 不过，0长的话，也确实没有必要进行额外的存储啊？！
        int xml_parser::parse_CharData(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            while (true) {
                if (sk.is_begin_with(s_beg, s_end, "]]>")) {
                    break;
                }
                if (!sk.skip_char_verse(s_beg, s_end, "<&")) {
                    break;
                }
            }
            r.commit();
            return this->consume(__func__, r.count());
        }

        // Comment         ::=   '<!--' ((Char - '-') | ('-' (Char - '-')))* '-->'
        // NOTE w3c 标准，不允许 <!-- --> 内部出现连续的 '-'；
        int xml_parser::parse_Comment(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);

            try {
                sk.skip_str(s_beg, s_end, "<!--", "requre \"<!--\"");
                while (true) {
                    if (!sk.is_begin_with(s_beg, s_end, '-') && this->parse_Char(s_beg, s_end)) {
                        continue;
                    }

                    Rewinder_t r2(s_beg);
                    if (sk.skip_char(s_beg, s_end, '-') &&
                        !sk.is_begin_with(s_beg, s_end, '-') &&
                        this->parse_Char(s_beg, s_end))
                    {
                        r2.commit();
                    }
                    else {
                        r2.rollback();
                        break;
                    }
                }
                sk.skip_str(s_beg, s_end, "-->", "requre \"-->\"");
                r.commit();
            }
            catch(skipper_t::Exception& e) {
                this->set_last_err(this->get_offset(s_beg), __func__, e.what());
                r.rollback();
            }
            catch(const char * msg) {
                this->set_last_err(this->get_offset(s_beg), __func__, msg);
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }

        // FIXME 2015-08-01 分词！
        // PI      ::=   '<?' PITarget (S (Char* - (Char* '?>' Char*)))? '?>'
        int xml_parser::parse_PI(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            try {
                sk.skip_str(s_beg, s_end, "<?", "requre \"<?\"");
                this->parse_PITarget(s_beg, s_end) || or_throw("requre PITarget");
                if (this->parse_S(s_beg, s_end)) {

                    while (true) {
                        if (sk.is_begin_with(s_beg, s_end, "?>")) {
                            break;
                        }
                        if (!this->parse_Char(s_beg, s_end)) {
                            break;
                        }
                    }
                }

                sk.skip_str(s_beg, s_end, "?>", "requre \"?>\"");
                r.commit();
            }
            catch(skipper_t::Exception& e) {
                this->set_last_err(this->get_offset(s_beg), __func__, e.what());
                r.rollback();
            }
            catch(const char * msg) {
                this->set_last_err(this->get_offset(s_beg), __func__, msg);
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }

        // PITarget   ::=   Name - (('X' | 'x') ('M' | 'm') ('L' | 'l'))
        int xml_parser::parse_PITarget(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            this->parse_Name(s_beg, s_end);
            std::string name = r.get_consume();
            std::for_each(name.begin(), name.end(), ::toupper);
            if (name == "XML") {
                r.rollback();
            }
            else {
                r.commit();
            }
            return this->consume(__func__, r.count());
        }

        // CDSect          ::=   CDStart CData CDEnd
        int xml_parser::parse_CDSect(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            try {
                this->parse_CDStart(s_beg, s_end) || or_throw("requre CDStart");
                this->parse_CData(s_beg, s_end);
                this->parse_CDEnd(s_beg, s_end)   || or_throw("requre CDEnd");
                r.commit();
            }
            catch(skipper_t::Exception& e) {
                this->set_last_err(this->get_offset(s_beg), __func__, e.what());
                r.rollback();
            }
            catch(const char * msg) {
                this->set_last_err(this->get_offset(s_beg), __func__, msg);
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }

        // CDStart         ::=   '<![CDATA['
        int xml_parser::parse_CDStart(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            return this->consume(__func__, sk.skip_str(s_beg, s_end, "<![CDATA["));
        }

        // CData           ::=   (Char* - (Char* ']]>' Char*))
        int xml_parser::parse_CData(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            while (true) {
                if (sk.is_begin_with(s_beg, s_end, "]]>")) {
                    break;
                }

                if (!this->parse_Char(s_beg, s_end)) {
                    break;
                }
            }
            r.commit();
            return this->consume(__func__, r.count());
        }

        // CDEnd           ::=   ']]>'
        int xml_parser::parse_CDEnd(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            return this->consume(__func__, sk.skip_str(s_beg, s_end, "]]>"));
        }

        // prolog          ::=   XMLDecl? Misc* (doctypedecl Misc*)?
        int xml_parser::parse_prolog(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            this->parse_XMLDecl(s_beg, s_end);
            while (this->parse_Misc(s_beg, s_end));
            if (this->parse_doctypedecl(s_beg, s_end)) {
                while (this->parse_Misc(s_beg, s_end));
            }
            r.commit();
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, r.get_consume());
            return this->consume(__func__, r.count());
        }

        // XMLDecl         ::=   '<?xml' VersionInfo EncodingDecl? SDDecl? S? '?>'
        int xml_parser::parse_XMLDecl(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            // TODO 如何记录？读取中的节点信息？
            // 我的想法是，改造or_throw; 比如，它还能接受一个字符串，表示操作中的函数名；
            // 同时，记录下，最后一次读取到的字符串；不过，像VersionInfo等东西
            // 的话，可能需要重新解析，以拆分……还好简单，以'='为界即可；
            // 因为"太深"……
            // 另外，还可能需要关心一下 parse_XMLDecl 的调用次数；毕竟，只能有
            // 一个xml_info节点；
            //
            // 我现在，改造了一下 xml_parser::consume 函数，添加了一个默认参数
            // ，使得其自动记录调用函数名；
            //
            // 不过，还有问题，1. 就是不是所有位置，都强制要求记录匹配情况！
            // 2. 循环应该如何处理？
            // 看样子，真得想办法，通过动态临时节点，来保存这些信息才行啊！
            //
            // NOTE，还需要注意的是parse_XMLDecl，它所用到的函数，并不只有它自己在使用！
            // 这几个函数：VersionInfo EncodingDecl，还有其他 <? xml .... ?>在使用！
            //
            // 难道说，只能深入到每个函数，去创建结构体的细节？
            // 像 DTD 的节点children限制，就允许嵌套，这显然是单独的循环等技巧，没法处理的；
            // 只能通过递归，一层一层地解决；
            //
            // 哎，怎么这么头痛？
            //
            // 其他的xml解析工具，难道也这么辛苦？
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            try {
                sk.skip_str(s_beg, s_end, "<?xml", "requre \'<?xml\'.");
                this->parse_VersionInfo(s_beg, s_end) || or_throw("requre VersionInfo");
                this->parse_EncodingDecl(s_beg, s_end);
                this->parse_SDDecl(s_beg, s_end);
                this->parse_S(s_beg, s_end);
                sk.skip_str(s_beg, s_end, "?>", "requre \'?>\'.");
                r.commit();
                SSS_LOG_EXPRESSION(sss::log::log_DEBUG, r.get_consume());
            }
            catch(skipper_t::Exception& e) {
                this->set_last_err(this->get_offset(s_beg), __func__, e.what());
                r.rollback();
            }
            catch(const char * msg) {
                this->set_last_err(this->get_offset(s_beg), __func__, msg);
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }
        // VersionInfo  ::= S 'version' Eq ("'" VersionNum "'" | '"' VersionNum '"')
        int xml_parser::parse_VersionInfo(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            try {
                this->parse_S(s_beg, s_end) || or_throw("requre S 970");
                sk.skip_str(s_beg, s_end, "version", "requre \'version\'.");
                this->parse_Eq(s_beg, s_end) || or_throw("requre Eq");

                if (sk.skip_char(s_beg, s_end, '\'')) {
                    this->parse_VersionNum(s_beg, s_end) || or_throw("requre VersionNum");
                    sk.skip_char(s_beg, s_end, '\'', "requre right \"'\"");
                }
                else if (sk.skip_char(s_beg, s_end, '"')) {
                    this->parse_VersionNum(s_beg, s_end) || or_throw("requre VersionNum");
                    sk.skip_char(s_beg, s_end, '"', "requre right '\"'");
                }
                else {
                    throw "requre left quote mark.";
                }

                r.commit();
            }
            catch(skipper_t::Exception& e) {
                this->set_last_err(this->get_offset(s_beg), __func__, e.what());
                r.rollback();
            }
            catch(const char * msg) {
                this->set_last_err(this->get_offset(s_beg), __func__, msg);
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }
        // Eq      ::=   S? '=' S?
        int xml_parser::parse_Eq(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            Rewinder_t r(s_beg);
            r.commit((this->parse_S(s_beg, s_end) || true) &&
                     sk.skip_char(s_beg, s_end, '=') &&
                     (this->parse_S(s_beg, s_end) || true));
            return this->consume(__func__, r.count());
        }
        // VersionNum   ::= '1.' [0-9]+
        int xml_parser::parse_VersionNum(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            r.commit(sk.skip_str(s_beg, s_end, "1.") && sk.skip_dec(s_beg, s_end));

            return this->consume(__func__, r.count());
        }

        // Misc    ::=   Comment | PI | S
        // 只有PI 和 Comment 有解析的意义；
        // 所以Misc本身，不做记录
        int xml_parser::parse_Misc(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            Rewinder_t r(s_beg);
            //XML_PARSER_SEG_HELPER(r);
            r.commit(this->parse_Comment(s_beg, s_end) ||
                     this->parse_PI(s_beg, s_end) ||
                     this->parse_S(s_beg, s_end));
            return this->consume(__func__, r.count());
        }

        // NOTE 晕掉，这如何匹配？
        // 中间的 (S ExternalID)? 和 S? 公用一个开头？
        // 我觉得，应该改一下：
        //  S? ExternalID?
        // 不对；原始的写法，是正确的……
        // doctypedecl   ::=  '<!DOCTYPE' S Name (S ExternalID)? S? ('[' intSubset ']' S?)? '>'
        //
        // NOTE 关于 html5_doctype <!DOCTYPE html> 的匹配
        //
        // 原始的表达式，本来就能匹配 <!DOCTYPE html> 当然，作为html5解析的话，
        // 只需要校验一下doctyee，是否与需要的相符即可……此时，可用正则表达式
        //
        // 貌似不用修改！

        int xml_parser::parse_doctypedecl(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
#if 1
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            Rewinder_t r2(s_beg);
            Rewinder_t r3(s_beg);
            SSS_LOG_DEBUG("i'm here!\n");
            std::string name;
            try {
                r.commit(sk.skip_str(s_beg, s_end, "<!DOCTYPE") &&
                         this->parse_S(s_beg, s_end) &&
                         this->parse_Name(s_beg, s_end) &&
                         ((r2.begin() && r2.commit(this->parse_S(s_beg, s_end) &&
                                                   this->parse_ExternalID(s_beg, s_end))) || true) &&
                         (this->parse_S(s_beg, s_end) || true) &&
                         ((r3.begin() && r3.commit(sk.skip_char(s_beg, s_end, '[') &&
                                                  this->parse_intSubset(s_beg, s_end) &&
                                                  sk.skip_char(s_beg, s_end, ']') &&
                                                  (this->parse_S(s_beg, s_end) || true))) || true) &&
                         sk.skip_char(s_beg, s_end, '>'));

                SSS_LOG_EXPRESSION(sss::log::log_DEBUG, r.count());
            }
            catch(skipper_t::Exception& e) {
                this->set_last_err(this->get_offset(s_beg), __func__, e.what());
                r.rollback();
            }
            catch(const char * msg) {
                this->set_last_err(this->get_offset(s_beg), __func__, msg);
                r.rollback();
            }
            return this->consume(__func__, r.count());
#else
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            try {
                sk.skip_str(s_beg, s_end, "<!DOCTYPE", "requre \"<!DOCTYPE\"");
                this->parse_S(s_beg, s_end) || or_throw("requre S 1060");
                this->parse_Name(s_beg, s_end) || or_throw("requre Name");

                {
                    Rewinder_t r2(s_beg);
                    if (this->parse_S(s_beg, s_end) && this->parse_ExternalID(s_beg, s_end)) {
                        r2.commit();
                    }
                    else {
                        r2.rollback();
                    }
                }

                this->parse_S(s_beg, s_end);

                {
                    Rewinder_t r3(s_beg);
                    if (sk.skip_char(s_beg, s_end, '[') &&
                        this->parse_intSubset(s_beg, s_end) &&
                        sk.skip_char(s_beg, s_end, ']'))
                    {
                        this->parse_S(s_beg, s_end);
                        r3.commit();
                    }
                    else {
                        r3.rollback();
                    }
                }

                sk.skip_char(s_beg, s_end, '>', "requre \">\"");
                r.commit();
            }
            catch(skipper_t::Exception& e) {
                this->set_last_err(this->get_offset(s_beg), __func__, e.what());
                r.rollback();
            }
            catch(const char * msg) {
                this->set_last_err(this->get_offset(s_beg), __func__, msg);
                r.rollback();
            }
            return this->consume(__func__, r.count());
#endif
        }
        // DeclSep         ::=     PEReference | S
        int xml_parser::parse_DeclSep(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            Rewinder_t r(s_beg);
            r.commit(this->parse_PEReference(s_beg, s_end) || this->parse_S(s_beg, s_end));
            return this->consume(__func__, r.count());
        }
        // intSubset   ::=    (markupdecl | DeclSep)*
        int xml_parser::parse_intSubset(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            while (this->parse_markupdecl(s_beg, s_end) || this->parse_DeclSep(s_beg, s_end));
            r.commit();
            return this->consume(__func__, r.count());
        }
        // markupdecl   ::=   elementdecl | AttlistDecl | EntityDecl | NotationDecl | PI | Comment
        // =>
        // markupdecl   ::=   S? (elementdecl | AttlistDecl | EntityDecl | NotationDecl | PI | Comment)
        int xml_parser::parse_markupdecl(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            this->parse_S(s_beg, s_end); // 这是我自己加上去的；貌似[ (markupdecl | DeclSep)* ] 中间允许空白符

            r.commit(this->parse_elementdecl(s_beg, s_end) ||
                     this->parse_AttlistDecl(s_beg, s_end) ||
                     this->parse_EntityDecl(s_beg, s_end) ||
                     this->parse_NotationDecl(s_beg, s_end) ||
                     this->parse_PI(s_beg, s_end) ||
                     this->parse_Comment(s_beg, s_end));
            return this->consume(__func__, r.count());
        }

        // extSubset       ::=          TextDecl? extSubsetDecl
        int xml_parser::parse_extSubset(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            Rewinder_t r(s_beg);
            r.commit((this->parse_TextDecl(s_beg, s_end) || true) &&
                     this->parse_extSubsetDecl(s_beg, s_end));
            return this->consume(__func__, r.count());
        }
        // extSubsetDecl           ::=          ( markupdecl | conditionalSect | DeclSep)*
        int xml_parser::parse_extSubsetDecl(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            Rewinder_t r(s_beg);
            while (this->parse_markupdecl(s_beg, s_end) ||
                   this->parse_conditionalSect(s_beg, s_end) ||
                   this->parse_DeclSep(s_beg, s_end));
            r.commit();
            return this->consume(__func__, r.count());
        }

        // SDDecl          ::=          S 'standalone' Eq SDDecl_Value
        int xml_parser::parse_SDDecl(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            try {
                this->parse_S(s_beg, s_end) || or_throw("requre S 1180");
                sk.skip_str(s_beg, s_end, "standalone", "requre \"standalone\"");
                this->parse_Eq(s_beg, s_end) || or_throw("requre Eq.");
                this->parse_SDDecl_Value(s_beg, s_end) || or_throw("requre SDDecl_Value");

            }
            catch(skipper_t::Exception& e) {
                this->set_last_err(this->get_offset(s_beg), __func__, e.what());
                r.rollback();
            }
            catch(const char * msg) {
                this->set_last_err(this->get_offset(s_beg), __func__, msg);
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }

        // SDDecl_Value    ::= ("'" ('yes' | 'no') "'") | ('"' ('yes' | 'no') '"')
        int xml_parser::parse_SDDecl_Value(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            if (sk.skip_char(s_beg, s_end, '\'')) {
                r.commit((sk.skip_str(s_beg, s_end, "yes") || sk.skip_str(s_beg, s_end, "no")) &&
                         sk.skip_char(s_beg, s_end, '\''));
            }
            else if (sk.skip_char(s_beg, s_end, '"')) {
                r.commit((sk.skip_str(s_beg, s_end, "yes") || sk.skip_str(s_beg, s_end, "no")) &&
                         sk.skip_char(s_beg, s_end, '\"'));
            }
            else {
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }

        // element   ::=  EmptyElemTag
        //              | STag content ETag
        // ==>
        // element   ::=  STag content? ETag
        //              | EmptyElemTag
        int xml_parser::parse_element(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            std::string tar(s_beg, s_end);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            // TODO FIXME NOTE 完蛋！我最担心的问题到了！content 是一个递归定义？
            // 何时才能完成对象树的构造？
            if (r.commit(this->parse_STag(s_beg, s_end) &&
                         (this->parse_content(s_beg, s_end) || true) &&
                         this->parse_ETag(s_beg, s_end)))
            {
                // NOTE
                SegmentXML * STag_name_seg = SegHandle(this->seg_get_current()).firstChild().firstChild().get();
                SegmentXML * ETag_name_seg = SegHandle(this->seg_get_current()).lastChild().firstChild().get();
                assert(STag_name_seg && ETag_name_seg); // TODO 我应该做一个自己的assert，以便获得更明确的信息
                assert(STag_name_seg->get_slice() == ETag_name_seg->get_slice());
            }
            else
            {
                r.commit(this->parse_EmptyElemTag(s_beg, s_end));
            }
            return this->consume(__func__, r.count());
        }

        // STag    ::=          '<' Name (S Attribute)* S? '>'
        // ==>
        // STag    ::=          '<' Name AttribList? S? '>'
        int xml_parser::parse_STag(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, std::string(s_beg, s_end));

            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            try {

                if (sk.skip_char(s_beg, s_end, '<') && this->parse_Name(s_beg, s_end)) {
                    this->parse_AttribList(s_beg, s_end);
                    this->parse_S(s_beg, s_end);
                    sk.skip_char(s_beg, s_end, '>', "requre right \">\"");
                    r.commit();
                }
                else {
                    r.rollback();
                }
                if (r.count()) {
                    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, r.get_consume());
                }
            }
            catch(skipper_t::Exception& e) {
                this->set_last_err(this->get_offset(s_beg), __func__, e.what());
                r.rollback();
            }
            catch(const char * msg) {
                this->set_last_err(this->get_offset(s_beg), __func__, msg);
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }

        // Attribute       ::=          Name Eq AttValue
        int xml_parser::parse_Attribute(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            try {
                this->parse_Name(s_beg, s_end) || or_throw("requre \"Name\".");
                this->parse_Eq(s_beg, s_end) || or_throw("requre \"Eq\".");
                this->parse_AttValue(s_beg, s_end) || or_throw("requre \"AttValue\".");
                r.commit();
            }
            catch(skipper_t::Exception& e) {
                this->set_last_err(this->get_offset(s_beg), __func__, e.what());
                r.rollback();
            }
            catch(const char * msg) {
                this->set_last_err(this->get_offset(s_beg), __func__, msg);
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }

        // ETag       ::=          '</' Name S? '>'
        int xml_parser::parse_ETag(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, std::string(s_beg, s_end));
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            r.commit(sk.skip_str(s_beg, s_end, "</") &&
                     this->parse_Name(s_beg, s_end) &&
                     (this->parse_S(s_beg, s_end) || true) &&
                     sk.skip_char(s_beg, s_end, '>'));
            if (r.count()) {
                SSS_LOG_EXPRESSION(sss::log::log_DEBUG, r.get_consume());
            }
            return this->consume(__func__, r.count());
        }

        // content    ::=          CharData? ((element | Reference | CDSect | PI | Comment) CharData?)*
        int xml_parser::parse_content(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, std::string(s_beg, s_end));
            Rewinder_t r(s_beg);
            this->parse_CharData(s_beg, s_end);
            while (true) {
                Rewinder_t r2(s_beg);
                if (this->parse_element(s_beg, s_end) ||
                    this->parse_Reference(s_beg, s_end) ||
                    this->parse_CDSect(s_beg, s_end) ||
                    this->parse_PI(s_beg, s_end) ||
                    this->parse_Comment(s_beg, s_end))
                {
                    this->parse_CharData(s_beg, s_end);
                    r2.commit();
                }
                else {
                    r2.rollback();
                    break;
                }
            }
            r.commit();
            if (r.count()) {
                SSS_LOG_EXPRESSION(sss::log::log_DEBUG, r.get_consume());
            }
            return this->consume(__func__, r.count());
        }

        // AttribList       ::=          (S Attribute)*
        int xml_parser::parse_AttribList(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);

            try {
                AttNameValidator att(this);
                while(true) {
                    Rewinder_t r2(s_beg);
                    if (this->parse_S(s_beg, s_end) && this->parse_Attribute(s_beg, s_end)) {
                        r2.commit();

                        att.checkIn();
                    }
                    else {
                        r2.rollback();
                        break;
                    }
                }
                r.commit(r.count());
            }
            catch(skipper_t::Exception& e) {
                this->set_last_err(this->get_offset(s_beg), __func__, e.what());
                r.rollback();
            }
            catch(const char * msg) {
                this->set_last_err(this->get_offset(s_beg), __func__, msg);
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }

        // EmptyElemTag       ::=          '<' Name (S Attribute)* S? '/>'
        // ==>
        // EmptyElemTag       ::=          '<' Name AttribList? S? '/>'
        int xml_parser::parse_EmptyElemTag(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, std::string(s_beg, s_end));
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);

            try {
                sk.skip_char(s_beg, s_end, '<') || or_throw("requre '<'");
                this->parse_Name(s_beg, s_end) || or_throw("requre parse_Name");
                this->parse_AttribList(s_beg, s_end);
                this->parse_S(s_beg, s_end);
                sk.skip_str(s_beg, s_end, "/>") || or_throw("requre \"/>\"");
                r.commit();
                //SSS_LOG_EXPRESSION(sss::log::log_DEBUG, r.get_consume());
            }
            catch(skipper_t::Exception& e) {
                this->set_last_err(this->get_offset(s_beg), __func__, e.what());
                r.rollback();
            }
            catch(const char * msg) {
                this->set_last_err(this->get_offset(s_beg), __func__, msg);
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }

        // elementdecl        ::=          '<!ELEMENT' S Name S contentspec S? '>'
        int xml_parser::parse_elementdecl(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
#if 0
            Rewinder_t r(s_beg);
            if (sk.skip_str(s_beg, s_end, "<!ELEMENT") &&
                this->parse_S(s_beg, s_end) &&
                this->parse_Name(s_beg, s_end) &&
                this->parse_S(s_beg, s_end) &&
                this->parse_contentspec(s_beg, s_end) &&
                (this->parse_S(s_beg, s_end) || true) &&
                sk.skip_str(s_beg, s_end, ">"))
            {
                r.commit();
            }
            else {
                r.rollback();
            }
            return this->consume(__func__, r.count());
#else
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, sss::utlstr::sample_string(s_beg, s_end));
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            try {
                sk.skip_str(s_beg, s_end, "<!ELEMENT") || or_throw("requre '<!ELEMENT'.");
                this->parse_S(s_beg, s_end) || or_throw("requre parse_S");
                this->parse_Name(s_beg, s_end) || or_throw("requre parse_Name");
                this->parse_S(s_beg, s_end) || or_throw("requre parse_S");
                this->parse_contentspec(s_beg, s_end) || or_throw("requre parse_contentspec");
                this->parse_S(s_beg, s_end);
                sk.skip_str(s_beg, s_end, ">") || or_throw("requre '>'.");
                r.commit();
                //SSS_LOG_EXPRESSION(sss::log::log_DEBUG, r.get_consume());
            }
            catch(skipper_t::Exception& e) {
                this->set_last_err(this->get_offset(s_beg), __func__, e.what());
                r.rollback();
            }
            catch(const char * msg) {
                this->set_last_err(this->get_offset(s_beg), __func__, msg);
                r.rollback();
            }
            return this->consume(__func__, r.count());
#endif
        }

        // contentspec        ::=          'EMPTY' | 'ANY' | Mixed | children
        int xml_parser::parse_contentspec(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            if (sk.skip_str(s_beg, s_end, "EMPTY")      ||
                sk.skip_str(s_beg, s_end, "ANY")        ||
                this->parse_Mixed(s_beg, s_end)         ||
                this->parse_children(s_beg, s_end))
            {
                r.commit();
            }
            else {
                r.rollback();
            }
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, r.get_consume());
            return this->consume(__func__, r.count());
        }

        // children   ::=   (choice | seq) ('?' | '*' | '+')?
        // =>
        // children   ::=   (choice | seq) Time_suffix?
        int xml_parser::parse_children(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            r.commit( ( this->parse_choice(s_beg, s_end) ||
                        this->parse_seq(s_beg, s_end)) &&
                      ( this->parse_Times_suffix(s_beg, s_end) ||
                        true));
            return this->consume(__func__, r.count());
        }

        // cp         ::=   (Name | choice | seq) ('?' | '*' | '+')?
        // =>
        // cp         ::=   (Name | choice | seq) Time_suffix?
        int xml_parser::parse_cp(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            Rewinder_t r(s_beg);
            r.commit( ( this->parse_Name(s_beg, s_end)      ||
                        this->parse_choice(s_beg, s_end)    ||
                        this->parse_seq(s_beg, s_end)) &&
                      ( this->parse_Times_suffix(s_beg, s_end) || true));
            return this->consume(__func__, r.count());
        }

        // Time_suffix ::= ('?' | '*' | '+')
        int xml_parser::parse_Times_suffix(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            r.commit( sk.skip_char(s_beg, s_end, '?')     ||
                      sk.skip_char(s_beg, s_end, '*')     ||
                      sk.skip_char(s_beg, s_end, '+'));
            return this->consume(__func__, r.count());
        }

        // NOTE 这里是分支的字符 '|' 而不是，分支“|”本身！
        // choice     ::=          '(' S? cp ( S? '|' S? cp )+ S? ')'
        int xml_parser::parse_choice(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            try {
                sk.skip_char(s_beg, s_end, '(', "requre \"(\"");
                this->parse_S(s_beg, s_end);
                this->parse_cp(s_beg, s_end) || or_throw("requre cp");

                {
                    int times = 0;
                    while (true)
                    {
                        Rewinder_t r2(s_beg);
                        if ((this->parse_S(s_beg, s_end) || true) &&
                            sk.skip_char(s_beg, s_end, '|') &&
                            (this->parse_S(s_beg, s_end), true) &&
                            this->parse_cp(s_beg, s_end))
                        {
                            times++;
                            r2.commit();
                        }
                        else {
                            r2.rollback();
                            break;
                        }
                    }
                    if (!times) {
                        throw "requre ( S? '|' S? cp )+ at least 1 time.";
                    }
                }

                this->parse_S(s_beg, s_end);
                sk.skip_char(s_beg, s_end, ')', "requre \")\"");
                r.commit();
            }
            catch(skipper_t::Exception& e) {
                this->set_last_err(this->get_offset(s_beg), __func__, e.what());
                r.rollback();
            }
            catch(const char * msg) {
                this->set_last_err(this->get_offset(s_beg), __func__, msg);
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }
        // seq        ::=          '(' S? cp ( S? ',' S? cp )* S? ')'
        int xml_parser::parse_seq(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            try {
                sk.skip_char(s_beg, s_end, '(', "requre \"(\"");
                this->parse_S(s_beg, s_end);
                this->parse_cp(s_beg, s_end) || or_throw("requre cp");

                {
                    while (true)
                    {
                        Rewinder_t r2(s_beg);
                        if ((this->parse_S(s_beg, s_end) || true) &&
                            sk.skip_char(s_beg, s_end, ',') &&
                            (this->parse_S(s_beg, s_end) || true) &&
                            this->parse_cp(s_beg, s_end))
                        {
                            r2.commit();
                        }
                        else {
                            r2.rollback();
                            break;
                        }
                    }
                }

                this->parse_S(s_beg, s_end);
                sk.skip_char(s_beg, s_end, ')', "requre \")\"");
                r.commit();
            }
            catch(skipper_t::Exception& e) {
                this->set_last_err(this->get_offset(s_beg), __func__, e.what());
                r.rollback();
            }
            catch(const char * msg) {
                this->set_last_err(this->get_offset(s_beg), __func__, msg);
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }

        // Mixed      ::=   '(' S? '#PCDATA' (S? '|' S? Name)* S? ')*'
        //                | '(' S? '#PCDATA'                   S? ')'
        // 可见，上述分支，在前三段，都是一致的；区别在于，第四段，'|' 这个字符！
        // 即，第一个非空白符，如果是'|'，就是分支一；反之，分支二！
        // 不过，我觉得，可以再将 S? 作为公用部分，重写为：
        //
        // Mixed      ::=   '(' S? '#PCDATA' S? (S? '|' S? Name)* S? ')*'
        //                | '(' S? '#PCDATA' S?                      ')'
        //
        // 这样，就可以用 '|' 作为分支标准了！
        // 哦，不对！不一定含有'|'；因为(S? '|' S? Name)* 后面是一个*！
        // 那岂不是要到最后的")*"才能确定分支？
        //
        // 再次重写：
        // Mixed      ::=   '(' S? '#PCDATA' S? ')' Mixed_suffix?
        //                | '(' S? '#PCDATA' S? (S? '|' S? Name)+ ')' Mixed_suffix
        //
        int xml_parser::parse_Mixed(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            try {

                if (sk.skip_char(s_beg, s_end, '(') &&
                    (this->parse_S(s_beg, s_end) || true) &&
                    sk.skip_str(s_beg, s_end, "#PCDATA") &&
                    (this->parse_S(s_beg, s_end) || true))
                {
                    Rewinder_t r2(s_beg);
                    if (sk.skip_char(s_beg, s_end, ')')) {
                        this->parse_Mixed_suffix(s_beg, s_end);
                        // 再增加一个递归函数即可；
                        r2.commit();
                    }
                    else if (sk.skip_char(s_beg, s_end, '|')) {
                        r2.rollback();

                        int times = 0;
                        while (true)
                        {
                            Rewinder_t r3(s_beg);
                            if ((this->parse_S(s_beg, s_end) || true) &&
                                sk.skip_char(s_beg, s_end, '|') &&
                                (this->parse_S(s_beg, s_end) || true) &&
                                this->parse_Name(s_beg, s_end))
                            {
                                r3.commit();
                                times++;
                            }
                            else {
                                r3.rollback();
                                break;
                            }
                        }
                        if (!times) {
                            throw "requre (S? '|' S? Name)+";
                        }
                        sk.skip_str(s_beg, s_end, ")", "requre \")\"");

                        this->parse_Mixed_suffix(s_beg, s_end) || or_throw("requre parse_Mixed_suffix");
                    }
                    else {
                        throw "requre \")\" or \"|\"";
                    }
                    r.commit();
                }
                else {
                    r.rollback();
                }
            }
            catch(skipper_t::Exception& e) {
                this->set_last_err(this->get_offset(s_beg), __func__, e.what());
                r.rollback();
            }
            catch(const char * msg) {
                this->set_last_err(this->get_offset(s_beg), __func__, msg);
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }

        // Mixed_suffix ::= '*'
        int xml_parser::parse_Mixed_suffix(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            r.commit(sk.skip_char(s_beg, s_end, '*'));
            return this->consume(__func__, r.count());
        }

        // AttlistDecl   ::=       '<!ATTLIST' S Name AttDef* S? '>'
        int xml_parser::parse_AttlistDecl(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            try {
                r.commit((sk.skip_str(s_beg, s_end, "<!ATTLIST") || or_throw("requre <!ATTLIST") )&&
                         (this->parse_S(s_beg, s_end) || or_throw("prequre arse_S")) &&
                         (this->parse_Name(s_beg, s_end) || or_throw("prequre arse_Name")) &&
                         (this->loop_times(s_beg, s_end, &xml_parser::parse_AttDef) || true) &&
                         (this->parse_S(s_beg, s_end) || true) &&
                         (sk.skip_str(s_beg, s_end, ">") || or_throw("requre >")));
            }
            catch(skipper_t::Exception& e) {
                this->set_last_err(this->get_offset(s_beg), __func__, e.what());
                r.rollback();
            }
            catch(const char * msg) {
                this->set_last_err(this->get_offset(s_beg), __func__, msg);
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }

        // AttDef     ::=          S Name S AttType S DefaultDecl
        int xml_parser::parse_AttDef(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            Rewinder_t r(s_beg);

            r.commit( this->parse_S(s_beg, s_end)       &&
                      this->parse_Name(s_beg, s_end)    &&
                      this->parse_S(s_beg, s_end)       &&
                      this->parse_AttType(s_beg, s_end) &&
                      this->parse_S(s_beg, s_end)       &&
                      this->parse_DefaultDecl(s_beg, s_end));
            return this->consume(__func__, r.count());
        }

        // AttType    ::=          StringType | TokenizedType | EnumeratedType
        int xml_parser::parse_AttType(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            Rewinder_t r(s_beg);

            r.commit( this->parse_StringType(s_beg, s_end)    ||
                      this->parse_TokenizedType(s_beg, s_end) ||
                      this->parse_EnumeratedType(s_beg, s_end));
            return this->consume(__func__, r.count());
        }

        // StringType         ::=          'CDATA'
        int xml_parser::parse_StringType(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            Rewinder_t r(s_beg);
            if (sk.skip_word(s_beg, s_end)) {
                std::string word;
                sk.last_matchs(word);
                if (word == "CDATA") {
                    r.commit();
                }
                else {
                    r.rollback();
                }
            }
            else {
                r.rollback();
            }

            return this->consume(__func__, r.count());
        }

        // TokenizedType ::= 'ID'
        //                 | 'IDREF'
        //                 | 'IDREFS'
        //                 | 'ENTITY'
        //                 | 'ENTITIES'
        //                 | 'NMTOKEN'
        //                 | 'NMTOKENS'
        // FIXME 这些串的匹配可能有问题！因为，部分值，恰好是另外部分的前缀！
        // 怎么办？应该是匹配一个word，然后，需要word在上述值之中！
        int xml_parser::parse_TokenizedType(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            Rewinder_t r(s_beg);
            if (sk.skip_word(s_beg, s_end)) {
                std::string word;
                sk.last_matchs(word);
                if ( word == "ID"       ||
                     word == "IDREF"    ||
                     word == "IDREFS"   ||
                     word == "ENTITY"   ||
                     word == "ENTITIES" ||
                     word == "NMTOKEN"  ||
                     word == "NMTOKENS")
                {
                    r.commit();
                }
                else {
                    r.rollback();
                }
            }
            else {
                r.rollback();
            }

            return this->consume(__func__, r.count());
        }

        // EnumeratedType     ::=  NotationType | Enumeration
        int xml_parser::parse_EnumeratedType(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            Rewinder_t r(s_beg);
            r.commit(this->parse_NotationType(s_beg, s_end) ||
                     this->parse_Enumeration(s_beg, s_end));
            return this->consume(__func__, r.count());
        }
        // NotationType       ::=  'NOTATION' S '(' S? Name (S? '|' S? Name)* S? ')'
        int xml_parser::parse_NotationType(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            Rewinder_t r(s_beg);
            try {
                if (sk.skip_str(s_beg, s_end, "NOTATION") &&
                    this->parse_S(s_beg, s_end) &&
                    sk.skip_str(s_beg, s_end, "(") &&
                    (this->parse_S(s_beg, s_end) || true) &&
                    this->parse_Name(s_beg, s_end))
                {
                    while (true)
                    {
                        Rewinder_t r3(s_beg);
                        if ((this->parse_S(s_beg, s_end) || true) &&
                            sk.skip_char(s_beg, s_end, '|') &&
                            (this->parse_S(s_beg, s_end) || true) &&
                            this->parse_Name(s_beg, s_end))
                        {
                            r3.commit();
                        }
                        else {
                            r3.rollback();
                            break;
                        }
                    }
                    this->parse_S(s_beg, s_end);
                    sk.skip_str(s_beg, s_end, ")", "requre \")\"");
                    r.commit();
                }
                else {
                    r.rollback();
                }
            }
            catch(skipper_t::Exception& e) {
                this->set_last_err(this->get_offset(s_beg), __func__, e.what());
                r.rollback();
            }
            catch(const char * msg) {
                this->set_last_err(this->get_offset(s_beg), __func__, msg);
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }
        // Enumeration        ::=          '(' S? Nmtoken (S? '|' S? Nmtoken)* S? ')'
        int xml_parser::parse_Enumeration(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            Rewinder_t r(s_beg);
            try {
                if (sk.skip_str(s_beg, s_end, "(") &&
                    (this->parse_S(s_beg, s_end) || true) &&
                    this->parse_Nmtoken(s_beg, s_end))
                {
                    while (true)
                    {
                        Rewinder_t r2(s_beg);
                        if ((this->parse_S(s_beg, s_end) || true) &&
                            sk.skip_char(s_beg, s_end, '|') &&
                            (this->parse_S(s_beg, s_end) || true) &&
                            this->parse_Nmtoken(s_beg, s_end))
                        {
                            r2.commit();
                        }
                        else {
                            r2.rollback();
                            break;
                        }
                    }
                    this->parse_S(s_beg, s_end);
                    sk.skip_str(s_beg, s_end, ")", "requre \")\"");
                    r.commit();
                }
                else {
                    r.rollback();
                }
            }
            catch(skipper_t::Exception& e) {
                this->set_last_err(this->get_offset(s_beg), __func__, e.what());
                r.rollback();
            }
            catch(const char * msg) {
                this->set_last_err(this->get_offset(s_beg), __func__, msg);
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }

        // DefaultDecl   ::= '#REQUIRED'
        //                 | '#IMPLIED'
        //                 | (('#FIXED' S)? AttValue)
        // 如何改写？
        // DefaultDecl   ::= '#REQUIRED'
        //                 | '#IMPLIED'
        //                 | '#FIXED' S AttValue
        //                 | AttValue
        int xml_parser::parse_DefaultDecl(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            if (sk.skip_str(s_beg, s_end, "#") && sk.skip_word(s_beg, s_end))
            {
                std::string word;
                sk.last_matchs(word);
                r.commit(word == "REQUIRED" ||
                         word == "IMPLIED" ||
                         (word == "FIXED" &&
                          this->parse_S(s_beg, s_end) &&
                          this->parse_AttValue(s_beg, s_end)));
            }
            else {
                r.rollback();
                r.commit(this->parse_AttValue(s_beg, s_end));
            }
            return this->consume(__func__, r.count());
        }

        // conditionalSect    ::=  includeSect | ignoreSect
        int xml_parser::parse_conditionalSect(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            Rewinder_t r(s_beg);
            if (this->parse_includeSect(s_beg, s_end) ||
                this->parse_ignoreSect(s_beg, s_end))
            {
                r.commit();
            }
            else {
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }

        // includeSect        ::=  '<![' S? 'INCLUDE' S? '[' extSubsetDecl ']]>'
        int xml_parser::parse_includeSect(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            if ( sk.skip_str(s_beg, s_end, "<![") &&
                 (this->parse_S(s_beg, s_end) || true) &&
                 sk.skip_str(s_beg, s_end, "INCLUDE") &&
                 (this->parse_S(s_beg, s_end) || true) &&
                 sk.skip_str(s_beg, s_end, "[") &&
                 this->parse_extSubsetDecl(s_beg, s_end) &&
                 sk.skip_str(s_beg, s_end, "]]>"))
            {
                r.commit();
            }
            else {
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }

        // ignoreSect         ::=          '<![' S? 'IGNORE' S? '[' ignoreSectContents* ']]>'
        int xml_parser::parse_ignoreSect(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            if ( sk.skip_str(s_beg, s_end, "<![") &&
                 (this->parse_S(s_beg, s_end) || true) &&
                 sk.skip_str(s_beg, s_end, "IGNORE") &&
                 (this->parse_S(s_beg, s_end) || true) &&
                 sk.skip_str(s_beg, s_end, "[") &&
                 (this->loop_times(s_beg, s_end, &xml_parser::parse_ignoreSectContents) || true) &&
                 sk.skip_str(s_beg, s_end, "]]>"))
            {
                r.commit();
            }
            else {
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }

        // ignoreSectContents         ::=          Ignore ('<![' ignoreSectContents ']]>' Ignore)*
        int xml_parser::parse_ignoreSectContents(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            Rewinder_t r(s_beg);
            if ( this->parse_Ignore(s_beg, s_end))
            {
                while (true)
                {
                    Rewinder_t r2(s_beg);
                    if (sk.skip_str(s_beg, s_end, "<![") &&
                        this->parse_ignoreSectContents(s_beg, s_end) &&
                        sk.skip_str(s_beg, s_end, "]]>") &&
                        this->parse_Ignore(s_beg, s_end))
                    {
                        r2.commit();
                    }
                    else {
                        r2.rollback();
                        break;
                    }
                }
                r.commit();
            }
            else {
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }

        // Ignore     ::=          Char* - (Char* ('<![' | ']]>') Char*)
        // 即，任意长的字符串，但不能包含 "<![" 或者 "]]>"
        // 另外，貌似这个允许匹配0长！那么，如何得到匹配成功或者失败的信息呢？
        // 难道说，我每个函数，都需要修改接口？
        // 只能返回bool类型？然后，利用内部成员，来记录到底当次，匹配了多少个字节的东西？
        // 不过，这种不包含的匹配，我觉得可以用while+预读；来代替。
        // 当然预读到的字符，等于禁忌字符串的时候，就表示可以停止了！
        int xml_parser::parse_Ignore(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            Rewinder_t r(s_beg);
            // NOTE 本函数，无法用skipper_t::skip_str_before() 代替！
            // 因为，有两种禁用情况！
            while (true) {
                if (sk.is_begin_with(s_beg, s_end, "<![") ||
                    sk.is_begin_with(s_beg, s_end, "]]>"))
                {
                    break;
                }

                // NOTE 如何避免死循环？
                // 貌似，退出条件，只有预读等于 "<![" 和 "]]>"的时候！不对，应当先预读，然后读取一个字符！
                if (!this->parse_Char(s_beg, s_end))
                {
                    // 竟然读取不到字符！说明可能遇到文件结尾了；
                    break;
                }
            }
            r.commit();
            return this->consume(__func__, r.count());
        }

        // CharRef    ::= '&#' [0-9]+ ';'
        //              | '&0x' [0-9a-fA-F]+ ';'
        int xml_parser::parse_CharRef(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            Rewinder_t r(s_beg);
            r.commit(sk.skip_str(s_beg, s_end, "&#") && sk.skip_dec(s_beg, s_end) && sk.skip_str(s_beg, s_end, ";")) ||
                r.commit(sk.skip_str(s_beg, s_end, "&0x") && sk.skip_hex(s_beg, s_end) && sk.skip_str(s_beg, s_end, ";"));
            return this->consume(__func__, r.count());
        }

        // Reference          ::=          EntityRef | CharRef
        int xml_parser::parse_Reference(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            Rewinder_t r(s_beg);
            r.commit(this->parse_EntityRef(s_beg, s_end) ||
                     this->parse_CharRef(s_beg, s_end));
            return this->consume(__func__, r.count());
        }
        // EntityRef          ::=          '&' Name ';'
        int xml_parser::parse_EntityRef(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);

            r.commit(sk.skip_str(s_beg, s_end, "&") &&
                     this->parse_Name(s_beg, s_end) &&
                     sk.skip_str(s_beg, s_end, ";"));

            return this->consume(__func__, r.count());
        }
        // PEReference        ::=          '%' Name ';'
        int xml_parser::parse_PEReference(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);

            r.commit(sk.skip_str(s_beg, s_end, "%") &&
                     this->parse_Name(s_beg, s_end) &&
                     sk.skip_str(s_beg, s_end, ";"));

            return this->consume(__func__, r.count());
        }

        // EntityDecl ::=   GEDecl | PEDecl
        int xml_parser::parse_EntityDecl(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            Rewinder_t r(s_beg);
            r.commit(this->parse_GEDecl(s_beg, s_end) ||
                     this->parse_PEDecl(s_beg, s_end));
            return this->consume(__func__, r.count());
        }
        // GEDecl     ::=   '<!ENTITY' S Name S EntityDef S? '>'
        int xml_parser::parse_GEDecl(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);

            r.commit(sk.skip_str(s_beg, s_end, "<!ENTITY") &&
                     this->parse_S(s_beg, s_end) &&
                     this->parse_Name(s_beg, s_end) &&
                     this->parse_S(s_beg, s_end) &&
                     this->parse_EntityRef(s_beg, s_end) &&
                     (this->parse_S(s_beg, s_end) || true) &&
                     sk.skip_str(s_beg, s_end, ">"));

            return this->consume(__func__, r.count());
        }
        // PEDecl     ::=   '<!ENTITY' S '%' S Name S PEDef S? '>'
        int xml_parser::parse_PEDecl(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);

            r.commit(sk.skip_str(s_beg, s_end, "<!ENTITY") &&
                     this->parse_S(s_beg, s_end) &&
                     sk.skip_str(s_beg, s_end, "%") &&
                     this->parse_S(s_beg, s_end) &&
                     this->parse_Name(s_beg, s_end) &&
                     this->parse_S(s_beg, s_end) &&
                     this->parse_PEDef(s_beg, s_end) &&
                     (this->parse_S(s_beg, s_end) || true) &&
                     sk.skip_str(s_beg, s_end, ">"));

            return this->consume(__func__, r.count());
        }
        // EntityDef  ::=   EntityValue | (ExternalID NDataDecl?)
        int xml_parser::parse_EntityDef(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);

            r.commit(this->parse_EntityValue(s_beg, s_end) ||
                     (this->parse_ExternalID(s_beg, s_end) && (this->parse_NDataDecl(s_beg, s_end) || true)));

            return this->consume(__func__, r.count());
        }
        // PEDef      ::=   EntityValue | ExternalID
        int xml_parser::parse_PEDef(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);

            r.commit(this->parse_EntityValue(s_beg, s_end) ||
                     this->parse_ExternalID(s_beg, s_end));

            return this->consume(__func__, r.count());
        }

        // ExternalID  ::=   'SYSTEM' S SystemLiteral
        //         | 'PUBLIC' S PubidLiteral S SystemLiteral
        int xml_parser::parse_ExternalID(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);

            r.commit((sk.skip_str(s_beg, s_end, "SYSTEM") &&
                      this->parse_S(s_beg, s_end) &&
                      this->parse_SystemLiteral(s_beg, s_end)) ||
                     (sk.skip_str(s_beg, s_end, "PUBLIC") &&
                      this->parse_S(s_beg, s_end) &&
                      this->parse_PubidLiteral(s_beg, s_end) &&
                      this->parse_S(s_beg, s_end) &&
                      this->parse_SystemLiteral(s_beg, s_end)));

            return this->consume(__func__, r.count());
        }
        // NDataDecl   ::=   S 'NDATA' S Name
        int xml_parser::parse_NDataDecl(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);

            r.commit( this->parse_S(s_beg, s_end) &&
                      sk.skip_str(s_beg, s_end, "NDATA") &&
                      this->parse_S(s_beg, s_end) &&
                      this->parse_Name(s_beg, s_end));

            return this->consume(__func__, r.count());
        }

        // TextDecl   ::=   '<?xml' VersionInfo? EncodingDecl S? '?>'
        int xml_parser::parse_TextDecl(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);

            r.commit( sk.skip_str(s_beg, s_end, "<?xml") &&
                      (this->parse_VersionInfo(s_beg, s_end) || true) &&
                      this->parse_EncodingDecl(s_beg, s_end) &&
                      (this->parse_S(s_beg, s_end) || true) &&
                      sk.skip_str(s_beg, s_end, "?>"));

            return this->consume(__func__, r.count());
        }

        // extParsedEnt   ::=   TextDecl? content
        int xml_parser::parse_extParsedEnt(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);

            r.commit ( (this->parse_TextDecl(s_beg, s_end) || true) &&
                       this->parse_content(s_beg, s_end));

            return this->consume(__func__, r.count());
        }

        // EncodingDecl   ::=   S 'encoding' Eq ('"' EncName '"' | "'" EncName "'" )
        // FIXME
        // <?xml version="1.0" encoding="'cp936'?>
        // 这种，也会被解析正确，成为：
        // <?xml version="1.0" encoding="cp936"?>
        // 问题的关键在于，连续动作，必须用 Rewinder_t 再次包裹起来，不然无法完全回退！
        //
        // 所有有分支的地方，都需要修改！
        //
        // TODO
        // 这样，如果为了保留链式操作，可以再增加一个bool
        // Rewinder_t::begin() 操作，这样，可以先把需要的Rewinder_t定义
        // 好，然后在需要的地方使用；——其功能是用当前s_beg值，更新构造函数时值；
        //
        // r2.begin() &&
        // (r2.commit(sk.skip_char(s_beg, s_end, '"') &&
        //   this->parse_EncName(s_beg, s_end) &&
        //   sk.skip_char(s_beg, s_end, '"')) ||
        //  r2.commit(sk.skip_char(s_beg, s_end, '\'') &&
        //   this->parse_EncName(s_beg, s_end) &&
        //   sk.skip_char(s_beg, s_end, '\'')))
        //
        // 基本能看！
        //
        //! 细节讨论
        //不同Rewinder_t 对象，析构时候的干扰。
        // 考虑
        // Rewinder_t r;
        // Rewinder_t r1;
        // {
        //   r1.begin()
        //   ...
        // }
        //
        // r1 如果恰好是一条分支的and序列；但并没有执行到，它的begin动作；
        // 到最后，对象的析构顺序，是r1.~Rewinder_t(); r.~Rewinder_t()；
        // 此时，貌似也能保证外层的r将s_beg定位到合适的位置！
        // 不管是否需要rewind()！
        //
        // 不对！我的回滚动作是rewind函数触发，而不是析构函数触发！
        // 得修改一下，才能保证动作被正确的顺序执行！
        //
        // 如果要回到开始，再重新选路，怎么办？那岂不是要立即回到标记位置？
        //
        // 像下面这种，关键在于 ::begin() 与 commit(bool)函数，还有内部的 && 序列 的顺序！
        // 虽然说，我规定，里面只能是&&操作，但是如果用户把||操作序列放进去，貌似也没有问题……
        //
        // r2.begin() &&
        // (r2.commit(sk.skip_char(s_beg, s_end, '"') &&
        //   this->parse_EncName(s_beg, s_end) &&
        //   sk.skip_char(s_beg, s_end, '"')) ||
        //  r2.commit(sk.skip_char(s_beg, s_end, '\'') &&
        //   this->parse_EncName(s_beg, s_end) &&
        //   sk.skip_char(s_beg, s_end, '\'')))
        // 类似上面的改进之后，下面的：
        //
        //   <?xml version="1.0" encoding="'cp936'?>
        //
        // 被解析为parse_PI……竟然通过了！说明PI的识别，确实有问题；它
        // 应该这样，先将其进行粉刺；可识别的，至少有：名字、字符串等元
        // 素；
        //
        // 当然，"?>"还是得作为其结束符；然后内部因为 "'cp936'
        // 并没有正常闭合，而失败……
        //
        // FIXME 即，parse_PI的产生式，需要再进行调整！
        int xml_parser::parse_EncodingDecl(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            Rewinder_t r1(s_beg);
            XML_PARSER_SEG_HELPER(r);
            if ((this->parse_S(s_beg, s_end) &&
                 sk.skip_str(s_beg, s_end, "encoding") &&
                 this->parse_Eq(s_beg, s_end)) &&

                r1.begin() &&

                (r1.commit(sk.skip_char(s_beg, s_end, '"') &&
                           this->parse_EncName(s_beg, s_end) &&
                           sk.skip_char(s_beg, s_end, '"')) ||
                 r1.commit(sk.skip_char(s_beg, s_end, '\'') &&
                           this->parse_EncName(s_beg, s_end) &&
                           sk.skip_char(s_beg, s_end, '\''))))
            {
                r.commit();
            }
            else {
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }

        // EncName   ::=   [A-Za-z] ([A-Za-z0-9._] | '-')* /* Encoding name contains only Latin characters */
        int xml_parser::parse_EncName(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            if (sk.skip_if_times(s_beg, s_end, ::isalpha, 1)) {
                while (sk.skip_if_times(s_beg, s_end, ::isalnum, 1) ||
                       sk.skip_char(s_beg, s_end, '.') ||
                       sk.skip_char(s_beg, s_end, '_') ||
                       sk.skip_char(s_beg, s_end, '-'));
                r.commit();
            }
            else {
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }

        // NotationDecl       ::=          '<!NOTATION' S Name S (ExternalID | PublicID) S? '>'
        int xml_parser::parse_NotationDecl(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);
            if (sk.skip_str(s_beg, s_end, "<!NOTATION") &&
                this->parse_S(s_beg, s_end) &&
                this->parse_Name(s_beg, s_end) &&
                this->parse_S(s_beg, s_end) &&

                (this->parse_ExternalID(s_beg, s_end) ||
                 this->parse_PublicID(s_beg, s_end)) &&

                (this->parse_S(s_beg, s_end) || true) &&
                sk.skip_char(s_beg, s_end, '>'))
            {
                r.commit();
            }
            else {
                r.rollback();
            }
            return this->consume(__func__, r.count());
        }

        // NOTE 未使用 ？ FIXME
        // PublicID           ::=          'PUBLIC' S PubidLiteral
        int xml_parser::parse_PublicID(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            XML_PARSER_SEG_HELPER(r);

            r.commit (sk.skip_str(s_beg, s_end, "PUBLIC") &&
                      this->parse_S(s_beg, s_end) &&
                      this->parse_PubidLiteral(s_beg, s_end));

            return this->consume(__func__, r.count());
        }

        // NOTE 未使用
        // Letter     ::=   BaseChar | Ideographic
        int xml_parser::parse_Letter(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            Rewinder_t r(s_beg);

            r.commit(this->parse_BaseChar(s_beg, s_end) &&
                     this->parse_Ideographic(s_beg, s_end));

            return this->consume(__func__, r.count());
        }
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
        int xml_parser::parse_BaseChar(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            Rewinder_t r(s_beg);

            // FIXME
            r.commit(sk.skip_char_any(s_beg, s_end));
            return this->consume(__func__, r.count());
        }

        // Ideographic   ::=   [0x4E00-0x9FA5] | 0x3007 | [0x3021-0x3029]
        int xml_parser::parse_Ideographic(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            // FIXME
            return this->parse_BaseChar(s_beg, s_end);
        }

        // NOTE 未使用
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
        int xml_parser::parse_CombiningChar(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            // FIXME
            return this->parse_BaseChar(s_beg, s_end);
        }

        // NOTE 未使用
        // Digit   ::=   [0x0030-0x0039] | [0x0660-0x0669] | [0x06F0-0x06F9] | [0x0966-0x096F] | [0x09E6-0x09EF]
        //                | [0x0A66-0x0A6F] | [0x0AE6-0x0AEF] | [0x0B66-0x0B6F] | [0x0BE7-0x0BEF] | [0x0C66-0x0C6F]
        //                | [0x0CE6-0x0CEF] | [0x0D66-0x0D6F] | [0x0E50-0x0E59] | [0x0ED0-0x0ED9] | [0x0F20-0x0F29]
        int xml_parser::parse_Digit(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            Rewinder_t r(s_beg);

            // FIXME
            r.commit(sk.skip_if_times(s_beg, s_end, ::isdigit, 1));

            return this->consume(__func__, r.count());
        }

        // NOTE 未使用
        // Extender  ::=   0x00B7 | 0x02D0 | 0x02D1 | 0x0387 | 0x0640 | 0x0E46 | 0x0EC6 | 0x3005 | [0x3031-0x3035]
        //                | [0x309D-0x309E] | [0x30FC-0x30FE]
        int xml_parser::parse_Extender(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            (void)s_beg;
            (void)s_end;
            return this->consume(__func__, 0);
        }

        void xml_parser::set_last_err(int off, const char * func_name, const std::string& msg)
        {
            if (this->last_err_offset < off) {
                this->last_err_offset = off;
                this->last_err_func_name = func_name;
                this->last_err_msg = msg;
            }
        }

        void xml_parser::print_last_err(std::ostream& out) const {
            out << __func__ << "@" << this->last_err_offset
                << " caller:" << this->last_err_func_name
                << " msg:" << this->last_err_msg
                << " sample: `"
                << std::string(this->sbeg_bak + this->last_err_offset,
                               this->sbeg_bak + this->last_err_offset + 100)
                << "`"
                << std::endl;
            this->seg_root->print(out, this->sbeg_bak);
        }
    }
}
