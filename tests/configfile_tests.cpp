#include "../ConfigFile.hpp"
#include "../path.hpp"
#include "../utlstring.hpp"

#include <cmath>

#include <gtest/gtest.h>

#include <iostream>

static const char * const INI_FILE_NEW_LINE_MARKER = "\r\n";

TEST(configfile, fromfile)
{
    try {
        sss::ConfigFile cf("cases/config.ini");

        std::string foo;
        std::string water;
        double      four = NAN;

        foo   = cf.value("section_1", "foo"  );
        water = cf.value("section_2", "water");
        four  = sss::string_cast<double>(cf.value("section_2", "four" ));

        std::cout << foo   << INI_FILE_NEW_LINE_MARKER;
        std::cout << water << INI_FILE_NEW_LINE_MARKER;
        std::cout << four  << INI_FILE_NEW_LINE_MARKER;

        GTEST_ASSERT_EQ(foo, " bar");
        GTEST_ASSERT_EQ(water, " wet");
        GTEST_ASSERT_EQ(four, 4.2);
    }
    catch (std::runtime_error & e) {
        std::cout << sss::path::getbin() << std::endl;
        std::cout << e.what() << std::endl;
    }
}

// TODO
//
// add parse_from_binary() api and testing
