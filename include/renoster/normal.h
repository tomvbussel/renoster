#ifndef RENOSTER_MATH_NORMAL_H_
#define RENOSTER_MATH_NORMAL_H_

#include "renoster/vector.h"

namespace renoster {

template <size_t D, typename T>
class Normal : public Vector<D, T> {
public:
    Normal() = default;

    Normal(const Normal &) = default;
    Normal & operator=(const Normal &) = default;

    Normal(Normal &&) = default;
    Normal & operator=(Normal &&) = default;

    explicit Normal(T a) : Vector<D, T>(a) {}

    template <typename... Ts>
    Normal(T a, T b, Ts... ts) : Vector<D, T>(a, b, ts...) {}

    template <typename S>
    Normal(const Vector<D, S> & v) : Vector<D, T>(v) {}

    template <typename S>
    Normal & operator=(const Vector<D, S> & v) {
        Vector<D, T>::operator=(v);
        return *this;
    }
};

using Normal3f = Normal<3, float>;

}  // namespace renoster

#endif  // RENOSTER_MATH_NORMAL_H_