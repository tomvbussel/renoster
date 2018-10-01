#ifndef RENOSTER_INTEGRATOR_H_
#define RENOSTER_INTEGRATOR_H_

#include <memory>

#include "renoster/filmaccumulator.h"
#include "renoster/ray.h"
#include "renoster/sampler.h"
#include "renoster/scene.h"
#include "renoster/util/allocator.h"

namespace renoster {

class IntegratorContext {
public:
    IntegratorContext(const Scene & scene, Sampler & sampler, Allocator & alloc)
        : scene(scene), sampler(sampler), alloc(alloc) {}

    const Scene & scene;
    Sampler & sampler;
    Allocator & alloc;
};

class Integrator {
public:
    virtual void Integrate(IntegratorContext & ctx, const Ray3f & ray,
                           FilmAccumulator * accum) const = 0;
};

RENO_API std::unique_ptr<Integrator> CreateIntegrator(const std::string & name,
                                                      ParameterList & params);

}  // namespace renoster

#endif  // RENOSTER_INTEGRATOR_H_
