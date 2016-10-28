#ifndef  __XHTML_DOC_HPP_1405087188__
#define  __XHTML_DOC_HPP_1405087188__

#include "xml_doc.hpp"

#include <string>

#include <sss/dom/html_util.hpp>
#include <sss/iConvpp.hpp>

#include <iostream>
#include <map>

namespace sss{
    namespace xml{

        class xhtml_doc : public xml_doc {

        public:
            // 创建空的xml文档对象
            xhtml_doc();

            // 创建字符集为charset的xml文档对象
            // 同时设置是否打印bom――NOTE bom只在打印的时候有效。
            xhtml_doc(const std::string& charset,
                      bool has_bom = false);

            // 创建根节点为root_name的xml文档对象
            // 其余同上；
            xhtml_doc(const std::string& root_name,
                      const std::string& charset,
                      bool has_bom = false);

            // ……
            virtual ~xhtml_doc();

        public:
            virtual node * root() const;
            virtual node * root(const std::string&);

            // 获取根节点名字
            virtual std::string root_name() const;

            virtual std::string set_charset(const std::string& );

            virtual std::string get_charset();

            virtual bool        validate() const;

        protected:

            // 确保描述charset的节点存在于根节点之前！
            node      * verify_charset_node();
        };
    }
}


#endif  /* __XHTML_DOC_HPP_1405087188__ */
