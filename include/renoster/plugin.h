#ifndef RENOSTER_PLUGIN_H_
#define RENOSTER_PLUGIN_H_

#include <memory>
#include <string>

#include "renoster/bounds.h"
#include "renoster/export.h"

namespace renoster {

RENO_API void SetPluginSearchPath(const std::string & searchPath);

RENO_API void AppendToPluginSearchPath(const std::string & searchPath);

}  // namespace renoster

#endif  // RENOSTER_PLUGIN_H_
