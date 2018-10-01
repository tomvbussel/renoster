#ifndef RENOSTER_FILTERTABLE_H_
#define RENOSTER_FILTERTABLE_H_

#include <vector>

#include "renoster/export.h"
#include "renoster/pixelfilter.h"
#include "renoster/point.h"
#include "renoster/sampling.h"
#include "renoster/vector.h"

namespace renoster {

class RENO_API FilterTable
{
public:
    FilterTable(const PixelFilter * filter, int tableSize);

    float Evaluate(const Point2f & p) const;

    float Sample(Point2f & p, const Point2f & uv, float * pdf) const;

private:
    const PixelFilter * _filter;
    int _tableSize;
    Vector2f _radius;
    Vector2f _invRadius;
    std::vector<float> _table;
    Distribution2D _pTable;
};

} // namespace renoster

#endif // RENOSTER_FILTERTABLE_H_
