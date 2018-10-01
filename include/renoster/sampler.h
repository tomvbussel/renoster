#ifndef RENOSTER_SAMPLER_H_
#define RENOSTER_SAMPLER_H_

#include <iostream>
#include <memory>

#include "renoster/export.h"
#include "renoster/paramlist.h"
#include "renoster/point.h"

namespace renoster {

class RENO_API Sampler {
public:
    Sampler(int spp) : samplesPerPixel_(spp) {}

    virtual float Get1D() = 0;

    virtual Point2f Get2D() = 0;

    virtual void StartPixel(const Point2i & pixel) {
        currentPixel_ = pixel;
        currentSample_ = 0;
    }

    virtual bool StartNextSample() {
        return currentSample_++ < samplesPerPixel_;
    }

    virtual std::unique_ptr<Sampler> Clone(int seed) = 0;

protected:
    int samplesPerPixel_;
    Point2i currentPixel_;
    int currentSample_;
};

RENO_API std::unique_ptr<Sampler> CreateSampler(const std::string & name,
                                                ParameterList & params);

}  // namespace renoster

#endif  // RENOSTER_SAMPLER_H_
