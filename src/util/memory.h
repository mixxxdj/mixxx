#ifndef MIXXX_UTIL_MEMORY_H
#define MIXXX_UTIL_MEMORY_H

#include <memory>

// Temporary workaround for the missing make_unique() template function
// in C++11 that will be available in C++14. Please note that this
// simplified version does not work for arrays!
namespace std {
    template<typename T, typename ...Args>
    inline unique_ptr<T> make_unique(Args&& ...args) {
        return unique_ptr<T>(new T(forward<Args>(args) ...));
    }
}

#endif // MIXXX_UTIL_MEMORY_H
