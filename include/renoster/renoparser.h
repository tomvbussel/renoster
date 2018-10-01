#ifndef RENOSTER_RENOPARSER_H_
#define RENOSTER_RENOPARSER_H_

#include <string>

#include "renoster/export.h"

namespace renoster {

RENO_API bool ParseRenoFile(const std::string & filename);

}  // namespace renoster

#endif  // RENOSTER_RENOPARSER_H_