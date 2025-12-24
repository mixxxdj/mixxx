#pragma once

#include <QByteArray>
#include <QMetaType>
#include <type_traits> // static_assert

namespace mixxx {

typedef quint64 cache_key_t;

// A signed integer is needed for storing cache keys as
// integer numbers with full precision in the database.
typedef qint64 cache_key_signed_t;

inline constexpr cache_key_t invalidCacheKey() {
    return 0;
}

inline constexpr bool isValidCacheKey(cache_key_t key) {
    return key != invalidCacheKey();
}

inline constexpr cache_key_t signedCacheKey(cache_key_t cacheKey) {
    static_assert(
            sizeof(cache_key_t) == sizeof(cache_key_signed_t),
            "size mismatch of signed/unsigned cache key types");
    return static_cast<cache_key_signed_t>(cacheKey);
}

// Truncate a message digest to obtain a cache key.
//
// The message digest should either be empty or contain at least as many
// bytes as the size (in bytes) of the cache key.
cache_key_t cacheKeyFromMessageDigest(const QByteArray& messageDigest);

} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::cache_key_t);
