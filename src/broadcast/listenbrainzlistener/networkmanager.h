#pragma once

#include <QNetworkAccessManager>
#include <QObject>

class QByteArray;
class QNetworkReply;
class QNetworkRequest;
class NetworkReply;
class NetworkRequest;

class NetworkManager : public QObject {
    Q_OBJECT
  public:
    virtual NetworkReply* post(const NetworkRequest* request, const QByteArray& data) = 0;
  signals:
    void finished(NetworkReply* reply);
};

class FakeNetworkManager : public NetworkManager {
    Q_OBJECT
  public:
    NetworkReply* post(const NetworkRequest* request, const QByteArray& data) override;
};
