#ifndef RENOSTER_ACCEL_SPLITTER_H_
#define RENOSTER_ACCEL_SPLITTER_H_

#include <algorithm>

#include "renoster/accel/binning.h"

namespace renoster {

class ObjectSplitter {
public:
    using Split = BinSplit;

    ObjectSplitter() = default;

    template <typename BuildRecord>
    Split Find(BuildRecord & cur) const {
        Vector3f d = cur.centBounds.Diagonal();
        BinInfo binner(cur);
        return binner.BestSplit();
    }

    template <typename BuildRecord>
    void PerformSplit(const BuildRecord & cur, const Split & split,
                      BuildRecord & left, BuildRecord & right) const {

        using PrimInfo = typename BuildRecord::PrimInfo;

        //
        auto mid = std::partition(cur.primInfo.begin(), cur.primInfo.end(),
                                  [&split](const PrimInfo & info) {
            int bin = split.mapping.Bin(info.centroid)[split.dim];
            return bin <= split.pos;
        });

        //
        left.primInfo = span<PrimInfo>(cur.primInfo.begin(), mid);
        for (const PrimInfo & info : left.primInfo) {
            left.bounds.ExpandBy(info.bounds);
            left.centBounds.ExpandBy(info.centroid);
        }

        //
        right.primInfo = span<PrimInfo>(mid, cur.primInfo.end());
        for (const PrimInfo & info : right.primInfo) {
            right.bounds.ExpandBy(info.bounds);
            right.centBounds.ExpandBy(info.centroid);
        }
    }
};

} // namespace renoster

#endif // RENOSTER_ACCEL_SPLITTER_H_
