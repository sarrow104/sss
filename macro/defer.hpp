// macro/defer.hpp
#pragma once

#include <sss/macro/concat.hpp>
#include <sss/util/scopeguard.hpp>

#define SSS_DEFER(fn) sss::scopeguard SSS_CONCAT(__deffer__, __LINE__) = [&]() {  fn ; }
// NOTE example
// std::ofstream f("/path/to/file");
// SSS_DEFER( f.close() );
//
// if multy-line statements are needed, use { } to parentheses them
