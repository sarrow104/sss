#include "xmlcfg_file.hpp"
#include <sss/log.hpp>

#include <sss/path.hpp>

namespace sss {
    namespace xml2 {
        xmlcfg_file::xmlcfg_file(const std::string& _fname)
            : fname(_fname), is_modified(false)
        {
#if 0
            if (sss::path::file_exists(fname)) {
                this->my_doc.load(_fname);
                SSS_LOG_EXPRESSION(sss::log::log_DEBUG, my_doc->root()->get_data());
            }
#else
            this->my_doc.load(_fname);
#endif
        }

        // 因为可以通过 sss::xml2::node * 节点指针来操作、修改xml结构，这就导致
        // 了，用 xmlcfg_file::is_modified 来判断是否经过修改，没有什么用的。
        xmlcfg_file::~xmlcfg_file()
        {
            //if (is_modified) {
            //    my_doc->save(this->fname);
            //}
        }

        int xmlcfg_file::subnode_count(const std::string& path)
        {
            try {
                sss::xml2::node * p_node = my_doc->locate(path);
                if (p_node) {
                    return p_node->size();
                }
                return 0;
            }
            catch (...) {
                return 0;
            }
        }

        sss::xml2::xmlcfg_file::iterator xmlcfg_file::sub_begin(const std::string& path)
        {
            sss::xml2::node * p_node = my_doc->locate(path);
            if (p_node) {
                return p_node->node_list.begin();
            }
            else {
                return my_doc->node_list.begin();
            }
        }

        sss::xml2::xmlcfg_file::iterator xmlcfg_file::sub_end(const std::string& path)
        {
            sss::xml2::node * p_node = my_doc->locate(path);
            if (p_node) {
                return p_node->node_list.end();
            }
            else {
                return my_doc->node_list.end();
            }
        }
        void sss::xml2::xmlcfg_file::print(std::ostream& o)
        {
            this->my_doc->print(o);
        }

        void sss::xml2::xmlcfg_file::update()
        {
            if (this->is_modified) {
                this->my_doc->save(fname);
            }
        }

        sss::xml2::xml_doc& sss::xml2::xmlcfg_file::get_xmldoc() {
            return *this->my_doc.get();
        }
    }
}
