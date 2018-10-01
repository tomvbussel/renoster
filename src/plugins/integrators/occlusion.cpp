#include "renoster/integrator.h"
#include "renoster/sampling.h"
#include "renoster/frame.h"

namespace renoster {

class Occlusion : public Integrator {
public:
    Occlusion(float maxDist, int numSamples)
        : _maxDist(maxDist), _numSamples(numSamples) {}

    void Integrate(IntegratorContext & ctx, const Ray3f & ray,
                   FilmAccumulator * accum) const;

private:
    float _maxDist;
    int _numSamples;
};

void Occlusion::Integrate(IntegratorContext & ctx, const Ray3f & ray,
                          FilmAccumulator * accum) const
{
    ShadingPoint sp;
    if (!ctx.scene.Intersect(ray, &sp)) {
        return;
    }

    for (int i = 0; i < _numSamples; ++i) {
        Point2f u = ctx.sampler.Get2D();
        Vector3f d = CosineSampleHemisphere(u);

        Frame frame(sp.ng);
        Vector3f dir = frame.ToWorld(d);

        Ray3f r(sp.p, dir, 0.01f, _maxDist, ray.time());
        
        if (!ctx.scene.Occluded(r)) {
            accum->AddSample(Color(1.f / _numSamples));
        }
    }
}

extern "C"
RENO_EXPORT
Integrator * CreateIntegrator(ParameterList & params) {
    float defMaxDist = Infinity;
    float maxDist = params.GetFloat("maxdist", &defMaxDist);

    int defNumSamples = 1;
    int numSamples = params.GetInt("numsamples", &defNumSamples);

    return new Occlusion(maxDist, numSamples);
}

} // namespace renoster
