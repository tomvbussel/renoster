#include "renoster/integrator.h"
#include "renoster/sampling.h"
#include "renoster/frame.h"

namespace renoster {

class NormalIntegrator : public Integrator {
public:
    NormalIntegrator() {}

    void Integrate(IntegratorContext & ctx, const Ray3f & ray,
                   FilmAccumulator * accum) const;
};

void NormalIntegrator::Integrate(IntegratorContext & ctx, const Ray3f & ray,
                                 FilmAccumulator * accum) const
{
    ShadingPoint sp;
    if (!ctx.scene.Intersect(ray, &sp)) {
        return;
    }

    accum->WriteValue(Color(std::abs(sp.ng.x()),
                            std::abs(sp.ng.y()),
                            std::abs(sp.ng.z())));
}

extern "C"
RENO_EXPORT
Integrator * CreateIntegrator(ParameterList &) {
    return new NormalIntegrator();
}

} // namespace renoster
