#ifndef  __FILE_HPP_1440340585__
#define  __FILE_HPP_1440340585__

#include <sss/time.hpp>

#include <string>
#include <vector>

#include <stdint.h>

namespace sss {
    namespace os {
        // 从系统的回收站，重建映射；
        // 可能用的查找关系是：
        // 1. 原路径；
        // 2. 原名；
        // 3. 删除时间；
        // 4. 文件大小等；
        class RecycleBin
        {
        public:
            RecycleBin();
            ~RecycleBin();

        public:
            struct item_t{
                std::string     _url;
                std::string     _name; // 在 回收站 中的名字；
                sss::time::Date _delTime;
                uint64_t        _fsize;
            };

            void sortByDeltime();
            void sortByFName(); // 回收站名——相当于原文件名
            void sortByFSize();
            void sortByDelPosition();

            void recycle(const std::string& path);

        public:
            static void clear(); // to empty the recycle bin

            static std::string genFilePath(const std::string& item_name)
            {
                return getTrashPath() + "/files/" + item_name;
            }
            static std::string genInfoPath(const std::string& item_name)
            {
                return getTrashPath() + "/info/" + item_name + ".trashinfo";
            }

        public:
            static const std::string& getTrashPath();

        private:
            std::vector<item_t> _data;
        };

        bool recycle(const std::string& path);
    }
}



#endif  /* __FILE_HPP_1440340585__ */
