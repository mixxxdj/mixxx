#include "broadcast/networkreply.h"

#include "moc_networkreply.cpp"

QNetworkReply::NetworkError FakeNetworkReply::error() const {
    return QNetworkReply::NoError;
}

unsigned int FakeNetworkReply::getHttpError() {
    return 200;
}

QByteArray FakeNetworkReply::readAll() {
    return QByteArray();
}

void FakeNetworkReply::setNetworkError(QNetworkReply::NetworkError error) {
    netError = error;
}

void FakeNetworkReply::setHttpError(unsigned int error) {
    httpError = error;
}

void FakeNetworkReply::setContents(QByteArray contents) {
    this->contents = contents;
}
