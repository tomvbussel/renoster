%{
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stack>
#include <string>

#include "renoparse.hpp"

#include "renoster/log.h"

#include "util/filesystem.h"

namespace renoster {

extern int lineNum;

struct IncludeInfo {
    std::string filename;
    YY_BUFFER_STATE bufState;
    int lineNum;
};

std::stack<IncludeInfo> includes;

void PushInclude(const std::string & filename)
{
    std::string newFile = filename; // TODO

    FILE * f = fopen(newFile.c_str(), "r");
    if (!f) {
        Error("Could not open included file \"%s\"", newFile);
    } else {
        extern std::string currentFile;
        IncludeInfo info;
        info.filename = currentFile;
        info.bufState = YY_CURRENT_BUFFER;
        info.lineNum = lineNum;
        includes.push(info);

        yyin = f;
        currentFile = newFile;
        lineNum = 0;

        yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
    }
}

void PopInclude()
{
    extern int lineNum;
    extern std::string currentFile;

    fclose(yyin);
    yy_delete_buffer(YY_CURRENT_BUFFER);
    yy_switch_to_buffer(includes.top().bufState);
    currentFile = includes.top().filename;
    lineNum = includes.top().lineNum;

    includes.pop();
}

} // namespace renoster

%}

WHITESPACE [ \t\v\f\r]+

ALPHA [a-zA-Z]

DIGIT [0-9]

INTEGER [-+]?{DIGIT}+

E [eE][-+]?{DIGIT}+
FLOAT1 [-+]?{DIGIT}+\.{DIGIT}*{E}?
FLOAT2 [-+]?{DIGIT}*\.{DIGIT}+{E}?
FLOAT3 [-+]?{DIGIT}+{E}
FLOAT {FLOAT1}|{FLOAT2}|{FLOAT3}

STR \"(\\.|[^\\"])*\"

COMMENT \#[^\n]*\n

%%

{COMMENT}       { /* ignore */ }

AttributeBegin  { return ATTRIBUTEBEGIN; }
AttributeEnd    { return ATTRIBUTEEND; }
Camera          { return CAMERA; }
Display         { return DISPLAY; }
Film            { return FILM; }
Geometry        { return GEOMETRY; }
GeometryLight   { return GEOMETRYLIGHT; }
Identity        { return IDENTITY; }
Include         { return INCLUDE; }
Integrator      { return INTEGRATOR; }
LookAt          { return LOOKAT; }
Material        { return MATERIAL; }
Orthographic    { return ORTHOGRAPHIC; }
Perspective     { return PERSPECTIVE; }
PixelFilter     { return PIXELFILTER; }
Rotate          { return ROTATE; }
Sampler         { return SAMPLER; }
Scale           { return SCALE; }
TransformBegin  { return TRANSFORMBEGIN; }
TransformEnd    { return TRANSFORMEND;}
Translate       { return TRANSLATE; }
WorldBegin      { return WORLDBEGIN; }
WorldEnd        { return WORLDEND; }

{INTEGER}       { yylval.num = std::atol(yytext);
                  return NUM; }

{FLOAT}         { yylval.num = std::atof(yytext);
                  return NUM; }

"["             { return LBRACK; }
"]"             { return RBRACK; }

{STR}           { std::strcpy(yylval.str, yytext);
                  return STRING;}

{WHITESPACE}    { /* ignore */ }

\n              { /* ignore */ }

.               { /* ignore */ }

%%

int yywrap()
{
    if (renoster::includes.empty()) {
        return 1;
    }
    renoster::PopInclude();
    return 0;
}
