#ifndef RENOSTER_VECTOR_H_
#define RENOSTER_VECTOR_H_

#include <array>
#include <cmath>
#include <ostream>

#include "renoster/types.h"

namespace renoster {

template <size_t D, typename T>
class Vector {
public:
    static constexpr size_t Dim = D;
    using Scalar = T;

    Vector() : _v{} {}

    explicit Vector(Scalar a) {
        for (size_t d = 0; d < Dim; ++d) {
            _v[d] = a;
        }
    }

    template <typename... Ts>
    Vector(Scalar a, Scalar b, Ts... ts) : _v({{a, b, ts...}}) {}

    Vector(const Vector &) = default;
    Vector & operator=(const Vector &) = default;

    Vector(Vector &&) = default;
    Vector & operator=(Vector &&) = default;

    template <typename S>
    explicit Vector(const Vector<D, S> & v) {
        for (size_t d = 0; d < Dim; ++d) {
            _v[d] = v[d];
        }
    }

    template <typename S>
    Vector & operator=(const Vector<D, S> & v) {
        for (size_t d = 0; d < Dim; ++d) {
            _v[d] = v[d];
        }
        return *this;
    }

    const Scalar & x() const {
        static_assert(Dim >= 1, "Vector does not have an x element");
        return _v[0];
    }

    Scalar & x() {
        static_assert(Dim >= 1, "Vector does not have an x element");
        return _v[0];
    }

    const Scalar & y() const {
        static_assert(Dim >= 2, "Vector does not have a y element");
        return _v[1];
    }

    Scalar & y() {
        static_assert(Dim >= 2, "Vector does not have a y element");
        return _v[1];
    }

    const Scalar & z() const {
        static_assert(Dim >= 3, "Vector does not have a z element");
        return _v[2];
    }

    Scalar & z() {
        static_assert(Dim >= 3, "Vector does not have a z element");
        return _v[2];
    }

    const Scalar & w() const {
        static_assert(Dim >= 4, "Vector does not have a w element");
        return _v[3];
    }

    Scalar & w() {
        static_assert(Dim >= 4, "Vector does not have a w element");
        return _v[3];
    }

    inline Scalar & operator[](size_t i) { return _v[i]; }

    Scalar operator[](size_t i) const { return _v[i]; }

    template <typename S>
    Vector & operator+=(const Vector<D, S> & rhs) {
        for (size_t d = 0; d < Dim; ++d) {
            _v[d] += rhs[d];
        }
        return *this;
    }

    template <typename S>
    Vector & operator-=(const Vector<D, S> & rhs) {
        for (size_t d = 0; d < Dim; ++d) {
            _v[d] -= rhs[d];
        }
        return *this;
    }

    Vector & operator*=(Scalar a) {
        for (size_t d = 0; d < Dim; ++d) {
            _v[d] *= a;
        }
        return *this;
    }

    Vector & operator/=(Scalar a) {
        Scalar inv = Scalar(1) / a;
        for (size_t d = 0; d < Dim; ++d) {
            _v[d] *= inv;
        }
        return *this;
    }

    size_t MaxDimension() const {
        size_t maxDim = 0;
        Scalar maxValue = _v[0];
        for (size_t d = 1; d < Dim; ++d) {
            if (_v[d] > maxValue) {
                maxDim = d;
                maxValue = _v[d];
            }
        }
        return maxDim;
    }

    Scalar LengthSquared() const {
        Scalar result(0);
        for (size_t d = 0; d < Dim; ++d) {
            result += _v[d] * _v[d];
        }
        return result;
    }

    Scalar Length() const { return std::sqrt(LengthSquared()); }

    Vector & Normalize() {
        Scalar invLength = Scalar(1) / Length();
        for (size_t d = 0; d < Dim; ++d) {
            _v[d] *= invLength;
        }
        return *this;
    }

private:
    std::array<Scalar, Dim> _v;
};

using Vector2i = Vector<2, int>;
using Vector2f = Vector<2, float>;
using Vector3i = Vector<3, int>;
using Vector3f = Vector<3, float>;

template <size_t D, typename T>
inline std::ostream & operator<<(std::ostream & os, const Vector<D, T> & v)
{
    os << "[ ";
    for (size_t d = 0; d < D; ++d) {
        os << v[d];
        if (d + 1 < D) {
            os << ", ";
        }
    }
    os << " ]";
    return os;
}

template <size_t D, typename T>
Vector<D, T> operator-(const Vector<D, T> & v)
{
    Vector<D, T> result;
    for (size_t d = 0; d < D; ++d) {
        result[d] = -v[d];
    }
    return result;
}

template <size_t D, typename T, typename S>
Vector<D, decltype(T() + S())> operator+(const Vector<D, T> & lhs,
                                         const Vector<D, S> & rhs)
{
    Vector<D, decltype(T() + S())> result;
    for (size_t d = 0; d < D; ++d) {
        result[d] = lhs[d] + rhs[d];
    }
    return result;
}

template <size_t D, typename T, typename S>
Vector<D, decltype(T() - S())> operator-(const Vector<D, T> & lhs,
                                         const Vector<D, S> & rhs)
{
    Vector<D, decltype(T() - S())> result;
    for (size_t d = 0; d < D; ++d) {
        result[d] = lhs[d] - rhs[d];
    }
    return result;
}

template <size_t D, typename T, typename S>
Vector<D, decltype(T() * S())> operator*(const Vector<D, T> & v, S c)
{
    Vector<D, decltype(T() * S())> result;
    for (size_t d = 0; d < D; ++d) {
        result[d] = v[d] * c;
    }
    return result;
}

template <size_t D, typename T, typename S>
Vector<D, decltype(T() * S())> operator*(T c, const Vector<D, S> & v)
{
    return v * c;
}

template <size_t D, typename T, typename S>
Vector<D, decltype(T() / S())> operator/(const Vector<D, T> & v, S c)
{
    decltype(T() / S()) inv = T(1) / c;
    Vector<D, decltype(T() / S())> result;
    for (size_t d = 0; d < D; ++d) {
        result[d] = v[d] * inv;
    }
    return result;
}

template <size_t D, typename T, typename S>
bool operator==(const Vector<D, T> & v1, const Vector<D, S> & v2)
{
    for (size_t d = 0; d < D; ++d) {
        if (v1[d] != v2[d]) {
            return false;
        }
    }
    return true;
}

template <size_t D, typename T, typename S>
bool operator!=(const Vector<D, T> & v1, const Vector<D, S> & v2)
{
    for (size_t d = 0; d < D; ++d) {
        if (v1[d] == v2[d]) {
            return false;
        }
    }
    return true;
}

template <size_t D, typename T>
T LengthSquared(const Vector<D, T> & v)
{
    return v.LengthSquared();
}

template <size_t D, typename T>
T Length(const Vector<D, T> & v)
{
    return v.Length();
}

template <size_t D, typename T>
Vector<D, T> Normalize(Vector<D, T> v)
{
    v.Normalize();
    return v;
}

template <size_t D, typename T, typename S>
decltype(T() * S()) Dot(const Vector<D, T> & v1, const Vector<D, S> & v2)
{
    decltype(T() * S()) result = 0;
    for (size_t d = 0; d < D; ++d) {
        result += v1[d] * v2[d];
    }
    return result;
}

template <typename T, typename S>
Vector<3, decltype(T() * S())> Cross(const Vector<3, T> & lhs,
                                     const Vector<3, S> & rhs)
{
    return Vector<3, decltype(T() * S())>(
            lhs.y() * rhs.z() - lhs.z() * rhs.y(),
            lhs.z() * rhs.x() - lhs.x() * rhs.z(),
            lhs.x() * rhs.y() - lhs.y() * rhs.x());
}

template <size_t D, typename T, typename S>
Vector<D, remove_cvref_t<decltype(std::min(T(), S()))>> Min(
    const Vector<D, T> & v1, const Vector<D, S> & v2)
{
    Vector<D, remove_cvref_t<decltype(std::min(T(), S()))>> result;
    for (size_t d = 0; d < D; ++d) {
        result[d] = std::min(v1[d], v2[d]);
    }
    return result;
}

template <size_t D, typename T, typename S>
Vector<D, remove_cvref_t<decltype(std::max(T(), S()))>> Max(
    const Vector<D, T> & v1, const Vector<D, S> & v2)
{
    Vector<D, remove_cvref_t<decltype(std::max(T(), S()))>> result;
    for (size_t d = 0; d < D; ++d) {
        result[d] = std::max(v1[d], v2[d]);
    }
    return result;
}

template <size_t D, typename T>
size_t MaxDimension(const Vector<D, T> & v)
{
    return v.MaxDimension();
}

template <size_t D, typename T>
Vector<D, T> Permute(const Vector<D, T> & v, const Vector<D, int> & k)
{
    Vector<D, T> result;
    for (size_t d = 0; d < D; ++d) {
        result[d] = v[k[d]];   
    }
    return result;
}

inline float Abs(float c)
{
    return std::abs(c);
}

template <size_t D, typename T>
Vector<D, T> Abs(const Vector<D, T> & v)
{
    Vector<D, T> result;
    for (size_t d = 0; d < D; ++d) {
        result[d] = Abs(v[d]);
    }
    return result;
}

}  // namespace renoster

#endif  // RENOSTER_MATH_VECTOR_H_
