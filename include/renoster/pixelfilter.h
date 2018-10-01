#ifndef RENOSTER_PIXELFILTER_H_
#define RENOSTER_PIXELFILTER_H_

#include <memory>

#include "renoster/export.h"
#include "renoster/paramlist.h"
#include "renoster/point.h"
#include "renoster/vector.h"

namespace renoster {

class RENO_API PixelFilter {
public:
    virtual ~PixelFilter() = default;

    virtual float Evaluate(const Point2f & p) const = 0;

    virtual Vector2f GetRadius() const = 0;
};

RENO_API std::unique_ptr<PixelFilter> CreatePixelFilter(
        const std::string & name, ParameterList & params);

}  // namespace renoster

#endif  // RENOSTER_PIXELFILTER_H_
