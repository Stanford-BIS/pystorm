#include <stdexcept>

///
/// Throw an exception if value is not in range
///
template<typename T>
void ValueIsInRange(T value, T min, T max, const char * msg) {
    if ((value < min) || (value > max)) {
        throw std::out_of_range(msg);
    }
}
