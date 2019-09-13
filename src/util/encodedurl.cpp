#include "util/encodedurl.h"

#include "util/assert.h"

namespace mixxx {

namespace {

constexpr QUrl::FormattingOptions kFormattingOptions =
        QUrl::StripTrailingSlash |
        QUrl::NormalizePathSegments;

constexpr QUrl::ParsingMode kParsingMode =
        QUrl::StrictMode;

} // anonymous namespace

//static
EncodedUrl EncodedUrl::fromQUrl(const QUrl& url) {
    if (url.isEmpty()) {
        return EncodedUrl{};
    }
    VERIFY_OR_DEBUG_ASSERT(url.isValid()) {
        return EncodedUrl{};
    }
    return fromEncodedQByteArray(url.toEncoded(kFormattingOptions));
}

//static
EncodedUrl EncodedUrl::fromQUrlWithTrailingSlash(const QUrl& url) {
    if (url.isEmpty()) {
        return EncodedUrl{};
    }
    VERIFY_OR_DEBUG_ASSERT(url.isValid()) {
        return EncodedUrl{};
    }
    auto encoded = url.toEncoded(kFormattingOptions);
    if (encoded.isEmpty() || encoded.back() != '/') {
        encoded.append('/');
    }
    return fromEncodedQByteArray(std::move(encoded));
}

QUrl EncodedUrl::toQUrl() const {
    return QUrl::fromEncoded(m_urlEncoded, kParsingMode);
}

} // namespace mixxx
