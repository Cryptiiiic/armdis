//
// Created by cryptic on 10/14/21.
//

#include "Logger.hpp"

Logger::Logger(bool init, bool debug) : _debug(debug) {
    if(init) {
        std::ios_base::sync_with_stdio(false);
        std::wcout.imbue(std::locale("en_US.UTF-8"));
    }
}

