#include "xmlcfg_file.hpp"

#include <sss/log.hpp>
#include <sss/path.hpp>

namespace sss {
    namespace xml3 {
        xmlcfg_file::xmlcfg_file()
            : is_modified(false)
        {
            // this->my_doc();
        }

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

        // 因为可以通过 sss::xml3::node * 节点指针来操作、修改xml结构，这就导致
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
                sss::xml3::node * p_node = my_doc->locate(path);
                if (p_node) {
                    return p_node->size();
                }
                return 0;
            }
            catch (...) {
                return 0;
            }
        }

        sss::xml3::xmlcfg_file::iterator xmlcfg_file::sub_begin(const std::string& path)
        {
            sss::xml3::node * p_node = my_doc->locate(path);
            if (p_node) {
                return sss::xml3::xmlcfg_file::iterator(p_node, true);
            }
            else {
                throw std::runtime_error(std::string(__func__) + " `" + path + "` not exists.");
            }
        }

        sss::xml3::xmlcfg_file::iterator xmlcfg_file::sub_end(const std::string& path)
        {
            sss::xml3::node * p_node = my_doc->locate(path);
            if (p_node) {
                return sss::xml3::xmlcfg_file::iterator(p_node, false);
            }
            else {
                throw std::runtime_error(std::string(__func__) + " `" + path + "` not exists.");
            }
        }
        void sss::xml3::xmlcfg_file::print(std::ostream& o)
        {
            this->my_doc->print(o);
        }

        void sss::xml3::xmlcfg_file::update()
        {
            if (this->is_modified) {
                this->my_doc->save(fname);
            }
        }

        void sss::xml3::xmlcfg_file::write(const std::string& name)
        {
            this->my_doc->save(name);
            this->fname = name;
        }

        sss::xml3::xml_doc& sss::xml3::xmlcfg_file::get_xmldoc() {
            return *this->my_doc.get();
        }
    }
}
