/**
 * "foreach宏"
 *
 * trackback:http://bbs.pfan.cn/post-284099.html
 *
 * 作者：gtkmm
 */

//=================================================
//         foreach.h
//=================================================

#ifndef traxex_foreach
#define traxex_foreach
#define _traxex_element_size( _foreach_array)            \
    ( (char*)( _foreach_array+1)-(char*)(_foreach_array))

#define _traxex_array_length( _foreach_array)                \
    ( sizeof( _foreach_array)/_traxex_element_size(_foreach_array))

#define _traxex_foreach( _foreach_element,_foreach_array)        \
    if( unsigned int _foreach_index=0){}else                \
    if( unsigned int _foreach_size=_traxex_array_length( _foreach_array)) \
    if( bool _foreach_next=true)                    \
    for(_foreach_index=0;!(_foreach_next=!_foreach_next) && ( _foreach_index<_foreach_size) ; _foreach_index++) \
    for( _foreach_element=_foreach_array[_foreach_index];!_foreach_next;_foreach_next=true)

#define foreach _traxex_foreach
#endif

