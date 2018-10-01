#ifndef RENOSTER_BSDF_H_
#define RENOSTER_BSDF_H

#include "renoster/color.h"
#include "renoster/export.h"
#include "renoster/sampler.h"
#include "renoster/shading.h"

namespace renoster {

class RENO_API BSDF {
public:
    BSDF(const ShadingPoint & sp) : _sp(sp) {}

    virtual Color Evaluate(Sampler & sampler, const Vector3f & wi,
                           float * pdf) const = 0;

    virtual Color Sample(Sampler & sampler, Vector3f * wi,
                         float * pdf) const = 0;

protected:
    const ShadingPoint & _sp;
};

class RENO_API LambertianBSDF : public BSDF {
public:
    LambertianBSDF(const ShadingPoint & sp, const Color & refl);

    Color Evaluate(Sampler & sampler, const Vector3f & wi, float * pdf) const;

    Color Sample(Sampler & sampler, Vector3f * wi, float * pdf) const;

private:
    Color _refl;
};

class RENO_API OrenNayarBSDF : public BSDF {
public:
    OrenNayarBSDF(const ShadingPoint & sp, const Color & refl, float sigma);

    Color Evaluate(Sampler & sampler, const Vector3f & wi, float * pdf) const;

    Color Sample(Sampler & sampler, Vector3f * wi, float * pdf) const;

private:
    Color _refl;
    float _sigma;
};

class RENO_API DielectricBSDF : public BSDF {
public:
    DielectricBSDF(const ShadingPoint & sp, const Color & refl,
                   const Color & trans, float eta);

    Color Evaluate(Sampler & sampler, const Vector3f & wi, float * pdf) const;

    Color Sample(Sampler & sampler, Vector3f * wi, float * pdf) const;

private:
    Color _refl;
    Color _trans;
    float _eta;
};

class RENO_API ConductorBSDF : public BSDF {
public:
    ConductorBSDF(const ShadingPoint & sp, const Color & refl,
                  const Color & eta, const Color & k);

    Color Evaluate(Sampler & sampler, const Vector3f & wi, float * pdf) const;

    Color Sample(Sampler & sampler, Vector3f * wi, float * pdf) const;

private:
    Color _refl;
    Color _eta;
    Color _k;
};

class RENO_API RoughDielectricBSDF : public BSDF {
public:
    RoughDielectricBSDF(const ShadingPoint & sp, const Color & refl,
                        const Color & trans, float eta, float alphaX,
                        float alphaY);

    Color Evaluate(Sampler & sampler, const Vector3f & wi, float * pdf) const;

    Color Sample(Sampler & sampler, Vector3f * wi, float * pdf) const;

private:
    Color _refl;
    Color _trans;
    float _eta;
    float _alphaX, _alphaY;
};

class RENO_API RoughConductorBSDF : public BSDF {
public:
    RoughConductorBSDF(const ShadingPoint & sp, const Color & refl,
                       const Color & eta, const Color & k, float alphaX,
                       float alphaY);

    Color Evaluate(Sampler & sampler, const Vector3f & wi, float * pdf) const;

    Color Sample(Sampler & sampler, Vector3f * wi, float * pdf) const;

private:
    Color _refl;
    Color _eta;
    Color _k;
    float _alphaX, _alphaY;
};

} // namespace renoster

#endif // RENOSTER_BSDF_H
