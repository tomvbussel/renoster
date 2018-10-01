#include "renoster/microfacet.h"

#include "renoster/frame.h"
#include "renoster/mathutil.h"

namespace renoster {

Vector3f Microfacet::Sample(const Point2f & u) const
{
    float x, y;
    switch(_distrib) {
    case Distribution::kBeckmann:
        x = _alphaX * std::sqrt(-std::log(u.x())) * std::cos(TwoPi * u.y());
        y = _alphaY * std::sqrt(-std::log(u.x())) * std::sin(TwoPi * u.y());
        break;

    case Distribution::kTrowbridgeReitz:
        x = _alphaX * std::sqrt(u.x() / (1.f - u.x())) * std::cos(TwoPi * u.y());
        y = _alphaY * std::sqrt(u.x() / (1.f - u.x())) * std::sin(TwoPi * u.y());
        break;
    }   
    return Normalize(Vector3f(-x, -y, 1.f));
}

float Microfacet::Pdf(const Vector3f & wm) const
{
    return D(wm) * std::abs(CosTheta(wm));
}

float Microfacet::Lambda(const Vector3f & wo) const
{
    float tanTheta = TanTheta(wo);
    if (std::isinf(tanTheta))
        return 0.f;

    float cosThetaSq = CosThetaSq(wo);
    float sinThetaSq = SinThetaSq(wo);

    float alpha = std::sqrt(cosThetaSq * _alphaX * _alphaX +
                            sinThetaSq * _alphaY * _alphaY);

    float a = 1.f / (alpha * tanTheta);

    switch(_distrib) {
        case Distribution::kBeckmann: {
            if (a >= 1.6f)
                return 0.f;
            float aSq = a * a;
            return (1.f - 1.259f * a + 0.396f * aSq) / (3.535f * a + 2.181f * aSq);
        }
        case Distribution::kTrowbridgeReitz: {
            return -0.5f + 0.5f * std::sqrt(1.f + 1.f / (a * a));
        }
    }
}

float Microfacet::D(const Vector3f & wm) const
{
    float tanThetaSq = TanThetaSq(wm);
    if (std::isinf(tanThetaSq))
        return 0.f;

    float cosPhiSq = CosPhiSq(wm);
    float sinPhiSq = SinPhiSq(wm);
    float c = cosPhiSq / (_alphaX * _alphaX) + sinPhiSq / (_alphaY * _alphaY);
    float cosThetaSq = CosThetaSq(wm);

    switch(_distrib) {
        case Distribution::kBeckmann: {
            return InvPi * std::exp(-tanThetaSq * c) / 
                (_alphaX * _alphaY * cosThetaSq * cosThetaSq);
        }
        case Distribution::kTrowbridgeReitz: {
            float k = 1.f * tanThetaSq * c;
            return InvPi / (_alphaX * _alphaY * cosThetaSq*cosThetaSq * k*k);
        }
    }
}

} // namespace renoster
