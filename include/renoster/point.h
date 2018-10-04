#ifndef RENOSTER_MATH_POINT_H_
#define RENOSTER_MATH_POINT_H_

#include <array>

#include "renoster/types.h"
#include "renoster/vector.h"

namespace renoster {

template <size_t D, typename T>
class Normal;

template <size_t D, typename T>
class Point {
public:
    static constexpr size_t Dim = D;
    using Scalar = T;
    using PointType = Point<D, T>;
    using VectorType = Vector<D, T>;
    using NormalType = Normal<D, T>;

    Point() : _p{} {}

    explicit Point(Scalar a) {
        for (size_t i = 0; i < Dim; ++i) {
            _p[i] = a;
        }
    }

    template <typename... Ts>
    Point(Scalar a, Scalar b, Ts... ts) : _p({{a, b, ts...}}) {}

    Point(const Point &) = default;
    Point & operator=(const Point &) = default;

    Point(Point &&) = default;
    Point & operator=(Point &&) = default;

    template <typename S>
    explicit Point(const Point<D, S> & p) {
        for (size_t d = 0; d < Dim; ++d) {
            _p[d] = p[d];
        }
    }

    template <typename S>
    Point & operator=(const Point<D, S> & p) {
        for (size_t d = 0; d < Dim; ++d) {
            _p[d] = p[d];
        }
        return *this;
    }

    const Scalar & x() const {
        static_assert(Dim >= 1, "Point does not have an x element");
        return _p[0];
    }

    Scalar & x() {
        static_assert(Dim >= 1, "Point does not have an x element");
        return _p[0];
    }

    const Scalar & y() const {
        static_assert(Dim >= 2, "Point does not have a y element");
        return _p[1];
    }

    Scalar & y() {
        static_assert(Dim >= 2, "Point does not have a y element");
        return _p[1];
    }

    const Scalar & z() const {
        static_assert(Dim >= 3, "Point does not have a z element");
        return _p[2];
    }

    Scalar & z() {
        static_assert(Dim >= 3, "Point does not have a z element");
        return _p[2];
    }

    const Scalar & w() const {
        static_assert(Dim >= 4, "Point does not have a w element");
        return _p[3];
    }

    Scalar & w() {
        static_assert(Dim >= 4, "Point does not have a w element");
        return _p[3];
    }

    Scalar & operator[](size_t i) { return _p[i]; }

    Scalar operator[](size_t i) const { return _p[i]; }

    template <typename S>
    Point & operator+=(const Vector<D, S> & rhs) {
        for (size_t d = 0; d < Dim; ++d) {
            _p[d] += rhs[d];
        }
        return *this;
    }

    template <typename S>
    Point & operator+=(const Point<D, S> & rhs) {
        for (size_t d = 0; d < Dim; ++d) {
            _p[d] += rhs[d];
        }
        return *this;
    }

    template <typename S>
    Point & operator-=(const Vector<D, S> & rhs) {
        for (size_t d = 0; d < Dim; ++d) {
            _p[d] -= rhs[d];
        }
        return *this;
    }

    template <typename S>
    Point & operator*=(S c) {
        for (size_t d = 0; d < Dim; ++d) {
            _p[d] *= c;
        }
        return *this;
    }

    template <typename S>
    Point & operator/=(S c) {
        for (size_t d = 0; d < Dim; ++d) {
            _p[d] /= c;
        }
        return *this;
    }

private:
    std::array<Scalar, Dim> _p;
};

using Point1i = Point<1, int>;
using Point1f = Point<1, float>;
using Point2i = Point<2, int>;
using Point2f = Point<2, float>;
using Point3i = Point<3, int>;
using Point3f = Point<3, float>;
using Point4i = Point<4, int>;
using Point4f = Point<4, float>;

template <size_t D, typename T>
inline std::ostream & operator<<(std::ostream & os, const Point<D, T> & p)
{
    os << "[ ";
    for (size_t d = 0; d < D; ++d) {
        os << p[d];
        if (d + 1 < D) {
            os << ", ";
        }
    }
    os << " ]";
    return os;
}

template <size_t D, typename T>
Point<D, T> operator-(Point<D, T> p)
{
    for (size_t d = 0; d < D; ++d) {
        p[d] = -p[d];
    }
    return p;
}

template <size_t D, typename T, typename S>
Point<D, decltype(T() + S())> operator+(const Point<D, T> & lhs,
                                        const Point<D, S> & rhs)
{
    Point<D, decltype(T() + S())> result;
    for (size_t d = 0; d < D; ++d) {
        result[d] = lhs[d] + rhs[d];
    }
    return result;
}

template <size_t D, typename T, typename S>
Point<D, decltype(T() + S())> operator+(const Point<D, T> & lhs,
                                        const Vector<D, S> & rhs)
{
    Point<D, decltype(T() + S())> result;
    for (size_t d = 0; d < D; ++d) {
        result[d] = lhs[d] + rhs[d];
    }
    return result;
}

template <size_t D, typename T, typename S>
Vector<D, decltype(T() - S())> operator-(const Point<D, T> & lhs,
                                         const Point<D, S> & rhs)
{
    Vector<D, decltype(T() - S())> result;
    for (size_t d = 0; d < D; ++d) {
        result[d] = lhs[d] - rhs[d];
    }
    return result;
}

template <size_t D, typename T, typename S>
Point<D, decltype(T() - S())> operator-(const Point<D, T> & lhs,
                                        const Vector<D, S> & rhs)
{
    Point<D, decltype(T() - S())> result;
    for (size_t d = 0; d < D; ++d) {
        result[d] = lhs[d] - rhs[d];
    }
    return result;
}

template <size_t D, typename T, typename S>
Point<D, decltype(T() * S())> operator*(const Point<D, T> & p, S c)
{
    Point<D, decltype(T() * S())> result;
    for (size_t d = 0; d < D; ++d) {
        result[d] = p[d] * c;
    }
    return result;
}

template <size_t D, typename T, typename S>
Point<D, decltype(T() * S())> operator*(T c, const Point<D, S> & p)
{
    Point<D, decltype(T() * S())> result;
    for (size_t d = 0; d < D; ++d) {
        result[d] = c * p[d];
    }
    return result;
}

template <size_t D, typename T, typename S>
Point<D, decltype(T() / S())> operator/(const Point<D, T> & p, S c)
{
    Point<D, decltype(T() / S())> result;
    for (size_t d = 0; d < D; ++d) {
        result[d] = p[d] / c;
    }
    return result;
}

template <size_t D, typename T, typename S>
Point<D, remove_cvref_t<decltype(std::min(T(), S()))>> Min(
        const Point<D, T> & lhs, const Point<D, S> & rhs)
{
    using R = remove_cvref_t<decltype(std::min(T(), S()))>;

    Point<D, R> result;
    for (size_t d = 0; d < D; ++d) {
        result[d] = std::min(lhs[d], rhs[d]);
    }
    return result;
}

template <size_t D, typename T, typename S>
Point<D, remove_cvref_t<decltype(std::max(T(), S()))>> Max(
        const Point<D, T> & lhs, const Point<D, S> & rhs)
{
    using R = remove_cvref_t<decltype(std::max(T(), S()))>;

    Point<D, R> result;
    for (size_t d = 0; d < D; ++d) {
        result[d] = std::max(lhs[d], rhs[d]);
    }
    return result;
}

template <size_t D, typename T>
Point<D, decltype(std::floor(T()))> Floor(const Point<D, T> & p)
{
    Point<D, decltype(std::floor(T()))> result;
    for (size_t d = 0; d < D; ++d) {
        result[d] = std::floor(p[d]);
    }
    return result;
}

template <size_t D, typename T>
Point<D, decltype(std::ceil(T()))> Ceil(const Point<D, T> & p)
{
    Point<D, decltype(std::ceil(T()))> result;
    for (size_t d = 0; d < D; ++d) {
        result[d] = std::ceil(p[d]);
    }
    return result;
}

template <size_t D, typename T, typename S>
bool operator==(const Point<D, T> & lhs, const Point<D, S> & rhs)
{
    for (size_t d = 0; d < D; ++d) {
        if (lhs[d] != rhs[d]) {
            return false;
        }
    }
    return true;
}

template <size_t D, typename T, typename S>
bool operator!=(const Point<D, T> & lhs, const Point<D, S> & rhs)
{
    for (size_t d = 0; d < D; ++d) {
        if (lhs[d] == rhs[d]) {
            return false;
        }
    }
    return true;
}

// TODO: move this somewhere better
template <typename T>
T Lerp(T v1, T v2, T t)
{
    return (T(1) - t) * v1 + t * v2;
}

template <size_t D, typename T>
Point<D, T> Lerp(const Point<D, T> & p1, const Point<D, T> & p2, T t)
{
    Point<D, T> result;
    for (size_t d = 0; d < D; ++d) {
        result[d] = Lerp(p1[d], p2[d], t);
    }
    return result;
}

template <size_t D, typename T, typename S>
auto Distance(const Point<D, T> & p1, const Point<D, S> & p2)
{
    return (p2 - p1).Length();
}

template <size_t D, typename T, typename S>
auto DistanceSquared(const Point<D, T> & p1, const Point<D, S> & p2)
{
    return (p2 - p1).LengthSquared();
}

}  // namespace renoster

#endif  // RENOSTER_MATH_POINT_H_
