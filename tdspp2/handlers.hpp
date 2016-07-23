#ifndef  __HANDLERS_HPP_1365605601__
#define  __HANDLERS_HPP_1365605601__

#include <sybdb.h>

namespace sss {
    namespace tdspp2 {
int msg_handler(DBPROCESS*, DBINT, int, int, char*, char*, char*, int);

int err_handler(DBPROCESS*, int, int, int, char*, char*);
    } // namespace tdspp2
} // namespace sss


#endif  /* __HANDLERS_HPP_1365605601__ */
