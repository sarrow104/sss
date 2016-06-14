#include "environ.hpp"

#include <cstdio>

namespace sss {
    namespace env {
        size_t count(char ** env)
        {
            char ** p_env = env;
            int ret = 0;

            while (p_env && p_env[0]) {
                // std::printf("%s\n", p_env[0]);
                p_env ++;
                ret ++;
            }
            return ret;
        }

        bool contain(const std::string& name, char ** env) {
            if (env == environ) {
                return ::getenv(name.c_str());
            }
            else {
                char ** p_env = env;
                while (p_env && p_env[0]) {
                    int eq_pos = std::strchr(p_env[0], '=') - p_env[0];
                    if (sss::is_equal(p_env[0], p_env[0] + eq_pos, name.begin(), name.end()) ) {
                        return true;
                    }
                    p_env++;
                }
                return false;
            }
        }
        char * get(const std::string& name, char ** env) {
            if (env == environ) {
                return ::getenv(name.c_str());
            }
            else {
                char ** p_env = env;
                while (p_env && p_env[0]) {
                    int eq_pos = std::strchr(p_env[0], '=') - p_env[0];
                    if (sss::is_equal(p_env[0], p_env[0] + eq_pos, name.begin(), name.end()) ) {
                        return p_env[0] + eq_pos + 1;
                    }
                    p_env++;
                }
                return 0;
            }
        }

        size_t dump(std::string& out, char ** env)
        {
            std::string tmp_out;
            size_t size = 0;
            size_t env_cnt = 0;
            for (char ** p_env = env; p_env && p_env[0]; p_env++)
            {
                size += std::strlen(p_env[0]) + 1;
                env_cnt ++;
            }
            tmp_out.reserve(size);
            for (char ** p_env = env; p_env && p_env[0]; p_env++)
            {
                tmp_out += p_env[0];
                tmp_out += '\0';
            }
            std::swap(out, tmp_out);

            return env_cnt;
        }

        char ** dup_c_str(char ** env) {
            size_t size = sizeof(char *);
            size_t env_cnt = 0;
            for (char ** p_env = env; p_env && p_env[0]; p_env++)
            {
                size += sizeof(char *) + std::strlen(p_env[0]) + 1;
                env_cnt ++;
            }
            char * buf = new char[size]; // ;
            char ** ret = reinterpret_cast<char**>(buf);
            char * content = buf + (1 + env_cnt) * sizeof(char *);
            int i = 0;
            for (char ** p_env = env; p_env && p_env[0]; p_env++)
            {
                ret[i] = content;
                size_t cnt = std::sprintf(content, "%s", p_env[0]);
                content += cnt + 1;
            }
            ret[env_cnt] = 0;

            return ret;
        }
    } // namespace env
} // namespace sss
