#include "renoster/accel/builder.h"
#include "renoster/accel/splitter.h"
#include "renoster/bvh.h"
#include "renoster/geometry.h"
#include "renoster/sampling.h"

#include <cassert>
#include <memory>
#include <vector>

namespace renoster {

class TriangleMesh;

struct Triangle : public Geometry {
public:
    Triangle() = default;

    Triangle(size_t face, const TriangleMesh * mesh);

    bool Intersect(const GeometryContext & ctx, const Ray3f & ray,
                   ShadingPoint * sp) const;

    bool Occluded(const GeometryContext & ctx, const Ray3f & ray) const;

    void ComputeShadingInfo(const GeometryContext & ctx,
                            ShadingPoint * sp) const;

    ShadingPoint Sample(const GeometryContext & ctx, Sampler & sampler,
                        float * pdf) const;

    float Pdf(const GeometryContext & ctx, const ShadingPoint & sp) const;

    Bounds3f GetObjectBounds() const;

    Bounds3f GetWorldBounds(const GeometryContext & ctx) const;

private:
    size_t _face;
    const TriangleMesh * _mesh;
};

class TriangleMesh : public Geometry {
public:
    TriangleMesh(std::vector<int> vertices, std::vector<Point3f> p,
                 std::vector<Normal3f> n, std::vector<Point2f> uv);

    bool Intersect(const GeometryContext & ctx, const Ray3f & ray,
                   ShadingPoint * sp) const;

    bool Occluded(const GeometryContext & ctx, const Ray3f & ray) const;

    void ComputeShadingInfo(const GeometryContext & ctx,
                            ShadingPoint * sp) const;

    ShadingPoint Sample(const GeometryContext & ctx, Sampler & sampler,
                        float * pdf) const;

    float Pdf(const GeometryContext & ctx, const ShadingPoint & sp) const;

    Bounds3f GetObjectBounds() const;

    Bounds3f GetWorldBounds(const GeometryContext & ctx) const;

    std::vector<int> _vertices;
    std::vector<Point3f> _p;
    std::vector<Normal3f> _n;
    std::vector<Point2f> _uv;

private:
    std::vector<Triangle> _triangles;
    std::unique_ptr<BVH> _bvh;
    Distribution1D _distrib;
};

Triangle::Triangle(size_t face, const TriangleMesh * mesh)
    : _face(face), _mesh(mesh)
{
}

bool Triangle::Intersect(const GeometryContext & ctx, const Ray3f & ray,
                         ShadingPoint * sp) const
{
    // Get vertex indices
    int v0 = _mesh->_vertices[3 * _face];
    int v1 = _mesh->_vertices[3 * _face + 1];
    int v2 = _mesh->_vertices[3 * _face + 2];

    // Get vertex positions
    Point3f p0 = ctx.ObjectToWorld(_mesh->_p[v0]);
    Point3f p1 = ctx.ObjectToWorld(_mesh->_p[v1]);
    Point3f p2 = ctx.ObjectToWorld(_mesh->_p[v2]);

    // Place ray origin at the origin
    Vector3f p0t = p0 - ray.o();
    Vector3f p1t = p1 - ray.o();
    Vector3f p2t = p2 - ray.o();

    // Make sure the z-value of the direction has the largest absolute value
    int kz = MaxDimension(Abs(ray.d()));
    int kx = (kz == 2) ? 0 : kz + 1;
    int ky = (kx == 2) ? 0 : kx + 1;

    // Preserve winding order
    if (ray.d()[kz] < 0.f) {
        std::swap(kx, ky);
    }

    Vector3i k(kx, ky, kz);
    p0t = Permute(p0t, k);
    p1t = Permute(p1t, k);
    p2t = Permute(p2t, k);
    Vector3f d = Permute(ray.d(), k);
    
    // Transform the direction to the unit direction
    float Sx = -d.x() / d.z();
    float Sy = -d.y() / d.z();
    float Sz = 1.f / d.z();

    p0t.x() += Sx * p0t.z();
    p0t.y() += Sy * p0t.z();
    p1t.x() += Sx * p1t.z();
    p1t.y() += Sy * p1t.z();
    p2t.x() += Sx * p2t.z();
    p2t.y() += Sy * p2t.z();

    // Calculate scaled barycentric coordinates
    float e0 = p1t.x() * p2t.y() - p1t.y() * p2t.x();
    float e1 = p2t.x() * p0t.y() - p2t.y() * p0t.x();
    float e2 = p0t.x() * p1t.y() - p0t.y() * p1t.x();

    if (e0 == 0 || e1 == 0 || e2 == 0) {
        double p1txp2ty = double(p1t.x()) * double(p2t.y());
        double p1typ2tx = double(p1t.y()) * double(p2t.x());
        e0 = float(p1txp2ty - p1txp2ty);

        double p2txp0ty = double(p2t.x()) * double(p0t.y());
        double p2typ0tx = double(p2t.y()) * double(p0t.x());
        e1 = float(p2txp0ty - p2typ0tx);

        double p0txp1ty = double(p0t.x()) * double(p1t.y());
        double p0typ1tx = double(p0t.y()) * double(p1t.x());
        e2 = float(p0txp1ty - p0typ1tx);
    }

    if ((e0 < 0 || e1 < 0 || e2 < 0) && (e0 > 0 || e1 > 0 || e2 > 0)) {
        return false;
    }

    float det = e0 + e1 + e2;
    if (det == 0) {
        return false;
    }

    p0t.z() *= Sz;
    p1t.z() *= Sz;
    p2t.z() *= Sz;

    // Calculate scaled hit distance
    float tScaled = e0 * p0t.z() + e1 * p1t.z() + e2 * p2t.z();

    if (det < 0 && (tScaled >= ray.tMin() * det || tScaled < ray.tMax() * det)) {
        return false;
    } else if (det > 0 && (tScaled <= ray.tMin() * det || tScaled > ray.tMax() * det)) {
        return false;
    }

    // Scale barycentric coordinates
    float invDet = 1.f / det;
    float b0 = e0 * invDet;
    float b1 = e1 * invDet;
    float b2 = e2 * invDet;
    
    ray.tMax() = tScaled * invDet;

    sp->p = ctx.ObjectToWorld(b0 * p0 + b1 * p1 + b2 * p2);
    sp->face = _face;
    sp->wo = -ray.d();

    Point2f uv0, uv1, uv2;
    if (!_mesh->_uv.empty()) {
        uv0 = _mesh->_uv[v0];   
        uv1 = _mesh->_uv[v0];   
        uv2 = _mesh->_uv[v0];
    } else {
        uv0 = Point2f(0.f, 0.f);
        uv1 = Point2f(1.f, 0.f);
        uv2 = Point2f(0.f, 1.f);
    }

    sp->u = b0 * uv0[0] + b1 * uv1[0] + b2 * uv2[0];
    sp->v = b0 * uv0[1] + b1 * uv1[1] + b2 * uv2[1];

    sp->ng = Normalize(Cross(p0 - p2, p1 - p2));

    return true;
}

bool Triangle::Occluded(const GeometryContext & ctx, const Ray3f & ray) const
{
    // Get the vertex indices
    int v0 = _mesh->_vertices[3 * _face];
    int v1 = _mesh->_vertices[3 * _face + 1];
    int v2 = _mesh->_vertices[3 * _face + 2];

    // Get the vertex positions
    Point3f p0 = ctx.ObjectToWorld(_mesh->_p[v0]);
    Point3f p1 = ctx.ObjectToWorld(_mesh->_p[v1]);
    Point3f p2 = ctx.ObjectToWorld(_mesh->_p[v2]);

    // Place ray origin at the origin
    Vector3f p0t = p0 - ray.o();
    Vector3f p1t = p1 - ray.o();
    Vector3f p2t = p2 - ray.o();

    // Make sure the z-value of the direction has the largest absolute value
    int kz = MaxDimension(Abs(ray.d()));
    int kx = (kz == 2) ? 0 : kz + 1;
    int ky = (kx == 2) ? 0 : kx + 1;

    // Preserve winding order
    if (ray.d()[kz] < 0.f) {
        std::swap(kx, ky);
    }

    Vector3i k(kx, ky, kz);
    p0t = Permute(p0t, k);
    p1t = Permute(p1t, k);
    p2t = Permute(p2t, k);
    Vector3f d = Permute(ray.d(), k);
    
    // Transform the direction to the unit direction
    float Sx = -d.x() / d.z();
    float Sy = -d.y() / d.z();
    float Sz = 1.f / d.z();

    p0t.x() += Sx * p0t.z();
    p0t.y() += Sy * p0t.z();
    p1t.x() += Sx * p1t.z();
    p1t.y() += Sy * p1t.z();
    p2t.x() += Sx * p2t.z();
    p2t.y() += Sy * p2t.z();

    // Calculate scaled barycentric coordinates
    float e0 = p1t.x() * p2t.y() - p1t.y() * p2t.x();
    float e1 = p2t.x() * p0t.y() - p2t.y() * p0t.x();
    float e2 = p0t.x() * p1t.y() - p0t.y() * p1t.x();

    if (e0 == 0 || e1 == 0 || e2 == 0) {
        double p1txp2ty = double(p1t.x()) * double(p2t.y());
        double p1typ2tx = double(p1t.y()) * double(p2t.x());
        e0 = float(p1txp2ty - p1txp2ty);

        double p2txp0ty = double(p2t.x()) * double(p0t.y());
        double p2typ0tx = double(p2t.y()) * double(p0t.x());
        e1 = float(p2txp0ty - p2typ0tx);

        double p0txp1ty = double(p0t.x()) * double(p1t.y());
        double p0typ1tx = double(p0t.y()) * double(p1t.x());
        e2 = float(p0txp1ty - p0typ1tx);
    }

    if ((e0 < 0 || e1 < 0 || e2 < 0) && (e0 > 0 || e1 > 0 || e2 > 0)) {
        return false;
    }

    float det = e0 + e1 + e2;
    if (det == 0) {
        return false;
    }

    p0t.z() *= Sz;
    p1t.z() *= Sz;
    p2t.z() *= Sz;

    // Calculate scaled hit distance
    float tScaled = e0 * p0t.z() + e1 * p1t.z() + e2 * p2t.z();

    if (det < 0 && (tScaled >= ray.tMin() * det || tScaled < ray.tMax() * det)) {
        return false;
    } else if (det > 0 && (tScaled <= ray.tMin() * det || tScaled > ray.tMax() * det)) {
        return false;
    }

    return true;
}

void Triangle::ComputeShadingInfo(const GeometryContext & ctx,
                                  ShadingPoint * sp) const
{
    // Get vertex indices
    int v0 = _mesh->_vertices[3 * _face];
    int v1 = _mesh->_vertices[3 * _face + 1];
    int v2 = _mesh->_vertices[3 * _face + 2];

    // Get vertex positions
    Point3f p0 = ctx.ObjectToWorld(_mesh->_p[v0]);
    Point3f p1 = ctx.ObjectToWorld(_mesh->_p[v1]);
    Point3f p2 = ctx.ObjectToWorld(_mesh->_p[v2]);

    // Get vertex uv coordinates
    Point2f uv0, uv1, uv2;
    if (!_mesh->_uv.empty()) {
        uv0 = _mesh->_uv[v0];
        uv1 = _mesh->_uv[v1];
        uv2 = _mesh->_uv[v2];
    } else {
        uv0 = Point2f(0.f, 0.f);
        uv1 = Point2f(1.f, 0.f);
        uv2 = Point2f(0.f, 1.f);
    }

    // Get vertex normals
    Normal3f n0, n1, n2;
    if (!_mesh->_n.empty()) {
        n0 = ctx.ObjectToWorld(_mesh->_n[v0]);
        n1 = ctx.ObjectToWorld(_mesh->_n[v1]);
        n2 = ctx.ObjectToWorld(_mesh->_n[v2]);
    } else {
        n0 = sp->ng;
        n1 = sp->ng;
        n2 = sp->ng;
    }

    // Recover barycentric coordinates
    Point2f uv(sp->u, sp->v);
    Vector2f duv = uv - uv0;
    Vector2f duv1 = uv1 - uv0, duv2 = uv1 - uv0;
    float det = duv1.x() * duv2.y() - duv2.x() * duv1.y();
    float invDet = 1.f / det;
    float b1 = invDet * (duv2.y() * duv.x() - duv2.x() * duv.y());
    float b2 = invDet * (duv1.x() * duv.y() - duv1.y() * duv.x());
    float b0 = 1.f - b1 - b2;

    // Compute shading normal
    sp->ns = Normalize(b0 * n0 + b1 * n1 + b2 * n2);

    // Compute position derivatives
    Vector3f dp1 = p1 - p0, dp2 = p2 - p0;
    sp->dpdu = invDet * (duv2.y() * dp1 - duv1.y() * dp2);
    sp->dpdv = invDet * (duv1.x() * dp2 - duv2.x() * dp1);

    // Compute geometric normal derivatives
    sp->dngdu = Normal3f(0.f);
    sp->dngdv = Normal3f(0.f);

    // Compute shading normal derivatives
    Normal3f dn1 = n1 - n0, dn2 = n2 - n0;
    sp->dnsdu = invDet * (duv2.y() * dn1 - duv1.y() * dn2);
    sp->dnsdv = invDet * (duv1.x() * dn2 - duv2.x() * dn1);
}

ShadingPoint Triangle::Sample(const GeometryContext & ctx, Sampler & sampler,
                              float * pdf) const
{
    // Get vertex indices
    int v0 = _mesh->_vertices[3 * _face];
    int v1 = _mesh->_vertices[3 * _face + 1];
    int v2 = _mesh->_vertices[3 * _face + 2];

    // Get vertex positions
    Point3f p0 = ctx.ObjectToWorld(_mesh->_p[v0]);
    Point3f p1 = ctx.ObjectToWorld(_mesh->_p[v1]);
    Point3f p2 = ctx.ObjectToWorld(_mesh->_p[v2]);

    // Get vertex uv coordinates
    Point2f uv0, uv1, uv2;
    if (!_mesh->_uv.empty()) {
        uv0 = _mesh->_uv[v0];
        uv1 = _mesh->_uv[v1];
        uv2 = _mesh->_uv[v2];
    } else {
        uv0 = Point2f(0.f, 0.f);
        uv1 = Point2f(1.f, 0.f);
        uv2 = Point2f(0.f, 1.f);
    }

    // Sample barycentric coordinates
    Point2f b = UniformSampleTriangle(sampler.Get2D());
    float b0 = 1.f - b.x() - b.y();
    float b1 = b[0];
    float b2 = b[1];

    ShadingPoint sp;
    sp.p = b0 * p0 + b1 * p1 + b2 * p2;
    sp.ng = Normalize(Cross(p1 - p0, p2 - p0));
    sp.u = b0 * uv0[0] + b1 * uv1[0] + b2 * uv2[0];
    sp.v = b0 * uv0[1] + b1 * uv1[1] + b2 * uv2[1];
    sp.face = _face;
    *pdf = 1.f / (0.5f * Cross(p1 - p0, p2 - p0).Length());
    return sp;
}

float Triangle::Pdf(const GeometryContext & ctx, const ShadingPoint & sp) const
{
    // Get vertex indices
    int v0 = _mesh->_vertices[3 * _face];
    int v1 = _mesh->_vertices[3 * _face + 1];
    int v2 = _mesh->_vertices[3 * _face + 2];

    // Get vertex positions
    Point3f p0 = ctx.ObjectToWorld(_mesh->_p[v0]);
    Point3f p1 = ctx.ObjectToWorld(_mesh->_p[v1]);
    Point3f p2 = ctx.ObjectToWorld(_mesh->_p[v2]);

    return 1.f / (0.5f * Cross(p1 - p0, p2 - p0).Length());
}

Bounds3f Triangle::GetObjectBounds() const
{
    // Get the vertex indices
    int v0 = _mesh->_vertices[3 * _face];
    int v1 = _mesh->_vertices[3 * _face + 1];
    int v2 = _mesh->_vertices[3 * _face + 2];

    // Get the vertex positions
    const Point3f & p0 = _mesh->_p[v0];
    const Point3f & p1 = _mesh->_p[v1];
    const Point3f & p2 = _mesh->_p[v2];

    Bounds3f bounds(p0);
    bounds.ExpandBy(p1);
    bounds.ExpandBy(p2);
    return bounds;
}

Bounds3f Triangle::GetWorldBounds(const GeometryContext & ctx) const
{
    // Get the vertex indices
    int v0 = _mesh->_vertices[3 * _face];
    int v1 = _mesh->_vertices[3 * _face + 1];
    int v2 = _mesh->_vertices[3 * _face + 2];

    // Get the vertex positions
    Point3f p0 = ctx.ObjectToWorld(_mesh->_p[v0]);
    Point3f p1 = ctx.ObjectToWorld(_mesh->_p[v1]);
    Point3f p2 = ctx.ObjectToWorld(_mesh->_p[v2]);

    Bounds3f bounds(p0);
    bounds.ExpandBy(p1);
    bounds.ExpandBy(p2);
    return bounds;
}

TriangleMesh::TriangleMesh(std::vector<int> vertices, std::vector<Point3f> p,
                           std::vector<Normal3f> n, std::vector<Point2f> uv)
    : _vertices(std::move(vertices)),
    _p(std::move(p)),
    _n(std::move(n)),
    _uv(std::move(uv))
{
    // Create triangles
    size_t numTriangles = _vertices.size() / 3;
    _triangles.resize(numTriangles);
    for (size_t i = 0; i < numTriangles; ++i) {
        _triangles[i] = Triangle(i, this);
    }
    std::vector<Triangle *> triPointers(numTriangles);
    for (size_t i = 0; i < numTriangles; ++i) {
        triPointers[i] = &_triangles[i];
    }

    // Build a BVH
    _bvh = std::make_unique<BVH>();
    ObjectSplitter splitter; 
    size_t MinLeafSize = 16;
    BVHBuilder<Triangle, ObjectSplitter> builder(_bvh.get(), splitter,
                                                 MinLeafSize);
    GeometryContext gCtx;
    builder.Build(gCtx, triPointers);

    // Make samplable
    std::vector<float> areas(numTriangles);
    for (size_t i = 0; i < numTriangles; ++i) {
        areas[i] = 1.f; // TODO
    }
    _distrib = Distribution1D(std::move(areas));
}

bool TriangleMesh::Intersect(const GeometryContext & ctx, const Ray3f & ray,
                             ShadingPoint * sp) const
{
    _bvh->Intersect<Triangle, GeometryContext>(ctx, ray, sp);
}

bool TriangleMesh::Occluded(const GeometryContext & ctx, const Ray3f & ray) const
{
    _bvh->Occluded<Triangle, GeometryContext>(ctx, ray);
}

void TriangleMesh::ComputeShadingInfo(const GeometryContext & ctx,
                                      ShadingPoint * sp) const
{
    _triangles[sp->face].ComputeShadingInfo(ctx, sp);
}

ShadingPoint TriangleMesh::Sample(const GeometryContext & ctx,
                                  Sampler & sampler, float * pdf) const
{
    // Select a triangle
    float facePdf;
    int face = _distrib.SampleDiscrete(sampler.Get1D(), &facePdf, nullptr);
    assert(face < _triangles.size());

    // Sample the triangle
    float trianglePdf;
    ShadingPoint sp = _triangles[face].Sample(ctx, sampler, &trianglePdf);

    *pdf = facePdf * trianglePdf;
    return sp;
}

float TriangleMesh::Pdf(const GeometryContext & ctx, const ShadingPoint & sp) const
{
    float facePdf = _distrib.PdfDiscrete(sp.face);
    float trianglePdf = _triangles[sp.face].Pdf(ctx, sp);
    return facePdf * trianglePdf;
}

Bounds3f TriangleMesh::GetObjectBounds() const
{
    Bounds3f bounds;
    for (const Point3f & p : _p) {
        bounds.ExpandBy(p);
    }
    return bounds;
}

Bounds3f TriangleMesh::GetWorldBounds(const GeometryContext & ctx) const
{
    Bounds3f bounds;
    for (const Point3f & p : _p) {
        bounds.ExpandBy(ctx.ObjectToWorld(p));
    }
    return bounds;
}

extern "C"
RENO_EXPORT
Geometry * CreateGeometry(const ParameterList & params)
{
    std::vector<int> vertices = params.GetInts("vertices");
    std::vector<Point3f> p = params.GetPoint3fs("P");
    std::vector<Normal3f> n = params.GetNormal3fs("N");
    std::vector<Point2f> uv = params.GetPoint2fs("uv");
    
    return new TriangleMesh(std::move(vertices), std::move(p),
                            std::move(n), std::move(uv));
}

} // namespace renoster
