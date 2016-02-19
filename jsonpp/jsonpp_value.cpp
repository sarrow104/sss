#include "jsonpp.hpp"

#include <stdexcept>
#include <iostream>

#include "jsonpp_vprinter.hpp"
#include "jsonpp_vcounter.hpp"

namespace sss
{
namespace jsonpp
{
    JValue::JValue(jtype_t t)
        : jtype(t)
    {
        switch(jtype) {
        case JSON_VALUE:
        case JSON_STRING:
        case JSON_NUMBER:
        case JSON_OBJECT:
        case JSON_ARRAY:
        case JSON_TRUE:
        case JSON_FALSE:
        case JSON_NULL:

        case JSON_BOOLEAN:
        case JSON_INT:
        case JSON_DOUBLE:
            break;

        default:
            throw std::runtime_error("unknown json type.");
        }
    }

    JValue::~JValue()
    {
    }

    JValue& JValue::operator[](int ) const
    {
        std::string msg(this->get_type_str());
        throw std::runtime_error( msg + " array[] not implement.");
    }

    JValue& JValue::operator[](const std::string&) const
    {
        std::string msg(this->get_type_str());
        throw std::runtime_error(msg + " object.key not implement.");
    }

    JValue& JValue::at(int) const
    {
        std::string msg(this->get_type_str());
        throw std::runtime_error(msg + " at() not implement.");
    }

    JValue& JValue::key(const std::string&) const
    {
        std::string msg(this->get_type_str());
        throw std::runtime_error(msg + " key() not implement.");
    }

    bool    JValue::has_key(const std::string&)     const
    {
        std::string msg(this->get_type_str());
        throw std::runtime_error(msg + __func__ + "() not implement.");
    }

    int     JValue::size()                          const
    {
        std::string msg(this->get_type_str());
        throw std::runtime_error(msg + __func__ + "() not implement.");
    }

    bool    JValue::add(JValue * )                      // JSON_ARRAY
    {
        std::string msg(this->get_type_str());
        throw std::runtime_error(msg + __func__ + "() not implement.");
    }

    bool    JValue::add(const std::string& , JValue * ) // JSON_OBJECT
    {
        std::string msg(this->get_type_str());
        throw std::runtime_error(msg + __func__ + "(,) not implement.");
    }

    bool   JValue::swap(JValue& )
    {
        std::string msg(this->get_type_str());
        throw std::runtime_error(msg + __func__ + "() not implement.");
        return false;
    }

    JValue * JValue::clone()                        const
    {
        std::string msg(this->get_type_str());
        throw std::runtime_error(msg + __func__ + "() not implement.");
        return 0;
    }

    // ----------------------------------------------------------------------
    bool    JValue::is_string() const
    {
        return this->get_type() == JSON_STRING;
    }

    bool    JValue::is_number() const
    {
        switch (this->get_type()) {
        case JSON_INT:
        case JSON_DOUBLE:
        case JSON_NUMBER:
            return true;

        default:
            return false;
        }
    }

    bool    JValue::is_object() const
    {
        return this->get_type() == JSON_OBJECT;
    }

    bool    JValue::is_array() const
    {
        return this->get_type() == JSON_ARRAY;
    }

    //----------------------------------------------------------------------
    bool    JValue::is_true() const
    {
        return this->get_type() == JSON_TRUE;
    }

    bool    JValue::is_false() const
    {
        return this->get_type() == JSON_FALSE;
    }

    bool    JValue::is_null() const
    {
        return this->get_type() == JSON_NULL;
    }

    // ----------------------------------
    bool    JValue::is_boolean() const
    {
        switch (this->get_type()) {
        case JSON_TRUE:
        case JSON_FALSE:
        case JSON_BOOLEAN:
            return true;
        default:
            return false;
        }
    }

    bool    JValue::is_int()        const
    {
        return this->get_type() == JSON_INT;
    }

    bool    JValue::is_double()     const
    {
        return this->get_type() == JSON_DOUBLE;
    }

    //----------------------------------------------------------------------
    std::string     JValue::get_string()            const
    {
        std::string msg(this->get_type_str());
        throw std::runtime_error(msg + "." + __func__ + "() not implement.");
    }

    int             JValue::get_int()               const
    {
        std::string msg(this->get_type_str());
        throw std::runtime_error(msg + "." + __func__ + "() not implement.");
    }
    double          JValue::get_double()            const
    {
        std::string msg(this->get_type_str());
        throw std::runtime_error(msg + "." + __func__ + "() not implement.");
    }

    bool            JValue::get_bool()              const
    {
        std::string msg(this->get_type_str());
        throw std::runtime_error(msg + "." + __func__ + "() not implement.");
    }

    void *          JValue::get_null()              const
    {
        std::string msg(this->get_type_str());
        throw std::runtime_error(msg + "." + __func__ + "() not implement.");
    }

    //----------------------------------------------------------------------

    jtype_t  JValue::get_type() const
    {
        return jtype;
    }

    const char * JValue::get_type_str() const
    {
        return JValue::type_id2name(this->get_type());
    }

    int             JValue::node_count()    const
    {
        sss::jsonpp::JVCounter cnt;
#if 0
        const_cast<JValue*>(this)->accept(cnt);
#else
        this->accept(cnt);
#endif
        return cnt.size();
    }

    //----------------------------------------------------------------------
    const char * JValue::type_id2name(int id)
    {
        // 问题来了，既然我尽量分来了类型，那么，我在返回类型的名称的时候，是否照顾到扩展部分？
        static const char * name_table[] = {
            "JSON_VALUE",
            "JSON_STRING",
            "JSON_NUMBER",
            "JSON_OBJECT",
            "JSON_ARRAY",
            "JSON_TRUE",
            "JSON_FALSE",
            "JSON_NULL",

            "JSON_BOOLEAN",   // 扩展
            "JSON_INT",
            "JSON_DOUBLE"
        };
        if ( id < 0 || id >= int(sizeof(name_table)/sizeof(name_table[0]))) {
            throw std::runtime_error("index bound exceed.");
        }
        return name_table[id];
    }

    std::string     JValue::to_str()                const
    {
        std::string msg(this->get_type_str());
        throw std::runtime_error(msg + "." + __func__ + "() not implement.");
        return "";
    }

    void JValue::accept(JVisitor& visitor) const
    {
        visitor.visit(this);
    }

    void JValue::print(std::ostream& os, bool is_pretty) const
    {
        jsonpp::JVPrinter jpos(os, is_pretty);
        this->accept(jpos);
    }
}
}
