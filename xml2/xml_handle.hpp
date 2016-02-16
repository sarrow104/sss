#ifndef  __XML_HANDLE_HPP_1438236519__
#define  __XML_HANDLE_HPP_1438236519__

#include "xml_node.hpp"
#include "xml_doc.hpp"

#include <iostream>
#include <string>

#include <assert.h>

namespace sss{
    namespace xml2 {
        class xml_doc;
        class node;
        class EleHandle {
        public:
            EleHandle()
                : _data(0)
            {
            }
            explicit EleHandle(node * p)
                : _data(p)
            {
            }
            EleHandle(const EleHandle & ref)
                : _data(ref._data)
            {
            }

        public:
            EleHandle& operator=(const EleHandle & ref)
            {
                if (this != &ref)
                {
                    this->_data = ref._data;
                }
                return *this;
            }

            EleHandle prevSibling()
            {
                return EleHandle(this->_data ? this->_data->prev_sibling() : 0);
            }

            EleHandle nextSibling()
            {
                return EleHandle(this->_data ? this->_data->next_sibling() : 0);
            }
            EleHandle firstChild()
            {
                return EleHandle(this->_data ? this->_data->first_child() : 0);
            }
            EleHandle lastChild()
            {
                return EleHandle(this->_data ? this->_data->last_child() : 0);
            }

            EleHandle findElementByID(const std::string & id)
            {
                return EleHandle(this->_data && this->_data->get_doc() ? this->_data->get_doc()->id_get(id) : 0);
            }

            EleHandle findElementByTag(const std::string & tag)
            {
                return EleHandle(this->_data ? this->_data->find_child(tag, 0) : 0);
            }

            node * operator->()
            {
                return this->_data;
            }

            node * get()
            {
                return this->_data;
            }

        private:
            node * _data;
        };

        class DocHandle {
        public:
            DocHandle();
            DocHandle(const std::string& fname);

            ~DocHandle();

        public:
            DocHandle& parse(const std::string& xml_str);

            xml2::node * parseSubtree(const std::string& xml_str);

        public:
            DocHandle& load(const std::string& fname);

            DocHandle& save(const std::string& fname);

            const DocHandle& write(const std::string& fname, const char * sep = "\t") const;

            const DocHandle& print(std::ostream& o) const;

            sss::xml2::node * root();

            xml_doc * operator->() const
            {
                assert(this->_pdoc);
                return this->_pdoc;
            }

            xml_doc * get() const
            {
                return this->_pdoc;
            }

        public:
            DocHandle(const DocHandle& ref)
                : _pdoc(dynamic_cast<sss::xml2::xml_doc*>(ref->clone(true))),
                  _fname(ref._fname)
            {
                if (!_pdoc) {
                    throw std::runtime_error("NULL");
                }
            }

        private:
            sss::xml2::xml_doc * _pdoc;
            std::string         _fname;
        };

        inline std::ostream& operator << (std::ostream& o, const DocHandle& doc)
        {
            doc.print(o);
            return o;
        }
    }
}


#endif  /* __XML_HANDLE_HPP_1438236519__ */
