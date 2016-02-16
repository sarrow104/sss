#ifdef ____MAKE_THIS_HAPPEN____
// http://bbs.csdn.net/topics/20390933
//     cosy
//date:2003-02-28
// 以前留下的代码，
//似乎没有遇到不能超过255的问题，
//你先拿去调试一下看。

// SQL Server connection structure
int print_row ( DBPROCESS *dbproc)
{
    int x,cols,size,datasize,colwidth,coltype;  // counters
    char *datavals;         // data buffer pointer
    char *data;             // column data pointer
    char *ptr;              // scratch pointer

    colwidth = row_size(dbproc,0);
    ptr = datavals = malloc(colwidth+1);    // get buffer

    cols = dbnumcols(dbproc);               // get number of columns
    for(x=1;x<=cols;x++)                    // do all columns
    {
        coltype = dbcoltype(dbproc,x);
        size = row_size(dbproc,x);  // determine size of this column
        memset(ptr,' ',size);               // set it to spaces
        data = (char *)dbdata(dbproc,x);    // get pointer to column's data
        if(data == (BYTE *)NULL)            // if NULL, use "NULL"
        {
            strncpy(ptr,"NULL",4);          // set NULL into buffer
            ptr += size;                    // point past this column in output buf
        }
        else                                // else have data, so convert to char
        {
            datasize = dbconvert(dbproc,coltype,data,dbdatlen(dbproc,x),
                                 SQLCHAR,ptr,(DBINT)size-1);
            if (datasize < size && (coltype == SQLNUMERIC ||
                                    coltype == SQLDECIMAL || coltype == SQLINT1 ||
                                    coltype == SQLINT2 || coltype == SQLINT4 ||
                                    coltype == SQLFLT8 || coltype == SQLFLT4))
            {
                memmove(ptr+size-1-datasize,ptr,datasize);
                memset(ptr,' ',size-1-datasize);
            }

            ptr += size;
        }
    }

    *ptr = '\0';                // null term string
    con_fprintf(hstdout,"%s\n",datavals);    // print row
    free(datavals);             // free buffer
}


//jhliusoft
//
//date:2003-02-28
//在Sql Server Query Analyzer中, 有一个设置选项, 点击 Query-> Current Connectio Options中的Advance中有 Maximum characters per, 设置这个选项, 可以控制varchar的返回列数, 可以试试看.
//
#endif
