#ifndef RENOSTER_ACCEL_BUILDER_H_
#define RENOSTER_ACCEL_BUILDER_H_

#include "renoster/bounds.h"
#include "renoster/bvh.h"
#include "renoster/mathutil.h"
#include "renoster/util/span.h"

namespace renoster {

template <typename Primitive,
          typename Splitter>
class BVHBuilder {
public:
    using Split = typename Splitter::Split;

    struct PrimInfo {
        Bounds3f bounds;
        Point3f centroid;
        Primitive * prim;
    };

    struct BuildRecord {
        using PrimInfo = BVHBuilder::PrimInfo;

        Bounds3f bounds;
        Bounds3f centBounds;
        span<PrimInfo> primInfo;
    };

    BVHBuilder(BVH * bvh, const Splitter & splitter, size_t minLeafSize)
        : _bvh(bvh),
        _splitter(splitter),
        _minLeafSize(minLeafSize) {}

    template <typename PrimitiveContext>
    void Build(const PrimitiveContext & ctx, const span<Primitive *> & prims) {
        // Create PrimInfo
        std::vector<PrimInfo> primInfo(prims.size());
        for (size_t i = 0; i < prims.size(); ++i) {
            primInfo[i].bounds = prims[i]->GetWorldBounds(ctx);
            primInfo[i].centroid = primInfo[i].bounds.Center();
            primInfo[i].prim = prims[i];
        }

        // Create initial BuildRecord
        BuildRecord bRec;
        for (size_t i = 0; i < prims.size(); ++i) {
            bRec.bounds.ExpandBy(primInfo[i].bounds);
            bRec.centBounds.ExpandBy(primInfo[i].bounds.Center());
        }
        bRec.primInfo = span<PrimInfo>(primInfo);

        _bvh->_root = BuildRecursive(bRec);
    }

    BVH::NodeRef BuildRecursive(const BuildRecord & cur) {
        // Build a leaf
        if (cur.primInfo.size() <= _minLeafSize) {
            return CreateLeaf(cur);
        }

        // Split into 4 children
        BuildRecord children[4];
        children[0] = cur;
        size_t numChildren = 1;

        while (numChildren < 4) {
            // Find the child to split
            float worstArea = -Infinity;
            size_t worstChild = -1;
            for (size_t i = 0; i < numChildren; ++i) {
                if (children[i].primInfo.size() <= _minLeafSize) {
                    continue;
                }

                float childArea = children[i].bounds.SurfaceArea();
                if (childArea > worstArea) {
                    worstArea = childArea;
                    worstChild = i;
                }
            }

            if (worstChild == -1) {
                break;
            }

            // Find the best split
            Split split = _splitter.Find(children[worstChild]);

            // Perform the split
            BuildRecord left;
            BuildRecord right;
            _splitter.PerformSplit(children[worstChild], split, left, right);
            children[worstChild] = left;
            children[numChildren] = right;
            numChildren++;
        }

        if (numChildren == 1) {
            return CreateLeaf(children[0]);
        }

        // Create the new node
        BVH::NodeRef nodeRef = CreateNode(children);
        BVH::BaseNode * node = nodeRef.GetBaseNode();

        // Recurse
        for (size_t i = 0; i < numChildren; ++i) {
            node->children[i] = BuildRecursive(children[i]);
        }

        return nodeRef;
    }

    BVH::NodeRef CreateNode(BuildRecord children[4]) {
        BVH::AlignedNode * node = _bvh->_alloc.New<BVH::AlignedNode>();
        for (size_t i = 0; i < 4; ++i) {
            for (size_t d = 0; d < 3; ++d) {
                node->bounds.min()[d][i] = children[i].bounds.min()[d];
                node->bounds.max()[d][i] = children[i].bounds.max()[d];
            }
        }
        return BVH::NodeRef(node);
    }

    BVH::NodeRef CreateLeaf(const BuildRecord & rec) {
        BVH::LeafNode<Primitive> * leaf =
            _bvh->_alloc.New<BVH::LeafNode<Primitive>>();
        leaf->numPrimitives = rec.primInfo.size();
        leaf->primitives = static_cast<Primitive **>(
            _bvh->_alloc.Alloc(sizeof(Primitive *) * leaf->numPrimitives));
        for (size_t i = 0; i < leaf->numPrimitives; ++i) {
            leaf->primitives[i] = rec.primInfo[i].prim;
        }
        return BVH::NodeRef(leaf);
    }

private:
    BVH * _bvh;
    const Splitter & _splitter;
    size_t _minLeafSize;
};

} // namespace renoster

#endif // RENOSTER_ACCEL_BUILDER_H_
