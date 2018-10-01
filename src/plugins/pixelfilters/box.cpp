#include "renoster/export.h"
#include "renoster/paramlist.h"
#include "renoster/pixelfilter.h"

namespace renoster {

class BoxFilter : public PixelFilter
{
public:
    BoxFilter(const Vector2f & radius);

    float Evaluate(const Point2f & p) const;

    Vector2f GetRadius() const;

private:
    Vector2f _radius;
};

BoxFilter::BoxFilter(const Vector2f & radius)
    : _radius(radius)
{
}

float BoxFilter::Evaluate(const Point2f & p) const
{
    if (std::abs(p.x()) <= _radius.x() && std::abs(p.y()) <= _radius.y())
    {
        return 1.f;
    }

    return 0.f;
}

Vector2f BoxFilter::GetRadius() const
{
    return _radius;
}

extern "C"
RENO_EXPORT
PixelFilter * CreatePixelFilter(ParameterList & params)
{
    Vector2f defaultRadius = Vector2f(1.f);
    Vector2f radius = params.GetVector2f("radius", &defaultRadius);
    return new BoxFilter(radius);
}

} // namespace renoster
