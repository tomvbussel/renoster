#ifndef RENOSTER_UTIL_VFLOAT4_H_
#define RENOSTER_UTIL_VFLOAT4_H_

#include <immintrin.h>

#include "renoster/util/vbool4.h"

namespace renoster {

struct vfloat4 {
    union {
        __m128 v;
        float f[4];
        int i[4];
    };

    vfloat4() {}
     
    constexpr vfloat4(const vfloat4 & f) : v(f.v) {}

    constexpr vfloat4(const __m128 v) : v(v) {}

    vfloat4(float x) : v(_mm_set_ps(x, x, x, x)) {}

    vfloat4(float x, float y, float z, float w)
        : v(_mm_set_ps(w, z, y, x)) {}

    operator const __m128 & () const {
        return v;
    }

    vfloat4 & operator=(const vfloat4 & f) {
        v = f.v;
        return *this;
    }

    vfloat4 operator-() const {
        return _mm_sub_ps(_mm_setzero_ps(), v);
    }

    vfloat4 & operator+=(const vfloat4 & a) {
        v = _mm_add_ps(v, a.v);
        return *this;
    }

    vfloat4 & operator-=(const vfloat4 & a) {
        v = _mm_sub_ps(v, a.v);
        return *this;
    }

    vfloat4 & operator*=(const vfloat4 & a) {
        v = _mm_mul_ps(v, a.v);
        return *this;
    }

    vfloat4 & operator/=(const vfloat4 & a) {
        v = _mm_div_ps(v, a.v);
        return *this;
    }

    float operator[](size_t i) const {
        return f[i];
    }

    float & operator[](size_t i) {
        return f[i];
    }
};

inline vfloat4 operator+(vfloat4 lhs, const vfloat4 & rhs) {
    return lhs += rhs;
}

inline vfloat4 operator-(vfloat4 lhs, const vfloat4 & rhs) {
    return lhs -= rhs;
}

inline vfloat4 operator*(vfloat4 lhs, const vfloat4 & rhs) {
    return lhs *= rhs;
}

inline vfloat4 operator/(vfloat4 lhs, const vfloat4 & rhs) {
    return lhs /= rhs;
}

inline vbool4 operator==(const vfloat4 & lhs, const vfloat4 & rhs) {
    return _mm_cmpeq_ps(lhs, rhs);
}

inline vbool4 operator!=(const vfloat4 & lhs, const vfloat4 & rhs) {
    return _mm_cmpneq_ps(lhs, rhs);
}

inline vbool4 operator<(const vfloat4 & lhs, const vfloat4 & rhs) {
    return _mm_cmplt_ps(lhs, rhs);
}

inline vbool4 operator>=(const vfloat4 & lhs, const vfloat4 & rhs) {
    return _mm_cmpge_ps(lhs, rhs);
}

inline vbool4 operator>(const vfloat4 & lhs, const vfloat4 & rhs) {
    return _mm_cmpgt_ps(lhs, rhs);
}

inline vbool4 operator<=(const vfloat4 & lhs, const vfloat4 & rhs) {
    return _mm_cmple_ps(lhs, rhs);
}

inline vfloat4 Max(const vfloat4 & lhs, const vfloat4 & rhs) {
    return _mm_max_ps(lhs, rhs);
}

inline vfloat4 Min(const vfloat4 & lhs, const vfloat4 & rhs) {
    return _mm_min_ps(lhs, rhs);
}

inline vfloat4 Sqrt(const vfloat4 & f) {
    return _mm_sqrt_ps(f);
}

} // namespace renoster

#endif // RENOSTER_UTIL_FLOAT4_H_
