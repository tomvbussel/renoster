#ifndef RENOSTER_MATH_TRANSFORM_H_
#define RENOSTER_MATH_TRANSFORM_H_

#include <algorithm>
#include <vector>

#include "renoster/bounds.h"
#include "renoster/export.h"
#include "renoster/matrix.h"
#include "renoster/normal.h"
#include "renoster/point.h"
#include "renoster/position.h"
#include "renoster/ray.h"
#include "renoster/util/span.h"
#include "renoster/vector.h"

namespace renoster {

class Transform {
public:
    Transform() {}

    Transform(float mat[4][4]) {
        _mat = Matrix4x4f(mat);
        _matInv = Inverse(_mat);
    }

    Transform(const Matrix4x4f & mat) : _mat(mat), _matInv(Inverse(mat)) {}

    Transform(const Matrix4x4f & mat, const Matrix4x4f & matInv)
        : _mat(mat), _matInv(matInv) {}

    const Matrix4x4f & GetMatrix() const { return _mat; }

    const Matrix4x4f & GetInverseMatrix() const { return _matInv; }

    Point3f operator()(const Point3f & p) const { return _mat * p; }

    Vector3f operator()(const Vector3f & v) const { return _mat * v; }

    Normal3f operator()(const Normal3f & n) const { return _matInv * n; }

    Position3f operator()(const Position3f & pos) const {
        const Transform & T = *this;
        Point3f p = T(pos.p());
        return Position3f(p, pos.tMin(), pos.tMax(), pos.time());
    }

    Ray3f operator()(const Ray3f & r) const {
        const Transform & T = *this;
        Point3f o = T(r.o());
        Vector3f d = T(r.d());
        return Ray3f(o, d, r.tMin(), r.tMax(), r.time());
    }

    Bounds3f operator()(const Bounds3f & b) const {
        const Transform & T = *this;

        Bounds3f ret(T(Point3f(b.min().x(), b.min().y(), b.min().z())));
        ret = Union(ret, T(Point3f(b.min().x(), b.min().y(), b.max().z())));
        ret = Union(ret, T(Point3f(b.min().x(), b.max().y(), b.min().z())));
        ret = Union(ret, T(Point3f(b.min().x(), b.max().y(), b.max().z())));
        ret = Union(ret, T(Point3f(b.max().x(), b.min().y(), b.min().z())));
        ret = Union(ret, T(Point3f(b.max().x(), b.min().y(), b.max().z())));
        ret = Union(ret, T(Point3f(b.max().x(), b.max().y(), b.min().z())));
        ret = Union(ret, T(Point3f(b.max().x(), b.max().y(), b.max().z())));

        return ret;
    }

    friend bool operator==(const Transform & lhs, const Transform & rhs) {
        return lhs._mat == rhs._mat;
    }

    friend bool operator!=(const Transform & lhs, const Transform & rhs) {
        return lhs._mat != rhs._mat;
    }

    friend Transform Inverse(const Transform & t) {
        return Transform(t._matInv, t._mat);
    }

    friend Transform Transpose(const Transform & t) {
        return Transform(Transpose(t._mat), Transpose(t._matInv));
    }

    friend Transform operator*(const Transform & lhs, const Transform & rhs) {
        return Transform(lhs._mat * rhs._mat, rhs._matInv * lhs._matInv);
    }

    friend Transform Lerp(const Transform & t0, const Transform & t1, float t);

private:
    Matrix4x4f _mat, _matInv;
};

inline Transform Lerp(const Transform & t0, const Transform & t1, float t) {
    return Transform(Lerp(t0._mat, t1._mat, t),
                     Lerp(t0._matInv, t1._matInv, t));
}

class AnimatedTransform {
public:
    AnimatedTransform(std::vector<Transform> transforms,
                      std::vector<float> times)
        : transforms_(std::move(transforms)), times_(std::move(times)) {}

    AnimatedTransform(const span<Transform> & transforms,
                      const span<float> & times)
        : transforms_(transforms.begin(), transforms.end()),
          times_(times.begin(), times.end()) {}

    Transform Lerp(float t) {
        auto it = std::lower_bound(times_.begin(), times_.end(), t);
        size_t i = it - times_.begin();

        float t0 = times_[i];
        float t1 = times_[i + 1];

        Transform T0 = transforms_[i];
        Transform T1 = transforms_[i + 1];

        return renoster::Lerp(T0, T1, (t - t0) / (t1 - t0));
    }

private:
    std::vector<Transform> transforms_;
    std::vector<float> times_;
};

RENO_API Transform Identity();

RENO_API Transform Translate(float dx, float dy, float dz);

inline Transform Translate(const Vector3f & d) {
    return Translate(d.x(), d.y(), d.z());
}

RENO_API Transform Scale(float sx, float sy, float sz);

inline Transform Scale(const Vector3f & s) {
    return Scale(s.x(), s.y(), s.z());
}

RENO_API Transform RotateX(float angle);

RENO_API Transform RotateY(float angle);

RENO_API Transform RotateZ(float angle);

RENO_API Transform Rotate(float angle, const Vector3f & axis);

RENO_API Transform LookAt(const Point3f & pos, const Point3f & look,
                          const Vector3f & up);

RENO_API Transform Orthographic(float zNear, float zFar);

RENO_API Transform Perspective(float fov, float zNear, float zFar);

}  // namespace renoster

#endif  // RENOSTER_MATH_TRANSFORM_H_
