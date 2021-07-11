#pragma once

#include <QByteArray>
#include <QUrl>
#include <QtDebug>

namespace mixxx {

// A URL-encoded QUrl
class EncodedUrl final {
  public:
    static EncodedUrl fromEncodedQByteArray(QByteArray&& urlEncoded) {
        return EncodedUrl(std::move(urlEncoded));
    }
    static EncodedUrl fromQUrl(const QUrl& url);
    static EncodedUrl fromQUrlWithTrailingSlash(const QUrl& url);

    EncodedUrl() = default;
    EncodedUrl(EncodedUrl&&) = default;
    EncodedUrl(const EncodedUrl&) = default;
    EncodedUrl& operator=(EncodedUrl&&) = default;
    EncodedUrl& operator=(const EncodedUrl&) = default;

    bool isValid() const {
        return !m_urlEncoded.isEmpty();
    }

    QString toQString() const {
        return m_urlEncoded;
    }

    QUrl toQUrl() const;

    friend bool operator==(
            const EncodedUrl& lhs,
            const EncodedUrl& rhs) {
        return lhs.m_urlEncoded == rhs.m_urlEncoded;
    }

    friend uint qHash(
            const EncodedUrl& key,
            uint seed = 0) {
        return qHash(key.m_urlEncoded, seed);
    }

  private:
    explicit EncodedUrl(QByteArray&& urlEncoded)
            : m_urlEncoded(std::move(urlEncoded)) {
    }

    QByteArray m_urlEncoded;
};

inline bool operator!=(const EncodedUrl& lhs, const EncodedUrl& rhs) {
    return !(lhs == rhs);
}

inline QDebug operator<<(QDebug dbg, const EncodedUrl& arg) {
    return dbg << arg.toQString();
}

} // namespace mixxx
