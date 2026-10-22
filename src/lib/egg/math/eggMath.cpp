#include <lib/egg/math/eggMath.h>

#include <math.h>

namespace EGG {

template <typename T>
T Math<T>::sqrt(T t) {
    return ::sqrt(t);
}

template <typename T>
T Math<T>::sin(T t) {
    return ::sin(t);
}

template <typename T>
T Math<T>::cos(T t) {
    return ::cos(t);
}

template <typename T>
T Math<T>::acos(T t) {
    return ::acos(t);
}

template <typename T>
T Math<T>::atan2(T y, T x) {
    return ::atan2(y, x);
}

template <typename T>
T Math<T>::log10(T t) {
    return ::log10(t);
}

template class Math<f32>;

} // namespace EGG
