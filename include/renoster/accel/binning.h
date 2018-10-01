#ifndef RENOSTER_ACCEL_BINNING_H_
#define RENOSTER_ACCEL_BINNING_H_

#include <cstdlib>

#include "renoster/mathutil.h"

namespace renoster {

class BinMapping {
public:
    static constexpr int NumBins = 16;

    BinMapping(const Bounds3f & centBounds) {
        offset_ = centBounds.min();
        Vector3f diag = centBounds.Diagonal();
        float scaleX = (diag.x() > Epsilon) ? NumBins / diag.x() : 0.f;
        float scaleY = (diag.y() > Epsilon) ? NumBins / diag.y() : 0.f;
        float scaleZ = (diag.z() > Epsilon) ? NumBins / diag.z() : 0.f;
        scale_ = Vector3f(scaleX, scaleY, scaleZ);
    }

    Point3i Bin(const Point3f & cent) const {
        int binX = std::floor((cent.x() - offset_.x()) * scale_.x());
        int binY = std::floor((cent.y() - offset_.y()) * scale_.y());
        int binZ = std::floor((cent.z() - offset_.z()) * scale_.z());
        return Point3i(std::min(binX, NumBins - 1),
                       std::min(binY, NumBins - 1),
                       std::min(binZ, NumBins - 1));
    }

private:
    Point3f offset_;
    Vector3f scale_;
};

struct BinSplit {
    BinSplit(const BinMapping & mapping)
        : mapping(mapping) {}

    BinMapping mapping;
    float sah = Infinity;
    size_t leftCount = 0, rightCount = 0;
    size_t dim = -1;
    size_t pos = -1;
};

class BinInfo {
public:
    static constexpr int NumBins = BinMapping::NumBins;

    template <typename BuildRecord>
    BinInfo(const BuildRecord & cur)
            : mapping_(cur.centBounds) {

        using PrimInfo = typename BuildRecord::PrimInfo;

        for (size_t b = 0; b < NumBins; ++b) {
            for (size_t d = 0; d < 3; ++d) {
                bounds_[b][d] = Bounds3f();
                counts_[b][d] = 0;
            }
        }

        for (const PrimInfo & info : cur.primInfo) {
            Point3i bin = mapping_.Bin(info.centroid);
            for (size_t d = 0; d < 3; ++d) {
                bounds_[bin[d]][d].ExpandBy(info.centroid);
                counts_[bin[d]][d] += 1;
            }
        }
    }

    BinSplit BestSplit() {
        size_t rightCounts[3][BinInfo::NumBins]{};

        BinSplit split(mapping_);
        for (size_t d = 0; d < 3; ++d) {
            float leftSAH[NumBins]{};
            Bounds3f leftBounds;
            size_t leftCount = 0;
            size_t leftCounts[NumBins]{};
            for (size_t b = 0; b < NumBins; ++b) {
                leftBounds.ExpandBy(bounds_[b][d]);
                leftCount += counts_[b][d];
                leftCounts[b] = leftCount;
                leftSAH[b] = leftCounts[b] * leftBounds.SurfaceArea();
            }

            float rightSAH[NumBins]{};
            Bounds3f rightBounds;
            size_t rightCount = 0;
            size_t rightCounts[NumBins]{};
            for (size_t b = NumBins; b-- > 0;) {
                rightBounds.ExpandBy(bounds_[b][d]);
                rightCount += counts_[b][d];
                rightCounts[b] = rightCount;
                rightSAH[b] = rightCounts[b] * rightBounds.SurfaceArea();
            }

            for (size_t p = 0; p < NumBins - 1; ++p) {
                float splitSAH = leftSAH[p] + rightSAH[p + 1];
                if (splitSAH < split.sah) {
                    split.dim = d;
                    split.pos = p;
                    split.sah = splitSAH;
                    split.leftCount = leftCounts[p];
                    split.rightCount = rightCounts[p + 1];
                }
            }
        }       

        return split;
    }

private:
    Bounds3f bounds_[NumBins][3];
    size_t counts_[NumBins][3];
    BinMapping mapping_;
};

}  // namespace renoster

#endif  // RENOSTER_ACCEL_BINNING_H_
