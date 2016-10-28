#include <fstream>
#include <sstream>
#include <cctype>

#include <sss/path.hpp>
#include <sss/utlstring.hpp>

#include "cygpath.hpp"

#ifndef VALUE_MSG
#define VALUE_MSG(a) (#a) << " = " << (a)
#endif

namespace  {
    inline bool isoctdigit(char ch) {
        return '0' <= ch && ch <= '7';
    }
    void unescape_str(std::string& s) {
        size_t i = 0, j = 0;
        // FIXME 是否转义其他序列？
        while (i < s.length()) {
            if (s[i] == '\\') {
                if (i + 3 < s.length() && s[i + 1] == '0' && ::isoctdigit(s[i + 2]) && ::isoctdigit(s[i + 3])) {
                    char ch = ((s[i + 2] - '0') << 3) + (s[i + 3] - '0');
                    i += 4;
                    s[j++] = ch;
                }
                else {
                    ++i;
                }
            }
            else {
                s[j++] = s[i++];
            }
        }
        s.resize(j);
    }
} // namespace

namespace sss {
    namespace cygpath {
        /**
         * @brief 返回 url 串类型
         *
         * @param url
         *
         * @return 'u' unix     /foo/bar
         *         'U' unc      //machine/path
         *         'w' windows  c:\windows\system32
         *         'm' mixed    d:/Program\040Files/
         *
         *         NOTE c: 是 'w' 还是 'm' ？ 
         *              鉴于fstab设定是要求'm'风格的windows路径，所以……
         */
        char url_type(const std::string& url)
        {
            if (url.empty()) {
                return '\0';
            }
            switch (url[0]) {
            case '/':
                return url.length() > 1 && url[1] == '/' ? 'n' : 'u';

            default:
                if (!std::isalnum(url[0]) || url.length() < 2 || url[1] != ':') {
                    return '\0';
                }
                return url.length() > 2 && url[2] == '\\' ? 'w' : 'm';
            }
        }
        fstab::fstab()
        {
        }
        fstab::fstab(const std::string& fstab_path)
        {
            load(fstab_path);
        }
        fstab::~fstab()
        {
        }

        /**
         * @brief 载入 fstab 配置文件
         *
         * @param fstab_path fstab文件路径.
         *        NOTE 路径是 native 路径风格；
         */
        void fstab::load(const std::string& fstab_path)
        {
            std::string line;
            std::ifstream ifs(fstab_path.c_str());
            if (!ifs.good()) {
                // TODO throw
                return;
            }
            this->Base_t::clear();
            if (sss::is_end_with(fstab_path, "\\etc\\fstab")) {
                this->msys_path = sss::path::dirname(fstab_path);
                this->msys_path = sss::path::dirname(this->msys_path);
                sss::replace_all(this->msys_path, "\\", "/");
                if (!sss::is_end_with(this->msys_path, "/")) {
                    this->msys_path.append("/");
                }
                std::cout << __func__ << ":" << VALUE_MSG(this->msys_path) << std::endl;
            }
            while (std::getline(ifs, line)) {
                if (sss::is_begin_with(line, "#")) {
                    continue;
                }
                sss::trim(line);
                if (line.empty()) {
                    continue;
                }
                this->read_command(line);
            }
        }

        void fstab::load_from_msys(const std::string& msys_base)
        {
            std::string fstab_full_path = msys_base;
            sss::path::append(fstab_full_path, "etc");
            sss::path::append(fstab_full_path, "fstab");
            this->load(fstab_full_path);
        }

        std::string fstab::match_mount_device(const std::string& mixed) const
        {
            std::string path_unix;
            for (Base_t::const_iterator it = this->begin(); it != this->end(); ++it) {
                if (this->convert_mixed_by_rule(mixed, path_unix, it->first, it->second)) {
                    return path_unix;
                }
            }
            if (!this->msys_path.empty()) {
                if (this->convert_mixed_by_rule(mixed, path_unix, this->msys_path, "/")) {
                    return path_unix;
                }
            }
            path_unix = mixed;
            path_unix[1] = path_unix[0];
            path_unix[0] = '/';
            return path_unix;
        }

        std::string fstab::match_mount_point(const std::string& unix) const
        {
            for (Base_t::const_iterator it = this->begin(); it != this->end(); ++it) {
                if (sss::is_begin_with(unix, it->second) &&
                    (sss::is_end_with(unix, "/") ||
                     it->second.length() == unix.length() ||
                     (it->second.length() > unix.length() && it->second[unix.length()] == '/')))
                {
                    size_t tail_pos = unix.length();
                    if (it->second.length() > tail_pos && it->second[tail_pos] == '/') {
                        ++tail_pos;
                    }
                    std::string tail = it->second.substr(tail_pos);
                    return it->first + tail;
                }
            }
            // 从 mount_point -> device 的反推，如果没有找到匹配的规则，那么有两种情况
            // 1. 满足cygpath 的基本规则 /e/foo/bar
            // 2. 是msys系统路径，如 /usr /bin 等等
            // 此时，需要知道MSYS_PATH这个信息，才能进行匹配！
            std::string path_mixed = unix;
            if (path_mixed.length() >= 2 && std::isalpha(path_mixed[1]) && (path_mixed.length() == 2 || (path_mixed.length() >= 3 && path_mixed[2] == '/'))) {
                path_mixed[0] = path_mixed[1];
                path_mixed[1] = ':';
            }
            else if (!this->msys_path.empty()){
                // 对 MSYS_PATH 下，对应子路径，是否存在；此部分代码，必须在windows+msys下！
                path_mixed = this->msys_path + path_mixed.substr(1);
                // NOTE 不需要检查存在性――此时由用户自己解决
            }
            return path_mixed;
        }

        bool fstab::convert_mixed_by_rule(const std::string& mixed, std::string& unix, const std::string& device, const std::string& mount_point) const
        {
            bool ret = false;
            if (sss::is_begin_with(mixed, device, true))
            {
                unix = mount_point + mixed.substr(device.length());
                ret = true;
            }
            else if (sss::is_begin_with(device, mixed, true) && device[mixed.length()] == '/') {
                unix = mount_point;
                ret = true;
            }
            std::cout
                << __func__
                << ": " << VALUE_MSG(mixed)
                << ", " << VALUE_MSG(unix)
                << ", " << VALUE_MSG(device)
                << ", " << VALUE_MSG(mount_point)
                << ", " << VALUE_MSG(ret)
                << std::endl;
            std::cout << VALUE_MSG(sss::is_begin_with(mixed, device, true)) << std::endl;
            return ret;
        }

        void fstab::read_command(const std::string& line)
        {
            std::cout << __func__ << " " << VALUE_MSG(line) << std::endl;
            std::istringstream iss(line);
            std::string mount_url;
            std::string mount_point;
            if (iss >> mount_url >> mount_point) {
                char mount_url_type = cygpath::url_type(mount_url);
                if (mount_url_type == 'w' && mount_url.length() == 2) {
                    std::cout << __func__ << ":" << VALUE_MSG(mount_url) << "; " << VALUE_MSG(mount_url_type) << std::endl;
                    mount_url_type = 'm';
                }
                if (mount_url_type != 'm') {
                    return;
                }
                if (cygpath::url_type(mount_point) != 'u') {
                    return;
                }
                ::unescape_str(mount_url);
                mount_url = this->mixed2windows(mount_url);
                std::cout << __func__ << " mount `" << mount_url << "` -> `" << mount_point << "`" << std::endl;
                if (!sss::is_end_with(mount_url, "/")) {
                    mount_url.append("/");
                }
                // NOTE 由于最长匹配原则；如果已经有载入点，则保留载入点路径最长的！
                if (this->Base_t::find(mount_url) != this->Base_t::end()) {
                    if (this->Base_t::operator[](mount_url).length() >= mount_point.length()) {
                        return;
                    }
                }
                this->operator[](mount_url) = mount_point;
            }
        }

        std::string fstab::to_unix(const std::string& path) const
        {
            switch (cygpath::url_type(path)) {
            case 'm':
                return this->mixed2unix(path);

            case 'w':
                return this->windows2unix(path);

            case 'u':
                return path;

            default:
                return "";
            }
        }

        std::string fstab::to_mixed(const std::string& path) const
        {
            switch (cygpath::url_type(path)) {
            case 'm':
                return path;

            case 'w':
                return this->windows2mixed(path);

            case 'u':
                return this->unix2mixed(path);

            default:
                return "";
            }
        }

        std::string fstab::to_windows(const std::string& path) const
        {
            switch (cygpath::url_type(path)) {
            case 'm':
                return this->mixed2windows(path);

            case 'w':
                return path;

            case 'u':
                return this->unix2windows(path);

            default:
                return "";
            }
        }

        std::string fstab::windows2unix(const std::string& path) const
        {
            return this->match_mount_device(this->windows2mixed(path));
        }

        std::string fstab::windows2mixed(const std::string& path) const
        {
            return sss::replace_all_copy(path, "\\", "/");
        }

        std::string fstab::unix2windows(const std::string& path) const
        {
            return this->match_mount_point(path);
        }

        std::string fstab::unix2mixed(const std::string& path) const
        {
            return this->windows2mixed(this->unix2windows(path));
        }

        std::string fstab::mixed2windows(const std::string& path) const
        {
            return sss::replace_all_copy(path, "/", "\\");
        }

        std::string fstab::mixed2unix(const std::string& path) const
        {
            return this->match_mount_device(path);
        }
    } // namespace cygpath
} // namespace sss
