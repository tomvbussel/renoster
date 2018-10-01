#ifndef RENOSTER_UTIL_TAGGED_POINTER_H_
#define RENOSTER_UTIL_TAGGED_POINTER_H_

#include <cassert>

namespace renoster {

class tagged_pointer
{
public:
    tagged_pointer()
        : _data(0)
    {
    }

    tagged_pointer(void * ptr)
    {
        reset(ptr);
    }

    tagged_pointer(uint16_t tag, void * ptr)
    {
        set(tag, ptr);
    }

    uint16_t tag() const noexcept
    {
        return _data >> 48;
    }

    void update(uint16_t tag) noexcept
    {
        set(tag, get());
    }

    void * get() const noexcept
    {
        return reinterpret_cast<void *>(_data & ((1ULL << 48) - 1));
    }

    void reset(void * ptr = nullptr) noexcept
    {
        set(tag(), ptr);
    }

private:
    void set(uint16_t tag, void * ptr) noexcept
    {
        _data = reinterpret_cast<uintptr_t>(ptr);
        assert((_data & (1ULL << 47)) == 0);
        _data |= static_cast<uintptr_t>(tag) << 48;
    }

    uintptr_t _data;
};

} // namespace renoster

#endif // RENOSTER_UTIL_TAGGED_POINTER_H_
