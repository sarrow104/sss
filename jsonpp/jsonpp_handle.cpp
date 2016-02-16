#include "jsonpp.hpp"

namespace sss {
namespace jsonpp
{
    JHandle::JHandle(const std::string& jsonstr)
        : data(0)
    {
        jsonpp::JParser jp;
        this->data = jp.falldown(jsonstr);
    }

    JHandle::JHandle(JValue * p_val)
        : data(p_val)
    {
    }

    JHandle::JHandle()
        : data(0)
    {
    }

    JHandle::~JHandle()
    {
        this->clear();
    }

    void JHandle::clear()
    {
        delete this->data;
        this->data = 0;
    }

    void JHandle::swap(JHandle& ref)
    {
        std::swap(this->data, ref.data);
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
        return prev_val;
    }
}
}
