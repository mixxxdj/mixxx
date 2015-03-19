#ifndef MIXXX_URLRESOURCE_H
#define MIXXX_URLRESOURCE_H

#include "util/assert.h"

#include <QUrl>

namespace Mixxx {

class UrlResource {
public:
    virtual ~UrlResource() {}

    const QUrl& getUrl() const {
        return m_url;
    }
    QString getUrlString() const {
        return m_url.toString();
    }

protected:
    explicit UrlResource(QUrl url)
        : m_url(url) {
    }

    inline QString getLocalFileName() const {
        return getUrl().toLocalFile();
    }
    inline QByteArray getLocalFileNameBytes() const {
        return getLocalFileName().toLocal8Bit();
    }

private:
    const QUrl m_url;
};

} // namespace Mixxx

#endif // MIXXX_URLRESOURCE_H
