#ifndef RENOSTER_GEOMETRY_H_
#define RENOSTER_GEOMETRY_H_

#include <memory>

#include "renoster/bounds.h"
#include "renoster/export.h"
#include "renoster/paramlist.h"
#include "renoster/position.h"
#include "renoster/ray.h"
#include "renoster/sampler.h"
#include "renoster/shading.h"
#include "renoster/transform.h"

namespace renoster {

/// GeometryContext
class RENO_API GeometryContext {
public:
    GeometryContext()
        : WorldToObject(Identity()), ObjectToWorld(Identity()) {}

    GeometryContext(const Transform & WorldToObject,
                    const Transform & ObjectToWorld)
        : WorldToObject(WorldToObject), ObjectToWorld(ObjectToWorld) {}

    Transform WorldToObject;
    Transform ObjectToWorld;
};

/// Geometry
class RENO_API Geometry {
public:
    virtual ~Geometry() = default;

    /// Calculate the intersection point of a ray and the geometry
    virtual bool Intersect(const GeometryContext & ctx, const Ray3f & ray,
                           ShadingPoint * sp) const = 0;

    /// Test if a ray intersects the geometry
    virtual bool Occluded(const GeometryContext & ctx, const Ray3f & ray) const;


    /// Compute the shading info at sampled or intersect point
    virtual void ComputeShadingInfo(const GeometryContext & ctx,
                                    ShadingPoint * sp) const = 0;

    /// Sample a point on the surface
    /// The unit of the pdf is m^-2
    virtual ShadingPoint Sample(const GeometryContext & ctx, Sampler & sampler,
                                float * pdf) const = 0;

    /// Calculate the pdf of sampling a point on the surface
    /// The unit of the pdf is m^-2
    virtual float Pdf(const GeometryContext & ctx,
                      const ShadingPoint & sp) const = 0;

    /// Sample a point on the surface with respect to a reference point
    /// The unit of the pdf is m^-2
    virtual ShadingPoint Sample(const GeometryContext & ctx, Sampler & sampler,
                                const ShadingPoint & ref, float * pdf) const;

    /// Calculate the pdf of sampling a point with respect to a ref point
    /// The unit of the pdf is m^-2
    virtual float Pdf(const GeometryContext & ctx, const ShadingPoint & ref,
                      const ShadingPoint & pos) const;

    /// Get a bounding box for the geometry in object space
    virtual Bounds3f GetObjectBounds() const = 0;

    /// Get a bounding box for the geometry in world space
    virtual Bounds3f GetWorldBounds(const GeometryContext & ctx) const;
};

RENO_API std::unique_ptr<Geometry> CreateGeometry(
        const std::string & name, const ParameterList & params);

}  // namespace renoster

#endif  // RENOSTER_GEOMETRY_H_
