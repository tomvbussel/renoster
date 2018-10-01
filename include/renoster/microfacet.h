#ifndef RENOSTER_MICROFACET_H_
#define RENOSTER_MICROFACET_H_

#include "renoster/point.h"
#include "renoster/vector.h"

namespace renoster {

class Microfacet {
  public:
    enum class Distribution {
        kTrowbridgeReitz,
        kBeckmann
    };

    Microfacet(Distribution distrib, float alpha)
        : _distrib(distrib), _alphaX(alpha), _alphaY(alpha) {}

    Microfacet(Distribution distrib, float alphaX, float alphaY)
        : _distrib(distrib), _alphaX(alphaX), _alphaY(alphaY) {}

    Vector3f Sample(const Point2f &u) const;

    float Pdf(const Vector3f & wm) const;

    float Lambda(const Vector3f & wo) const;

    float D(const Vector3f & wm) const;

    float G1(const Vector3f & wo) const {
        return 1.f / (1.f + Lambda(wo));
    }

    float G2(const Vector3f & wo, const Vector3f & wi) const {
        return 1.f / (1.f + Lambda(wo) + Lambda(wi));
    }

  private:
    Distribution _distrib;
    float _alphaX, _alphaY;   
};

}  // namespace renoster

#endif  // RENOSTER_MICROFACET_H_
