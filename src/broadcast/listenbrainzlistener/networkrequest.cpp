#include "broadcast/listenbrainzlistener/networkrequest.h"


void QtNetworkRequest::setRawHeader(const QByteArray &header, const QByteArray &value) {
    m_request.setRawHeader(header, value);
}

QList<QByteArray> QtNetworkRequest::rawHeaderList() const {
    return m_request.rawHeaderList();
}
