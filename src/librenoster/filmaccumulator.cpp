#include "renoster/filmaccumulator.h"

namespace renoster {

FilmAccumulator::FilmAccumulator()
{
}

void FilmAccumulator::AddSample(const Color & value)
{
    value_ += value;
}

void FilmAccumulator::WriteValue(const Color & value)
{
    value_ = value;
}

void FilmAccumulator::GetValue(Color & value) const
{
    value = value_;
}

void FilmAccumulator::Reset()
{
    value_ = Color(0.f);
}

} // namespace renoster
