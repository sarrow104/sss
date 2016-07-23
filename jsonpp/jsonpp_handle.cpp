#include "jsonpp.hpp"

#include <stdexcept>
#include <sss/utlstring.hpp>
#include <sstream>

namespace sss {
namespace jsonpp
{
    JHandle::JHandle(const std::string& jsonstr)
        : data(0), m_is_owner(false)
    {
        jsonpp::JHandle tmp_json;
        tmp_json.init(jsonstr);
        this->swap(tmp_json);
    }

    JHandle::JHandle(JValue * p_val)
        : data(p_val), m_is_owner(false)
    {
    }

    JHandle::JHandle()
        : data(0), m_is_owner(false)
    {
    }

    JHandle::JHandle(const JHandle& rhs)
        : data(rhs.data), m_is_owner(rhs.m_is_owner)
    {
        if (!data) {
            m_is_owner = false;
        }
        if (m_is_owner && data) {
            JValue * tmp = this->data->clone();
            std::swap(data, tmp);
        }
    }

    JHandle& JHandle::operator = (const JHandle& rhs)
    {
        if (this == &rhs) {
            return *this;
        }
        JHandle tmp_json(rhs);
        this->swap(tmp_json);
        return *this;
    }

    JHandle::~JHandle()
    {
        this->clear();
    }

    void JHandle::clear()
    {
        if (this->m_is_owner) {
            delete this->data;
        }
        this->data = 0;
        this->m_is_owner = false;
    }

    JHandle& JHandle::init(const std::string& jsonstr)
    {
        try {
            jsonpp::JParser jp;
            jsonpp::JValue * tmp_data = jp.falldown(jsonstr);

            JHandle tmp_json(tmp_data);
            tmp_json.m_is_owner = true;
            this->swap(tmp_json);
            return *this;
        }
        catch (const char * msg) {
            std::ostringstream oss;
            oss << "parsing `" << sss::utlstr::sample_string(jsonstr) << "`; error: "
                << msg;
            throw std::runtime_error(oss.str());
        }
        catch (...) {
            throw;
        }
    }

    void JHandle::swap(JHandle& ref)
    {
        std::swap(this->data, ref.data);
        std::swap(this->m_is_owner, ref.m_is_owner);
    }

    JHandle& JHandle::assign(const std::string& jsonstr)
    {
        jsonpp::JHandle tmp_json(jsonstr);
        this->swap(tmp_json);
        return *this;
    }

    JHandle& JHandle::assign(JValue* p_val)
    {
        jsonpp::JHandle tmp_json(p_val);
        this->swap(tmp_json);
        return *this;
    }

    JValue * JHandle::release()
    {
        JValue * prev_val = this->data;
        this->data = 0;
        this->m_is_owner = false;
        return prev_val;
    }

    JHandle JHandle::operator[] (int idx)
    {
        if (this->data && this->data->get_type() == JSON_ARRAY) {
            return JHandle(&this->data->operator[](idx));
        }
        return JHandle(0);
    }

    JHandle JHandle::operator[] (const std::string& key)
    {
        if (this->data && this->data->get_type() == JSON_OBJECT) {
            return JHandle(&this->data->operator[](key));
        }
        return JHandle(0);
    }

    void JHandle::print(std::ostream& o, bool is_pretty) const
    {
        if (data) {
            data->print(o, is_pretty);
        }
    }

    int JHandle::size() const
    {
        if (!data) {
            return 0;
        }
        switch (data->get_type()) {
        case JSON_ARRAY:
            return data->size();

        case JSON_OBJECT:
            return data->size();

        default:
            return 1;
        }
    }

    jtype_t JHandle::type() const
    {
        if (!data) {
            return JSON_VALUE;
        }
        return data->get_type();
    }

    std::string JHandle::to_string() const
    {
        return data ? data->get_string() : "";
    }

    int         JHandle::to_int()    const
    {
        return data ? data->get_int() : 0;
    }

    double      JHandle::to_double() const
    {
        return data ? data->get_double() : 0.0;
    }

    bool        JHandle::to_bool()   const
    {
        return data ? data->get_bool() : false;
    }

}
}
