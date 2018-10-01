#ifndef RENOSTER_BVH_H_
#define RENOSTER_BVH_H_

#include <vector>

#include "renoster/bounds.h"
#include "renoster/geometry.h"
#include "renoster/matrix.h"
#include "renoster/position.h"
#include "renoster/ray.h"
#include "renoster/shading.h"
#include "renoster/util/allocator.h"
#include "renoster/util/tagged_pointer.h"
#include "renoster/util/vbool4.h"
#include "renoster/util/vfloat4.h"

namespace renoster {

using Matrix4x4v4f = Matrix<4, 4, vfloat4>;
using Bounds1v4f = Bounds<1, vfloat4>;
using Bounds3v4f = Bounds<3, vfloat4>;

using Point3vf4 = Point<3, vfloat4>;
using Vector3vf4 = Vector<3, vfloat4>;

struct TraversalRay {
    TraversalRay(const Ray3f & ray)
        : org(ray.o()),
        dir(ray.d()),
        time(ray.time()),
        tMin(ray.tMin()),
        tMax(ray.tMax()) {

        invDir = Vector3vf4(
                vfloat4(1.f / ray.d().x()),
                vfloat4(1.f / ray.d().y()),
                vfloat4(1.f / ray.d().z()));

        neg[0] = ray.d().x() < 0.f;
        neg[1] = ray.d().y() < 0.f;
        neg[2] = ray.d().z() < 0.f;
    }

    Point3vf4 org;
    Vector3vf4 dir;
    Vector3vf4 invDir;
    bool neg[3];
    vfloat4 time;
    vfloat4 tMin;
    vfloat4 tMax;
};

class RENO_API BVH {
public:
    struct BaseNode;
    struct AlignedNode;
    template <typename P> struct LeafNode;

    enum NodeType {
        kLeaf = 0x0001,
        kUnaligned = 0x0002,
        kMotion = 0x0004,
        kMotion4D = 0x0008
    };

    struct NodeRef {
    public:
        NodeRef() {}

        NodeRef(AlignedNode * node)
            : ptr_(AlignedNode::Type, node) {}

        template <typename Primitive>
        NodeRef(LeafNode<Primitive> * leaf)
            : ptr_(LeafNode<Primitive>::Type, leaf) {}

        uint16_t GetType() const { return ptr_.tag(); }

        BaseNode * GetBaseNode() const {
            return static_cast<BaseNode *>(ptr_.get());
        }

        template <typename Primitive>
        LeafNode<Primitive> * GetLeafNode() const {
            return static_cast<LeafNode<Primitive> *>(ptr_.get());
        }

    private:
        tagged_pointer ptr_;
    };

    struct BaseNode {
        vbool4 Intersect(uint16_t type, const TraversalRay & ray,
                         vfloat4 & dist) const;

        NodeRef children[4];
    };

    struct AlignedNode : public BaseNode {
        static constexpr uint16_t Type = 0;

        vbool4 Intersect(const TraversalRay & ray, vfloat4 & dist) const;

        Bounds3v4f bounds;
    };

    struct AlignedNodeMB : public BaseNode {
        static constexpr uint16_t Type = kMotion;

        vbool4 Intersect(const TraversalRay & ray, vfloat4 & dist) const;

        Bounds3v4f bounds0;
        Bounds3v4f bounds1;
    };

    struct AlignedNodeMB4D : public AlignedNodeMB {
        static constexpr uint16_t Type = kMotion4D;

        vbool4 Intersect(const TraversalRay & ray, vfloat4 & dist) const;

        Bounds1v4f timeBounds;
    };

    struct UnalignedNode : public BaseNode {
        static constexpr uint16_t Type = kUnaligned;

        vbool4 Intersect(const TraversalRay & ray, vfloat4 & dist) const;

        Matrix4x4v4f space;
    };

    struct UnalignedNodeMB : public BaseNode {
        static constexpr uint16_t Type = kUnaligned | kMotion;

        vbool4 Intersect(const TraversalRay & ray, vfloat4 & dist) const;

        Matrix4x4v4f space0;
        Bounds3v4f bounds1;
    };

    struct UnalignedNodeMB4D : public UnalignedNodeMB {
        static constexpr uint16_t Type = kUnaligned | kMotion4D;

        vbool4 Intersect(const TraversalRay & ray, vfloat4 & dist) const;

        Bounds1v4f timeBounds;
    };

    template <typename Primitive>
    struct LeafNode {
        static constexpr uint16_t Type = kLeaf;

        size_t numPrimitives;
        Primitive ** primitives;
    };

    BVH() = default;

    template <typename Primitive, typename PrimitiveContext>
    bool Intersect(const PrimitiveContext & ctx, const Ray3f & ray,
                   ShadingPoint * sp) const;

    template <typename Primitive, typename PrimitiveContext>
    bool Occluded(const PrimitiveContext & ctx, const Ray3f & ray) const;

    NodeRef _root;
    Allocator _alloc;
};

void TraverseNode(const BVH::BaseNode * node, vfloat4 vdist, vbool4 vmask,
                  BVH::NodeRef *& stackPtr);

void TraverseNodeOccluded(const BVH::BaseNode * node, vfloat4 vdist,
                          vbool4 vmask, BVH::NodeRef *& stackPtr);

template <typename Primitive, typename PrimitiveContext>
bool BVH::Intersect(const PrimitiveContext & ctx, const Ray3f & ray,
                    ShadingPoint * sp) const
{
    // Prepare ray for traversal
    TraversalRay travRay(ray);

    vfloat4 vdist;
    bool hit = false;
    BVH::NodeRef stack[64];
    BVH::NodeRef * stackPtr = stack + 1;
    stack[0] = _root;

    while (stackPtr != stack) {
        BVH::NodeRef cur = *(--stackPtr);

        uint16_t type = cur.GetType();
        if (type != BVH::LeafNode<Primitive>::Type) {
            // Get the node
            BVH::BaseNode * node = cur.GetBaseNode();

            // Intersect the ray with the node
            vbool4 vmask = node->Intersect(type, travRay, vdist);

            // Push the children on the stack
            TraverseNode(node, vdist, vmask, stackPtr);
        } else {
            //
            BVH::LeafNode<Primitive> * leaf = cur.GetLeafNode<Primitive>();

            //
            for (int i = 0; i < leaf->numPrimitives; ++i) {
                hit |= leaf->primitives[i]->Intersect(ctx, ray, sp);
            }
            
            //
            travRay.tMax = vfloat4(ray.tMax());
        }
    }

    return hit;
}

template <typename Primitive, typename PrimitiveContext>
bool BVH::Occluded(const PrimitiveContext & ctx, const Ray3f & ray) const
{
    // Prepare ray for traversal
    TraversalRay travRay(ray);

    vfloat4 vdist;
    BVH::NodeRef stack[64];
    BVH::NodeRef * stackPtr = stack + 1;
    stack[0] = _root;

    while (stackPtr != stack) {
        BVH::NodeRef cur = *(--stackPtr);

        uint16_t type = cur.GetType();
        if (type != BVH::LeafNode<Primitive>::Type) {
            // Get the node
            BVH::BaseNode * node = cur.GetBaseNode();

            // Intersect the ray with the node
            vbool4 vmask = node->Intersect(type, travRay, vdist);

            // Push the children on the stack
            TraverseNodeOccluded(node, vdist, vmask, stackPtr);
        } else {
            //
            auto * leaf = cur.GetLeafNode<Primitive>();

            //
            for (int i = 0; i < leaf->numPrimitives; ++i) {
                if(leaf->primitives[i]->Occluded(ctx, ray)) {
                    return true;
                }
            }
            
            //
            travRay.tMax = vfloat4(ray.tMax());
        }
    }

    return false;
}

}  // namespace renoster

#endif  // RENOSTER_BVH_H_
