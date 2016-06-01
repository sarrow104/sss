#include "dosini.hpp"

#include <fstream>
#include <iostream>

#include <sss/log.hpp>

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

    void trimRange(const std::string& line, StrRange& r) {
        while (r.left < r.right && std::isspace(line[r.left])) {
            r.left++;
        }
        while (r.left < r.right && std::isspace(line[r.right - 1])) {
            r.right--;
        }
    }

    bool parseBlock(const std::string& line, StrRange& r)
    {
        // sss::regex::simpleregex reg_ini_block("^\\s*\\[\\([^\\]]+\\)\\]\\s*$");
        int  last = -1;
        char ch = '\0';
        int cnt = -1;
        if (0 == sscanf(line.c_str(), " [%n%*[^]]%n]%n", &r.left, &r.right, &last) &&
            last > 0 &&
            (cnt = sscanf(line.c_str() + last, " %c", &ch), (cnt == 0 || cnt == EOF)))
        {
            return true;
        }
        else {
            return false;
        }
    }

    bool parseComment(const std::string& line)
    {
        // sss::regex::simpleregex reg_ini_linecomment("^;\\(.+\\)$");
        if (sss::is_begin_with(line, ";")) {
            return true;
        }
        else {
            return false;
        }
    }

    bool parseKeyValuePair(const std::string& line, StrRange& r1, StrRange& r2)
    {
        // sss::regex::simpleregex reg_ini_keyvalue("^\\s*\\(\\<[^=]+\\)=\\(.*\\)$");
        int last = -1;
        if (0 == sscanf(line.c_str(), " %n%*[^=]%n=%n", &r1.left, &r1.right, &last) && last > 0) {
            trimRange(line, r1);
            r2.left = last;
            r2.right = line.size();
            trimRange(line, r2);
            return true;
        }
        else {
            return false;
        }
    }
}

namespace sss {
    dosini::dosini(const std::string& cfg_fname)
        : fname(cfg_fname)
    {
        if (!fname.empty()) {
            load();
        }
    }

    dosini::dosini()
    {
    }

    dosini::~dosini()
    {
    }

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
            if (parseBlock(line, r1)) {
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
            else if (parseComment(line)) {
                // std::cout << "linecomment = \"" << line << "\"" << std::endl;
                this->append_line(line);
            }
            else if (parseKeyValuePair(line, r1, r2)) {
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

    bool dosini::write(const std::string& fname) const
    {
        std::ofstream ofs(fname.c_str(), std::ios::binary);
        if (ofs) {
            this->print(ofs);
            return true;
        }
        return false;
    }

    void dosini::print(std::ostream& out) const
    {
        for (linenos_t::const_iterator it = this->linenos.begin();
             it != this->linenos.end();
             ++it)
        {
            out << this->data[*it] << std::endl;
        }
    }

    void dosini::print_clean(std::ostream& out) const
    {
        for (blocks_t::const_iterator it = this->blocks.begin();
             it != this->blocks.end();
             ++it)
        {
            if (!it->first.empty()) {
                out << "[" << it->first << "]" << std::endl;
            }
            for (block_t::const_iterator ki = it->second.begin();
                 ki != it->second.end();
                 ++ki)
            {
                out << "\t{"
                    << ki->first << "} = {"
                    << this->get_data_slice(ki->second)
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
        bool ret = std::getline(ifs, line);
        if (ret) {
            sss::trim(line, "\r");
        }
        return ret;
    }

    dosini::value_t dosini::append_line(const std::string& line)
    {
        // SSS_LOG_DEBUG("%s\n", line.c_str());
        this->data.push_back(line);
        int lineno = data.size() - 1;
        this->linenos.push_back(lineno);
        return dosini::value_t(lineno, 0, line.length());
    }

    // 添加一个block；并往data_t 里面，添加一行；
    dosini::value_t dosini::append_block(const std::string& block)
    {
        if (!block.empty()) {
            return this->append_block("[" + block + "]", 1, block.length() + 1);
        }
        return dosini::value_t();
    }

    dosini::value_t dosini::append_block(const std::string& line, int bk_start, int bk_end)
    {
        std::string block = line.substr(bk_start, bk_end - bk_start);
        SSS_LOG_DEBUG("\"%s\"\n", block.c_str());
        this->blocks[block] = block_t();
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
        block_t & B(this->blocks[block]);
        std::string key = line.substr(key_start, key_end - key_start);
        value_t index;
        if (B.find(key) == B.end()) {
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
        block_t & B(this->blocks[block]);
        value_t index;

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
