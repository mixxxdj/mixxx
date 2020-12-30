#pragma once

#include <QNetworkReply>
#include <QObject>

// Interface definition for Network replies
class NetworkReply : public QObject {
    Q_OBJECT
  public:
    virtual QNetworkReply::NetworkError error() const = 0;
    virtual unsigned int getHttpError() = 0;
    virtual QByteArray readAll() = 0;
  signals:
    void finished();
};
