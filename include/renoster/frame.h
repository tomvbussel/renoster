#ifndef RENOSTER_FRAME_H_
#define RENOSTER_FRAME_H_

#include <cassert>
#include <cmath>

#include "renoster/mathutil.h"
#include "renoster/normal.h"
#include "renoster/vector.h"

namespace renoster {

class Frame {
public:
    Frame(const Normal3f & n) : n(n) {
        float sign = std::copysignf(1.f, n.z());
        float a = -1.f / (sign + n.z());
        float b = n.x() * n.y() * a;
        s = Vector3f(1.f + sign * n.x() * n.x() * a, sign * b, -sign * n.x());
        t = Vector3f(b, sign + n.y() * n.y() * a, -n.y());
    }

    Frame(const Normal3f & n, const Vector3f & u) : n(n), t(u) {
        s = Cross(n, t);
        t = Cross(s, n);
    }

    Vector3f ToWorld(const Vector3f & v) {
        return v.x() * s + v.y() * t + v.z() * n;
    }

    Vector3f ToLocal(const Vector3f & v) {
        return Vector3f(Dot(v, s), Dot(v, t), Dot(v, n));
    }

    Normal3f n;
    Vector3f s, t;
};

inline float CosThetaSq(const Vector3f & v)
{
    return v.z() * v.z();
}

inline float CosTheta(const Vector3f & v)
{
    return v.z();
}

inline float SinThetaSq(const Vector3f & v)
{
    return 1.f - CosThetaSq(v);
}

inline float SinTheta(const Vector3f & v)
{
    float sinThetaSq = SinThetaSq(v);
    if (sinThetaSq <= 0.f) {
        return 0.f;
    }
    return std::sqrt(sinThetaSq);
}

inline float TanThetaSq(const Vector3f & v)
{
    return SinThetaSq(v) / CosThetaSq(v);
}

inline float TanTheta(const Vector3f & v)
{
    return SinTheta(v) / CosTheta(v);
}

inline float CosPhi(const Vector3f & v)
{
    float sinTheta = SinTheta(v);
    if (sinTheta == 0.f) {
        return 1.f;
    }
    return Clamp(v.x() / sinTheta, -1.f, 1.f);
}

inline float CosPhiSq(const Vector3f & v)
{
    float sinThetaSq = SinTheta(v);
    if (sinThetaSq == 0.f) {
        return 1.f;
    }
    return Clamp(v.x() * v.x() / sinThetaSq, 0.f, 1.f);
}

inline float SinPhi(const Vector3f & v)
{
    float sinTheta = SinTheta(v);
    if (sinTheta == 0.f) {
        return 0.f;
    }
    return Clamp(v.y() / sinTheta, -1.f, 1.f);
}

inline float SinPhiSq(const Vector3f & v)
{
    float sinThetaSq = SinTheta(v);
    if (sinThetaSq == 0.f) {
        return 0.f;
    }
    return Clamp(v.y() * v.y() / sinThetaSq, 0.f, 1.f);
}

inline float TanPhi(const Vector3f & v)
{
    return SinPhi(v) / CosPhi(v);
}

inline float TanPhiSq(const Vector3f & v)
{
    return SinPhiSq(v) / CosPhiSq(v);
}

} // namespace renoster

#endif // RENOSTER_FRAME_H_
