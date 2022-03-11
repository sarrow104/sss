#include <gtest/gtest.h>

#include "../log.hpp"

TEST(log, basic)
{
    sss::log::level(sss::log::log_WARN);
    sss::log::timef();
    sss::log::error("%s func %s, line %d, level %d log_ERROR\n", __FILE__, __func__, __LINE__, sss::log::log_ERROR);
    sss::log::warn("%s line %d %d log_WARN\n", __func__, __LINE__, sss::log::log_WARN);
    sss::log::info("%s line %d %d log_INFO\n", __func__, __LINE__, sss::log::log_INFO);
    sss::log::debug("%s line %d %d log_DEBUG\n", __func__, __LINE__, sss::log::log_DEBUG);
}
