#ifndef RENOSTER_SCENE_H_
#define RENOSTER_SCENE_H_

#include <memory>
#include <vector>

#include "renoster/bvh.h"
#include "renoster/camera.h"
#include "renoster/export.h"
#include "renoster/primitive.h"
#include "renoster/sampling.h"

namespace renoster {

class RENO_API Scene {
public:
    Scene() = default;

    Scene(std::vector<Primitive *> geometries, std::vector<Primitive *> lights);

    bool Intersect(const Ray3f & ray, ShadingPoint * sp) const;

    bool Occluded(const Ray3f & ray) const;

    Color SampleDirect(Sampler & sampler, const ShadingPoint & ref,
                       ShadingPoint * pos, float * pdf) const;

    Color EvaluateDirect(const ShadingPoint & ref, const ShadingPoint & pos,
                         float * pdf) const;

    Color SampleEmission(Sampler & sampler, ShadingPoint * pos,
                         float * pdf) const;

    Color EvaluateEmission(const ShadingPoint & pos, float * pdf) const;

private:
    std::unique_ptr<BVH> _bvh;
    std::vector<Primitive *> _lights;

    Distribution1D _lightDistrib;
};

}  // namespace renoster

#endif  // RENOSTER_SCENE_H_
