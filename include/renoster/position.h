#ifndef RENOSTER_POSITION_H_
#define RENOSTER_POSITION_H_

namespace renoster {

template <size_t D, typename T>
class Position {
public:
    static constexpr size_t Dim = D;
    using Scalar = T;
    using PointType = Point<Dim, Scalar>;
    using VectorType = Vector<Dim, Scalar>;

    Position();

    Position(const PointType & p, Scalar tMin, Scalar tMax, float time)
        : _p(p), _tMin(tMin), _tMax(tMax), _time(time) {}

    const PointType & p() const { return _p; }

    PointType & p() { return _p; }

    Scalar & tMin() const { return _tMin; }

    Scalar & tMax() const { return _tMax; }

    const float & time() const { return _time; }

    float & time() { return _time; }

private:
    PointType _p;
    mutable Scalar _tMin;
    mutable Scalar _tMax;
    float _time;
};

using Position3f = Position<3, float>;

}  // namespace renoster

#endif  // RENOSTER_POSITION_H_
