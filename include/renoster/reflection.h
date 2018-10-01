#ifndef RENOSTER_REFLECTION_H_
#define RENOSTER_REFLECTION_H_

#include <cmath>

#include "renoster/color.h"
#include "renoster/frame.h"

namespace renoster {

inline Vector3f Reflect(const Vector3f & w)
{
    return Vector3f(-w.x(), -w.y(), w.z());
}

inline bool Refract(const Vector3f & wi, float etaI, float etaT, Vector3f * wt)
{
    float eta = etaI / etaT;

    float cosThetaI = CosTheta(wi);
    float sinThetaISq = SinThetaSq(wi);
    float sinThetaTSq = eta * eta * sinThetaISq;

    if (sinThetaTSq >= 1.f) {
        return false;
    }

    float cosThetaTSq = 1.f - sinThetaTSq;
    float cosThetaT = std::sqrt(cosThetaTSq);

    *wt = Vector3f(-eta * wi.x(), -eta * wi.y(), cosThetaT);

    return true;
}

inline float FresnelDielectric(Vector3f & wi, float etaI, float etaT)
{
    Vector3f wt;
    if (!Refract(wi, etaI, etaT, &wt)) {
        return 1.f;
    }

    float cosThetaI = CosTheta(wi);
    float cosThetaT = CosTheta(wt);

    float Rparl = (etaT * cosThetaI - etaI * cosThetaT)
                / (etaT * cosThetaI + etaI * cosThetaT);
    float Rperp = (etaI * cosThetaI - etaT * cosThetaT)
                / (etaI * cosThetaI + etaT * cosThetaT);

    return 0.5f * (Rparl * Rparl + Rperp * Rperp);
}

inline Color FresnelConductor(const Vector3f & wi, const Color & etaI,
                              const Color & etaT, const Color & kT)
{
    Color eta = etaT / etaI;
    Color etaSq = eta * eta;
    Color k = kT / etaI;
    Color kSq = k * k;

    float cosThetaI = CosTheta(wi);
    float cosThetaISq = cosThetaI * cosThetaI;
    float sinThetaISq = 1.f - cosThetaISq;

    Color t0 = etaSq - kSq - Color(sinThetaISq);
    Color aSqplusbSq = Sqrt(t0 * t0 + 4.f * etaSq * kSq);
    Color a = Sqrt(0.5f * (aSqplusbSq + t0));

    Color t1 = aSqplusbSq + Color(cosThetaISq);
    Color t2 = 2.f * a * cosThetaI;
    Color rhoPerp = (t1 - t2) / (t1 + t2);

    Color t3 = cosThetaISq * aSqplusbSq + Color(sinThetaISq * sinThetaISq);
    Color t4 = t2 * sinThetaISq;
    Color rhoParl = rhoPerp * (t3 - t4) / (t3 + t4);

    return 0.5f * (rhoPerp + rhoParl);
}

} // namespace renoster

#endif // RENOSTER_REFLECTION_H_
