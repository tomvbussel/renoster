#include "renoster/geometry.h"

namespace renoster {

bool Geometry::Occluded(const GeometryContext & ctx, const Ray3f & ray) const
{
    return Intersect(ctx, ray, nullptr);
}

ShadingPoint Geometry::Sample(const GeometryContext & ctx, Sampler & sampler,
                              const ShadingPoint &, float * pdf) const
{
    return Sample(ctx, sampler, pdf);
}

float Geometry::Pdf(const GeometryContext & ctx, const ShadingPoint &,
                    const ShadingPoint & pos) const
{
    return Pdf(ctx, pos);
}

Bounds3f Geometry::GetWorldBounds(const GeometryContext & ctx) const
{
    return ctx.ObjectToWorld(GetObjectBounds());
}

} // namespace renoster
