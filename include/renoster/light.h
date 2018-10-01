#ifndef RENOSTER_LIGHT_H_
#define RENOSTER_LIGHT_H_

#include <memory>

#include "renoster/color.h"
#include "renoster/geometry.h"
#include "renoster/sampler.h"
#include "renoster/shading.h"
#include "renoster/transform.h"

namespace renoster {

class RENO_API LightContext {
public:
    LightContext() = default;

    LightContext(const Transform & WorldToLight,
                 const Transform & LightToWorld)
        : WorldToLight(WorldToLight),
        LightToWorld(LightToWorld) {}

    Transform WorldToLight;
    Transform LightToWorld;
};

/// Light
class RENO_API Light {
public:
    virtual ~Light() = default;

    /// Sample a position on the light visible from a reference point
    /// Unit of the pdf is m^-2
    virtual Color SampleDirect(const LightContext & ctx, Sampler & sampler,
                               const ShadingPoint & ref, ShadingPoint * pos,
                               float * pdf) const = 0;

    /// Evaluate a position on the light visible from a reference point
    /// Unit of the pdf is m^-2
    virtual Color EvaluateDirect(const LightContext & ctx,
                                 const ShadingPoint & ref,
                                 const ShadingPoint & pos,
                                 float * pdf) const = 0;

    /// Sample a emission on the light and a direction
    /// Unit of the pdf is m^-2 sr^-1
    virtual Color SampleEmission(const LightContext & ctx, Sampler & sampler,
                                 ShadingPoint * sp, float * pdf) const = 0;

    /// Evaluate a emission on the light and a direction
    /// Unit of the pdf is m^-2 sr^-1
    virtual Color EvaluateEmission(const LightContext & ctx,
                                   const ShadingPoint & sp,
                                   float * pdf) const = 0;
};

/// A GeometryLight is a Light attached to some Geometry
class RENO_API GeometryLight : public Light {
public:
    GeometryLight(const std::shared_ptr<Geometry> & geometry)
        : _geometry(geometry) {}

protected:
    std::shared_ptr<Geometry> _geometry;
};

std::unique_ptr<Light> CreateLight(const std::string & name,
                                   const ParameterList & params);

std::unique_ptr<GeometryLight> CreateGeometryLight(
        const std::string & name, const ParameterList & params,
        const std::shared_ptr<Geometry> & geometry);

}  // namespace renoster

#endif  // RENOSTER_LIGHT_H_
