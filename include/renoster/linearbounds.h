#ifndef RENOSTER_LINEARBOUNDS_H_
#define RENOSTER_LINEARBOUNDS_H_

#include "renoster/bounds.h"
#include "renoster/util/span.h"

namespace renoster {

template <size_t D, typename T>
class LinearBounds {
public:
    using Scalar = T;
    using BoundsType = Bounds<D, T>;
    using PointType = typename BoundsType::PointType;
    using VectorType = typename BoundsType::VectorType;
    static constexpr size_t Dim = D;

    LinearBounds() {}

    LinearBounds(const BoundsType & b0, const BoundsType & b1)
        : bounds0_(b0), bounds1_(b1) {}

    LinearBounds(const span<BoundsType> & bounds) {
        bounds0_ = *bounds.begin();
        bounds1_ = *bounds.end();

        VectorType dMin = bounds1_.min() - bounds0_.min();
        VectorType dMax = bounds1_.max() - bounds0_.max();

        for (size_t i = 1; i < bounds.size() - 1; ++i) {
            Scalar t = Scalar(i) / Scalar(bounds.size());

            BoundsType bt = Lerp(bounds0_, bounds1_);
            BoundsType b0(bt.min() - t * dMin, bt.max() - t * dMax);
            BoundsType b1(b0.min() + dMin, b0.max() + dMax);

            bounds0_.ExpandBy(b0);
            bounds1_.ExpandBy(b1);
        }
    }

    LinearBounds(const span<BoundsType> & bounds, const span<T> & times) {
        Scalar t0 = *times.begin();
        Scalar t1 = *times.end();
        BoundsType boundst0 = *bounds.begin();
        BoundsType boundst1 = *bounds.end();

        VectorType dMin = boundst1.min() - boundst0.min();
        VectorType dMax = boundst1.max() - boundst0.max();

        for (size_t i = 1; i < bounds.size() - 1; ++i) {
            Scalar ti = times[i];
            Scalar dt = (ti - t0) / (t1 - t0);

            BoundsType bt = Lerp(bounds0_, bounds1_);
            BoundsType bt0(bt.min() - dt * dMin, bt.max() - dt * dMax);
            BoundsType bt1(bt0.min() + dMin, bt0.max() + dMax);

            boundst0.ExpandBy(bt0);
            boundst1.ExpandBy(bt1);
        }

        bounds0_ = Lerp(boundst0, boundst1, (Scalar(0) - t0) / (t1 - t0));
        bounds1_ = Lerp(boundst0, boundst1, (Scalar(1) - t0) / (t1 - t0));
    }

    void Lerp(Scalar t) const { return Lerp(bounds0_, bounds0_, t); }

    Bounds3f GlobalBounds(const Bounds1f & t) const {
        Bounds3f bt0 = Lerp(t.min().x());
        Bounds3f bt1 = Lerp(t.max().x());
        return Union(bt0, bt1);
    }

    const BoundsType & Bounds0() const { return bounds0_; }

    BoundsType & Bounds0() { return bounds0_; }

    const BoundsType & Bounds1() const { return bounds1_; }

    BoundsType & Bounds1() { return bounds1_; }

    const BoundsType & operator[](size_t i) const {
        if (i == 0) {
            return bounds0_;
        } else {
            return bounds1_;
        }
    }

    BoundsType & operator[](size_t i) {
        if (i == 0) {
            return bounds0_;
        } else {
            return bounds1_;
        }
    }

private:
    BoundsType bounds0_;
    BoundsType bounds1_;
};

using LinearBounds3f = LinearBounds<3, float>;

template <size_t D, typename T, typename S>
bool operator==(const LinearBounds<D, T> & lb1,
                const LinearBounds<D, S> & lb2) {
    return lb1.BoundsBegin() == lb2.BoundsBegin() &&
           lb1.BoundsEnd() == lb2.BoundsEnd();
}

template <size_t D, typename T, typename S>
bool operator!=(const LinearBounds<D, T> & lb1,
                const LinearBounds<D, S> & lb2) {
    return lb1.BoundsBegin() != lb2.BoundsBegin() ||
           lb1.BoundsEnd() != lb2.BoundsEnd();
}

}  // namespace renoster

#endif  // RENOSTER_LINEARBOUNDS_H_