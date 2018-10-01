#include "renoster/paramlist.h"

#include <cstdlib>
#include <map>

#include <boost/variant.hpp>

#include "renoster/log.h"

namespace renoster {

class ParameterList::Impl
{
public:
#define DEFINE_PARAM_ACCESSOR_DECLARATIONS(Type, TypeName) \
    void Set##TypeName##s(const std::string & name, std::vector<Type> values); \
    std::vector<Type> Get##TypeName##s(const std::string & name) const; \
    std::vector<Type> Get##TypeName##s(const std::string & name); \
    Type Get##TypeName(const std::string & name, const Type * defValue) const;

    DEFINE_PARAM_ACCESSOR_DECLARATIONS(bool, Bool)
    DEFINE_PARAM_ACCESSOR_DECLARATIONS(int, Int)
    DEFINE_PARAM_ACCESSOR_DECLARATIONS(float, Float)
    DEFINE_PARAM_ACCESSOR_DECLARATIONS(std::string, String)
    DEFINE_PARAM_ACCESSOR_DECLARATIONS(Color, Color)
    DEFINE_PARAM_ACCESSOR_DECLARATIONS(Point2f, Point2f)
    DEFINE_PARAM_ACCESSOR_DECLARATIONS(Point3f, Point3f)
    DEFINE_PARAM_ACCESSOR_DECLARATIONS(Vector2f, Vector2f)
    DEFINE_PARAM_ACCESSOR_DECLARATIONS(Vector3f, Vector3f)
    DEFINE_PARAM_ACCESSOR_DECLARATIONS(Normal3f, Normal3f)

#undef DEFINE_PARAM_ACCESSOR_DECLARATIONS

    void Clear();

private:
    using ParamValues = boost::variant<
        std::vector<bool>,
        std::vector<int>,
        std::vector<float>,
        std::vector<std::string>,
        std::vector<Color>,
        std::vector<Point2f>,
        std::vector<Point3f>,
        std::vector<Vector2f>,
        std::vector<Vector3f>,
        std::vector<Normal3f>>;

    enum class ParamType
    {
        kBool,
        kInt,
        kFloat,
        kString,
        kColor,
        kPoint2f,
        kPoint3f,
        kVector2f,
        kVector3f,
        kNormal3f
    };

    struct Parameter
    {
        Parameter(std::vector<bool> values);
        Parameter(std::vector<int> values);
        Parameter(std::vector<float> values);
        Parameter(std::vector<std::string> values);
        Parameter(std::vector<Color> values);
        Parameter(std::vector<Point2f> values);
        Parameter(std::vector<Point3f> values);
        Parameter(std::vector<Vector2f> values);
        Parameter(std::vector<Vector3f> values);
        Parameter(std::vector<Normal3f> values);

        ParamType type;
        ParamValues values;
    };

    template <typename Type, ParamType paramType>
    void Set(const std::string & name, std::vector<Type> values);

    template <typename Type, ParamType paramType>
    std::vector<Type> Get(const std::string & name) const;

    template <typename Type, ParamType paramType>
    std::vector<Type> Get(const std::string & name);

    template <typename Type, ParamType paramType>
    Type Get(const std::string & name, const Type * defValue) const;

    std::map<std::string, Parameter> _params;
};

template <typename Type, ParameterList::Impl::ParamType paramType>
void ParameterList::Impl::Set(const std::string & name, std::vector<Type> values)
{
    if (_params.find(name) != _params.end())
    {
        // TODO: delete and warn instead ???
        Error("");
        std::exit(0);
    }

    _params.emplace(name, Parameter(std::move(values)));
}

template <typename Type, ParameterList::Impl::ParamType paramType>
std::vector<Type> ParameterList::Impl::Get(const std::string & name) const
{
    auto it = _params.find(name);
    if (it == _params.end())
    {
        return std::vector<Type>();
    }
    if (it->second.type != paramType)
    {
        return std::vector<Type>();
    }
    return boost::get<std::vector<Type>>(it->second.values);
}

template <typename Type, ParameterList::Impl::ParamType paramType>
std::vector<Type> ParameterList::Impl::Get(const std::string & name)
{
    auto it = _params.find(name);
    if (it == _params.end())
    {
        return std::vector<Type>();
    }
    if (it->second.type != paramType)
    {
        return std::vector<Type>();
    }
    return std::move(boost::get<std::vector<Type>>(it->second.values));
}

template <typename Type, ParameterList::Impl::ParamType paramType>
Type ParameterList::Impl::Get(const std::string & name, const Type * defValue) const
{
    auto it = _params.find(name);
    if (it == _params.end())
    {
        if (defValue)
        {
            return *defValue;
        }
        else
        {
            Error("");
            std::exit(0);
        }
    }
    if (it->second.type != paramType)
    {
        if (defValue)
        {
            Warning("");
            return *defValue;
        }
        else
        {
            Error("");
            std::exit(0);
        }
    }

    const auto & values = boost::get<std::vector<Type>>(it->second.values);

    if (values.size() < 1)
    {
        if (defValue)
        {
            Warning("");
            return *defValue;
        }
        else
        {
            Error("");
            std::exit(0);
        }
    }
    else if (values.size() > 1)
    {
        Warning("");
    }

    return values[0];
}

#define DEFINE_PARAM_ACCESSORS(Type, TypeName)                                \
    void ParameterList::Impl::Set##TypeName##s(const std::string & name,      \
            std::vector<Type> values)                                         \
    {                                                                         \
        Set<Type, ParamType::k##TypeName>(name, std::move(values));           \
    }                                                                         \
                                                                              \
    void ParameterList::Set##TypeName##s(const std::string & name,            \
            std::vector<Type> values)                                         \
    {                                                                         \
        _impl->Set##TypeName##s(name, std::move(values));                     \
    }                                                                         \
                                                                              \
    std::vector<Type> ParameterList::Impl::Get##TypeName##s(                  \
            const std::string & name) const                                   \
    {                                                                         \
        return Get<Type, ParamType::k##TypeName>(name);                       \
    }                                                                         \
                                                                              \
    std::vector<Type> ParameterList::Get##TypeName##s(                        \
            const std::string & name) const                                   \
    {                                                                         \
        return _impl->Get##TypeName##s(name);                                 \
    }                                                                         \
                                                                              \
    std::vector<Type> ParameterList::Impl::Get##TypeName##s(                  \
            const std::string & name)                                         \
    {                                                                         \
        return Get<Type, ParamType::k##TypeName>(name);                       \
    }                                                                         \
                                                                              \
    std::vector<Type> ParameterList::Get##TypeName##s(                        \
            const std::string & name)                                         \
    {                                                                         \
        return _impl->Get##TypeName##s(name);                                 \
    }                                                                         \
                                                                              \
    Type ParameterList::Impl::Get##TypeName(const std::string & name,         \
            const Type * defValue) const                                      \
    {                                                                         \
        return Get<Type, ParamType::k##TypeName>(name, defValue);             \
    }                                                                         \
                                                                              \
    Type ParameterList::Get##TypeName(const std::string & name,               \
            const Type * defValue) const                                      \
    {                                                                         \
        return _impl->Get##TypeName(name, defValue);                          \
    }

DEFINE_PARAM_ACCESSORS(bool, Bool)
DEFINE_PARAM_ACCESSORS(int, Int)
DEFINE_PARAM_ACCESSORS(float, Float)
DEFINE_PARAM_ACCESSORS(std::string, String)
DEFINE_PARAM_ACCESSORS(Color, Color)
DEFINE_PARAM_ACCESSORS(Point2f, Point2f)
DEFINE_PARAM_ACCESSORS(Point3f, Point3f)
DEFINE_PARAM_ACCESSORS(Vector2f, Vector2f)
DEFINE_PARAM_ACCESSORS(Vector3f, Vector3f)
DEFINE_PARAM_ACCESSORS(Normal3f, Normal3f)

#undef DEFINE_PARAM_ACCESSORS

void ParameterList::Impl::Clear()
{
    _params.clear();
}

#define DEFINE_PARAM_CONSTRUCTOR(Type, TypeName)                              \
    ParameterList::Impl::Parameter::Parameter(std::vector<Type> values)       \
        : type(ParamType::k##TypeName), values(std::move(values))             \
    {                                                                         \
                                                                              \
    }

DEFINE_PARAM_CONSTRUCTOR(bool, Bool)
DEFINE_PARAM_CONSTRUCTOR(int, Int)
DEFINE_PARAM_CONSTRUCTOR(float, Float)
DEFINE_PARAM_CONSTRUCTOR(std::string, String)
DEFINE_PARAM_CONSTRUCTOR(Color, Color)
DEFINE_PARAM_CONSTRUCTOR(Point2f, Point2f)
DEFINE_PARAM_CONSTRUCTOR(Point3f, Point3f)
DEFINE_PARAM_CONSTRUCTOR(Vector2f, Vector2f)
DEFINE_PARAM_CONSTRUCTOR(Vector3f, Vector3f)
DEFINE_PARAM_CONSTRUCTOR(Normal3f, Normal3f)

#undef DEFINE_PARAM_CONSTRUCTOR

ParameterList::ParameterList()
    : _impl(std::make_unique<Impl>())
{
}

ParameterList::~ParameterList() = default;

ParameterList::ParameterList(const ParameterList & params)
    : _impl(std::make_unique<Impl>(*params._impl))
{
}

ParameterList & ParameterList::operator=(const ParameterList & params)
{
    _impl = std::make_unique<Impl>(*params._impl);
}

void ParameterList::Clear()
{
    _impl->Clear();
}

} // namespace renoster
