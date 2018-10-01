#ifndef RENOSTER_MATERIAL_H_
#define RENOSTER_MATERIAL_H_

#include "renoster/bsdf.h"
#include "renoster/shading.h"
#include "renoster/util/allocator.h"

namespace renoster {

class RENO_API Material {
public:
    virtual BSDF * GetBSDF(const ShadingPoint & sp, Allocator & alloc) const = 0;
};

RENO_API std::unique_ptr<Material> CreateMaterial(
        const std::string & name, const ParameterList & params);

}  // namespace renoster

#endif  // RENOSTER_MATERIAL_H_
