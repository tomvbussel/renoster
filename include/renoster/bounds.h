#ifndef RENOSTER_MATH_BOUNDS_H_
#define RENOSTER_MATH_BOUNDS_H_

#include <limits>

#include "renoster/point.h"
#include "renoster/vector.h"

namespace renoster {

template <size_t D, typename T>
class Bounds {
public:
    using Scalar = T;
    using PointType = Point<D, T>;
    using VectorType = Vector<D, T>;
    static constexpr size_t Dim = D;

    Bounds() {
        _min = PointType(std::numeric_limits<Scalar>::max());
        _max = PointType(std::numeric_limits<Scalar>::lowest());
    }

    ~Bounds() = default;

    explicit Bounds(const PointType & p) : _min(p), _max(p) {}

    Bounds(const PointType & min, const PointType & max)
        : _min(min), _max(max) {}

    Bounds(const Bounds &) = default;
    Bounds & operator=(const Bounds &) = default;

    Bounds(Bounds &&) = default;
    Bounds & operator=(Bounds &&) = default;

    template <typename S>
    explicit Bounds(const Bounds<D, S> & b) : _min(b.min()), _max(b.max()) {}

    template <typename S>
    Bounds & operator=(const Bounds<D, S> & b) {
        _min = b.min();
        _max = b.max();
    }

    const PointType & min() const { return _min; }

    PointType & min() { return _min; }

    const PointType & max() const { return _max; }

    PointType & max() { return _max; }

    const PointType & operator[](size_t i) const {
        if (i == 0) {
            return _min;
        } else {
            return _max;
        }
    }

    PointType & operator[](size_t i) {
        if (i == 0) {
            return _min;
        } else {
            return _max;
        }
    }

    VectorType Diagonal() const { return _max - _min; }

    PointType Center() const { return _min + Diagonal() / 2; }

    bool IsDegenerate() const {
        for (size_t d = 0; d < Dim; ++d) {
            if (_min[d] >= _max[d]) {
                return true;
            }
        }
        return false;
    }

    bool Contains(const PointType & p) const {
        for (size_t d = 0; d < Dim; ++d) {
            if (p[d] < _min[d] || p[d] > _max[d]) {
                return false;
            }
        }
        return true;
    }

    Scalar Volume() const {
        Scalar volume = 1;
        VectorType diag = Diagonal();
        for (size_t d = 0; d < Dim; ++d) {
            volume *= diag[d];
        }
        return volume;
    }

    Scalar SurfaceArea() const {
        Scalar area = 0;
        VectorType diag = Diagonal();
        for (size_t i = 0; i < Dim; ++i) {
            Scalar term = 1;
            for (size_t j = 0; j < Dim - 1; ++j) {
                term *= diag[i];
            }
            area += term;
        }
        return 2 * area;
    }

    void ExpandBy(const PointType & p) {
        _min = Min(_min, p);
        _max = Max(_max, p);
    }

    void ExpandBy(const Bounds & b) {
        _min = Min(_min, b._min);
        _max = Max(_max, b._max);
    }

private:
    PointType _min;
    PointType _max;
};

using Bounds1i = Bounds<1, int>;
using Bounds1f = Bounds<1, float>;
using Bounds2i = Bounds<2, int>;
using Bounds2f = Bounds<2, float>;
using Bounds3i = Bounds<3, int>;
using Bounds3f = Bounds<3, float>;

template <size_t D, typename T>
Vector<D, T> Diagonal(const Bounds<D, T> & b) {
    return b.Diagonal();
}

template <size_t D, typename T>
Point<D, T> Center(const Bounds<D, T> & b) {
    return b.Center();
}

template <size_t D, typename T>
bool Inside(const Point<D, T> & p, const Bounds<D, T> & b) {
    return b.Contains(p);
}

template <size_t D, typename T, typename S>
bool Overlaps(const Bounds<D, T> & b1, const Bounds<D, S> & b2) {
    for (size_t d = 0; d < D; ++d) {
        if (b1.max()[d] < b2.min()[d] || b1.min()[d] > b2.max()[d]) {
            return false;
        }
    }
    return true;
}

template <size_t D, typename T, typename S>
auto Union(const Bounds<D, T> & b1, const Bounds<D, S> & b2)
    -> Bounds<D, typename decltype(Min(b1.min(), b2.min()))::Scalar> {
    using R = typename decltype(Min(b1.min(), b2.min()))::Scalar;

    Point<D, R> min = Min(b1.min(), b2.min());
    Point<D, R> max = Max(b1.max(), b2.max());
    return Bounds<D, R>(min, max);
}

template <size_t D, typename T, typename S>
auto Union(const Bounds<D, T> & b, const Point<D, S> & p)
    -> Bounds<D, typename decltype(Min(b.min(), p))::Scalar> {
    using R = typename decltype(Min(b.min(), p))::Scalar;
    Point<D, R> min = Min(b.min(), p);
    Point<D, R> max = Max(b.max(), p);
    return Bounds<D, R>(min, max);
}

template <size_t D, typename T, typename S>
auto Union(const Point<D, T> & p, const Bounds<D, S> & b)
    -> Bounds<D, typename decltype(Min(p, b.min()))::Scalar> {
    using R = typename decltype(Min(p, b.min()))::Scalar;
    Point<D, R> min = Min(p, b.min());
    Point<D, R> max = Max(p, b.max());
    return Bounds<D, R>(min, max);
}

template <size_t D, typename T, typename S>
auto Intersection(const Bounds<D, T> & b1, const Bounds<D, S> & b2)
    -> Bounds<D, typename decltype(Min(b1.min(), b2.min()))::Scalar> {
    using R = typename decltype(Min(b1.min(), b2.min()))::Scalar;

    Point<D, R> min = Max(b1.min(), b2.min());
    Point<D, R> max = Min(b1.max(), b2.max());
    return Bounds<D, R>(min, max);
}

template <size_t D, typename T>
Bounds<D, T> Lerp(const Bounds<D, T> & b1, const Bounds<D, T> & b2, T t) {
    Point<D, T> min = Lerp(b1.min(), b2.min(), t);
    Point<D, T> max = Lerp(b1.max(), b2.max(), t);
    return Bounds<D, T>(min, max);
}

template <size_t D, typename T>
T Volume(const Bounds<D, T> & b) {
    return b.Volume();
}

template <size_t D, typename T>
T SurfaceArea(const Bounds<D, T> & b) {
    return b.SurfaceArea();
}

template <size_t D, typename T, typename S>
bool operator==(const Bounds<D, T> & b1, const Bounds<D, S> & b2) {
    return b1.min() == b2.min() && b1.max() == b2.max();
}

template <size_t D, typename T, typename S>
bool operator!=(const Bounds<D, T> & b1, const Bounds<D, S> & b2) {
    return b1.min() != b2.min() || b1.max() != b2.max();
}

template <size_t D, typename T>
class BoundsIterator : public std::forward_iterator_tag {
public:
    using BoundsType = Bounds<D, T>;
    using PointType = Point<D, T>;
    using Scalar = T;
    static constexpr size_t Dim = D;

    BoundsIterator(const BoundsType & bounds, const PointType & p, bool end)
        : _bounds(bounds), _p(p), _end(end) {}

    BoundsIterator & operator++() {
        for (size_t d = 0; d < D; ++d) {
            ++_p[d];
            if (_p[d] >= _bounds.max()[d]) {
                _p[d] = _bounds.min()[d];
            } else {
                return *this;
            }
        }
        _end = true;
        return *this;
    }

    BoundsIterator operator++(int) {
        auto old = *this;
        ++(*this);
        return old;
    }

    friend bool operator==(const BoundsIterator & lhs,
                           const BoundsIterator & rhs) {
        return lhs._p == rhs._p && lhs._bounds == rhs._bounds &&
               lhs._end == rhs._end;
    }

    friend bool operator!=(const BoundsIterator & lhs,
                           const BoundsIterator & rhs) {
        return lhs._p != rhs._p || lhs._bounds != rhs._bounds ||
               lhs._end != rhs._end;
    }

    PointType operator*() const { return _p; }

private:
    const Bounds2i & _bounds;
    Point<D, T> _p;
    bool _end;
};

using Bounds2iIterator = BoundsIterator<2, int>;
using Bounds3iIterator = BoundsIterator<3, int>;

template <size_t D, typename T>
BoundsIterator<D, T> begin(const Bounds<D, T> & b) {
    return BoundsIterator<D, T>(b, b.min(), false);
}

template <size_t D, typename T>
BoundsIterator<D, T> end(const Bounds<D, T> & b) {
    if (b.IsDegenerate()) {
        return begin(b);
    }

    return BoundsIterator<D, T>(b, b.min(), true);
}

}  // namespace renoster

#endif  // RENOSTER_MATH_BOUNDS_H_
