#pragma once

#include <QObject>
#include <QNetworkReply>

class NetworkReply : public QObject {
    Q_OBJECT
  public:
    virtual QNetworkReply::NetworkError error() const = 0;
    virtual unsigned int getHttpError() = 0;
    virtual QByteArray readAll() = 0;
  signals:
    void finished();
};

class FakeNetworkReply : public NetworkReply {
    Q_OBJECT
  public:
    QNetworkReply::NetworkError error() const override;
    unsigned int getHttpError() override;
    QByteArray readAll() override;

    void setNetworkError(QNetworkReply::NetworkError error);
    void setHttpError(unsigned int error);
    void setContents(QByteArray contents);
  private:
    QByteArray contents;
    QNetworkReply::NetworkError netError;
    unsigned int httpError;
};
