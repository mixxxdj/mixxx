#include "broadcast/networkrequest.h"

#include "moc_networkrequest.cpp"


void QtNetworkRequest::setRawHeader(const QByteArray &header, const QByteArray &value) {
    m_request.setRawHeader(header,value);
}

QList<QByteArray> QtNetworkRequest::rawHeaderList() const {
    return m_request.rawHeaderList();
}
