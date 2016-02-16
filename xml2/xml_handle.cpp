#include "xml_handle.hpp"
#include "xml_parser.hpp"

#include <sss/path.hpp>
#include <iostream>

namespace sss{
    namespace xml2 {

DocHandle::DocHandle()
    : _pdoc(0)
{
}

DocHandle::DocHandle(const std::string& fname)
    : _pdoc(0)
{
    this->load(fname);
}

DocHandle::~DocHandle()
{
    delete _pdoc;
    this->_pdoc = 0;
}

DocHandle& DocHandle::parse(const std::string& xml_str)
{
    delete _pdoc;
    sss::xml2::xml_parser xp;
    this->_pdoc = dynamic_cast<sss::xml2::xml_doc*>(xp.parse(xml_str));
    return *this;
}

node * DocHandle::parseSubtree(const std::string& xml_str)
{
    if (!this->_pdoc) {
        this->_pdoc = new sss::xml2::xml_doc;
    }
    return this->_pdoc->create_subtree(xml_str);
}

DocHandle& DocHandle::load(const std::string& fname)
{
    if (sss::path::file_exists(fname)) {
        std::string xml_str;
        sss::path::file2string(fname, xml_str);
        this->parse(xml_str);
    }
    else {
        this->_pdoc = new xml2::xml_doc;
    }
    return *this;
}

DocHandle& DocHandle::save(const std::string& fname)
{
    this->_pdoc->save(fname);
    return *this;
}

const DocHandle& DocHandle::write(const std::string& fname, const char * sep) const
{
    this->_pdoc->write(fname, sep);
    return *this;
}

const DocHandle& DocHandle::print(std::ostream& o) const
{
    this->_pdoc->print(o);
    return *this;
}

sss::xml2::node * DocHandle::root()
{
    return this->_pdoc->root();
}

    }
}
