#include "renoster/bsdf.h"

#include "renoster/frame.h"
#include "renoster/mathutil.h"
#include "renoster/microfacet.h"
#include "renoster/reflection.h"
#include "renoster/sampling.h"

namespace renoster {

LambertianBSDF::LambertianBSDF(const ShadingPoint & sp, const Color & refl)
    : BSDF(sp),
    _refl(refl)
{
}

Color LambertianBSDF::Evaluate(Sampler &, const Vector3f & wi,
                               float * pdf) const
{
    Frame frame(_sp.ng);
    Vector3f wiLocal = frame.ToLocal(wi);
    *pdf = CosineSampleHemispherePdf(wiLocal);
    return InvPi * _refl * CosTheta(wiLocal);
}

Color LambertianBSDF::Sample(Sampler & sampler, Vector3f * wi,
                             float * pdf) const
{
    Frame frame(_sp.ng);
    Vector3f wiLocal = CosineSampleHemisphere(sampler.Get2D());
    *wi = frame.ToWorld(wiLocal);
    *pdf = CosineSampleHemispherePdf(wiLocal);
    return _refl;
}

OrenNayarBSDF::OrenNayarBSDF(const ShadingPoint & sp, const Color & refl,
                             float sigma)
    : BSDF(sp),
    _refl(refl),
    _sigma(sigma)
{
}

Color OrenNayarBSDF::Evaluate(Sampler &, const Vector3f & wi, float * pdf) const
{
    Frame frame(_sp.ng);
    Vector woLocal = frame.ToLocal(_sp.wo);
    Vector wiLocal = frame.ToLocal(wi);
    
    float sigmaSq = _sigma * _sigma;
    float A = 1.f - 0.5f * sigmaSq / (sigmaSq + 0.33f);
    float B = 0.45f * sigmaSq / (sigmaSq + 0.09f);

    float sinThetaO = SinTheta(woLocal);
    float sinThetaI = SinTheta(wiLocal);

    float cosPhiDiff = 0.f;
    if (sinThetaO > Epsilon && sinThetaI > Epsilon) {
        cosPhiDiff = CosPhi(wiLocal) * CosPhi(woLocal)
                   + SinPhi(wiLocal) * SinPhi(woLocal);
    }
    
    float sinAlpha, tanBeta;
    if (std::abs(CosTheta(wiLocal)) > std::abs(CosTheta(woLocal))) {
        sinAlpha = sinThetaO;
        tanBeta = sinThetaI / std::abs(CosTheta(wiLocal));
    } else {
        sinAlpha = sinThetaI;
        tanBeta = sinThetaO / std::abs(CosTheta(woLocal));
    }
    
    *pdf = CosineSampleHemispherePdf(wiLocal);
    return _refl * InvPi
        * (A + B * std::max(0.f, cosPhiDiff) * sinAlpha * tanBeta);
}

Color OrenNayarBSDF::Sample(Sampler & sampler, Vector3f * wi,
                            float * pdf) const
{
    Frame frame(_sp.ng);
    Vector3f wiLocal = CosineSampleHemisphere(sampler.Get2D());
    *wi = frame.ToWorld(wiLocal);
    return Evaluate(sampler, *wi, pdf);
}

DielectricBSDF::DielectricBSDF(const ShadingPoint & sp, const Color & refl,
                               const Color & trans, float eta)
    : BSDF(sp),
    _refl(refl),
    _trans(trans),
    _eta(eta)
{
}

Color DielectricBSDF::Evaluate(Sampler &, const Vector3f & wi,
                               float * pdf) const
{
    *pdf = 0.f;
    return Color(0.f);
}

Color DielectricBSDF::Sample(Sampler & sampler, Vector3f * wi,
                             float * pdf) const
{
    Frame frame(_sp.ng);
    Vector3f woLocal = frame.ToLocal(_sp.wo);

    float F = FresnelDielectric(woLocal, 1.f, _eta);
    float u = sampler.Get1D();
    if (u < F) {
        *wi = frame.ToWorld(Reflect(woLocal));
        *pdf = 0.f;
        return _refl;
    } else {
        Vector3f wiLocal;
        Refract(woLocal, 1.f, _eta, &wiLocal);
        *wi = frame.ToWorld(wiLocal);
        *pdf = 0.f;
        return _trans;
    }
}

ConductorBSDF::ConductorBSDF(const ShadingPoint & sp, const Color & refl,
                             const Color & eta, const Color & k)
    : BSDF(sp),
    _refl(refl),
    _eta(eta),
    _k(k)
{
}

Color ConductorBSDF::Evaluate(Sampler &, const Vector3f & wi, float * pdf) const
{
    *pdf = 0.f;
    return Color(0.f);
}

Color ConductorBSDF::Sample(Sampler & sampler, Vector3f * wi, float * pdf) const
{
    Frame frame(_sp.ng);
    Vector3f woLocal = frame.ToLocal(_sp.wo);
    float cosThetaI = CosTheta(woLocal);
    if (cosThetaI <= 0.f) {
        *pdf = 0.f;
        *wi = Vector3f(0.f);
        return Color(0.f);
    }
    
    Color F = FresnelConductor(woLocal, Color(1.f), _eta, _k);
    *wi = frame.ToWorld(Reflect(woLocal));
    *pdf = 0.f;
    return F * _refl;
}

RoughDielectricBSDF::RoughDielectricBSDF(const ShadingPoint & sp,
                                         const Color & refl,
                                         const Color & trans,
                                         float eta, float alphaX,
                                         float alphaY)
    : BSDF(sp),
    _refl(refl),
    _trans(trans),
    _eta(eta),
    _alphaX(alphaX),
    _alphaY(alphaY)
{
}

Color RoughDielectricBSDF::Evaluate(Sampler &, const Vector3f & wi,
                                    float * pdf) const
{
    Frame frame(_sp.ng);
    Vector3f woLocal = frame.ToLocal(_sp.wo);
    Vector3f wiLocal = frame.ToLocal(wi);

    Microfacet microfacets(Microfacet::Distribution::kTrowbridgeReitz,
                           _alphaX, _alphaY);

    if (CosTheta(woLocal) * CosTheta(wiLocal) >= 0.f) {
        Vector3f wm = Normalize(woLocal + wiLocal);

        // TODO: use wm as normal
        float F = FresnelDielectric(woLocal, 1.f, _eta);

        *pdf = F * microfacets.Pdf(wm) / (4.f * Dot(woLocal, wm));
        return _refl * F * microfacets.G2(woLocal, wiLocal)
            * microfacets.D(wm) / (4.f * std::abs(CosTheta(woLocal)));
    } else {
        Vector3f wm = Normalize(woLocal + _eta * wiLocal);

        // TODO: use wm as normal
        float F = FresnelDielectric(woLocal, 1.f, _eta);

        float sqrtDenom = Dot(woLocal, wm) + _eta * Dot(wiLocal, wm);
        *pdf = (1.f - F) * microfacets.Pdf(wm) * _eta * _eta
            * std::abs(Dot(wiLocal, wm)) / (sqrtDenom * sqrtDenom);
        return _trans * (1.f - F)
            * microfacets.G2(woLocal, wiLocal) * microfacets.D(wm)
            * std::abs(Dot(woLocal, wm)) * std::abs(Dot(wiLocal, wm))
            / (std::abs(CosTheta(woLocal)) * sqrtDenom * sqrtDenom);
    }
}

Color RoughDielectricBSDF::Sample(Sampler & sampler, Vector3f * wi,
                                  float * pdf) const
{
    Frame frame(_sp.ng);
    Vector3f woLocal = frame.ToLocal(_sp.wo);
    
    Microfacet microfacets(Microfacet::Distribution::kTrowbridgeReitz,
                           _alphaX, _alphaY);

    Vector3f wm = microfacets.Sample(sampler.Get2D());

    // TODO: wm as normal
    float F = FresnelDielectric(woLocal, 1.f, _eta);
    if (sampler.Get1D() <= F) {
        // TODO: wm as normal
        Vector3f wiLocal = Reflect(woLocal);

        if (CosTheta(wiLocal) * CosTheta(woLocal) <= 0.f) {
            *wi = Vector3f(0.f);
            *pdf = 0.f;
            return Color(0.f);
        }

        *wi = frame.ToWorld(wiLocal);
        *pdf = F * microfacets.Pdf(wm) / (4.f * Dot(woLocal, wm));
        return _refl * std::abs(Dot(woLocal, wm))
            * microfacets.G2(wiLocal, woLocal) 
            / (std::abs(CosTheta(woLocal)) * std::abs(CosTheta(wm)));
    } else {
        Vector3f wiLocal;
        // TODO: wm as normal
        Refract(woLocal, 1.f, _eta, &wiLocal);

        *wi = frame.ToWorld(wiLocal);

        float sqrtDenom = Dot(woLocal, wm) + _eta * Dot(wiLocal, wm);
        *pdf = (1.f - F) * microfacets.Pdf(wm) * _eta * _eta
            * std::abs(Dot(wiLocal, wm)) / (sqrtDenom * sqrtDenom);
        return _trans * std::abs(Dot(woLocal, wm))
            * microfacets.G2(wiLocal, woLocal) 
            / (std::abs(CosTheta(woLocal)) * std::abs(CosTheta(wm)));
    }
}

RoughConductorBSDF::RoughConductorBSDF(const ShadingPoint & sp,
                                       const Color & refl,
                                       const Color & eta, const Color & k,
                                       float alphaX, float alphaY)
    : BSDF(sp),
    _refl(refl),
    _eta(eta),
    _k(k),
    _alphaX(alphaX),
    _alphaY(alphaY)
{
}

Color RoughConductorBSDF::Evaluate(Sampler & sampler, const Vector3f & wi,
                                   float * pdf) const
{
    Frame frame(_sp.ng);
    Vector3f woLocal = frame.ToLocal(_sp.wo);
    Vector3f wiLocal = frame.ToLocal(wi);
    
    if (CosTheta(woLocal) <= 0.f || CosTheta(wiLocal) <= 0.f) {
        *pdf = 0.f;
        return Color(0.f);
    }

    Vector3f wm = Normalize(woLocal + wiLocal);
    
    Microfacet microfacets(Microfacet::Distribution::kTrowbridgeReitz,
                           _alphaX, _alphaY);
    float G = microfacets.G2(woLocal, wiLocal);

    *pdf = microfacets.Pdf(wm) * G / (4.f * std::abs(CosTheta(woLocal)));
    return _refl * FresnelConductor(woLocal, Color(1.f), _eta, _k)
        * microfacets.D(wm) * G / (4.f * std::abs(CosTheta(woLocal)));
}

Color RoughConductorBSDF::Sample(Sampler & sampler, Vector3f * wi,
                                 float * pdf) const
{
    Frame frame(_sp.ng);
    Vector3f woLocal = frame.ToLocal(_sp.wo);

    if (CosTheta(woLocal) <= 0.f) {
        *wi = Vector3f(0.f);
        *pdf = 0.f;
        return Color(0.f);
    }

    Microfacet microfacets(Microfacet::Distribution::kTrowbridgeReitz,
                           _alphaX, _alphaY);
    Vector3f wm = microfacets.Sample(sampler.Get2D());

    // TODO: wm
    Vector3f wiLocal = Reflect(woLocal);
    if (CosTheta(wiLocal) <= 0.f) {
        *wi = Vector3f(0.f);
        *pdf = 0.f;
        return Color(0.f);
    }

    *wi = frame.ToWorld(wiLocal);
    *pdf = microfacets.Pdf(wm) / (4.f * Dot(woLocal, wm));
    return _refl
        * FresnelConductor(woLocal, Color(1.f), _eta, _k)
        * microfacets.G2(woLocal, wiLocal) * std::abs(Dot(woLocal, wm))
        / (std::abs(CosTheta(woLocal)) * std::abs(CosTheta(wm)));
}

} // namespace renoster
