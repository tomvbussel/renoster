#ifndef RENOSTER_RNG_H_
#define RENOSTER_RNG_H_

namespace renoster {

static constexpr uint64_t DefaultState = 0x853c49e6748fea9bULL;
static constexpr uint64_t DefaultStream = 0xda3e39cb94b95bdbULL;
static constexpr uint64_t Multiplier = 0x5851f42d4c957f2dULL;
static constexpr float OneMinusEpsilon = 0x1.fffffep-1;

class RNG {
public:
    RNG() : _state(DefaultState), _inc(DefaultStream) {}

    void Seed(uint32_t initSeq) {
        _state = 0ULL;
        _inc = (initSeq << 1ULL) | 1ULL;
        UniformUInt32();
        _state += DefaultState;
        UniformUInt32();
    }

    uint32_t UniformUInt32() {
        uint64_t oldState = _state;
        _state = oldState * Multiplier + _inc;
        uint32_t xorShifted = ((oldState >> 18u) ^ oldState) >> 27u;
        uint32_t rot = oldState >> 59u;
        return (xorShifted >> rot) | (xorShifted << ((~rot + 1u) & 31));
    }

    float UniformFloat() {
        return std::min(UniformUInt32() * 0x1p-32f, OneMinusEpsilon);
    };

private:
    uint64_t _state;
    uint64_t _inc;
};

}  // namespace renoster

#endif  // RENOSTER_RNG_H_