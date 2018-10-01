#include "renoster/bvh.h"

#include <iostream>

#include "renoster/log.h"
#include "renoster/mathutil.h"

namespace renoster {

namespace {

// TODO: move this somewhere better
inline size_t CountTrailingZeros(size_t mask) { return __builtin_ctz(mask); }

} // anonymous namespace

void TraverseNode(const BVH::BaseNode * node, vfloat4 vdist, vbool4 vmask,
                  BVH::NodeRef *& stackPtr)
{
    size_t mask = MoveMask(vmask);

    // 0 hits
    if (mask == 0) {
        return;
    }

    // 1 hit
    size_t i0 = CountTrailingZeros(mask);
    mask &= mask - 1;
    BVH::NodeRef c0 = node->children[i0];
    if (mask == 0) {
        *(stackPtr++) = c0;
        return;
    }

    // 2 hits
    size_t i1 = CountTrailingZeros(mask);
    mask &= mask - 1;
    BVH::NodeRef c1 = node->children[i1];
    if (vdist[i1] < vdist[i0]) {
        std::swap(i0, i1);
        std::swap(c0, c1);
    }
    if (mask == 0) {
        *(stackPtr++) = c1;
        *(stackPtr++) = c0;
        return;
    }

    // 3 hits
    size_t i2 = CountTrailingZeros(mask);
    mask &= mask - 1;
    BVH::NodeRef c2 = node->children[i2];
    if (vdist[i2] < vdist[i1]) {
        std::swap(i1, i2);
        std::swap(c1, c2);
    }
    if (vdist[i1] < vdist[i0]) {
        std::swap(i0, i1);
        std::swap(c0, c1);
    }
    if (mask == 0) {
        *(stackPtr++) = c2;
        *(stackPtr++) = c1;
        *(stackPtr++) = c0;
        return;
    }

    // 4 hits
    size_t i3 = CountTrailingZeros(mask);
    mask &= mask - 1;
    BVH::NodeRef c3 = node->children[i3];
    if (vdist[i3] < vdist[i2]) {
        std::swap(i2, i3);
        std::swap(c2, c3);
    }
    if (vdist[i2] < vdist[i1]) {
        std::swap(i1, i2);
        std::swap(c1, c2);
    }
    if (vdist[i1] < vdist[i0]) {
        std::swap(i0, i1);
        std::swap(c0, c1);
    }
    *(stackPtr++) = c3;
    *(stackPtr++) = c2;
    *(stackPtr++) = c1;
    *(stackPtr++) = c0;
}

void TraverseNodeOccluded(const BVH::BaseNode * node, vfloat4 vdist,
                          vbool4 vmask, BVH::NodeRef *& stackPtr)
{
    size_t mask = MoveMask(vmask);

    if (mask == 0) {
        return;
    }

    while(true) {
        size_t i = CountTrailingZeros(mask);
        mask &= mask - 1;
        *(stackPtr++) = node->children[i];
        if (mask == 0) {
            return;
        }
    }
}

vbool4 BVH::BaseNode::Intersect(uint16_t type, const TraversalRay & ray,
                                vfloat4 & dist) const
{
    switch (type) {
    case BVH::AlignedNode::Type:
        return static_cast<const BVH::AlignedNode *>(this)
                ->Intersect(ray, dist);

    case BVH::AlignedNodeMB::Type:
        return static_cast<const BVH::AlignedNodeMB *>(this)
                ->Intersect(ray, dist);

    case BVH::AlignedNodeMB4D::Type:
        return static_cast<const BVH::AlignedNodeMB4D *>(this)
                ->Intersect(ray, dist);

    case BVH::UnalignedNode::Type:
        return static_cast<const BVH::UnalignedNode *>(this)
                ->Intersect(ray, dist);

    case BVH::UnalignedNodeMB::Type:
        return static_cast<const BVH::UnalignedNodeMB *>(this)
                ->Intersect(ray, dist);

    case BVH::UnalignedNodeMB4D::Type:
        return static_cast<const BVH::UnalignedNodeMB4D *>(this)
                ->Intersect(ray, dist);

    default:
        Error("BVH::BaseNode::Intersect()");
        return vbool4(false);
    }
}

namespace {

vbool4 IntersectBounds(const TraversalRay & ray, const Bounds3v4f & bounds,
                       vfloat4 & dist)
{
    const vfloat4 & pMinX = bounds[ray.neg[0]].x();
    const vfloat4 & pMinY = bounds[ray.neg[1]].y();
    const vfloat4 & pMinZ = bounds[ray.neg[2]].z();
    vfloat4 tMinX = (pMinX - ray.org.x()) * ray.invDir.x();
    vfloat4 tMinY = (pMinY - ray.org.y()) * ray.invDir.y();
    vfloat4 tMinZ = (pMinZ - ray.org.z()) * ray.invDir.z();

    const vfloat4 & pMaxX = bounds[1 - ray.neg[0]].x();
    const vfloat4 & pMaxY = bounds[1 - ray.neg[1]].y();
    const vfloat4 & pMaxZ = bounds[1 - ray.neg[2]].z();
    vfloat4 tMaxX = (pMaxX - ray.org.x()) * ray.invDir.x();
    vfloat4 tMaxY = (pMaxY - ray.org.y()) * ray.invDir.y();
    vfloat4 tMaxZ = (pMaxZ - ray.org.z()) * ray.invDir.z();

    vfloat4 tMin = Max(Max(tMinX, tMinY), Max(tMinZ, ray.tMin));
    vfloat4 tMax = Min(Min(tMaxX, tMaxY), Min(tMaxZ, ray.tMax));

    dist = tMin;

    return tMin <= tMax;
}

}  // anonymous namespace

vbool4 BVH::AlignedNode::Intersect(const TraversalRay & ray,
                                   vfloat4 & dist) const
{
    return IntersectBounds(ray, bounds, dist);
}

vbool4 BVH::AlignedNodeMB::Intersect(const TraversalRay & ray,
                                     vfloat4 & dist) const
{
    Bounds3v4f bounds = Lerp(bounds0, bounds1, ray.time);
    return IntersectBounds(ray, bounds, dist);
}

vbool4 BVH::AlignedNodeMB4D::Intersect(const TraversalRay & ray,
                                       vfloat4 & dist) const
{
    vbool4 mask = BVH::AlignedNodeMB::Intersect(ray, dist);
    mask &= timeBounds.min().x() <= ray.time;
    mask &= ray.time <= timeBounds.max().x();
    return mask;
}

vbool4 BVH::UnalignedNode::Intersect(const TraversalRay & ray,
                                     vfloat4 & dist) const
{
    Point3vf4 org = space * ray.org;
    Vector3vf4 dir = space * ray.dir;
    Vector3vf4 invDir(vfloat4(1.f) / dir.x(), vfloat4(1.f) / dir.y(),
                      vfloat4(1.f) / dir.z());

    vfloat4 tLowerX = -org.x() * invDir.x();
    vfloat4 tLowerY = -org.y() * invDir.y();
    vfloat4 tLowerZ = -org.z() * invDir.z();
    vfloat4 tUpperX = tLowerX + invDir.x();
    vfloat4 tUpperY = tLowerY + invDir.y();
    vfloat4 tUpperZ = tLowerZ + invDir.z();

    vfloat4 tMinX = Min(tLowerX, tUpperX);
    vfloat4 tMinY = Min(tLowerY, tUpperY);
    vfloat4 tMinZ = Min(tLowerZ, tUpperZ);
    vfloat4 tMaxX = Max(tLowerX, tUpperX);
    vfloat4 tMaxY = Max(tLowerY, tUpperY);
    vfloat4 tMaxZ = Max(tLowerZ, tUpperZ);

    vfloat4 tMin = Max(Max(tMinX, tMinY), Max(tMinZ, ray.tMin));
    vfloat4 tMax = Min(Min(tMaxX, tMaxY), Min(tMaxZ, ray.tMax));

    dist = tMin;

    return tMin <= tMax;
}

static const Bounds3v4f bounds0(Point3vf4(vfloat4(0.f)),
                                Point3vf4(vfloat4(1.f)));

vbool4 BVH::UnalignedNodeMB::Intersect(const TraversalRay & ray,
                                       vfloat4 & dist) const
{
    Bounds3v4f bounds = Lerp(bounds0, bounds1, ray.time);

    Point3vf4 org = space0 * ray.org;
    Vector3vf4 dir = space0 * ray.dir;
    Vector3vf4 invDir(vfloat4(1.f) / dir.x(), vfloat4(1.f) / dir.y(),
                      vfloat4(1.f) / dir.z());

    vfloat4 tLowerX = (bounds.min().x() - org.x()) * invDir.x();
    vfloat4 tLowerY = (bounds.min().y() - org.y()) * invDir.y();
    vfloat4 tLowerZ = (bounds.min().z() - org.z()) * invDir.z();
    vfloat4 tUpperX = (bounds.max().x() - org.x()) * invDir.x();
    vfloat4 tUpperY = (bounds.max().y() - org.y()) * invDir.y();
    vfloat4 tUpperZ = (bounds.max().z() - org.z()) * invDir.z();

    vfloat4 tMinX = Min(tLowerX, tUpperX);
    vfloat4 tMinY = Min(tLowerY, tUpperY);
    vfloat4 tMinZ = Min(tLowerZ, tUpperZ);
    vfloat4 tMaxX = Max(tLowerX, tUpperX);
    vfloat4 tMaxY = Max(tLowerY, tUpperY);
    vfloat4 tMaxZ = Max(tLowerZ, tUpperZ);

    vfloat4 tMin = Max(Max(tMinX, tMinY), Max(tMinZ, ray.tMin));
    vfloat4 tMax = Min(Min(tMaxX, tMaxY), Min(tMaxZ, ray.tMax));

    dist = tMin;

    return tMin <= tMax;
}

vbool4 BVH::UnalignedNodeMB4D::Intersect(const TraversalRay & ray,
                                         vfloat4 & dist) const
{
    vbool4 mask = BVH::UnalignedNodeMB4D::Intersect(ray, dist);
    mask &= timeBounds.min().x() <= ray.time;
    mask &= ray.time <= timeBounds.max().x();
    return mask;
}

}  // namespace renoster
