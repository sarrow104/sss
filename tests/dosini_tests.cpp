#include <gtest/gtest.h>

#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "../dosini/dosini.hpp"

// TODO
// http://stackoverflow.com/questions/16135285/iterate-over-ini-file-on-c-probably-using-boostproperty-treeptree
// boost::ptree风格：
// 

TEST(dosini, boost_ptree)
{
    using boost::property_tree::ptree;
    ptree pt;

    read_ini("cases/input.txt", pt);

    // NOTE TODO
    //
    // ptree 可能不支持匿名 section
    for (auto& section : pt)
    {
        std::cout << '[' << section.first << "]\n";
        for (auto& key : section.second) {
            std::cout << key.first << "=" << key.second.get_value<std::string>() << "\n";
        }
    }
    // throw 1;
}

TEST(dosini, dosini)
{
    sss::dosini cfg("./cases/input.txt");
    for (const auto& section : cfg) {
        std::cout << '[' << section.first << "]\n";
        for (const auto& kv : section.second) {
            std::cout << kv.first << "=" << kv.second << std::endl;
        }
    }
    // throw 1;
}
