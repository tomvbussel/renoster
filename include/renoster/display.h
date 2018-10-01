#ifndef RENOSTER_DISPLAY_H_
#define RENOSTER_DISPLAY_H_

#include <memory>

#include "renoster/export.h"
#include "renoster/paramlist.h"
#include "renoster/vector.h"

namespace renoster {

class RENO_API Display {
public:
    virtual bool Open(const Vector2i & resolution) = 0;

    virtual bool WriteData(float * pixels) = 0;

    virtual bool Close() = 0;

    virtual std::string GetError() = 0;

    virtual ~Display() {}
};

RENO_API std::unique_ptr<Display> CreateDisplay(const std::string & name,
                                                ParameterList & params);

}  // namespace renoster

#endif  // RENOSTER_DISPLAY_H_
