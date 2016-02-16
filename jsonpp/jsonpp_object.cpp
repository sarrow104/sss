#include "jsonpp.hpp"

#include <stdexcept>
#include <sstream>
#include <iostream>

namespace sss
{
namespace jsonpp
{
    JObject::JObject()
        : JValue(JSON_OBJECT)
    {
    }

    JObject::~JObject()
    {
        for (data_t::iterator it = this->data.begin();
             it != this->data.end();
             ++it)
        {
            delete it->second;
        }
    }

    JValue& JObject::operator[](const std::string& name) const
    {
        return this->key(name);
    }

    JValue& JObject::key(const std::string& name) const
    {
        data_t::const_iterator it = this->data.find(name);
        if (it == this->data.end()) {
            for (data_t::const_iterator it = this->data.begin();
                 it != this->data.end();
                 ++it)
            {
                std::cout << it->first << std::endl;
            }
            throw std::runtime_error("query object key name \"" + name + "\" not exists.");
        }
        return *(it->second);
    }

    bool    JObject::has_key(const std::string& name)     const
    {
        return this->data.end() != this->data.find(name);
    }

    int     JObject::size()                          const
    {
        return this->data.size();
    }

    bool    JObject::add(const std::string& name, JValue * val)
    {
        data_t::iterator it = this->data.find(name);
        if (it == this->data.end()) {
            this->data.insert(std::make_pair(name, val));
        }
        else {
            JHandle jh(it->second);
            it->second = val;
        }
        return true;
    }

    bool   JObject::swap(JValue& val)
    {
        JObject * ref = dynamic_cast<JObject*>(&val);
        if (ref) {
            this->data.swap(ref->data);
            return true;
        }
        return false;
    }

    JValue * JObject::clone()                        const
    {
        JHandle jh(new jsonpp::JObject);
        for (data_t::const_iterator it = this->data.begin();
             it != this->data.end();
             ++it)
        {
            jh->add(it->first, it->second->clone());
        }
        return jh.release();
    }

    std::string     JObject::to_str()                const
    {
        std::ostringstream oss;
        this->print(oss);
        return oss.str();
    }

    const JObject::data_t& JObject::get_data() const
    {
        return this->data;
    }
}
}
