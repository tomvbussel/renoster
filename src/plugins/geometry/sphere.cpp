#include "renoster/export.h"
#include "renoster/geometry.h"
#include "renoster/mathutil.h"
#include "renoster/paramlist.h"

namespace renoster {

class Sphere : public Geometry {
public:
    Sphere(float radius, float zMin, float zMax, float phiMax);

    bool Intersect(const GeometryContext & ctx, const Ray3f & ray,
                   ShadingPoint * sp) const;

    bool Occluded(const GeometryContext & ctx, const Ray3f & ray) const;

    void ComputeShadingInfo(const GeometryContext & ctx,
                            ShadingPoint * sp) const;

    ShadingPoint Sample(const GeometryContext & ctx, Sampler & sampler,
                        float * pdf) const;

    float Pdf(const GeometryContext & ctx, const ShadingPoint & sp) const;

    Bounds3f GetObjectBounds() const;

private:
    float Phi(const Point3f & p) const {
        float phi = std::atan2(p.y(), p.x());
        if (phi < 0.f) {
            phi += TwoPi;
        }
        return phi;
    }

    float _radius;
    float _zMin;
    float _zMax;
    float _phiMax;
    float _thetaMin;
    float _thetaMax;
};

Sphere::Sphere(float radius, float zMin, float zMax, float phiMax)
    : _radius(radius),
    _zMin(zMin),
    _zMax(zMax),
    _phiMax(phiMax)
{
    _thetaMin = std::acos(Clamp(_zMin / _radius, -1.f, 1.f));
    _thetaMax = std::acos(Clamp(_zMax / _radius, -1.f, 1.f));
}

bool Sphere::Intersect(const GeometryContext & ctx, const Ray3f & ray,
                       ShadingPoint * sp) const
{
    // Transform to object space
    Ray3f rayLocal = ctx.WorldToObject(ray);
    Point3f o = rayLocal.o();
    Vector3f d = rayLocal.d();

    //
    float a = d.x() * d.x() + d.y() * d.y() + d.z() * d.z();
    float b = 2.f * (d.x() * o.x() + d.y() * o.y() + d.z() * o.z());
    float c = o.x() * o.x() + o.y() * o.y() + o.z() * o.z() - _radius * _radius;

    float t0, t1;
    if (!SolveQuadratic(a, b, c, &t0, &t1)) {
        return false;
    }

    if (t0 > ray.tMax() || t1 < ray.tMin()) {
        return false;
    }

    float tHit = t0;
    if (tHit < ray.tMin()) {
        tHit = t1;
        if (tHit > ray.tMax()) {
            return false;
        }
    }

    //
    Point3f pHit = rayLocal(tHit);
    Normal3f nHit = Normalize(pHit - Point3f(0.f));
    float phi = Phi(pHit);
    float theta = std::acos(pHit.z() / _radius);

    //
    if (pHit.z() < _zMin || pHit.z() > _zMax || phi > _phiMax) {
        if (tHit == t1) {
            return false;
        }

        tHit = t1;

        if (tHit > ray.tMax()) {
            return false;
        }

        pHit = rayLocal(tHit);
        phi = Phi(pHit);
         
        if (pHit.z() < _zMin || pHit.z() > _zMax || phi > _phiMax) {
            return false;
        }
    }


    //
    ray.tMax() = tHit;

    //
    sp->p = ctx.ObjectToWorld(pHit);
    sp->wo = -ray.d();
    sp->ng = sp->ns = ctx.ObjectToWorld(nHit);
    sp->u = phi / _phiMax;
    sp->v = (theta - _thetaMin) / (_thetaMax - _thetaMin);

    return true;
}

bool Sphere::Occluded(const GeometryContext & ctx, const Ray3f & ray) const
{
    Ray3f rayLocal = ctx.WorldToObject(ray);
    Point3f o = rayLocal.o();
    Vector3f d = rayLocal.d();

    //
    float a = d.x() * d.x() + d.y() * d.y() + d.z() * d.z();
    float b = 2.f * (d.x() * o.x() + d.y() * o.y() + d.z() * o.z());
    float c = o.x() * o.x() + o.y() * o.y() + o.z() * o.z() - _radius * _radius;

    float t0, t1;
    if (!SolveQuadratic(a, b, c, &t0, &t1)) {
        return false;
    }

    if (t0 > ray.tMax() || t1 < ray.tMin()) {
        return false;
    }

    float tHit = t0;
    if (tHit < ray.tMin()) {
        tHit = t1;
        if (tHit > ray.tMax()) {
            return false;
        }
    }

    Point3f pHit = rayLocal(tHit);
    float phi = Phi(pHit);

    if (pHit.z() < _zMin || pHit.z() > _zMax || phi > _phiMax) {
        if (tHit == t1) {
            return false;
        }

        tHit = t1;

        if (tHit > ray.tMax()) {
            return false;
        }

        pHit = rayLocal(tHit);
        phi = Phi(pHit);
         
        if (pHit.z() < _zMin || pHit.z() > _zMax || phi > _phiMax) {
            return false;
        }
    }

    return true;
}

void Sphere::ComputeShadingInfo(const GeometryContext & ctx,
                                ShadingPoint * sp) const
{
    // Transform to object space
    Point3f pHit = ctx.WorldToObject(sp->p);
    Normal3f n = ctx.WorldToObject(sp->ng);

    // Recover spherical coordinates
    float phi = sp->u * _phiMax;
    float theta = sp->v * (_thetaMax - _thetaMin) + _thetaMin;
    float cosPhi = std::cos(phi);
    float sinPhi = std::sin(phi);
    float sinTheta = std::sin(theta);

    // Compute position derivatives
    Vector3f dpdu(-_phiMax * pHit.y(), _phiMax * pHit.x(), 0.f);
    Vector3f dpdv = (_thetaMax - _thetaMin) * 
        Vector3f(pHit.z() * cosPhi, pHit.z() * sinPhi, -_radius * sinTheta);

    // Compute second order derivatives
    Vector3f d2pdu2 = -_phiMax * _phiMax * Vector3f(pHit.x(), pHit.y(), 0.f);
    Vector3f d2pdudv = (_thetaMax - _thetaMin) * pHit.z() * _phiMax *
        Vector3f(-sinPhi, cosPhi, 0.f);
    Vector3f d2pdv2 = -(_thetaMax - _thetaMin) * (_thetaMax - _thetaMin) *
        Vector3f(pHit.x(), pHit.y(), pHit.z());

    // Calculate the first fundamental form at pHit
    float E = Dot(dpdu, dpdu);
    float F = Dot(dpdu, dpdv);
    float G = Dot(dpdv, dpdv);

    // Calculate the second fundamental form at pHit
    float L = Dot(n, d2pdu2);
    float M = Dot(n, d2pdudv);
    float N = Dot(n, d2pdv2);

    // Calculate normal derivatives
    float invEGFF = 1.f / (E * G - F * F);
    Normal3f dndu = (M * F - L * G) * invEGFF * dpdu +
                    (L * F - M * E) * invEGFF * dpdv;
    Normal3f dndv = (N * F - M * G) * invEGFF * dpdu +
                    (M * F - N * E) * invEGFF * dpdv;

    // Transform to world coordinates
    sp->dpdu = ctx.ObjectToWorld(dpdu);
    sp->dpdv = ctx.ObjectToWorld(dpdv);
    sp->dngdu = sp->dnsdu = ctx.ObjectToWorld(dndu);
    sp->dngdv = sp->dnsdv = ctx.ObjectToWorld(dndv);
}

ShadingPoint Sphere::Sample(const GeometryContext & ctx, Sampler & sampler,
                            float * pdf) const
{
    Point2f uv = sampler.Get2D();
    float phi = _phiMax * uv[0];
    float z = _zMin + (_zMax - _zMin) * uv[1];
    float r = std::sqrt(_radius * _radius - z * z);
    float theta = std::acos(z / _radius);

    Point3f p(r * std::cos(phi), r * std::sin(phi), z);
    Normal3f n = Normalize(Normal3f(p.x(), p.y(), p.z()));

    ShadingPoint sp;
    sp.p = ctx.ObjectToWorld(p);
    sp.ng = ctx.ObjectToWorld(n);
    sp.u = phi / _phiMax;
    sp.v = (theta - _thetaMin) / (_thetaMax - _thetaMin);

    // TODO: obtain scale from context
    *pdf = 1.f / (_phiMax * _radius * (_zMax - _zMin));

    return sp;
}

float Sphere::Pdf(const GeometryContext & ctx, const ShadingPoint & sp) const
{
    // TODO: obtain scale from context
    return 1.f / (_phiMax * _radius * (_zMax - _zMin));
}

Bounds3f Sphere::GetObjectBounds() const
{
    return Bounds3f(Point3f(-_radius, -_radius, _zMin),
                    Point3f(_radius, _radius, _zMax));
}

extern "C"
RENO_EXPORT
Geometry * CreateGeometry(const ParameterList & params)
{
    float defaultRadius = 1.f;
    float radius = params.GetFloat("radius", &defaultRadius);

    float defaultZMin = -radius;
    float zMin = params.GetFloat("zMin", &defaultZMin);

    float defaultZMax = radius;
    float zMax = params.GetFloat("zMax", &defaultZMax);

    float defaultPhiMax = TwoPi;
    float phiMax = params.GetFloat("phiMax", &defaultPhiMax);

    return new Sphere(radius, zMin, zMax, phiMax);
};

} // namespace renoster
