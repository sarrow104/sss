#include "jsonpp.hpp"

namespace sss {
// 访问者模式
namespace jsonpp
{
    // visitor dispatch according to node token type
    void JVisitor::visit(const JValue * v) {
        switch ( v->get_type() ) {
        case JSON_ARRAY:    this->visit(dynamic_cast<const JArray*>(v));        break;
        case JSON_OBJECT:   this->visit(dynamic_cast<const JObject*>(v));       break;

        case JSON_STRING:   this->visit(dynamic_cast<const JString*>(v));       break;
        case JSON_DOUBLE:   this->visit(dynamic_cast<const JDouble*>(v));       break;
        case JSON_INT:      this->visit(dynamic_cast<const JInt*>(v));          break;
        case JSON_BOOLEAN:  this->visit(dynamic_cast<const JBool*>(v));         break;
        case JSON_NULL:     this->visit(dynamic_cast<const JNull*>(v));         break;
        default : throw std::logic_error("UnsupportedOperationException"); break;
        }
    }
}
}
