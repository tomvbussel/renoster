#ifndef RENOSTER_MATH_RAY_H_
#define RENOSTER_MATH_RAY_H_

#include "renoster/point.h"
#include "renoster/vector.h"

namespace renoster {

template <size_t D, typename T>
class Ray {
public:
    static constexpr size_t Dim = D;
    using Scalar = T;
    using PointType = Point<Dim, Scalar>;
    using VectorType = Vector<Dim, Scalar>;

    Ray() {}

    Ray(const PointType & o, const VectorType & d, Scalar tMin, Scalar tMax,
        float time)
        : _o(o), _d(d), _tMin(tMin), _tMax(tMax), _time(time) {}

    const PointType & o() const { return _o; }

    PointType & o() { return _o; }

    const VectorType & d() const { return _d; }

    VectorType & d() { return _d; }

    Scalar & tMin() const { return _tMin; }

    Scalar & tMax() const { return _tMax; }

    Scalar time() const { return _time; }

    float & time() { return _time; }

    PointType operator()(Scalar t) const { return _o + t * _d; }

private:
    PointType _o;
    VectorType _d;
    mutable Scalar _tMin;
    mutable Scalar _tMax;
    float _time;
};

using Ray3f = Ray<3, float>;

}  // namespace renoster

#endif  // RENOSTER_MATH_RAY_H_
