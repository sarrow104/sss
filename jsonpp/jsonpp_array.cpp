#include "jsonpp.hpp"

#include <sstream>

namespace sss{
namespace jsonpp {

    JArray::JArray()
        : JValue(JSON_ARRAY)
    {
    }

    JArray::~JArray()
    {
        while (!this->data.empty()) {
            delete this->data.back();
            this->data.pop_back();
        }
    }

    JValue& JArray::operator[](int idx)             const
    {
        return this->at(idx);
    }

    JValue& JArray::at(int idx)                     const
    {
        return *this->data.at(idx);
    }

    int     JArray::size()                          const
    {
        return this->data.size();
    }

    std::string     JArray::to_str()                const
    {
        std::ostringstream oss;
        this->print(oss);
        return oss.str();
    }

    bool   JArray::add(JValue * val)
    {
        this->data.push_back(val);
        return true;
    }

    bool   JArray::swap(JValue& val)
    {
        JArray * ref = dynamic_cast<JArray*>(&val);
        if (ref) {
            this->data.swap(ref->data);
            return true;
        }
        return false;
    }

    JValue * JArray::clone()                        const
    {
        JHandle jh(new jsonpp::JArray);
        for (data_t::const_iterator it = this->data.begin();
             it != this->data.end();
             ++it)
        {
            jh->add((*it)->clone());
        }
        return jh.release();
    }

    const JArray::data_t& JArray::get_data() const
    {
        return this->data;
    }
}
}
