#ifndef RENOSTER_PRIMITIVE_H_
#define RENOSTER_PRIMITIVE_H_

#include <memory>

#include "renoster/bounds.h"
#include "renoster/camera.h"
#include "renoster/geometry.h"
#include "renoster/light.h"
#include "renoster/material.h"
#include "renoster/ray.h"
#include "renoster/transform.h"
#include "renoster/util/allocator.h"

namespace renoster {

///
class PrimitiveContext {
public:
    PrimitiveContext()
        : WorldToPrimitive(Identity()),
        PrimitiveToWorld(Identity()) {}

    PrimitiveContext(const Transform & WorldToPrimitive,
                     const Transform & PrimitiveToWorld)
        : WorldToPrimitive(WorldToPrimitive),
        PrimitiveToWorld(PrimitiveToWorld) {}

    Transform WorldToPrimitive;
    Transform PrimitiveToWorld;
};

///
class Primitive {
public:
    virtual ~Primitive();

    /// Test whether a ray intersects the primitive,
    /// and compute the intersection point.
    virtual bool Intersect(const PrimitiveContext & ctx, const Ray3f & ray,
                           ShadingPoint * sp) const;

    /// Test whether a ray intersects the primitive
    virtual bool Occluded(const PrimitiveContext & ctx,
                          const Ray3f & ray) const;

    /// Compute the shading information at a position on the primitive
    virtual void ComputeShadingInfo(const PrimitiveContext & ctx,
                                    ShadingPoint * sp) const;

    /// Compute the BSDF at a position on the primitive
    virtual void ComputeScatteringFunctions(const PrimitiveContext & ctx,
                                            Allocator & alloc,
                                            ShadingPoint * sp) const;

    /// Sample a position on the light visible from a reference point
    /// Unit of the pdf is m^-2
    virtual Color SampleDirect(const PrimitiveContext & ctx, Sampler & sampler,
                               const ShadingPoint & ref, ShadingPoint * pos,
                               float * pdf) const;

    /// Evaluate a position on the light visible from a reference point
    /// Unit of the pdf is m^-2
    virtual Color EvaluateDirect(const PrimitiveContext & ctx,
                                 const ShadingPoint & ref,
                                 const ShadingPoint & pos,
                                 float * pdf) const;

    /// Sample a emission on the light and a direction
    /// Unit of the pdf is m^-2 sr^-1
    virtual Color SampleEmission(const PrimitiveContext & ctx,
                                 Sampler & sampler, ShadingPoint * sp,
                                 float * pdf) const;

    /// Evaluate a emission on the light and a direction
    /// Unit of the pdf is m^-2 sr^-1
    virtual Color EvaluateEmission(const PrimitiveContext & ctx,
                                   const ShadingPoint & sp,
                                   float * pdf) const;

    /// Get the bounds of the primitive in world space
    virtual Bounds3f GetWorldBounds(const PrimitiveContext & ctx) const;

    size_t GetLightId() const { return _lightId; }

    size_t SetLightId(size_t lightId) { _lightId = lightId; }

private:
    size_t _lightId = -1;
};

///
class GeometricPrimitive : public Primitive {
public:
    GeometricPrimitive(const std::shared_ptr<Geometry> & geometry,
                       const std::shared_ptr<GeometryLight> & light,
                       const std::shared_ptr<Material> & material,
                       const Transform & WorldToGeometry,
                       const Transform & GeometryToWorld);

    bool Intersect(const PrimitiveContext & ctx, const Ray3f & ray,
                   ShadingPoint * sp) const;

    bool Occluded(const PrimitiveContext & ctx, const Ray3f & ray) const;

    void ComputeShadingInfo(const PrimitiveContext & ctx,
                            ShadingPoint * sp) const;

    void ComputeScatteringFunctions(const PrimitiveContext & ctx,
                                    Allocator & alloc,
                                    ShadingPoint * sp) const;

    Color SampleDirect(const PrimitiveContext & ctx, Sampler & sampler,
                       const ShadingPoint & ref, ShadingPoint * pos,
                       float * pdf) const;

    Color EvaluateDirect(const PrimitiveContext & ctx,
                         const ShadingPoint & ref,
                         const ShadingPoint & pos,
                         float * pdf) const;

    Color SampleEmission(const PrimitiveContext & ctx,
                         Sampler & sampler, ShadingPoint * sp,
                         float * pdf) const;

    Color EvaluateEmission(const PrimitiveContext & ctx,
                           const ShadingPoint & sp,
                           float * pdf) const;

    Bounds3f GetLocalBounds() const;

    Bounds3f GetWorldBounds(const PrimitiveContext & ctx) const;

private:
    std::shared_ptr<Geometry> _geometry;
    std::shared_ptr<GeometryLight> _light;
    std::shared_ptr<Material> _material;
    Transform _WorldToGeometry;
    Transform _GeometryToWorld;
};

///
class LightPrimitive : public Primitive {
public:
    LightPrimitive(const std::shared_ptr<Light> & light,
                   const Transform & WorldToLight,
                   const Transform & LightToWorld);

    Color SampleDirect(const PrimitiveContext & ctx, Sampler & sampler,
                       const ShadingPoint & ref, ShadingPoint * pos,
                       float * pdf) const;

    Color EvaluateDirect(const PrimitiveContext & ctx,
                         const ShadingPoint & ref,
                         const ShadingPoint & pos,
                         float * pdf) const;

    Color SampleEmission(const PrimitiveContext & ctx,
                         Sampler & sampler, ShadingPoint * sp,
                         float * pdf) const;

    Color EvaluateEmission(const PrimitiveContext & ctx,
                           const ShadingPoint & sp,
                           float * pdf) const;

private:
    std::shared_ptr<Light> _light;
    Transform _WorldToLight;
    Transform _LightToWorld;
};

///
class TransformedPrimitive : public Primitive {
public:
    TransformedPrimitive(const std::shared_ptr<Primitive> & primitive,
                         const Transform & CurrentToPrimitive,
                         const Transform & PrimitiveToCurrent);
    
    bool Intersect(const PrimitiveContext & ctx, const Ray3f & ray,
                   ShadingPoint * sp) const;

    bool Occluded(const PrimitiveContext & ctx, const Ray3f & ray) const;

    void ComputeShadingInfo(const PrimitiveContext & ctx,
                             ShadingPoint * sp) const;

    void ComputeScatteringFunctions(const PrimitiveContext & ctx,
                                    Allocator & alloc,
                                    ShadingPoint * sp) const;

    Color SampleDirect(const PrimitiveContext & ctx, Sampler & sampler,
                       const ShadingPoint & ref, ShadingPoint * pos,
                       float * pdf) const;

    Color EvaluateDirect(const PrimitiveContext & ctx,
                         const ShadingPoint & ref,
                         const ShadingPoint & pos,
                         float * pdf) const;

    Color SampleEmission(const PrimitiveContext & ctx,
                         Sampler & sampler, ShadingPoint * sp,
                         float * pdf) const;

    Color EvaluateEmission(const PrimitiveContext & ctx,
                           const ShadingPoint & sp,
                           float * pdf) const;

    Bounds3f GetLocalBounds() const;

    Bounds3f GetWorldBounds(const PrimitiveContext & ctx) const;

private:
    std::shared_ptr<Primitive> _primitive;
    Transform _WorldToPrimitive;
    Transform _PrimitiveToWorld;
};

}  // namespace renoster

#endif  // RENOSTER_PRIMITIVE_H_
