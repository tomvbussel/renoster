%{

#include <iostream>

#include <cstdlib>
#include <string>
#include <vector>

#include "renoster/log.h"
#include "renoster/reno.h"
#include "renoster/paramlist.h"

void yyerror(const char *err)
{
    renoster::Error("Parsing error: %s", err);
    std::exit(1);
}

extern int yylex();

namespace renoster {

ParameterList params;
std::string name;

extern void PushInclude(const std::string & filename);
std::string currentFile;
int lineNum = 0;

enum class Type {
    kBool,
    kColor,
    kFloat,
    kInt,
    kNormal3f,
    kPoint3f,
    kPoint2f,
    kString,
    kVector3f,
    kVector2f
} array_type;

std::vector<bool> bool_array;
std::vector<int> int_array;
std::vector<float> float_array;
std::vector<std::string> string_array;
std::vector<Color> color_array;
std::vector<Normal3f> normal3f_array;
std::vector<Point2f> point2f_array;
std::vector<Point3f> point3f_array;
std::vector<Vector2f> vector2f_array;
std::vector<Vector3f> vector3f_array;

int i;
float values[3];

} // namespace renoster

using namespace renoster;

%}

%union
{
    double num;
    char str[1024];
}

%locations

%token <num> NUM
%token <str> STRING

%token LBRACK
%token RBRACK

%token ATTRIBUTEBEGIN
%token ATTRIBUTEEND
%token CAMERA
%token DISPLAY
%token FILM
%token GEOMETRY
%token GEOMETRYLIGHT
%token IDENTITY
%token INCLUDE
%token INTEGRATOR
%token LOOKAT
%token MATERIAL
%token ORTHOGRAPHIC
%token PERSPECTIVE
%token PIXELFILTER
%token ROTATE
%token SAMPLER
%token SCALE
%token TRANSFORMBEGIN
%token TRANSFORMEND
%token TRANSLATE
%token WORLDBEGIN
%token WORLDEND

%token HIGH_PRECEDENCE

%start reno_file;

%%

reno_file
: reno_stmt_list
;

num_array
: num_array num_array_entry
| num_array_entry
;

num_array_entry
: NUM
{
    if (array_type == Type::kFloat)
    {
        float_array.push_back($1);
    }
    else if (array_type == Type::kColor)
    {
        values[i++] = $1;
        if (i >= 3)
        {
            i = 0;
            color_array.emplace_back(values[0], values[1], values[2]);
        }
    }
    else if (array_type == Type::kInt)
    {
        int_array.push_back($1);
    }
    else if (array_type == Type::kNormal3f)
    {
        values[i++] = $1;
        if (i >= 3)
        {
            i = 0;
            normal3f_array.emplace_back(values[0], values[1], values[2]);
        }
    }
    else if (array_type == Type::kPoint3f)
    {
        values[i++] = $1;
        if (i >= 3)
        {
            i = 0;
            point3f_array.emplace_back(values[0], values[1], values[2]);
        }
    }
    else if (array_type == Type::kPoint2f)
    {
        values[i++] = $1;
        if (i >= 2)
        {
            i = 0;
            point2f_array.emplace_back(values[0], values[1]);
        }
    }
    else if (array_type == Type::kVector3f)
    {
        values[i++] = $1;
        if (i >= 3)
        {
            i = 0;
            vector3f_array.emplace_back(values[0], values[1], values[2]);
        }
    }
    else if (array_type == Type::kVector2f)
    {
        values[i++] = $1;
        if (i >= 2)
        {
            i = 0;
            vector2f_array.emplace_back(values[0], values[1]);
        }
    }
    else
    {
        Error("Numerical parameter array of wrong type");
    }
}
;

string_array
: string_array string_array_entry
| string_array_entry
;

string_array_entry
: STRING
{
    std::string str($1);
    str = str.substr(1, str.length() - 2);

    if (array_type == Type::kBool)
    {
        if (str == "true")
        {
            bool_array.push_back(true);
        }
        else if (str == "false")
        {
            bool_array.push_back(false);
        }
        else
        {
             Error("Bools should be either \"true\" or \"false\"");
        }
    }
    else if (array_type == Type::kString)
    {
        string_array.push_back(str);
    }
    else
    {
        Error("String parameter array of wrong type");
    }
}

paramlist
: paramlist_init paramlist_contents
;

paramlist_init
: 
;

paramlist_contents
: paramlist_entry paramlist_contents
| /* nothing */
;

paramlist_entry
: paramlist_entry_name LBRACK num_array RBRACK
{
    if ((array_type == Type::kNormal3f
        || array_type == Type::kPoint3f
        || array_type == Type::kVector3f
        || array_type == Type::kColor) 
        && i != 0)
    {
        Error("Vector/Normal/Point values should be in multiples of 3");
    }
    if ((array_type == Type::kPoint2f || array_type == Type::kVector2f)
      && i != 0)
    {
        Error("Vector2/Point2 values should be in multiples of 2");
    }

    if (array_type == Type::kColor)
    {
        params.SetColors(name, std::move(color_array));
        color_array.clear();
    }
    else if (array_type == Type::kFloat)
    {
        params.SetFloats(name, std::move(float_array));
        float_array.clear();
    }
    else if (array_type == Type::kInt)
    {
        params.SetInts(name, std::move(int_array));
        int_array.clear();
    }
    else if (array_type == Type::kNormal3f)
    {
        params.SetNormal3fs(name, std::move(normal3f_array));
        normal3f_array.clear();
    }
    else if (array_type == Type::kPoint3f)
    {
        params.SetPoint3fs(name, std::move(point3f_array));
        point3f_array.clear();
    }
    else if (array_type == Type::kPoint2f)
    {
        params.SetPoint2fs(name, std::move(point2f_array));
        point2f_array.clear();
    }
    else if (array_type == Type::kVector3f)
    {
        params.SetVector3fs(name, std::move(vector3f_array));
        vector3f_array.clear();
    }
    else if (array_type == Type::kVector2f)
    {
        params.SetVector2fs(name, std::move(vector2f_array));
        vector2f_array.clear();
    }
    else
    {
        Error("Numerical parameter list of wrong type");
    }
}
| paramlist_entry_name LBRACK string_array RBRACK
{
    if (array_type == Type::kBool)
    {
        params.SetBools(name, std::move(bool_array));
        bool_array.clear();
    }
    else if (array_type == Type::kString)
    {
        params.SetStrings(name, std::move(string_array));
        string_array.clear();
    }
    else
    {
        Error("String parameter list of wrong type");
    }
}
;

paramlist_entry_name
: STRING
{
    std::string sname($1);
    sname = sname.substr(1, sname.length() - 2);
    const auto type_begin_pos = sname.find_first_not_of(" \n");
    const auto type_end_pos = sname.find_first_of(" \n", type_begin_pos);
    if (type_begin_pos == std::string::npos)
    {
        Error("Parameter has no type or name");
    }
    std::string stype = sname.substr(type_begin_pos, type_end_pos - type_begin_pos);

    const auto name_begin_pos = sname.find_first_not_of(" \n", type_end_pos);
    const auto name_end_pos = sname.find_first_of(" \n", name_begin_pos);
    if (name_begin_pos == std::string::npos)
    {
        Error("Parameter \"%s\" is missing a type", sname);
    }
    name = sname.substr(name_begin_pos, name_end_pos - name_begin_pos);
    if (name.empty())
    {
        Error("Parameter \"%s\" is missing a type or name", sname);
    }

    if (stype == "bool")
    {
        array_type = Type::kBool;
    }
    else if (stype == "color")
    {
        array_type = Type::kColor;
    }
    else if (stype == "float")
    {
        array_type = Type::kFloat;
    }
    else if (stype == "int")
    {
        array_type = Type::kInt;
    }
    else if (stype == "normal")
    {
        array_type = Type::kNormal3f;
    }
    else if (stype == "point")
    {
        array_type = Type::kPoint3f;
    }
    else if (stype == "point2")
    {
        array_type = Type::kPoint2f;
    }
    else if (stype == "string")
    {
        array_type = Type::kString;
    }
    else if (stype == "vector")
    {
        array_type = Type::kVector3f;
    }
    else if (stype == "vector2")
    {
        array_type = Type::kVector2f;
    }
    else
    {
        Error("Unknown type \"%s\"", stype);
    }
}
;

reno_stmt_list
: reno_stmt_list reno_stmt
| reno_stmt
;

reno_stmt
: ATTRIBUTEBEGIN
{
    RenoAttributeBegin();
}
| ATTRIBUTEEND
{
    RenoAttributeEnd();
}
| CAMERA STRING paramlist
{
    std::string name($2);
    name = name.substr(1, name.length() - 2);
    RenoCamera(name, params);
    params.Clear();
}
| DISPLAY STRING paramlist
{
    std::string name($2);
    name = name.substr(1, name.length() - 2);
    RenoDisplay(name, params);
    params.Clear();
}
| GEOMETRY STRING paramlist
{
    std::string name($2);
    name = name.substr(1, name.length() - 2);
    RenoGeometry(name, params);
    params.Clear();
}
| GEOMETRYLIGHT STRING paramlist
{
    std::string name($2);
    name = name.substr(1, name.length() - 2);
    RenoGeometryLight(name, params);
    params.Clear();
}
| FILM paramlist
{
    RenoFilm(params);
    params.Clear();
}
| IDENTITY
{
    RenoIdentity();
}
| INCLUDE STRING
{
    std::string str($2);
    str = str.substr(1, str.length() - 2);
    PushInclude(str);
}
| INTEGRATOR STRING paramlist
{
    std::string name($2);
    name = name.substr(1, name.length() - 2);
    RenoIntegrator(name, params);
    params.Clear();
}
| LOOKAT NUM NUM NUM NUM NUM NUM NUM NUM NUM
{
    RenoLookAt($2, $3, $4, $5, $6, $7, $8, $9, $10);
}
| MATERIAL STRING paramlist
{
    std::string name($2);
    name = name.substr(1, name.length() - 2);
    RenoMaterial(name, params);
    params.Clear();
}
| ORTHOGRAPHIC NUM NUM
{
    RenoOrthographic($2, $3);
}
| PERSPECTIVE NUM NUM NUM
{
    RenoPerspective($2, $3, $4);
}
| PIXELFILTER STRING paramlist
{
    std::string name($2);
    name = name.substr(1, name.length() - 2);
    RenoPixelFilter(name, params);
    params.Clear();
}
| ROTATE NUM NUM NUM NUM
{
    RenoRotate($2, $3, $4, $5);
}
| SAMPLER STRING paramlist
{
    std::string name($2);
    name = name.substr(1, name.length() - 2);
    RenoSampler(name, params);
    params.Clear();
}
| SCALE NUM NUM NUM
{
    RenoScale($2, $3, $4);   
}
| TRANSFORMBEGIN
{
    RenoTransformBegin();
}
| TRANSFORMEND
{
    RenoTransformEnd();
}
| TRANSLATE NUM NUM NUM
{
    RenoTranslate($2, $3, $4);
}
| WORLDBEGIN
{
    RenoWorldBegin();
}
| WORLDEND
{
    RenoWorldEnd();
}
;
