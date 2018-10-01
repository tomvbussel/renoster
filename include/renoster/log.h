#ifndef RENOSTER_LOG_H_
#define RENOSTER_LOG_H_

#include <iostream>

#include "renoster/util/strutil.h"

namespace renoster {

template <typename... Args>
inline void Error(const std::string & fmt, const Args &... args) {
    std::cout << "Error: " << Format(fmt, args...) << std::endl;
}

template <typename... Args>
inline void Warning(const std::string & fmt, const Args &... args) {
    std::cout << "Warning: " << Format(fmt, args...) << std::endl;
}

template <typename... Args>
inline void Info(const std::string & fmt, const Args &... args) {
    std::cout << "Info: " << Format(fmt, args...) << std::endl;
}

}  // namespace renoster

#endif  // RENOSTER_LOG_H_