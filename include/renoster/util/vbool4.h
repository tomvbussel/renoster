#ifndef RENOSTER_UTIL_VBOOL4_H_
#define RENOSTER_UTIL_VBOOL4_H_

#include <immintrin.h>

namespace renoster {

const __m128 mm_lookupmask_ps[16] = {
    _mm_castsi128_ps(_mm_set_epi32( 0,  0,  0,  0)),
    _mm_castsi128_ps(_mm_set_epi32( 0,  0,  0, -1)),
    _mm_castsi128_ps(_mm_set_epi32( 0,  0, -1,  0)),
    _mm_castsi128_ps(_mm_set_epi32( 0,  0, -1, -1)),
    _mm_castsi128_ps(_mm_set_epi32( 0, -1,  0,  0)),
    _mm_castsi128_ps(_mm_set_epi32( 0, -1,  0, -1)),
    _mm_castsi128_ps(_mm_set_epi32( 0, -1, -1,  0)),
    _mm_castsi128_ps(_mm_set_epi32( 0, -1, -1, -1)),
    _mm_castsi128_ps(_mm_set_epi32(-1,  0,  0,  0)),
    _mm_castsi128_ps(_mm_set_epi32(-1,  0,  0, -1)),
    _mm_castsi128_ps(_mm_set_epi32(-1,  0, -1,  0)),
    _mm_castsi128_ps(_mm_set_epi32(-1,  0, -1, -1)),
    _mm_castsi128_ps(_mm_set_epi32(-1, -1,  0,  0)),
    _mm_castsi128_ps(_mm_set_epi32(-1, -1,  0, -1)),
    _mm_castsi128_ps(_mm_set_epi32(-1, -1, -1,  0)),
    _mm_castsi128_ps(_mm_set_epi32(-1, -1, -1, -1))
};

struct vbool4
{
    union
    {
        __m128 v;
        int i[4];
    };

    vbool4()
    {
    }

    vbool4(const vbool4 & b)
        : v(b.v)
    {
    }

    vbool4(const __m128 v)
        : v(v)
    {
    }

    vbool4(bool b)
        : v(mm_lookupmask_ps[(size_t(b) << 3) | (size_t(b) << 2) |
                             (size_t(b) << 1) | size_t(b)])
    {
    }

    operator const __m128 & () const
    {
        return v;
    }

    operator const __m128i() const
    {
        return _mm_castps_si128(v);
    }

    operator const __m128d() const
    {
        return _mm_castps_pd(v);
    }

    vbool4 & operator=(const vbool4 & b)
    {
        v = b.v;
        return *this;
    }

    vbool4 & operator&=(const vbool4 & b)
    {
        v = _mm_and_ps(v, b.v);
        return *this;
    }

    vbool4 & operator|=(const vbool4 & b)
    {
        v = _mm_or_ps(v, b.v);
        return *this;
    }

    vbool4 & operator^=(const vbool4 &b)
    {
        v = _mm_xor_ps(v, b.v);
        return *this;
    }

    bool operator[](size_t index) const
    {
        return i[index];
    }
};

inline vbool4 operator&(vbool4 lhs, const vbool4 & rhs)
{
    return lhs &= rhs;
}

inline vbool4 operator|(vbool4 lhs, const vbool4 & rhs)
{
    return lhs |= rhs;
}

inline vbool4 operator^(vbool4 lhs, const vbool4 & rhs)
{
    return lhs ^= rhs;
}

inline vbool4 operator==(const vbool4 & lhs, const vbool4 & rhs)
{
    return _mm_castsi128_ps(_mm_cmpeq_epi32(lhs, rhs));
}

inline vbool4 operator!=(const vbool4 & lhs, const vbool4 & rhs)
{
    return _mm_xor_ps(lhs, rhs);
}

inline size_t MoveMask(const vbool4 & b)
{
    return _mm_movemask_ps(b);
}

} // namespace renoster;

#endif // RENOSTER_UTIL_VBOOL4_H_
