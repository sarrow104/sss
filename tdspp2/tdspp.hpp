// {o: -lsybdb -lws2_32 -liconv}
#ifndef  __TDSPP_HPP_1364391251__
#define  __TDSPP_HPP_1364391251__

#include "Login.hpp"
#include "DBLink.hpp"
#include "query.hpp"
#include "field.hpp"

#include <exception>

namespace sss {
namespace tdspp2 {

/** Exception class for tds++ */
class Exception : public std::exception
{
public :
    /** Error message */
    std::string message;

    /** Constructor */
    explicit Exception(const std::string& msg="") throw() :message(msg)
    { }
    /** Destructor */
    virtual ~Exception() throw()
    { }

public:
    virtual const char * what() const throw();
};


} // namespace tdspp2
} // namespace sss

#endif  /* __TDSPP_HPP_1364391251__ */
