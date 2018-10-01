#ifndef RENOSTER_UTIL_STRUTIL_H_
#define RENOSTER_UTIL_STRUTIL_H_

#include <vector>

#ifndef TINYFORMAT_USE_VARIADIC_TEMPLATES
# define TINYFORMAT_USE_VARIADIC_TEMPLATES
#endif
#include "tinyformat.h"

namespace renoster {

template <typename... Args>
inline std::string Format(const std::string & fmt, const Args & ... args)
{
    return tinyformat::format(fmt.c_str(), args...);
}

inline std::vector<std::string> Tokenize(const std::string & str,
                                         const std::string & sep,
                                         bool keepEmpty)
{
    std::vector<std::string> tokens;

    size_t oldPos = 0;
    while (oldPos != std::string::npos) {
        size_t pos = str.find_first_of(sep, oldPos);

        if (oldPos != pos || keepEmpty) {
            tokens.push_back(str.substr(oldPos, pos - oldPos));
        }

        oldPos = pos;
        if (oldPos != std::string::npos) {
            oldPos += 1;
        }
    }

    return tokens;
}

} // namespace renoster

#endif // RENOSTER_UTIL_STRUTIL_H_
