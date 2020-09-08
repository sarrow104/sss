// macro/pp_debug.hpp
#ifndef __PP_DEBUG_HPP_1569563753__
#define __PP_DEBUG_HPP_1569563753__

#include <sss/macro/cast.hpp>
#include <sss/macro/cast_to_string.hpp>

#define SSS_PP_DEBUG( __X__ ) SSS_PP_CAST( ( __X__ ), SSS_PP_CAST_TO_STRING)
#define SSS_PP_DEBUG_AORW( __X__ , __a_or_w__ ) SSS_PP_CAST( ( __X__ ), __a_or_w__)


#endif /* __PP_DEBUG_HPP_1569563753__ */
