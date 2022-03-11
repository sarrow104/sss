#include "dosini.hpp"

#include <fstream>
#include <iostream>

#include <sss/log.hpp>
#include <sss/dosini/IniParser.hpp>

namespace {
struct StrRange
{
    int left;
    int right;

    StrRange()
        : left(-1), right(-1)
    {
    }
};
} // namespace

namespace sss {

dosini::dosini(const std::string& cfg_fname)
    : fname(cfg_fname)
{
    if (!fname.empty()) {
        load();
    }
}

dosini::dosini() = default;

dosini::~dosini() = default;

bool dosini::load()
{
    // FIXME 貌似我的 simpleregex 不支持中文！
    // 所以，最好还是用skip来做？
    std::string   line;
    std::ifstream ifs(fname.c_str(), std::ios_base::binary);

    // ^\s*\[[^\]]+\]\s*$

    std::string block;
    while (dosini::getline(ifs, line)) {
        std::string matched;
        StrRange r1;
        StrRange r2;
        int pos = 0;

        char none_space = sss::firstNoneSpace(line, pos);
        if (none_space == 0) {
            // 空行，仍然保留
            this->append_line(line);
            continue;
        }

        switch (none_space) {
        case '[':
            if (sss::iniParseSection(line, r1.left, r1.right, pos) != 0)
            {
                block.assign(line, r1.left, r1.right - r1.left);
                this->append_block(line,
                                   r1.left,
                                   r1.right);
                // std::cout << "block = \""
                //     << line.substr(r1.left,
                //                    r1.right - r1.left)
                //     << "\""
                //     << std::endl;
            }
            else {
                this->append_line(line);
            }
            break;

        case ';':
            // NOTE 如果以';'开始则是注释；操作是append_line(line);
            // 如果pos != 0；则无法识别；操作还是append_line(line);
            this->append_line(line);
            break;

        default:
            if (sss::iniParseKeyValue(line,
                                      r1.left, r1.right,
                                      r2.left, r2.right,
                                      pos) != 0)
            {
                // std::cout << "key = \""
                //     << line.substr(r1.left,
                //                    r1.right - r1.left)
                //     << "\""
                //     << std::endl;
                // std::cout << "value = \""
                //     << line.substr(r2.left,
                //                    r2.right - r2.left)
                //     << "\""
                //     << std::endl;
                this->append_key(block,
                                 line,
                                 r1.left,
                                 r1.right,
                                 r2.left,
                                 r2.right);
            }
            else {
                this->append_line(line);
            }
            break;
        }
    }
    return true;
}

bool dosini::save() const
{
    if (this->fname.empty()) {
        return false;
    }
    std::ofstream ofs(this->fname.c_str(), std::ios::binary);
    if (ofs) {
        this->print(ofs);
        return true;
    }
    return false;
}

bool dosini::write(const std::string& fpath) const
{
    std::ofstream ofs(fpath.c_str(), std::ios::binary);
    if (ofs) {
        this->print(ofs);
        return true;
    }
    return false;
}

void dosini::print(std::ostream& out) const
{
    for (int lineno : this->linenos)
    {
        out << this->data[lineno] << std::endl;
    }
}

void dosini::print_clean(std::ostream& out) const
{
    for (const auto & block : this->blocks)
    {
        if (!block.first.empty()) {
            out << "[" << block.first << "]" << std::endl;
        }
        for (const auto & ki : block.second)
        {
            out << "\t{"
                << ki.first << "} = {"
                << this->get_data_slice(ki.second)
                << "}"
                << std::endl;
        }
        out << std::endl;
    }
}

std::string dosini::get_data_slice(const value_t& index) const
{
    std::string ret;
    if (index.lineno >= 0 and index.lineno < int(data.size())) {
        ret = data[index.lineno].substr(index.pos, index.len);
    }
    return ret;
}

bool dosini::getline(std::istream& ifs, std::string& line)
{
    // NOTE C++14 中，std::getline() 返回 std::istream& ；并不能直接转换为bool。
    if (std::getline(ifs, line)) {
        sss::trim(line, "\r");
        return true;
    }
    return false;
}

dosini::value_t dosini::append_line(const std::string& line)
{
    // SSS_LOG_DEBUG("%s\n", line.c_str());
    this->data.push_back(line);
    int lineno = data.size() - 1;
    this->linenos.push_back(lineno);
    return dosini::value_t(lineno, 0, line.length(), this);
}

// 添加一个block；并往data_t 里面，添加一行；
dosini::value_t dosini::append_block(const std::string& block)
{
    if (!block.empty()) {
        return this->append_block("[" + block + "]", 1, block.length() + 1);
    }
    return {};
}

dosini::value_t dosini::append_block(const std::string& line, int bk_start, int bk_end)
{
    std::string block = line.substr(bk_start, bk_end - bk_start);
    SSS_LOG_DEBUG("\"%s\"\n", block.c_str());
    this->blocks[block] = section_t();
    return this->append_line(line);
}

// 按行添加（已经定位key,val起止）
dosini::value_t dosini::append_key(const std::string& block, const std::string& line,
                                   int key_start, int key_end,
                                   int val_start, int val_end)
{
    if (this->blocks.find(block) == this->blocks.end()) {
        this->append_block(block);
    }
    section_t & B(this->blocks[block]);
    std::string key = line.substr(key_start, key_end - key_start);
    value_t index;
    if (B.find(key) == B.end()) {
        index.p_dosini = this;
        index.pos = val_start;
        index.len = val_end - val_start;

        data.push_back(line);
        index.lineno = data.size() - 1;

        B[key] = index;
        this->linenos.push_back(this->data.size() - 1);
    }
    return index;
}

// 用户自己添加
dosini::value_t dosini::append_key(const std::string& block, const std::string& key, const std::string& val)
{
    SSS_LOG_DEBUG("[%s/%s]={%s}\n",
                  block.c_str(),
                  key.c_str(),
                  val.c_str());
    if (this->blocks.find(block) == this->blocks.end()) {
        this->append_block(block);
    }
    section_t & B(this->blocks[block]);
    value_t index;
    index.p_dosini = this;

    if (B.find(key) == B.end()) {
        // 不管有没有值，直接附加……
    }

    std::string line = key + "=";
    index.pos = line.length();
    index.len = val.length();
    line += val;
    data.push_back(line);
    index.lineno = data.size() - 1;
    B[key] = index;
    this->linenos.push_back(this->data.size() - 1);
    SSS_LOG_DEBUG("%s\n", this->get_data_slice(index).c_str());
    return index;
}

// NOTE 如果是覆盖的话，

} // end of namespace sss
