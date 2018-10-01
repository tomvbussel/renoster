#ifndef RENOSTER_TYPES_H_
#define RENOSTER_TYPES_H_

#include <type_traits>

namespace renoster {

template <typename T>
struct remove_cvref {
    using type = std::remove_cv_t<std::remove_reference_t<T>>;
};

template <typename T>
using remove_cvref_t = typename remove_cvref<T>::type;

}  // namespace renoster

#endif  // RENOSTER_TYPES_H_