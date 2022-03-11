// NOTE TODO
#include <gtest/gtest.h>

TEST(tdspp2, windows)
{

}

#ifdef __NOT_EXIST__
// g++ main.cc -lsss -liconv.dll -lsybdb -lws2_32
#include <stdio.h>
#include <string.h>
#include <ctpublic.h>
#include <iostream>

#include "tdspp.h"

int main(int argc, char **argv) {

    Login db;
    try {
        /* Connect to database. */
        db.connect("127.0.0.1:1433", "lzdyf", "ws65635588");
        db.use_db("st_ccerp");
        /* Execute command. */
        db.execute("use ls_ccerp");
        /* Create query. */
        // "select top 10 dzyid, dzyname from zhiydoc -- where beactive = '是'"
        Query *q = db.sql("select top 10 dzyid from zhiydoc where beactive = '是'");

        try {
            /* Execute SQL query. */
            q->execute();
            /* Print table headers, i.e. column names. */
            q->printheader();
            FieldList * fl = 0;
            while (!(fl = q->fetch()) ) {
                std::cout << *fl << std::endl;
            }
        }
        catch(Login::Exception &e) {
            std::cerr << e.message << std::endl;
        }
        delete q;
    }
    catch(Login::Exception &e) {
        std::cerr << e.message << std::endl;
    }
    return 0;
}
#endif
