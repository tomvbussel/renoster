#ifndef RENOSTER_UTIL_PARAMTABLE_H_
#define RENOSTER_UTIL_PARAMTABLE_H_

namespace renoster {

class ParameterInfo
{
public:
    enum class Type : unsigned int
    {
        kInvalid,
        kInteger,
        kFloat,
        kBool,
        kColor,
        kPoint,
        kVector,
        kNormal,
    };

    enum class Access : unsigned int
    {
        kInvalid,
        kInput,
        kOutput
    };

private:
    std::string _name;
    Type _type;
    Access _access;
};

class ParameterTable
{
private:
    std::vector<ParameterInfo> _table;
};

} // namespace renoster

#endif // RENOSTER_UTIL_PARAMTABLE_H_
