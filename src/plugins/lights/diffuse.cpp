#include "renoster/light.h"
#include "renoster/frame.h"
#include "renoster/mathutil.h"
#include "renoster/sampling.h"

namespace renoster {

class DiffuseGeometryLight : public GeometryLight {
public:
    DiffuseGeometryLight(const std::shared_ptr<Geometry> & geometry,
                         const Color & radiance, bool twoSided);

    Color SampleDirect(const LightContext & ctx, Sampler & sampler,
                       const ShadingPoint & ref, ShadingPoint * pos,
                       float * pdf) const;

    Color EvaluateDirect(const LightContext & ctx, const ShadingPoint & ref,
                         const ShadingPoint & pos, float * pdf) const;

    Color SampleEmission(const LightContext & ctx, Sampler & sampler,
                         ShadingPoint * sp, float * pdf) const;

    Color EvaluateEmission(const LightContext & ctx, const ShadingPoint & sp,
                           float * pdf) const;

private:
    Color _radiance;
    bool _twoSided;
};

DiffuseGeometryLight::DiffuseGeometryLight(
        const std::shared_ptr<Geometry> & geometry, const Color & radiance,
        bool twoSided)
    : GeometryLight(geometry),
    _radiance(radiance),
    _twoSided(twoSided)
{
}

Color DiffuseGeometryLight::SampleDirect(const LightContext & lCtx,
                                         Sampler & sampler,
                                         const ShadingPoint & ref,
                                         ShadingPoint * pos, float * pdf) const
{
    GeometryContext gCtx(lCtx.WorldToLight, lCtx.LightToWorld);
    *pos = _geometry->Sample(gCtx, sampler, ref, pdf);
    Vector3f wi = Normalize(pos->p - ref.p);
    return (_twoSided || Dot(pos->ng, wi) < 0.f) ? _radiance : Color(0.f);
}

Color DiffuseGeometryLight::EvaluateDirect(const LightContext & lCtx,
                                           const ShadingPoint & ref,
                                           const ShadingPoint & pos,
                                           float * pdf) const
{
    GeometryContext gCtx(lCtx.WorldToLight, lCtx.LightToWorld);
    *pdf = _geometry->Pdf(gCtx, ref, pos);
    Vector3f wi = Normalize(pos.p - ref.p);
    return (_twoSided || Dot(pos.ng, wi) < 0.f) ? _radiance : Color(0.f);
}

Color DiffuseGeometryLight::SampleEmission(const LightContext & lCtx,
                                           Sampler & sampler,
                                           ShadingPoint * sp,
                                           float * pdf) const
{
    // Sample position
    float pdfPos;
    GeometryContext gCtx(lCtx.WorldToLight, lCtx.LightToWorld);
    *sp = _geometry->Sample(gCtx, sampler, &pdfPos);

    // Sample direction
    float pdfDir;
    Vector3f woLocal;
    if (_twoSided) {
        Point2f xi = sampler.Get2D();
        bool flip = false;
        if (xi[0] < 0.5f) {
            xi[0] = std::min(2.f * xi[0], 1.f - Epsilon);
            flip = true;
        } else {
            xi[0] = std::min(2.f * (xi[0] - 0.5f), 1.f - Epsilon);
        }
        woLocal = CosineSampleHemisphere(sampler.Get2D());
        pdfDir = 0.5f * CosineSampleHemispherePdf(woLocal);
        woLocal.z() *= -1.f;
    } else {
        woLocal = CosineSampleHemisphere(sampler.Get2D());
        pdfDir = CosineSampleHemispherePdf(woLocal);
    }

    Frame frame(sp->ng);
    sp->wo = frame.ToWorld(woLocal);

    // Calculate resulting pdf (m^-2 sr^-1)
    *pdf = pdfPos * pdfDir;

    return (_twoSided || Dot(sp->ng, sp->wo) > 0.f) ? _radiance : Color(0.f);
}

Color DiffuseGeometryLight::EvaluateEmission(const LightContext &lCtx,
                                             const ShadingPoint & sp,
                                             float * pdf) const
{
    GeometryContext gCtx(lCtx.WorldToLight, lCtx.LightToWorld);
    float pdfPos = _geometry->Pdf(gCtx, sp);

    Frame frame(sp.ng);
    Vector3f woLocal = frame.ToLocal(sp.wo);

    float pdfDir;
    if (_twoSided) {
        woLocal.z() = std::abs(woLocal.z());
        pdfDir = 0.5f * CosineSampleHemispherePdf(woLocal);
    } else {
        pdfDir = CosineSampleHemispherePdf(woLocal);
    }

    *pdf = pdfPos * pdfDir;

    return (_twoSided || Dot(sp.ng, sp.wo) > 0.f) ? _radiance : Color(0.f);
}

extern "C"
RENO_EXPORT
GeometryLight * CreateGeometryLight(const ParameterList & params,
                                    const std::shared_ptr<Geometry> geometry)
{
    Color defRadiance(1.f);
    Color radiance = params.GetColor("L", &defRadiance);

    bool defTwoSided = false;
    bool twoSided = params.GetBool("twosided", &defTwoSided);

    return new DiffuseGeometryLight(geometry, radiance, twoSided);
}

} // namespace renoster
