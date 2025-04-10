#include "util/cache.h"

#include "util/assert.h"
#include "util/math.h"

namespace mixxx {

cache_key_t cacheKeyFromMessageDigest(const QByteArray& messageDigest) {
    cache_key_t key = invalidCacheKey();
    DEBUG_ASSERT(!isValidCacheKey(key));
    if (messageDigest.isEmpty()) {
        return key;
    }
    // Source: FIPS 180-4 Secure Hash Standard (SHS)
    // SP 800-107
    // 5 Hash function Usage
    // 5.1 Truncated Message Digest
    const int significantByteCount = math_min(
            static_cast<int>(messageDigest.size()),
            static_cast<int>(sizeof(cache_key_t)));
    for (int i = 0; i < significantByteCount; ++i) {
        // Only 8 bits are relevant and we don't want the sign
        // extension of a (signed) char during the conversion.
        const cache_key_t nextByte =
                static_cast<unsigned char>(messageDigest.at(i));
        DEBUG_ASSERT(nextByte == (nextByte & 0xFF));
        key <<= 8;
        key |= nextByte;
    }
    if (!isValidCacheKey(key)) {
        // Unlikely but possible
        key = ~key;
    }
    DEBUG_ASSERT(isValidCacheKey(key));
    return key;
}

} // namespace mixxx
