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
            // �����յ�xml�ĵ�����
            xhtml_doc();

            // �����ַ���Ϊcharset��xml�ĵ�����
            // ͬʱ�����Ƿ��ӡbom����NOTE bomֻ�ڴ�ӡ��ʱ����Ч��
            xhtml_doc(const std::string& charset,
                      bool has_bom = false);

            // �������ڵ�Ϊroot_name��xml�ĵ�����
            // ����ͬ�ϣ�
            xhtml_doc(const std::string& root_name,
                      const std::string& charset,
                      bool has_bom = false);

            // ����
            virtual ~xhtml_doc();

        public:
            virtual node * root() const;
            virtual node * root(const std::string&);

            // ��ȡ���ڵ�����
            virtual std::string root_name() const;

            virtual std::string set_charset(const std::string& );

            virtual std::string get_charset();

            virtual bool        validate() const;

        protected:

            // ȷ������charset�Ľڵ�����ڸ��ڵ�֮ǰ��
            node      * verify_charset_node();
        };
    }
}


#endif  /* __XHTML_DOC_HPP_1405087188__ */
