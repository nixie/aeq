#ifndef __LOGBUF_H__
#define __LOGBUF_H__

#include <iostream>
#include <string>

class LogBuf {
    private:
        std::string buf;
    public:
        void add_msg(std::string m){ buf += m; buf += '\n';}
        void flush(){ std::cerr << buf; }
};

#endif

