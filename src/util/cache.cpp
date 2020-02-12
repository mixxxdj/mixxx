#include "util/cache.h"

#include "util/assert.h"

namespace mixxx {

cache_key_t calculateCacheKey(const QByteArray& bytes) {
    cache_key_t key = invalidCacheKey();
    DEBUG_ASSERT(!isValidCacheKey(key));
    if (bytes.isEmpty()) {
        return key;
    }
    // We should not make any assumptions about the significance of
    // the given bytes, even if they are supposed to be the result of
    // applying a hash function. Instead of truncating the information
    // by using only the first sizeof(cache_key_t) bytes we slice the
    // array into groups of sizeof(cache_key_t) bytes and combine them
    // with XOR.
    size_t leftShiftBytes = 0;
    for (const auto nextByte : bytes) {
        // Only 8 bits are relevant and we don't want the sign
        // extension of a (signed) char during conversion.
        const cache_key_t nextBits =
                static_cast<unsigned char>(nextByte);
        DEBUG_ASSERT(nextBits == (nextBits & static_cast<cache_key_t>(0xff)));
        key ^= nextBits << (leftShiftBytes * 8);
        leftShiftBytes = (leftShiftBytes + 1) % sizeof(cache_key_t);
    }
    if (!isValidCacheKey(key)) {
        // Unlikely but possible
        key = ~key;
    }
    DEBUG_ASSERT(isValidCacheKey(key));
    return key;
}

} // namespace mixxx
