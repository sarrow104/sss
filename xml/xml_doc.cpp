#include "xml_doc.hpp"
#include "xml_parser.hpp"

#include <sss/utlstring.hpp>
#include <sss/regex/simpleregex.hpp>
#include <sss/spliter.hpp>
#include <sss/path.hpp>
#include <sss/log.hpp>

#include <stdexcept>
#include <fstream>
#include <map>
#include <sstream>

namespace sss{
    namespace xml{

#ifdef __WIN32__
#define TERMINAL_ENCODING "cp936"
#else
#define TERMINAL_ENCODING "utf-8"
#endif

        bom_str_table_t bom_str_table;

        bool init_bom_str_table() {
            if (bom_str_table.empty()) {
                bom_str_table["utf-8"] = "\xef\xbb\xbf";
                return true;
            }
            return false;
        }

        const static bool bom_str_table_status = init_bom_str_table();

        xml_doc::xml_doc()
            : node("", this, type_xmldoc),
              is_bom_enable(false)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
        }

        xml_doc::xml_doc(const std::string& root_name,
                         const std::string& charset,
                         int has_bom)
            : node("", this, type_xmldoc),
              is_bom_enable(has_bom),
              fencoding(charset)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            this->icv.set(charset, TERMINAL_ENCODING);

            // NOTE�������Ⱥ������node_info�͸��ڵ㣺
            this->append_child(this->create_info("xml"));
            this->first_child()->set("encoding", charset);

            this->append_child(this->create_node(root_name));
        }

        xml_doc::xml_doc(const std::string& charset, int has_bom)
            : node("", this, type_xmldoc),
              is_bom_enable(has_bom)
              //icv(charset, TERMINAL_ENCODING)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            this->icv.set(charset, TERMINAL_ENCODING);

            // NOTE����������˸��ڵ㣺
            this->append_child(this->create_info("xml"));
            this->first_child()->set("encoding", charset);
        }

        xml_doc::~xml_doc()
        {
            this->clear();
        }

        node * xml_doc::root() const
        {
            node_list_t roots = this->find_child("*");
            node * ret = 0;
            switch (roots.size()) {
            case 0:
                ret = 0;
                break;

            case 1:
                ret = roots[0];
                break;

            default:
                for (size_t i = 0; i != this->size(); ++i) {
                    std::cout << "sub " << i << ":" << this->node_list[i]->get_data()
                        << "|" << this->node_list[i]->is_node("*")
                        << std::endl;
                }
                throw std::logic_error("xml:multy root node error!");
            }
            return ret;
        }

        node * xml_doc::root(const std::string& name)
        {
            node * root = this->root();
            if (root) {
                root->set_data(name);
            }
            else {
                root = this->append_child(this->create_node(name));
            }
            return root;
        }

        std::string xml_doc::root_name() const
        {
            std::string name;
            node * root = this->root();
            if (root) {
                name = root->get_data();
            }
            return name;
        }

        std::string xml_doc::set_charset(const std::string& charset)
        {
            node * p_info = this->verify_charset_node();
            std::string old_encoding = p_info->get("encoding");

            bool reset = (old_encoding != charset);

            if (reset) {
                p_info->set("encoding", charset);
            }

            if (reset) {
                this->icv.set(charset, TERMINAL_ENCODING);
            }

            return old_encoding;
        }

        std::string xml_doc::get_charset()
        {
            return this->verify_charset_node()->get("encoding");
        }

        // TODO
        // ���charset,���ڵ�ȱ�Ҫ��Ϣ����ȷ��
        bool        xml_doc::validate() const
        {
            //if (this->bom_str.empty()) {
            //    tmp_doc->set_charset("utf-8");
            //}
            //else {
            //    tmp_doc->set_charset(this->bom_str);
            //}
            //tmp_doc->root();
            return true;
        }
        node * xml_doc::verify_charset_node()
        {
            node * ret = this->first_child();
            if ( ret && ret->node_type == type_info) {
                return ret;
            }
            else {
                ret = this->create_info("xml");
                // NOTE ��Ҫ������Ĭ��ֵ��
                ret->set("encoding", "");
                this->insert_before(ret);
                return ret;
            }
        }

        typedef std::map<std::string, std::string> bom_str_table_t;

        void xml_doc::print(std::ostream& out, const char * sep) const
        {
            for (size_t i = 0; i != this->size(); ++i) {
                this->node_list[i]->print(out, sep);
            }
        }

        void xml_doc::write(const std::string& fname, const char * sep)
        {
            std::ostringstream oss;
            this->print(oss, sep);
            std::ofstream ofs(fname.c_str(), std::ios::binary);
            std::string out_str;
            if (!this->icv.is_ok()) {
                this->icv.set("cp936","cp936");
            }
            icv.convert(out_str, oss.str());

            if (is_bom_enable) {
                bom_str_table_t::const_iterator it
                    = bom_str_table.find(this->get_charset());
                if (it != bom_str_table.end()) {
                    ofs << it->second;
                }
            }
            ofs << out_str;
        }

        // ���ⲿ�ļ�����
        void xml_doc::load(const std::string& fname)
        {
            std::string fcontent;
            sss::path::file2string(fname, fcontent);
            xml_parser xp;
            xp.parse(this, fcontent);
        }

        // д���ⲿ�ļ���
        void xml_doc::save(const std::string& fname)
        {
            this->write(fname, "\t");
        }

        void xml_doc::clear()
        {
            this->is_bom_enable = false;
            this->node::clear();
        }

        std::string xml_doc::safe_xml_entites(const std::string& raw_string)
        {
            std::string ret = raw_string;
            sss::replace_all(ret, "&", "&amp;");
            sss::replace_all(ret, "<", "&lt;");
            sss::replace_all(ret, ">", "&gt;");
            return ret;
        }

        node_text * xml_doc::create_text(const std::string& text)
        {
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, xml_doc::safe_xml_entites(text));
            return new node_text(xml_doc::safe_xml_entites(text), this);
        }

        node *      xml_doc::create_node(const std::string& name)
        {
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, name);
            return new node(name, this);
        }

        // FIXME 2015-07-10 ��Ҫ��ע��������ʹ��"--" ��
        // "--" ֻ��ʹ���� XHTML ע�͵Ŀ�ͷ�ͽ��������ܳ�����ע�͵������С�����
        // ��д�����ǲ�����ģ�
        // <!--Invalid -- and so is the classic "separator" below. -->
        // <!------------------------------------>
        node *      xml_doc::create_comment(const std::string& data)
        {
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, data);
            return new node_comment(data, this);
        }

        node *      xml_doc::create_info(const std::string& data)
        {
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, data);
            return new node_info(data, this);
        }

        node *      xml_doc::create_doctype(const std::string& data)
        {
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, data);
            return new node_doctype(data, this);
        }

        node *      xml_doc::create_cdata(const std::string& data)
        {
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, data);
            return new node_cdata(data, this);
        }

        // Ŀ�ģ���a#id[href="#"]*10���ֽ�Ϊ��tag��id��class��properties_str
        // ����ֵҪ�㣺
        //  1. �������Ե�ֵ�����⣬�����ܺ��пո�
        //  2. #id �� .class ����ͬʱ�ṩ������ .class ���Զ���ṩ�����ɵĶ���
        //     <a class="A B" href="jsjidji" ></a>
        //  3. *timesֻ�ܳ��������
        //  4. .class����$ռλ����
        // ��ˣ�����ͨ��[]���������ַ���������Ϊ3�����֣�
        // tag��\(.class��\|#id��\)
        //      [key="value"[ ...]]
        //              *times
        class xml_zen_matcher{
        public:
            xml_zen_matcher(const std::string& zen_string)
                : zen_str(zen_string), times(1)
            {
                SSS_LOG_DEBUG("(%s)\n", zen_string.c_str());

                static sss::regex::simpleregex
                    reg_properties("\\[\\([^\\[\\]]*\\)\\]");

                std::string::const_iterator z_beg = zen_str.begin();
                std::string::const_iterator z_end = zen_str.end();

                // NOTE b:means block brase [];
                std::string::const_iterator b_left;
                std::string::const_iterator b_right;

                while (reg_properties.match(z_beg, z_end, b_left, b_right)) {
                    zen_str =
                        std::string(z_beg, b_left) + std::string(b_right, z_end);
                    z_beg = zen_str.begin();
                    z_end = zen_str.end();
                    this->properties.parse(reg_properties.get_submatch(1));
                }

                if (sss::regex::simpleregex("\\s").match(zen_str)) {
                    throw std::runtime_error(zen_string + ":contains invalid \" \"");
                }

                if (sss::regex::simpleregex("\\*").match(zen_str)) {
                    sss::regex::simpleregex reg_times("\\*\\(\\d+\\)$");
                    if (!reg_times.match(zen_str)) {
                        throw std::runtime_error(zen_string + ":not end with \\*\\d");
                    }
                    this->times = sss::string_cast<int>(reg_times.get_submatch(1));
                    zen_str.resize(reg_times.start_pos);
                }

                static sss::regex::simpleregex
                    reg_tag_only("^[^#.]+$");

                SSS_LOG_DEBUG("zen_str = \"%s\"\n", zen_str.c_str());
                SSS_LOG_DEBUG("zen_string = \"%s\"\n", zen_string.c_str());
                if (reg_tag_only.match(zen_str)) {
                    this->tag = zen_str;
                }
                else {
                    static sss::regex::simpleregex
                        reg_tag("^\\(\\<\\c\\w*\\>\\)[#.]");

                    z_beg = zen_str.begin();
                    z_end = zen_str.end();
                    reg_tag.match(z_beg, z_end, b_left, b_right);
                    this->tag = reg_tag.get_submatch(1);
                    z_beg = b_right - 1;

                    SSS_LOG_DEBUG("this->tag = \"%s\"\n", this->tag.c_str());

                    // FIXME ����������ʽ����Ҫ֧��̰��������
                    static sss::regex::simpleregex
                        reg_id("^#\\(\\<[^#.]+\\>\\)");
                        //reg_id("^#\\(\\<abcd\\>\\)");

                    zen_str = std::string(z_beg, z_end);
                    z_beg = zen_str.begin();
                    z_end = zen_str.end();
                    SSS_LOG_DEBUG("std::count(\"%s\", '#') = %d\n",
                                  zen_str.c_str(),
                                  std::count(z_beg, z_end, '#'));

                    switch (std::count(z_beg, z_end, '#')) {
                    case 0:
                        break;

                    case 1:
                        if (reg_id.match(z_beg, z_end, b_left, b_right)) {
                            SSS_LOG_DEBUG("id = \"%s\"\n",
                                          std::string(b_left, b_right).c_str());
                            this->id = reg_id.get_submatch(1);
                            SSS_LOG_DEBUG("this->id = \"%s\"\n", this->id.c_str());
                            zen_str =
                                std::string(z_beg, b_left)
                                + std::string(b_right, z_end);
                            z_beg = zen_str.begin();
                            z_end = zen_str.end();
                        }
                        else {
                            throw std::runtime_error(zen_string + ":wrong id");
                        }
                        break;

                    default:
                        throw std::runtime_error(zen_string + ":multy id");
                    }

                    std::replace(zen_str.begin(), zen_str.end(), '.', ' ');
                    this->Class = zen_str;
                    sss::trim(this->Class);
                    SSS_LOG_DEBUG(".Class = \"%s\"\n", this->Class.c_str());
                }
                zen_str = zen_string;
                SSS_LOG_DEBUG("(%s) - out\n", zen_string.c_str());
            }

            ~xml_zen_matcher()
            {
            }

        public:
            std::string         zen_str;
            std::string         tag;
            std::string         id;
            std::string         Class;
            int                 times;
            properties_t        properties;
        };

        // html>head+body>div.test_div>ul>li.list-item*10>a[href="#"]>p>span[style="color:red;"]
        // ���ϣ�����ʶ�����Ϊ��
        //  tag��
        //      .class��
        //      #id��
        //
        //  ��.class ���� #id ���κ�� tag�����������ٸ�����[]���������ԣ�
        //  ��󣬻����Ը���һ�� *times ����ʾ�ظ��Ĵ�����
        node_list_t xml_doc::create_zencoding(const std::string& script)
        {
            node_list_t ret;

            //static sss::regex::simpleregex reg_is_node_with_id("^\\c\\w*#\\c\\w*$");

            //static sss::regex::simpleregex reg_is_node_with_index("^\\c\\w*:\\d+$");

            //static sss::regex::simpleregex reg_is_node_with_multy("^\\c\\w*\\*\\d+$");

            static sss::regex::simpleregex reg_is_node_with_sibling("\\+");

            // NOTE ֧���ֵܽڵ�չ�� {{{1
            // �鷳���ڣ������"*"��ʽ��ϣ�
            //
            // ע�⣬'>'���ص��ǣ�"β���"��
            //
            // ����˵������div+p*3>a��չ�����ǣ�
            //
            // <div>
            // </div>
            // <p><a></a></p>
            // <p><a></a></p>
            // <p><a></a></p>
            //
            // ������
            // <div><a></a></div>
            // <p><a></a></p>
            // <p><a></a></p>
            // <p><a></a></p>
            //
            // ����������󣬿��Կ������ݹ��з֣����ǿ��Խ����������ġ�
            //
            // һ��һ��ؿ��ǣ�
            //
            // ���ȣ���\w+\*\d\+Ϊ��Ԫ�����в��ң�
            // ����������'+'�������µĲ��֣���'+'��֣�
            // �ֳ�ѭ�����������������һ���ڵ���жϾͳ������⡪������ýڵ�
            // ������'>'���ţ������µ�script�����ָ�����
            //
            // ��
            //          body>div*2+block+p*3>span>a+em
            //
            // ��������Ϊ��
            //          body
            //                  div*2
            //                  block
            //                  p*3>span
            //                          a
            //                          em
            // NOTE �ȣ�����'>'�з֣�Ȼ�����ÿһ�ϣ���'+'�п���ѭ������
            // Ȼ��'>'�ĺ������֣�ֻ��'+'�����һ����������ã�
            // }}}1

            sss::Spliter sp(script, '>');
            std::string stem;
            if (sp.fetch_next(stem)) {

                std::string rest_script;
                if (script.length() >= stem.length() + 1) {
                    rest_script = script.substr(stem.length() + 1 );
                }

                if (reg_is_node_with_sibling.match(stem)) {
                    std::vector<std::string> items;
                    sss::util::split(stem, '+', items);

                    if (!rest_script.empty()) {
                        items.back() += '>' + rest_script;
                    }

                    for (size_t i = 0; i != items.size(); ++i) {
                        node_list_t tmp_subs
                            = this->create_zencoding(items[i]);
                        std::copy(tmp_subs.begin(),
                                  tmp_subs.end(),
                                  std::back_inserter(ret));
                    }
                }

                else {// ����'*'
                    xml_zen_matcher zen_stem(stem);
                    node * tmp = 0;
                    for (int i = 0; i < zen_stem.times; ++i) {
                        tmp = this->create_node(zen_stem.tag);
                        tmp->properties = zen_stem.properties;

                        if (!zen_stem.id.empty()) {
                            tmp->set("id", zen_stem.id);
                        }

                        if (!zen_stem.Class.empty()) {
                            tmp->set("class", zen_stem.Class);
                        }

                        if (!rest_script.empty()) {
                            tmp->append_child(this->create_zencoding(rest_script));
                        }

                        ret.push_back(tmp);
                    }
                }
            }

            return ret;
        }
    }
}
