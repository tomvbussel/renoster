#ifndef RENOSTER_SHADING_H_
#define RENOSTER_SHADING_H_

#include "renoster/normal.h"
#include "renoster/point.h"
#include "renoster/vector.h"
#include "renoster/util/allocator.h"

namespace renoster {

class BSDF;
class Primitive;

class ShadingPoint {
public:
    ShadingPoint() = default;

    void ComputeShadingInfo();

    void ComputeScatteringFunctions(Allocator & alloc);

    // Surface position
    Point3f p;
    Vector3f dpdx, dpdy;

    // Viewing direction
    Vector3f wo;

    // Geometric normal
    Normal3f ng;

    // Shading normal
    Normal3f ns;

    // Surface parametrization
    float u;
    float dudx, dudy;
    float v;
    float dvdx, dvdy;

    // Surface tangents
    Vector3f dpdu, dpdv;

    // Normal derivatives
    Normal3f dngdu, dngdv;
    Normal3f dnsdu, dnsdv;

    // Time
    float time;

    // Hit object and face
    const Primitive * primitive = nullptr;
    int face = 0;

    // Material
    BSDF * bsdf = nullptr;
};

}  // namespace renoster

#endif  // RENOSTER_SHADING_H_
