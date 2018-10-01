#include "renoster/renoparser.h"

#include <cstdio>

#include "renoster/plugin.h"

#include "util/filesystem.h"

extern FILE * yyin;
extern int yyparse();

namespace renoster {

bool ParseRenoFile(const std::string & filename)
{
    AppendToPluginSearchPath(ParentPath(filename));
    yyin = std::fopen(filename.c_str(), "r");
    if (yyin != nullptr)
    {
        yyparse();
        std::fclose(yyin);
    }

    return yyin != nullptr;
}

} // namespace renoster
