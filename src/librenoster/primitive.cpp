#include "renoster/primitive.h"

namespace renoster {

Primitive::~Primitive() = default;

bool Primitive::Intersect(const PrimitiveContext &, const Ray3f &,
                          ShadingPoint *) const
{
    return false;
}

bool Primitive::Occluded(const PrimitiveContext &, const Ray3f &) const
{
    return false;
}

void Primitive::ComputeShadingInfo(const PrimitiveContext &,
                                   ShadingPoint *) const
{
}

void Primitive::ComputeScatteringFunctions(const PrimitiveContext & ctx,
                                           Allocator & alloc,
                                           ShadingPoint * sp) const
{
}

Color Primitive::SampleDirect(const PrimitiveContext &, Sampler &,
                              const ShadingPoint &, ShadingPoint *,
                              float * pdf) const
{
    *pdf = 0.f;
    return Color(0.f);
}

Color Primitive::EvaluateDirect(const PrimitiveContext &, const ShadingPoint &,
                                const ShadingPoint &, float * pdf) const
{
    *pdf = 0.f;
    return Color(0.f);
}

Color Primitive::SampleEmission(const PrimitiveContext &, Sampler &,
                                ShadingPoint *, float * pdf) const
{
    *pdf = 0.f;
    return Color(0.f);
}

Color Primitive::EvaluateEmission(const PrimitiveContext &,
                                  const ShadingPoint &, float * pdf) const
{
    *pdf = 0.f;
    return Color(0.f);
}

Bounds3f Primitive::GetWorldBounds(const PrimitiveContext & ctx) const
{
    return Bounds3f();
}

GeometricPrimitive::GeometricPrimitive(
        const std::shared_ptr<Geometry> & geometry,
        const std::shared_ptr<GeometryLight> & light,
        const std::shared_ptr<Material> & material,
        const Transform & WorldToGeometry,
        const Transform & GeometryToWorld)
    : _geometry(geometry),
    _light(light),
    _material(material),
    _WorldToGeometry(WorldToGeometry),
    _GeometryToWorld(GeometryToWorld)
{
}

bool GeometricPrimitive::Intersect(const PrimitiveContext & pCtx,
                                   const Ray3f & ray, ShadingPoint * sp) const
{
    GeometryContext gCtx(_WorldToGeometry * pCtx.WorldToPrimitive,
                         pCtx.PrimitiveToWorld * _GeometryToWorld);
    if (!_geometry->Intersect(gCtx, ray, sp)) {
        return false;
    }
    sp->primitive = this;
    return true;
}

bool GeometricPrimitive::Occluded(const PrimitiveContext & pCtx,
                                  const Ray3f & ray) const
{
    GeometryContext gCtx(_WorldToGeometry * pCtx.WorldToPrimitive,
                         pCtx.PrimitiveToWorld * _GeometryToWorld);
    return _geometry->Occluded(gCtx, ray);
}

void GeometricPrimitive::ComputeShadingInfo(const PrimitiveContext & pCtx,
                                            ShadingPoint * sp) const
{
    GeometryContext gCtx(_WorldToGeometry * pCtx.WorldToPrimitive,
                         pCtx.PrimitiveToWorld * _GeometryToWorld);
    return _geometry->ComputeShadingInfo(gCtx, sp);
}

void GeometricPrimitive::ComputeScatteringFunctions(const PrimitiveContext & ctx,
                                                    Allocator & alloc,
                                                    ShadingPoint * sp) const
{
    if (_material) {
        sp->bsdf = _material->GetBSDF(*sp, alloc);
    }
}

Color GeometricPrimitive::SampleDirect(const PrimitiveContext & pCtx,
                                      Sampler & sampler,
                                      const ShadingPoint & ref,
                                      ShadingPoint * pos,
                                      float * pdf) const
{
    if (_light) {
        LightContext lCtx(_WorldToGeometry * pCtx.WorldToPrimitive,
                          pCtx.PrimitiveToWorld * _GeometryToWorld);
        pos->primitive = this;
        return _light->SampleDirect(lCtx, sampler, ref, pos, pdf);
    } else {
        *pdf = 0.f;
        return Color(0.f);
    }
}

Color GeometricPrimitive::EvaluateDirect(const PrimitiveContext & pCtx,
                                         const ShadingPoint & ref,
                                         const ShadingPoint & pos,
                                         float * pdf) const
{
    if (_light) {
        LightContext lCtx(_WorldToGeometry * pCtx.WorldToPrimitive,
                          pCtx.PrimitiveToWorld * _GeometryToWorld);
        return _light->EvaluateDirect(lCtx, ref, pos, pdf);
    } else {
        *pdf = 0.f;
        return Color(0.f);
    }
}

Color GeometricPrimitive::SampleEmission(const PrimitiveContext & pCtx,
                                         Sampler & sampler, ShadingPoint * sp,
                                         float * pdf) const
{
    if (_light) {
        LightContext lCtx(_WorldToGeometry * pCtx.WorldToPrimitive,
                          pCtx.PrimitiveToWorld * _GeometryToWorld);
        sp->primitive = this;
        return _light->SampleEmission(lCtx, sampler, sp, pdf);
    } else {
        *pdf = 0.f;
        return Color(0.f);
    }
}

Color GeometricPrimitive::EvaluateEmission(const PrimitiveContext & pCtx,
                                           const ShadingPoint & sp,
                                           float * pdf) const
{
    if (_light) {
        LightContext lCtx(_WorldToGeometry * pCtx.WorldToPrimitive,
                          pCtx.PrimitiveToWorld * _GeometryToWorld);
        return _light->EvaluateEmission(lCtx, sp, pdf);
    } else {
        *pdf = 0.f;
        return Color(0.f);
    }
}

Bounds3f GeometricPrimitive::GetWorldBounds(const PrimitiveContext & pCtx) const
{
    GeometryContext gCtx(_WorldToGeometry * pCtx.WorldToPrimitive,
                         pCtx.PrimitiveToWorld * _GeometryToWorld);
    return _geometry->GetWorldBounds(gCtx);
}

LightPrimitive::LightPrimitive(const std::shared_ptr<Light> & light,
                               const Transform & WorldToLight,
                               const Transform & LightToWorld)
    : _light(light),
    _WorldToLight(WorldToLight),
    _LightToWorld(LightToWorld)
{
}

Color LightPrimitive::SampleDirect(const PrimitiveContext & pCtx,
                                  Sampler & sampler, const ShadingPoint & ref,
                                  ShadingPoint * pos, float * pdf) const
{
    LightContext lCtx(_WorldToLight * pCtx.WorldToPrimitive,
                      pCtx.PrimitiveToWorld * _LightToWorld);
    pos->primitive = this;
    return _light->SampleDirect(lCtx, sampler, ref, pos, pdf);
}

Color LightPrimitive::EvaluateDirect(const PrimitiveContext & pCtx,
                                     const ShadingPoint & ref,
                                     const ShadingPoint & pos,
                                     float * pdf) const
{
    LightContext lCtx(_WorldToLight * pCtx.WorldToPrimitive,
                      pCtx.PrimitiveToWorld * _LightToWorld);
    return _light->EvaluateDirect(lCtx, ref, pos, pdf);
}

Color LightPrimitive::SampleEmission(const PrimitiveContext & pCtx,
                                     Sampler & sampler, ShadingPoint * sp,
                                     float * pdf) const
{
    LightContext lCtx(_WorldToLight * pCtx.WorldToPrimitive,
                      pCtx.PrimitiveToWorld * _LightToWorld);
    sp->primitive = this;
    return _light->SampleEmission(lCtx, sampler, sp, pdf);
}

Color LightPrimitive::EvaluateEmission(const PrimitiveContext & pCtx,
                                       const ShadingPoint & sp,
                                       float * pdf) const
{
    LightContext lCtx(_WorldToLight * pCtx.WorldToPrimitive,
                      pCtx.PrimitiveToWorld * _LightToWorld);
    return _light->EvaluateEmission(lCtx, sp, pdf);
}

TransformedPrimitive::TransformedPrimitive(
        const std::shared_ptr<Primitive> & primitive,
        const Transform & WorldToPrimitive,
        const Transform & PrimitiveToWorld)
    : _primitive(primitive),
    _WorldToPrimitive(WorldToPrimitive),
    _PrimitiveToWorld(PrimitiveToWorld)
{
}

bool TransformedPrimitive::Intersect(const PrimitiveContext & ctx,
                                     const Ray3f & ray,
                                     ShadingPoint * sp) const
{
    PrimitiveContext newCtx(_WorldToPrimitive * ctx.WorldToPrimitive,
                            ctx.PrimitiveToWorld * _PrimitiveToWorld);
    if (!_primitive->Intersect(newCtx, ray, sp)) {
        return false;
    }
    sp->primitive = this;
    return true;
}

bool TransformedPrimitive::Occluded(const PrimitiveContext & ctx,
                                    const Ray3f & ray) const
{
    PrimitiveContext newCtx(_WorldToPrimitive * ctx.WorldToPrimitive,
                            ctx.PrimitiveToWorld * _PrimitiveToWorld);
    return _primitive->Occluded(newCtx, ray);
}

void TransformedPrimitive::ComputeShadingInfo(const PrimitiveContext & ctx,
                                              ShadingPoint * sp) const
{
    PrimitiveContext newCtx(_WorldToPrimitive * ctx.WorldToPrimitive,
                            ctx.PrimitiveToWorld * _PrimitiveToWorld);
    return _primitive->ComputeShadingInfo(newCtx, sp);
}

void TransformedPrimitive::ComputeScatteringFunctions(
        const PrimitiveContext & ctx, Allocator & alloc,
        ShadingPoint * sp) const
{
    PrimitiveContext newCtx(_WorldToPrimitive * ctx.WorldToPrimitive,
                            ctx.PrimitiveToWorld * _PrimitiveToWorld);
    _primitive->ComputeScatteringFunctions(newCtx, alloc, sp);
}

Color TransformedPrimitive::SampleDirect(const PrimitiveContext & ctx,
                                         Sampler & sampler,
                                         const ShadingPoint & ref,
                                         ShadingPoint * pos,
                                         float * pdf) const
{
    PrimitiveContext newCtx(_WorldToPrimitive * ctx.WorldToPrimitive,
                            ctx.PrimitiveToWorld * _PrimitiveToWorld);
    Color L = _primitive->SampleDirect(newCtx, sampler, ref, pos, pdf);
    pos->primitive = this;
    return L;
}

Color TransformedPrimitive::EvaluateDirect(const PrimitiveContext & ctx,
                                           const ShadingPoint & ref,
                                           const ShadingPoint & pos,
                                           float * pdf) const
{
    PrimitiveContext newCtx(_WorldToPrimitive * ctx.WorldToPrimitive,
                            ctx.PrimitiveToWorld * _PrimitiveToWorld);
    return _primitive->EvaluateDirect(newCtx, ref, pos, pdf);
}

Color TransformedPrimitive::SampleEmission(const PrimitiveContext & ctx,
                                           Sampler & sampler,
                                           ShadingPoint * sp,
                                           float * pdf) const
{
    PrimitiveContext newCtx(_WorldToPrimitive * ctx.WorldToPrimitive,
                            ctx.PrimitiveToWorld * _PrimitiveToWorld);
    Color L = _primitive->SampleEmission(ctx, sampler, sp, pdf);
    sp->primitive = this;
    return L;
}

Color TransformedPrimitive::EvaluateEmission(const PrimitiveContext & ctx,
                                             const ShadingPoint & sp,
                                             float * pdf) const
{
    PrimitiveContext newCtx(_WorldToPrimitive * ctx.WorldToPrimitive,
                            ctx.PrimitiveToWorld * _PrimitiveToWorld);
    return _primitive->EvaluateEmission(ctx, sp, pdf);
}

Bounds3f TransformedPrimitive::GetWorldBounds(const PrimitiveContext & ctx) const
{
    PrimitiveContext newCtx(_WorldToPrimitive * ctx.WorldToPrimitive,
                            ctx.PrimitiveToWorld * _PrimitiveToWorld);
    return _primitive->GetWorldBounds(newCtx);
}

}  // namespace renoster
