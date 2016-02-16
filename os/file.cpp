#include "file.hpp"

#include <sss/path/glob_path.hpp>
#include <sss/utlstring.hpp>
#include <sss/time.hpp>
#include <sss/path.hpp>
#include <sss/ps.hpp>

#include <iomanip>
#include <stdlib.h>

namespace {

    inline std::string encoding(const std::string& url)
    {
        std::ostringstream oss;
        for (std::string::const_iterator it = url.begin(); it != url.end(); ++it) {
            switch (*it) {
            case ' ':
            case ':':
            case '*':
            case '?':
            case '\\':
                oss << '%' << std::hex << std::setw(2) << std::setfill('0') << (*it & 0xFFu);
                break;

            default:
                if (*it & 0x80u) {
                    oss << '%' << std::hex << std::setw(2) << std::setfill('0') << (*it & 0xFFu);
                }
                else {
                    oss << *it;
                }
                break;
            }
        }
        return oss.str();
    }
}

namespace sss {
    namespace os {
        bool recycle(const std::string& path) {
            // 1. 生成可用的回收站名
            // 2. mv源文件，记录删除信息；
            // 3. 将删除信息，写入info文件夹；

            std::string fpath = sss::path::full_of_copy(path);
            if (!sss::path::file_exists(fpath)) {
                return false;
            }
            std::string rb_name = sss::path::basename(fpath);
            std::string rb_name_nf = sss::path::no_suffix(rb_name);
            std::string rb_name_fx = sss::path::suffix(rb_name);

            int num = 2;
            while (true) {
                std::string rec_file_path = RecycleBin::genFilePath(rb_name);
                std::string rec_info_path = RecycleBin::genInfoPath(rb_name);

                if (sss::path::file_exists(rec_file_path) ||
                    sss::path::file_exists(rec_info_path))
                {
                    rb_name = rb_name_nf + "." + sss::cast_string(num) + rb_name_fx;
                    num++;
                    assert(num < 500);
                }
                else {
                    sss::ps::StringPipe ps;
                    ps << "mv";
                    ps.add(fpath);
                    ps.add(rec_file_path);
                    ps.run();

                    std::ofstream ofs(rec_info_path.c_str());
                    // [Trash Info]
                    // Path=/home/$USER/path/to/delete/position/a.txt
                    // DeletionDate=2015-08-23T22:01:36
                    ofs << "[Trash Info]\n"
                        << "Path=" << ::encoding(fpath) << '\n'
                        << "DeletionDate="
                        << sss::time::strftime("%FT%T")
                        << std::endl;
                    return true;
                }
            }
            return false;
        }

        // to empty the recycle bin
        void RecycleBin::clear()
        {
            sss::path::dirClear(getTrashPath() + "/files/");
            sss::path::dirClear(getTrashPath() + "/info/");
        }

        // 貌似让应用程序，获取所有回收站文件信息，没什么卵用。
        RecycleBin::RecycleBin()
        {
            std::string recyclebin_path =
                "/home/" + std::string(getenv("USER")) + "/.local/share/Trash";

            std::string recbin_files = recyclebin_path + "/files";
            std::string recbin_info = recyclebin_path + "/info";

            std::cout << recyclebin_path << std::endl;
            sss::path::file_descriptor fd;
            sss::path::glob_path pg(recyclebin_path, fd);
        }

        const std::string& RecycleBin::getTrashPath()
        {
            static std::string _trash_path = "/home/" + std::string(getenv("USER")) + "/.local/share/Trash";
            return _trash_path;
        }

        RecycleBin::~RecycleBin()
        {
        }
    }
}
