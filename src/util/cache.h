#pragma once

#include <QByteArray>
#include <QMetaType>

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

// Transforms a byte array containing a hash result into an
// unsigned integer number that should be almost unique.
cache_key_t calculateCacheKey(const QByteArray& bytes);

} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::cache_key_t);
