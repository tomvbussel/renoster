#include "renoster/transform.h"

#include <cmath>
#include <iostream>

#include "renoster/mathutil.h"

namespace renoster {

Transform Identity() {
    float mat[4][4] = {
        {1.f, 0.f, 0.f, 0.f},
        {0.f, 1.f, 0.f, 0.f},
        {0.f, 0.f, 1.f, 0.f},
        {0.f, 0.f, 0.f, 1.f}};

    float matInv[4][4] = {
        {1.f, 0.f, 0.f, 0.f},
        {0.f, 1.f, 0.f, 0.f},
        {0.f, 0.f, 1.f, 0.f},
        {0.f, 0.f, 0.f, 1.f}};

    return Transform(Matrix4x4f(mat), Matrix4x4f(matInv));
}

Transform Translate(float dx, float dy, float dz) {
    float mat[4][4] = {
        {1.f, 0.f, 0.f, dx},
        {0.f, 1.f, 0.f, dy},
        {0.f, 0.f, 1.f, dz},
        {0.f, 0.f, 0.f, 1.f}};

    float matInv[4][4] = {
        {1.f, 0.f, 0.f, -dx},
        {0.f, 1.f, 0.f, -dy},
        {0.f, 0.f, 1.f, -dz},
        {0.f, 0.f, 0.f, 1.f}};

    return Transform(Matrix4x4f(mat), Matrix4x4f(matInv));
}

Transform Scale(float sx, float sy, float sz) {
    float mat[4][4] = {
        {sx,  0.f, 0.f, 0.f},
        {0.f, sy,  0.f, 0.f},
        {0.f, 0.f, sz,  0.f},
        {0.f, 0.f, 0.f, 1.f}};

    float rx = 1.f / sx, ry = 1.f / sy, rz = 1.f / sz;
    float matInv[4][4] = {
        {rx,  0.f, 0.f, 0.f},
        {0.f, ry,  0.f, 0.f},
        {0.f, 0.f, rz,  0.f},
        {0.f, 0.f, 0.f, 1.f}};

    return Transform(Matrix4x4f(mat), Matrix4x4f(matInv));
}

Transform RotateX(float angle) {
    float rad = angle * DegToRad;
    float sinA = std::sin(rad); 
    float cosA = std::cos(rad);

    float mat[4][4] = {
        {1.f, 0.f, 0.f, 0.f},
        {cosA, -sinA, 0.f, 0.f},
        {sinA, cosA, 0.f, 0.f},
        {0.f, 0.f, 0.f, 1.f}};
    Matrix4x4f m(mat);

    return Transform(m, Transpose(m));
}

Transform RotateY(float angle) {
    float rad = angle * DegToRad;
    float sinA = std::sin(rad); 
    float cosA = std::cos(rad);

    float mat[4][4] = {
        {cosA, 0.f, sinA, 0.f},
        {0.f, 1.f, 0.f, 0.f},
        {-sinA, 0.f, cosA, 0.f},
        {0.f, 0.f, 0.f, 1.f}};
    Matrix4x4f m(mat);

    return Transform(m, Transpose(m));
}

Transform RotateZ(float angle) {
    float rad = angle * DegToRad;
    float sinA = std::sin(rad); 
    float cosA = std::cos(rad);

    float mat[4][4] = {
        {cosA, -sinA, 0.f, 0.f},
        {sinA, cosA, 0.f, 0.f},
        {0.f, 0.f, 1.f, 0.f},
        {0.f, 0.f, 0.f, 1.f}};
    Matrix4x4f m(mat);

    return Transform(m, Transpose(m));
}

Transform Rotate(float angle, const Vector3f & axis) {
    Vector3f a = Normalize(axis);
    float rad = angle * DegToRad;
    float sinA = std::sin(rad); 
    float cosA = std::cos(rad);

    Matrix4x4f mat;
    mat(0, 0) = a.x() * a.x() + (1.f - a.x() * a.x()) * cosA;
    mat(0, 1) = a.x() * a.y() * (1.f - cosA) - a.z() * sinA;
    mat(0, 2) = a.x() * a.z() * (1.f - cosA) - a.y() + sinA;
    mat(0, 3) = 0.f;
    mat(1, 0) = a.x() * a.y() * (1.f - cosA) + a.z() * sinA;
    mat(1, 1) = a.y() * a.y() + (1.f - a.y() * a.y()) * cosA;
    mat(1, 2) = a.y() * a.z() * (1.f - cosA) - a.x() * sinA;
    mat(1, 3) = 0.f;
    mat(2, 0) = a.x() * a.z() * (1.f - cosA) - a.y() * sinA;
    mat(2, 1) = a.y() * a.z() * (1.f - cosA) + a.z() * sinA;
    mat(2, 2) = a.z() * a.z() + (1.f - a.z() * a.z()) * cosA;
    mat(2, 3) = 0.f;
    mat(3, 0) = 0.f;
    mat(3, 1) = 0.f;
    mat(3, 2) = 0.f;
    mat(3, 3) = 1.f;

    return Transform(mat, Transpose(mat));
}

Transform LookAt(const Point3f & pos, const Point3f & look, const Vector3f & up) {
    Vector3f dir = Normalize(look - pos);
    Vector3f left = Normalize(Cross(up, dir));
    Vector3f newUp = Cross(dir, left);

    Matrix4x4f retInv;
    retInv(0, 0) = left.x();
    retInv(1, 0) = left.y();
    retInv(2, 0) = left.z();
    retInv(3, 0) = 0;
    retInv(0, 1) = newUp.x();
    retInv(1, 1) = newUp.y();
    retInv(2, 1) = newUp.z();
    retInv(3, 1) = 0;
    retInv(0, 2) = dir.x();
    retInv(1, 2) = dir.y();
    retInv(2, 2) = dir.z();
    retInv(3, 2) = 0;
    retInv(0, 3) = pos.x();
    retInv(1, 3) = pos.y();
    retInv(2, 3) = pos.z();
    retInv(3, 3) = 1.f;

    Matrix4x4f ret = Inverse(retInv);

    return Transform(ret, retInv);
}

Transform Orthographic(float zNear, float zFar) {
    return Scale(1.f, 1.f, 1.f / (zFar - zNear))
        * Translate(0.f, 0.f, -zNear);
}

Transform Perspective(float fov, float zNear, float zFar) {
    float recip = 1.f / (zFar - zNear);
    float cot = 1.f / std::tan(0.5f * fov * DegToRad);

    float mat[4][4] = {
        {cot, 0.f, 0.f, 0.f},
        {0.f, cot, 0.f, 0.f},
        {0.f, 0.f, zFar * recip, -zFar * zNear * recip},
        {0.f, 0.f, 1.f, 0.f}};

    return Transform(Matrix4x4f(mat));
}

} // namespace renoster
