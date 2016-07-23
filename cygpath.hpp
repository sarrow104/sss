#ifndef __CYGPATH_HPP_1466163894__
#define __CYGPATH_HPP_1466163894__

#include <string>
#include <map>

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
        char url_type(const std::string& url);

        // 载入fstab文件配置;
        // NOTE 对于msys2的话，还应当载入 fstab.d/`id -un` 这个配置文件；
        class fstab : private std::map<std::string, std::string> {
        private:
            std::string msys_path; // 当从外部 fstab 文件载入配置时，同时记录根目录信息；

            typedef std::map<std::string, std::string> Base_t;

        private:
            using Base_t::operator[];

        public:
            fstab();
            explicit fstab(const std::string& fstab_path);
            ~fstab();

        protected:
            /**
             * @brief 从载入设备信息开始匹配
             *
             * @param mixed
             *
             * @return 空串表示没有找到
             */
            std::string match_mount_device(const std::string& mixed) const;
            /**
             * @brief 从载入点进行匹配；
             *
             * @param mixed
             *
             * @return 空串表示没有找到；
             */
            std::string match_mount_point(const std::string& mixed) const;


            bool convert_mixed_by_rule(const std::string& mixed,
                                       std::string& unix,
                                       const std::string& device,
                                       const std::string& mount_point) const;

        public:
            void load(const std::string& fstab_path);
            void load_from_msys(const std::string& msys_base);
            void read_command(const std::string& line);

            std::string to_unix     (const std::string& path) const;
            std::string to_mixed    (const std::string& path) const;
            std::string to_windows  (const std::string& path) const;

            std::string windows2unix(const std::string& path) const;
            std::string windows2mixed(const std::string& path) const;
            std::string unix2windows(const std::string& path) const;
            std::string unix2mixed  (const std::string& path) const;
            std::string mixed2windows(const std::string& path) const;
            std::string mixed2unix  (const std::string& path) const;
        };

        inline std::string windows2unix(const std::string& path)
        {
            return fstab().windows2unix(path);
        }
        inline std::string windows2mixed(const std::string& path)
        {
            return fstab().windows2mixed(path);
        }
        inline std::string unix2windows(const std::string& path)
        {
            return fstab().unix2windows(path);
        }
        inline std::string unix2mixed(const std::string& path)
        {
            return fstab().unix2mixed(path);
        }
        inline std::string mixed2windows(const std::string& path)
        {
            return fstab().mixed2windows(path);
        }
        inline std::string mixed2unix(const std::string& path)
        {
            return fstab().mixed2unix(path);
        }
    } // namespace cygpath
} // namespace sss



#endif /* __CYGPATH_HPP_1466163894__ */
