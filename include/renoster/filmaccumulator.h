#ifndef RENOSTER_FILMACCUMULATOR_H_
#define RENOSTER_FILMACCUMULATOR_H_

#include "renoster/color.h"
#include "renoster/export.h"

namespace renoster {

class RENO_API FilmAccumulator {
public:
    FilmAccumulator();

    void AddSample(const Color & value);

    void WriteValue(const Color & value);

    void GetValue(Color & value) const;

    void Reset();

private:
    Color value_;
};

} // namespace renoster

#endif // RENOSTER_FILMACCUMULATOR_H_
