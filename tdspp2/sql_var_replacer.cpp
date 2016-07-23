#include "sql_var_replacer.hpp"

#include <sstream>
#include <iostream>
#include <cctype>

#include <sss/log.hpp>

namespace sss {
namespace tdspp2 {

sql_var_replacer::sql_var_replacer(const std::string& sql)
    : feeder(&sql_var_replacer::do_other)
{
    std::istringstream iss(sql);
    char ch;

    while(ch = iss.get(), !iss.eof()) {
        (this->*feeder)(ch);
    }

    if (!stem.empty()) {
        add_stem(stem);
    }
}

sql_var_replacer::~sql_var_replacer()
{
    for (colon_vars_t::iterator it = colon_vars.begin();
         it != colon_vars.end(); ++it) {
        delete it->second;
    }
}

void sql_var_replacer::add_stem(const std::string& stem) {
    SSS_LOG_DEBUG("%s\n", stem.c_str());
    stems.push_back(stem);
}

void sql_var_replacer::do_s_colon(char ch)
{
    if (ch == ':') {
        stem += ch;
        feeder = &sql_var_replacer::do_d_colon;
    }
    else if (is_id_char(ch)) {
        stem += ch;
    }
    else {
        add_stem(stem);
        stem.resize(1);
        stem[0] = ch;
        feeder = &sql_var_replacer::do_other;
    }
}

void sql_var_replacer::do_d_colon(char ch)
{
    if (is_id_char(ch)) {
        stem += ch;
    }
    else {
        add_stem(stem);
        stem.resize(1);
        stem[0] = ch;
        feeder = &sql_var_replacer::do_other;
    }
}

void sql_var_replacer::do_other(char ch)
{
    if (ch == ':') {
        add_stem(stem);
        stem = ":";
        feeder = &sql_var_replacer::do_s_colon;
    }
    else {
        stem += ch;
    }
}

bool sql_var_replacer::is_id_char(char ch) const {
    return std::isalnum(ch) || ch == '_' || ((unsigned char)ch & 0x80) > 0;
}

void sql_var_replacer::set_var(const std::string& name,
                               const std::string& value)
{
    colon_vars_t::iterator it = colon_vars.find(name);
    if (it != colon_vars.end()) {
        SSS_LOG_DEBUG("chage [%s], from \"%s\" to \"%s\"\n",
                      name.c_str(),
                      it->second->to_string().c_str(),
                      value.c_str());
        // FIXME 应该采用 临时对象，加swap的方式；这样不容易内存泄漏
        delete it->second;
        it->second = new ValueVar(value);
    }
    else {
        SSS_LOG_DEBUG("set [%s] map \"%s\"\n", name.c_str(), value.c_str());
        colon_vars.insert(it, std::make_pair(name, new ValueVar(value)));
    }
}

std::string sql_var_replacer::generate() const
{
    std::ostringstream oss;
    for (stems_t::const_iterator it = stems.begin(); it != stems.end(); ++it) {
        const std::string& s = *it;
        if (!s.empty() && s[0] == ':') {
            colon_vars_t::const_iterator var_it = colon_vars.find(s);
            if (var_it != colon_vars.end()) {
                oss << var_it->second->to_string();
            }
            else {
                // FIXME 分号变量必须替换完毕！
                // 这里应该抛出异常！
                oss << s;
            }
        }
        else {
            oss << s;
        }
    }
    return oss.str();
}

}
}
