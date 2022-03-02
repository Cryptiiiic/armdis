//
// Created by cryptic on 10/14/21.
//

#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__
#include "Global.hpp"

class Logger {
    private:
        bool _debug = false;
    public:
        Logger(bool init, bool debug);
        template<typename... Args>
        int print(const char *format, Args... args){return printf(format, args...);};
        template<typename... Args>
        int debugPrint(const char *format, Args... args) {return (!_debug) ? 0 : printf(format, args...);};
};

extern Logger *logger;

#endif // __LOGGER_HPP__