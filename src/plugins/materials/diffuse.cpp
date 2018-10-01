#include "renoster/material.h"

namespace renoster {

class Diffuse : public Material {
public:
    Diffuse(const Color & refl);

    BSDF * GetBSDF(const ShadingPoint & sp, Allocator & alloc) const;

private:
    Color _refl;
};

Diffuse::Diffuse(const Color & refl)
    : _refl(refl)
{
}

BSDF * Diffuse::GetBSDF(const ShadingPoint & sp, Allocator & alloc) const
{
    return alloc.New<LambertianBSDF>(sp, _refl);
}

extern "C"
RENO_EXPORT
Material * CreateMaterial(const ParameterList & params)
{
    Color dfltRefl(1.f);
    Color refl = params.GetColor("reflectance", &dfltRefl);

    return new Diffuse(refl);
}

}  // namespace renoster
