#ifndef RENOSTER_MATHUTIL_H_
#define RENOSTER_MATHUTIL_H_

#include <algorithm>
#include <cmath>
#include <limits>

namespace renoster {

// clang-format off
static constexpr float Pi        =  3.141592654f;
static constexpr float TwoPi     =  6.283185307f;
static constexpr float FourPi    = 12.566370614f;
static constexpr float InvPi     =  0.318309886f;
static constexpr float InvTwoPi  =  0.159154943f;
static constexpr float InvFourPi =  0.079477472f;
static constexpr float PiDivTwo  =  1.570796327f;
static constexpr float PiDivFour =  0.785398163f;
static constexpr float SqrtPi    =  1.772453851f;
static constexpr float InvSqrtPi =  0.564189584f;
static constexpr float DegToRad  =  0.017453293f;
static constexpr float RadToDeg  = 57.295779513f;
static constexpr float Infinity  = std::numeric_limits<float>::infinity();
static constexpr float Epsilon   =  1e-9f;
// clang-format on

template <typename T>
inline T Clamp(T value, T min, T max) {
    return std::min(std::max(value, min), max);
};

inline bool SolveQuadratic(float a, float b, float c, float * t0, float * t1) {
    float discrim = b * b - 4.f * a * c;

    if (discrim < 0.f) {
        return false;
    }

    float sqrtDiscrim = std::sqrt(discrim);

    float q;
    if (b < 0.f) {
        q = -0.5f * (b - sqrtDiscrim);
    } else {
        q = -0.5f * (b + sqrtDiscrim);
    }

    *t0 = q / a;
    *t1 = c / q;

    if (*t0 > *t1) {
        std::swap(*t0, *t1);
    }

    return true;
}

}  // namespace renoster

#endif  // RENOSTER_MATHUTIL_H_
