#include "renoster/export.h"
#include "renoster/paramlist.h"
#include "renoster/rng.h"
#include "renoster/sampler.h"

#include <random>

namespace renoster {

class IndependentSampler : public Sampler {
public:
    IndependentSampler(int spp, int seed);

    float Get1D();

    Point2f Get2D();

    std::unique_ptr<Sampler> Clone(int seed);

private:
    RNG _rng;
};

IndependentSampler::IndependentSampler(int spp, int seed)
    : Sampler(spp)
{

    _rng.Seed(seed);
}

float IndependentSampler::Get1D()
{
    return _rng.UniformFloat();
}

Point2f IndependentSampler::Get2D()
{
    return Point2f(Get1D(), Get1D());
}

std::unique_ptr<Sampler> IndependentSampler::Clone(int seed)
{
    return std::make_unique<IndependentSampler>(samplesPerPixel_, seed);
}

extern "C"
RENO_EXPORT
Sampler * CreateSampler(ParameterList & params)
{
    int defaultSpp = 1;
    int spp = params.GetInt("spp", &defaultSpp);

    int defaultSeed = 0;
    int seed = params.GetInt("seed", &defaultSeed);

    return new IndependentSampler(spp, seed);
}

}  // namespace renoster
